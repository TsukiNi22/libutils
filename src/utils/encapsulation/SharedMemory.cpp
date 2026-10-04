/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 21/09/2026 by @author Tsukini

File Name:
##  @file SharedMemory.cpp

File Description:
##  Methods definition for the shared memory encapsulation
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/basic/NoneException.hpp"
#include "utils/encapsulation/SharedMemory.hpp"
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_map>
#include <functional>
#include <optional>
#include <utility>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <limits>
#include <mutex>

_hot static inline void futex_wait(std::atomic<std::uint32_t>* addr, std::uint32_t expected)
{
    (void)static_cast<int>(::syscall(
        SYS_futex,
        reinterpret_cast<std::uint32_t*>(addr),
        FUTEX_WAIT,
        expected,
        nullptr,
        nullptr,
        0
    ));
}

// wake every waiter (the number of waiters to wake is an int)
_hot static inline void futex_wake(std::atomic<std::uint32_t>* addr)
{
    (void)static_cast<int>(::syscall(
        SYS_futex,
        reinterpret_cast<std::uint32_t*>(addr),
        FUTEX_WAKE,
        std::numeric_limits<int>::max(),
        nullptr,
        nullptr,
        0
    ));
}

_cold void utils::encapsulation::SharedMemory::init_(void)
{
    // Do not start the thread if the shm is not initialized yet
    if (!this->_metadata) return;
    {std::lock_guard<std::mutex> lock(this->_lock); this->_closed = false;}

    // last signal seen (taken before the thread start)
    const std::uint32_t start = this->_metadata->readable.load(std::memory_order_acquire);

    this->_thread = std::jthread([this, start](std::stop_token stoken) {
        // allow the thread to be awake when a stop is requested
        std::stop_callback wakeOnStop(stoken, [this](void) {
            this->_metadata->readable.fetch_add(1, std::memory_order_release);
            futex_wake(&this->_metadata->readable);
        });

        // read the requests already waiting (sent before the connection)
        this->read_();

        std::uint32_t current = start;
        while (!stoken.stop_requested()) {
            // loop for each wakeup called
            const std::uint32_t observed = this->_metadata->readable.load(std::memory_order_acquire);
            if (current >= observed) {
                // wait for trigger from atomic notifier in metatdata
                futex_wait(&this->_metadata->readable, observed);
            } else ++current;

            // awake can also be trigger by a stop request
            if (stoken.stop_requested()) break;

            // redirect on internal read
            this->read_();
        }
    });
}

