/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 29/08/2026 by @author Tsukini

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
#include <sys/mman.h>
#include <unistd.h>
#include <optional>
#include <cstddef>
#include <cstring>
#include <thread>
#include <vector>
#include <string>
#include <mutex>

_cold void utils::encapsulation::SharedMemory::init_(void)
{
    /*
    auto current = this->_metadata->readable.load(std::memory_order_relaxed);
    this->_metadata->readable.wait(current, std::memory_order_acquire);
    */
}

_hot void utils::encapsulation::SharedMemory::read_(void)
{
    // try to find a spot with data to read
    std::vector<utils::encapsulation::shm::Slot> slots;
    for (const utils::encapsulation::shm::Slot& slot: this->_slots) {
        if (slot.metadata->flag.load(std::memory_order_acquire) == 2) {
            slot.metadata->reader.fetch_add(1, std::memory_order_acquire);

            // cancel action if flag was edited before reader added
            if (slot.metadata->flag.load(std::memory_order_acquire) != 2) _unlikely {
                slot.metadata->reader.fetch_sub(1, std::memory_order_relaxed);
                continue;
            }

            // check if it's was destined to itself   
            else if (slot.metadata->target.ownership != this->_ownership && !slot.metadata->target.global) {
                slot.metadata->reader.fetch_sub(1, std::memory_order_relaxed);
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
                slot.metadata->reader.fetch_sub(1, std::memory_order_relaxed);
                continue;
            }

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
                std::memory_order_acquire, std::memory_order_relaxed
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
        bool last = slot.metadata->last;
        if (id.ownership != this->_ownership) {
            if (!last) this->_ownerships.insert(id); // can fail (failsafe)
            else if (this->_ownerships.contains(id)) this->_ownerships.erase(id);
        } else if (last) this->_await.insert(id.id); // shouldn't be able to fail in any case (failsafe)

        // clear it's presence has a reader (if it's the last, then empty the slot)
        if (global) {
            // if there is other reader
            if (slot.metadata->reader.fetch_sub(1, std::memory_order_acq_rel) > 1) continue;
            expected = 2;
        } else { // sole reader assured
            expected = 1;
        }

        // set flag to signal an empty slot
        (void)slot.metadata->flag.compare_exchange_strong(
            expected, 0,
            std::memory_order_acquire, std::memory_order_relaxed
        );
    }
}

_cold void utils::encapsulation::SharedMemory::close(void)
{
    // decrement connected counter
    this->_metadata->connected.fetch_sub(1, std::memory_order_relaxed);

    // stop the internal reader thread
    this->_thread.request_stop();
    this->_metadata->readable.fetch_add(1, std::memory_order_relaxed);
    this->_metadata->readable.notify_all();
    if (this->_thread.joinable()) this->_thread.join(); // wait for the stop

    // close
    if (this->_ptr) ::munmap(this->_ptr, this->_size);
    if (this->_fd != -1) ::close(this->_fd);

    // reset
    this->_idHandler.free();
    this->_data.clear();
    this->_ownerships.clear();
    this->_await.clear();
    this->_metadata = nullptr;
    this->_slots.clear();
    this->_ptr = nullptr;
    this->_fd = -1;
}

_hot void utils::encapsulation::SharedMemory::send_(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Id& id, bool last, bool failsafe)
{
    // check the size of the memory to write
    if (bytes.size() > this->_size) _unlikely {
        //this->_idHandler.free(id);
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, std::to_string(bytes.size()) + " > " + std::to_string(this->_size) + " (actual limits per 'slot')");
    }

    // try to find a spot to write data
    utils::encapsulation::shm::Slot emptySlot = {nullptr, nullptr};
    while (!emptySlot && failsafe) {
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
        std::this_thread::yield();
    }

    // no empty memory slot find
    if (!emptySlot) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfMemory);
    }

    // write the memory
    emptySlot.metadata->id = id;
    emptySlot.metadata->last = last;
    emptySlot.metadata->size = bytes.size();
    std::memcpy(emptySlot.bytes, bytes.data(), bytes.size());

    // set the flag to: data to read
    emptySlot.metadata->flag.store(2, std::memory_order_release);
    this->_metadata->readable.fetch_add(1, std::memory_order_relaxed);
    this->_metadata->readable.notify_all();
}