_hot void utils::encapsulation::SharedMemory::read_(void)
{
    // only one read at a time for this instance (internal thread & trigger)
    std::lock_guard<std::mutex> readLock(this->_readLock);

    // try to find a spot with data to read
    std::vector<utils::encapsulation::shm::Slot> slots;
    for (std::size_t i = 0; i < this->_slots.size(); ++i) {
        const utils::encapsulation::shm::Slot& slot = this->_slots[i];
        if (slot.metadata->flag.load(std::memory_order_acquire) == 2) {
            slot.metadata->reader.fetch_add(1, std::memory_order_acquire);

            // cancel action if flag was edited before reader added
            if (slot.metadata->flag.load(std::memory_order_acquire) != 2) _unlikely {
                slot.metadata->reader.fetch_sub(1, std::memory_order_release);
                continue;
            }

            // already read by this instance (a global request stay readable until all the readers read it)
            else if (slot.metadata->sequence == this->_seen[i]) {
                slot.metadata->reader.fetch_sub(1, std::memory_order_release);
                continue;
            }

            // check if it's was destined to itself
            else if (slot.metadata->target.sender == this->_ownership || (slot.metadata->target.ownership != this->_ownership && !slot.metadata->target.global)) {
                slot.metadata->reader.fetch_sub(1, std::memory_order_release);
                continue;
            }

            // try to increment the number of read
            std::size_t expected = 0;
            bool fail = true;
            while ((expected = slot.metadata->target.readed.load(std::memory_order_acquire)) < slot.metadata->target.limit) {
                if (slot.metadata->target.readed.compare_exchange_strong(
                    expected, expected + 1,
                    std::memory_order_acquire, std::memory_order_relaxed)
                ) _likely {
                    fail = false;
                    break;
                }
            }
            if (fail) {
                slot.metadata->reader.fetch_sub(1, std::memory_order_release);
                continue;
            }

            this->_seen[i] = slot.metadata->sequence;
            slots.push_back(slot);
        }
    }

    // nothing to read, data was already read and for other process
    if (slots.empty()) return;

    // for each valid slot to read
    std::lock_guard<std::mutex> lock(this->_lock);
    std::uint8_t expected = 0;
    for (const utils::encapsulation::shm::Slot& slot: slots) {
        bool global = slot.metadata->target.global;

        // if it's the sole process to own the request switch it in reading mode (1)
        if (!global) {
            // switch the flag to reading mode (unique)
            expected = 2;
            (void)slot.metadata->flag.compare_exchange_strong(
                expected, 1,
                std::memory_order_acq_rel, std::memory_order_relaxed
            );

            // wait until it's the sole reader of the request
            while (slot.metadata->target.readed.load(std::memory_order_acquire) > 1)
                std::this_thread::yield();
        }

        // read data
        const utils::encapsulation::shm::Id& id = slot.metadata->id;
        std::vector<std::byte> bytes(slot.bytes, slot.bytes + slot.metadata->size);
        this->_data[id].push_back(std::move(bytes));

        // handle ownership storage?
        const auto &[lastSending, lastTransmission] = slot.metadata->last;
        if (id.ownership != this->_ownership) {
            if (!lastTransmission) this->_ownerships.insert(id); // can fail (failsafe)
            else if (this->_ownerships.contains(id)) this->_ownerships.erase(id);
        } else if (lastTransmission) this->_await.insert(id.id); // shouldn't be able to fail in any case (failsafe)

        // Notify join calls
        if (lastSending || lastTransmission) {
            this->_last.fetch_add(1, std::memory_order_relaxed);
            this->_lastIds.insert(id);
        }
        this->_cv.notify_all();

        // clear it's presence has a reader (if it's the last, then empty the slot)
        if (global) {
            // if the limit is not reach, leave it for the other readers (without staying counted as a reader)
            if (slot.metadata->target.readed.load(std::memory_order_acquire) < slot.metadata->target.limit) {
                slot.metadata->reader.fetch_sub(1, std::memory_order_acq_rel);
                continue;
            }
            expected = 2;
        } else { // sole reader assured
            expected = 1;
        }

        // set to reset mode (no more readed allowed)
        (void)slot.metadata->flag.compare_exchange_strong(
            expected, 3,
            std::memory_order_acq_rel, std::memory_order_relaxed
        );

        // wait until there is no more reader
        slot.metadata->reader.fetch_sub(1, std::memory_order_acq_rel);
        slot.metadata->waiting.fetch_add(1, std::memory_order_acq_rel);
        while (slot.metadata->reader.load(std::memory_order_acquire) != 0)
            std::this_thread::yield();

        // reset metadata value
        std::size_t readed = slot.metadata->target.readed.load(std::memory_order_acquire);
        (void)slot.metadata->target.readed.compare_exchange_strong(
            readed, 0,
            std::memory_order_acquire, std::memory_order_relaxed
        );

        // wait until there is no more reader in waiting status
        slot.metadata->waiting.fetch_sub(1, std::memory_order_acq_rel);
        while (slot.metadata->waiting.load(std::memory_order_acquire) != 0)
            std::this_thread::yield();

        // set flag to signal an empty slot only when limit is reached
        expected = 3;
        (void)slot.metadata->flag.compare_exchange_strong(
            expected, 0,
            std::memory_order_acq_rel, std::memory_order_relaxed
        );
    }
}

_cold void utils::encapsulation::SharedMemory::close(void)
{
    if (this->_metadata) {
        // decrement connected counter
        this->_metadata->connected.fetch_sub(1, std::memory_order_relaxed);

        // stop the internal reader thread
        this->_thread.request_stop();
        this->_metadata->readable.fetch_add(1, std::memory_order_relaxed);
        futex_wake(&this->_metadata->readable);
        if (this->_thread.joinable()) this->_thread.join(); // wait for the stop
    }

    // close
    if (this->_ptr) ::munmap(this->_ptr, this->_mapped);
    if (this->_fd != -1) ::close(this->_fd);

    // reset (the join calls are woken up, nothing more will be read)
    std::lock_guard<std::mutex> lock(this->_lock);
    this->_closed = true;
    this->_cv.notify_all();
    this->_idHandler.free();
    this->_data.clear();
    this->_ownerships.clear();
    this->_await.clear();
    this->_metadata = nullptr;
    this->_slots.clear();
    this->_seen.clear();
    this->_lastIds.clear();
    this->_last = 0;
    this->_ptr = nullptr;
    this->_mapped = 0;
    this->_fd = -1;
}

_hot void utils::encapsulation::SharedMemory::send_(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Id& id, const utils::encapsulation::shm::Target& target, std::pair<bool, bool> last, bool failsafe)
{
    // check the size of the memory to write
    if (bytes.size() > this->_size) _unlikely {
        //this->_idHandler.free(id);
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, std::to_string(bytes.size()) + " > " + std::to_string(this->_size) + " (actual limits per 'slot')");
    }

    // try to find a spot to write data (once, or until one is found in failsafe mode)
    utils::encapsulation::shm::Slot emptySlot = {nullptr, nullptr};
    do {
        for (const utils::encapsulation::shm::Slot& slot: this->_slots) {
            std::uint8_t expected = 0;
            if (slot.metadata->flag.compare_exchange_strong(
                expected, 1,
                std::memory_order_acquire, std::memory_order_relaxed)
            ) {
                emptySlot = slot;
                break;
            }
        }
        if (!emptySlot && failsafe) std::this_thread::yield();
    } while (!emptySlot && failsafe);

    // no empty memory slot find
    if (!emptySlot) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfMemory);
    }

    // setup target
    utils::encapsulation::shm::Target& metadataTarget = emptySlot.metadata->target;
    metadataTarget.sender = this->_ownership;
    metadataTarget.limit = target.limit;
    if (metadataTarget.limit == 0) { // 0 = every reader connected (except the sender) for a global request, the given reader otherwise
        const pid_t connected = this->_metadata->connected.load(std::memory_order_relaxed);
        metadataTarget.limit = (target.global && connected > 2) ? static_cast<std::size_t>(connected - 1) : 1;
    }
    metadataTarget.ownership = target.ownership;
    metadataTarget.global = target.global;

    // write the memory
    emptySlot.metadata->sequence = this->_metadata->sequence.fetch_add(1, std::memory_order_relaxed) + 1;
    emptySlot.metadata->id = id;
    emptySlot.metadata->last = last;
    emptySlot.metadata->size = bytes.size();
    std::memcpy(emptySlot.bytes, bytes.data(), bytes.size());

    // set the flag to: data to read
    emptySlot.metadata->flag.store(2, std::memory_order_release);
    this->_metadata->readable.fetch_add(1, std::memory_order_relaxed);
    futex_wake(&this->_metadata->readable);
}

_hot void utils::encapsulation::SharedMemory::send(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Id& id, const utils::encapsulation::shm::Target& target, bool lastSending, bool lastTransmission, bool failsafe)
{
    // determine the ownership (lock only for the check: the reader thread need the lock to free a slot)
    {
        std::lock_guard<std::mutex> lock(this->_lock);
        if (id.ownership == this->_ownership) _unlikely { // force alloc of id
            //this->_idHandler.use(id.id);
        } else if (this->_ownerships.contains(id)) _likely { // remove usless id
            if (lastTransmission) this->_ownerships.erase(id);
        } else _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidId, "This id is not registered in the internal storage, unknown pair of id/ownership...");
        }
    }

    // redirect the call
    this->send_(bytes, id, target, {lastSending, lastTransmission}, failsafe);
}

_hot utils::encapsulation::shm::Id utils::encapsulation::SharedMemory::send(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Target& target, bool lastSending, bool lastTransmission, bool failsafe)
{
    // allocate an id
    utils::encapsulation::shm::Id id = {lastTransmission ? 0 : this->_idHandler.allocate(), this->_ownership};

    // redirect the call (free the id if nothing was sent)
    try {
        this->send_(bytes, id, target, {lastSending, lastTransmission}, failsafe);
    } catch (...) {
        if (id.id != 0) this->_idHandler.free(id.id);
        throw;
    }

    return id;
}