_hot void utils::encapsulation::SharedMemory::send(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Id& id, bool last, bool failsafe)
{
    std::lock_guard<std::mutex> lock(this->_lock);

    // determine the ownership
    if (id.ownership == this->_ownership) _unlikely { // force alloc of id
        this->_idHandler.use(id.id);
    } else if (this->_ownerships.contains(id)) _likely { // remove usless id
        if (last) this->_ownerships.erase(id);
    } else _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidId, "This id is not registered in the internal storage, unknown pair of id/ownership...");
    }

    // redirect the call
    this->send_(bytes, id, last, failsafe);
}

_hot utils::encapsulation::shm::Id utils::encapsulation::SharedMemory::send(const std::vector<std::byte>& bytes, bool last, bool failsafe)
{
    // allocate an id
    utils::encapsulation::shm::Id id = {last ? 0 : this->_idHandler.allocate(), this->_ownership};

    // redirect the call
    this->send_(bytes, id, last, failsafe);

    return id;
}

_hot _nodiscard std::optional<std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>>> utils::encapsulation::SharedMemory::read(const utils::encapsulation::shm::ReadFilter filter)
{
    // nothing to read
    if (this->_data.empty()) _unlikely {return std::nullopt;}
    std::lock_guard<std::mutex> lock(this->_lock);

    // get only data depending on the filter
    std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>> values;
    if (filter == utils::encapsulation::shm::ReadFilter::All) {
        this->_data.swap(values);
    } else {
        for (auto it = this->_data.begin(); it != this->_data.end();) {
            bool isZero = (it->first.id == 0);
            bool match = ((filter == utils::encapsulation::shm::ReadFilter::ZeroOnly) ? isZero : !isZero);
            if (match) {
                values.emplace(std::move(it->first), std::move(it->second));
                it = this->_data.erase(it);
            } else {
                ++it;
            }
        }
        if (values.empty()) return std::nullopt;
    }

    // erase awaiting id
    for (const auto& [id, _]: values) {
        if (!(id.ownership == this->_ownership && _likely_c(this->_await.contains(id.id)))) continue;
        this->_await.erase(id.id);
        this->_idHandler.free(id.id);
    }

    if (values.empty()) return std::nullopt;
    return values;
}

_hot _nodiscard std::optional<std::vector<std::vector<std::byte>>> utils::encapsulation::SharedMemory::read(const utils::encapsulation::shm::Id& id)
{
    // nothing to read
    if (!this->readable(id)) _unlikely {return std::nullopt;}
    std::lock_guard<std::mutex> lock(this->_lock);

    // remove the value readed from storage
    auto it = this->_data.find(id);
    std::vector<std::vector<std::byte>> value = std::move(it->second);
    const utils::encapsulation::shm::Id& key = it->first; // can't name 2 var has 'id'
    if (key.ownership == this->_ownership && _likely_c(this->_await.contains(key.id))) {
        this->_await.erase(key.id);
        this->_idHandler.free(key.id);
    }
    this->_data.erase(it);

    return value;
}

_hot _nodiscard bool utils::encapsulation::SharedMemory::readable(const utils::encapsulation::shm::ReadFilter filter) const
{
    std::lock_guard<std::mutex> lock(this->_lock);
    if (filter == utils::encapsulation::shm::ReadFilter::All) return !this->_data.empty();

    // On the first valid id return
    bool wantZero = (filter == utils::encapsulation::shm::ReadFilter::ZeroOnly);
    for (const auto& [id, _]: this->_data) {
        if ((id.id == 0) == wantZero) return true;
    }
    return false;
}