_hot _nodiscard std::optional<std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>>> utils::encapsulation::SharedMemory::read(const utils::encapsulation::shm::ReadFilter filter)
{
    std::lock_guard<std::mutex> lock(this->_lock);
    if (this->_data.empty()) _unlikely {return std::nullopt;} // nothing to read

    // get only data depending on the filter
    std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>> values;
    if (filter == utils::encapsulation::shm::ReadFilter::All) {
        this->_data.swap(values);
        this->_lastIds.clear();
    } else {
        for (auto it = this->_data.begin(); it != this->_data.end();) {
            const utils::encapsulation::shm::Id key = it->first; // keep a copy before any erase
            bool match = false;
            if (filter == utils::encapsulation::shm::ReadFilter::LastOnly) {
                match = this->_lastIds.contains(key);
            } else {
                bool isZero = (key.id == 0);
                match = ((filter == utils::encapsulation::shm::ReadFilter::ZeroOnly) ? isZero : !isZero);
            }
            if (match) {
                values.emplace(std::move(it->first), std::move(it->second));
                it = this->_data.erase(it);
                this->_lastIds.erase(key);
            } else {
                ++it;
            }
        }
        if (values.empty()) return std::nullopt;
    }

    // erase awaiting id
    for (const auto &[id, _]: values) {
        if (!(id.ownership == this->_ownership && _likely_c(this->_await.contains(id.id)))) continue;
        this->_await.erase(id.id);
        this->_idHandler.free(id.id);
    }

    if (values.empty()) return std::nullopt;
    return values;
}

_hot _nodiscard std::optional<std::vector<std::vector<std::byte>>> utils::encapsulation::SharedMemory::read(const utils::encapsulation::shm::Id& id)
{
    std::lock_guard<std::mutex> lock(this->_lock);

    // nothing to read
    auto it = this->_data.find(id);
    if (it == this->_data.end()) _unlikely {return std::nullopt;}

    // remove the value readed from storage
    std::vector<std::vector<std::byte>> value = std::move(it->second);
    const utils::encapsulation::shm::Id& key = it->first; // can't name 2 var has 'id'
    if (key.ownership == this->_ownership && _likely_c(this->_await.contains(key.id))) {
        this->_await.erase(key.id);
        this->_idHandler.free(key.id);
    }
    this->_lastIds.erase(key);
    this->_data.erase(it);

    return value;
}

_hot _nodiscard bool utils::encapsulation::SharedMemory::readable(const utils::encapsulation::shm::ReadFilter filter) const
{
    std::lock_guard<std::mutex> lock(this->_lock);
    if (filter == utils::encapsulation::shm::ReadFilter::All) return !this->_data.empty();
    if (filter == utils::encapsulation::shm::ReadFilter::LastOnly) {
        for (const utils::encapsulation::shm::Id& id: this->_lastIds)
            if (this->_data.contains(id)) return true;
        return false;
    }

    // On the first valid id return
    bool wantZero = (filter == utils::encapsulation::shm::ReadFilter::ZeroOnly);
    for (const auto &[id, _]: this->_data) {
        if ((id.id == 0) == wantZero) return true;
    }
    return false;
}

_hot void utils::encapsulation::SharedMemory::join(bool last) const
{
    std::unique_lock<std::mutex> lock(this->_lock);
    if (this->_closed) return; // shm not initialized or closed

    if (!last) {
        this->_cv.wait(lock, [this](void) {return this->_closed || !this->_data.empty();});
    } else {
        std::size_t generation = this->_last.load(std::memory_order_relaxed);
        this->_cv.wait(lock, [this, generation](void) {return this->_closed || this->_last.load(std::memory_order_relaxed) != generation;});
    }
}

_hot void utils::encapsulation::SharedMemory::join(const utils::encapsulation::shm::Id& id, bool last) const
{
    std::unique_lock<std::mutex> lock(this->_lock);
    if (this->_closed) return; // shm not initialized or closed

    if (!last) {
        this->_cv.wait(lock, [this, &id](void) {return this->_closed || this->_data.contains(id);});
    } else {
        this->_cv.wait(lock, [this, &id](void) {return this->_closed || this->_lastIds.contains(id);});
    }
}
