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
##  @file SharedMemory.hpp

File Description:
##  Encapsulation for shared memory
\**************************************************************/

#ifndef SHAREDMEMORY_H
    #define SHAREDMEMORY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"        // utils::security::observer::Observer
    #include "../attribute/Attribute.hpp"               // _cold, _hot, _nodiscard, _unlikely, _deprecated, _legacy, std::hardware_destructive_interference_size
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Type, utils::exception::InternalCode
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../system/IdHandler.hpp"                  // utils::system::IdHandler<T>
    #include <sys/mman.h>                               // mmap, shm_open
    #include <sys/types.h>                              // pid_t, off_t
    #include <sys/stat.h>                               // fstat
    #include <unistd.h>                                 // close, ftruncate, getpid
    #include <fcntl.h>                                  // O_CREAT, O_RDWR
    #include <condition_variable>                       // std::condition_variable
    #include <unordered_map>                            // std::unordered_map
    #include <unordered_set>                            // std::unordered_set
    #include <type_traits>                              // std::is_standard_layout_v
    #include <functional>                               // std::hash
    #include <optional>                                 // std::optional
    #include <compare>                                  // std::strong_ordering
    #include <utility>                                  // std::pair
    #include <memory>                                   // std::construct_at
    #include <cstring>                                  // strerror
    #include <cstddef>                                  // std::size_t, std::byte
    #include <cstdint>                                  // std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t
    #include <thread>                                   // std::jthread, std::this_thread::yield
    #include <atomic>                                   // std::atomic
    #include <vector>                                   // std::vector
    #include <string>                                   // std::string
    #include <cerrno>                                   // errno
    #include <mutex>                                    // std::mutex

namespace utils::encapsulation::shm { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

struct ShmMetadata {
    //std::atomic<bool> lock{false}; // id handler lock | 0 = unlock, 1 = lock
    std::size_t size = 0;
    std::size_t queue = 0;
    std::atomic<pid_t> connected{0}; // number of connected
    std::atomic<std::uint32_t> readable{1}; // (signal) awake the reader
    std::atomic<std::uint64_t> sequence{0}; // last sequence number given to a request (0 = none)
};

struct Id {
    std::size_t id = 0; // 0 == invalid/unset id
    pid_t ownership = 0; // ownership of the id

    // ------------ Operator ---------- //
    bool operator==(const utils::encapsulation::shm::Id& other) const {return this->id == other.id && this->ownership == other.ownership;};
    std::strong_ordering operator<=>(const utils::encapsulation::shm::Id&) const = default;

    // ---------- Constructor --------- //
    Id() = default;
    Id(std::size_t id): id{id} {};
    Id(std::size_t id, pid_t ownership): id{id}, ownership{ownership} {};
};

struct Target {
    pid_t sender = 0; // ownership of the sender
    std::size_t limit = 0; // number of people who will read it (0 = every connected reader for a global request)
    pid_t ownership = 0; // to only a specific reader
    bool global = false; // to all reader
    std::atomic<std::size_t> readed{0}; // number of time readed

    // ---------- Constructor --------- //
    Target() = default;
    Target(const std::size_t limit): limit{limit}, global{true} {};
    Target(const std::size_t limit, const pid_t ownership): limit{limit}, ownership{ownership} {};
};

struct ShmRequestMetadata {
    std::atomic<std::uint8_t> flag{0}; // request flag | 0 = no data / readed, 1 = in writting/reading (unique), 2 = data / to read (shared), 3 = reset in process
    std::atomic<std::size_t> reader{0}; // number of reader (if value need to be edited wait until reader == 1)
    std::atomic<std::size_t> waiting{0}; // number of reader in waiting status (reset of data)
    utils::encapsulation::shm::Id id;
    utils::encapsulation::shm::Target target;
    std::pair<bool, bool> last = {false, false}; // [last sending] only trigger the join | [last transmition] free the id on read if it's is ownership, otherwhise remove it from it's storage
    std::size_t size = 0; // total size of the bytes to not read the whole 'slot' with garbage from before
    std::uint64_t sequence = 0; // unique number of the request, used by a reader to not read twice the same request
};

struct Slot {
    utils::encapsulation::shm::ShmRequestMetadata* metadata = nullptr;
    std::byte* bytes = nullptr;

    // ------------ Operator ---------- //
    bool operator!(void) const {return (!this->metadata || !this->bytes);};

    // ---------- Constructor --------- //
    Slot() = default;
    Slot(utils::encapsulation::shm::ShmRequestMetadata* metadata, std::byte* bytes): metadata{metadata}, bytes{bytes} {};
};

//static_assert(std::atomic<bool>::is_always_lock_free);
static_assert(std::atomic<std::uint32_t>::is_always_lock_free);
static_assert(std::is_standard_layout_v<utils::encapsulation::shm::ShmMetadata>);
static_assert(std::is_standard_layout_v<utils::encapsulation::shm::ShmRequestMetadata>);

//----------------------------------------------------------------//
/* ENUM */

enum class ReadFilter {All, ZeroOnly, NonZeroOnly, LastOnly};
enum class LayoutPolicy {
    Compact,             // (meta,meta,...)(byte,byte,...)  — 1 writer, N readers
    CompactSemiAligned,  // idem + alignement per groups    — some writers, some at the same time
    Interleaved,         // (meta,byte,meta,byte,...)       — N writers moderated, payload >= cache-line (advise)
    InterleavedAligned,  // idem + alignas(hardware_destructive_interference_size) per slot — N writers, many at the same time
};

//----------------------------------------------------------------//
/* PROTOTYPE */

/* layout */
_cold _nodiscard inline constexpr std::size_t align_up(const std::size_t size, const std::size_t alignment)
{return (size + alignment - 1) / alignment * alignment;}

// stride of an interleaved slot (metadata + bytes), keep the next metadata aligned
_cold _nodiscard inline constexpr std::size_t interleaved_stride(const std::size_t size)
{return utils::encapsulation::shm::align_up(sizeof(utils::encapsulation::shm::ShmRequestMetadata) + size, alignof(utils::encapsulation::shm::ShmRequestMetadata));}

_cold _nodiscard inline std::size_t align_ceil(const std::size_t size)
{
    constexpr std::size_t alignment = std::hardware_destructive_interference_size;
    const std::size_t remainder = size % alignment;
    return size + (remainder ? alignment - remainder : 0);
}

} // namespace end

// Simple hash function for Id class
namespace std { // namespace start
    template<>
    struct hash<utils::encapsulation::shm::Id> {
        _hot _nodiscard std::size_t operator()(const utils::encapsulation::shm::Id& id) const
        {
            std::size_t h1 = std::hash<std::size_t>{}(id.id);
            std::size_t h2 = std::hash<pid_t>{}(id.ownership);
            return h1 ^ (h2 << 1);
        };
    };
} // namespace end

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class SharedMemory: private utils::security::observer::Observer<"SharedMemory"> {
    private:
        mutable std::mutex _lock;
        utils::system::IdHandler<std::size_t> _idHandler;
        std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>> _data;
        std::unordered_set<utils::encapsulation::shm::Id> _ownerships;
        std::unordered_set<std::size_t> _await; // id that await to be free on user read
        mutable std::condition_variable _cv;
        std::atomic<std::size_t> _last{0};
        std::unordered_set<utils::encapsulation::shm::Id> _lastIds;
        bool _closed = true; // wake the join calls on close (guarded by _lock)

        /* shm */
        std::size_t _queue = 1;
        std::size_t _size = 0;
        pid_t _ownership = 0; // 0 is reserved for unilateral sending
        int _fd = -1;
        void* _ptr = nullptr;
        std::size_t _mapped = 0; // size of the mapping (for munmap)
        utils::encapsulation::shm::ShmMetadata* _metadata = nullptr;
        std::vector<utils::encapsulation::shm::Slot> _slots;
        std::vector<std::uint64_t> _seen; // last sequence read on each slot
        std::mutex _readLock; // read_ can be called by the internal thread & trigger()

        /* internal */
        std::jthread _thread; // auto read

        // ---------- Pre-Function -------- //
        void init_(void); // create the internal thread
        void read_(void); // function call by the thread
        void send_(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Id& id, const utils::encapsulation::shm::Target& target, std::pair<bool, bool> last, bool failsafe);

    public:
        // ---------- Pre-Function -------- //
        /* setup */
        void close(void);

        /* communication */
        void send(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Id& id, const utils::encapsulation::shm::Target& target, bool lastSending = false, bool lastTransmission = false, bool failsafe = false); // force an id for the request
        utils::encapsulation::shm::Id send(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Target& target, bool lastSending = false, bool lastTransmission = false, bool failsafe = false); // allocate an id for this request
        std::optional<std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>>> read(const utils::encapsulation::shm::ReadFilter filter = utils::encapsulation::shm::ReadFilter::All);
        std::optional<std::vector<std::vector<std::byte>>> read(const utils::encapsulation::shm::Id& id);
        bool readable(const utils::encapsulation::shm::ReadFilter filter = utils::encapsulation::shm::ReadFilter::All) const;
        void join(bool last = false) const; // wait for anything to be readed (or any last to be readed)
        void join(const utils::encapsulation::shm::Id& id, bool last = false) const; // for a specifc id to be readed (wait until the last awnser or just any)

        // ------------ Function ---------- //
        _cold inline void ownership(pid_t ownership)       {this->_ownership = ownership;};
        _hot _nodiscard inline pid_t ownership(void) const {return this->_ownership;};

        /* setup */
        template<bool create = false, utils::encapsulation::shm::LayoutPolicy policy = utils::encapsulation::shm::LayoutPolicy::Compact, std::size_t groupSize = 2, bool forced = false>
        _cold void init(const std::string& name, std::size_t size = 1, std::size_t queue = 1) // size/queue ignored in 'client' mode | size is only for a single section, real size: sizeof(IdHandler lock) + (size + sizeof(metadata)) * queue
        {
            if constexpr (create && !forced) {
                if (size == 0) throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "Why? Do you want to create a empty memory??? (required: size > 0)");
            }

            // setup
            this->_queue = queue;
            this->_size = size;

            // open the shm
            this->_fd = ::shm_open(name.data(), (create ? (O_CREAT | O_RDWR) : O_RDWR), 0600);
            if (this->_fd == -1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::ShmOpen, ::strerror(errno));
            }

            // In 'client' mode extract size/queue
            if constexpr (!create) {
                struct stat st{};
                if (::fstat(this->_fd, &st) == -1) _unlikely {
                    ::close(this->_fd);
                    this->_fd = -1;
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Fstat, ::strerror(errno));
                }

                // Alloc using real size
                this->_mapped = static_cast<std::size_t>(st.st_size);
                this->_ptr = ::mmap(
                    nullptr,
                    this->_mapped,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    this->_fd,
                    0
                );
                if (this->_ptr == MAP_FAILED) _unlikely {
                    ::close(this->_fd);
                    this->_fd = -1;
                    this->_ptr = nullptr;
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Mmap, ::strerror(errno));
                }

                // Extract sub size information
                utils::encapsulation::shm::ShmMetadata* metadata = static_cast<utils::encapsulation::shm::ShmMetadata*>(this->_ptr);
                this->_size = metadata->size;
                this->_queue = metadata->queue;
            }

            // compute size: metadata + (size + metadata) * queue
            std::size_t groupCount = (this->_queue + groupSize - 1) / groupSize;
            std::size_t memSize = utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmMetadata));
            if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Compact) {
                memSize += (this->_size + sizeof(utils::encapsulation::shm::ShmRequestMetadata)) * this->_queue;
            } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Interleaved) {
                memSize += utils::encapsulation::shm::interleaved_stride(this->_size) * this->_queue; // keep each metadata aligned
            } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::CompactSemiAligned) {
                memSize += utils::encapsulation::shm::align_ceil(this->_size * groupSize) * groupCount + utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmRequestMetadata) * groupSize) * groupCount;
            } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::InterleavedAligned) {
                memSize += utils::encapsulation::shm::align_ceil(this->_size + sizeof(utils::encapsulation::shm::ShmRequestMetadata)) * this->_queue;
            }

            // setup the memory in create mode
            if constexpr (create) {
                if (::ftruncate(this->_fd, static_cast<off_t>(memSize)) == -1) _unlikely {
                    ::close(this->_fd);
                    this->_fd = -1;
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Ftruncate, ::strerror(errno));
                }

                // Allocate the memory
                this->_mapped = memSize;
                this->_ptr = ::mmap(
                    nullptr,
                    memSize,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    this->_fd,
                    0
                );
                if (this->_ptr == MAP_FAILED) _unlikely {
                    ::close(this->_fd);
                    this->_fd = -1;
                    this->_ptr = nullptr;
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Mmap, ::strerror(errno));
                }
            }

            // init global metadata and starting address
            utils::encapsulation::shm::ShmRequestMetadata* metadata = nullptr;
            std::byte* ptrMetadata = nullptr;
            std::byte* ptrBytes = nullptr;
            std::byte* lastPtrMetadata = nullptr;
            std::byte* lastPtrBytes = nullptr;
            std::byte* ptr = static_cast<std::byte*>(this->_ptr);
            if constexpr (create) {
                this->_metadata = std::construct_at(reinterpret_cast<utils::encapsulation::shm::ShmMetadata*>(ptr));
                this->_metadata->size = this->_size;
                this->_metadata->queue = this->_queue;
            } else this->_metadata = reinterpret_cast<utils::encapsulation::shm::ShmMetadata*>(ptr);
            ptr += utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmMetadata));
            ptrMetadata = ptr; ptrBytes = ptr;
            if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Compact) {
                ptrBytes += sizeof(utils::encapsulation::shm::ShmRequestMetadata) * this->_queue;
            } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::CompactSemiAligned) {
                ptrBytes += utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmRequestMetadata) * groupSize) * groupCount;
            } else {
                ptrBytes += sizeof(utils::encapsulation::shm::ShmRequestMetadata);
            }
            lastPtrMetadata = ptrMetadata; lastPtrBytes = ptrBytes;

            // for each slot init memory and store address
            for (std::size_t i = 0; i < this->_queue; ++i) {
                // setup address at actual emplacement
                metadata = reinterpret_cast<utils::encapsulation::shm::ShmRequestMetadata*>(ptrMetadata);
                if constexpr (create) std::construct_at(metadata);
                this->_slots.emplace_back(metadata, ptrBytes);

                // setup next emplacement
                if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Compact) {
                    ptrMetadata += sizeof(utils::encapsulation::shm::ShmRequestMetadata);
                    ptrBytes += this->_size;
                } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::CompactSemiAligned) {
                    if (groupSize == 1 || ((i + 1) % groupSize == 0)) {
                        lastPtrMetadata += utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmRequestMetadata) * groupSize);
                        lastPtrBytes += utils::encapsulation::shm::align_ceil(this->_size * groupSize);
                        ptrMetadata = lastPtrMetadata;
                        ptrBytes = lastPtrBytes;
                    } else {
                        ptrMetadata += sizeof(utils::encapsulation::shm::ShmRequestMetadata);
                        ptrBytes += this->_size;
                    }
                } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Interleaved) {
                    ptrMetadata += utils::encapsulation::shm::interleaved_stride(this->_size); // keep each metadata aligned
                    ptrBytes = ptrMetadata + sizeof(utils::encapsulation::shm::ShmRequestMetadata);
                } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::InterleavedAligned) {
                    ptr += utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmRequestMetadata) + this->_size);
                    ptrMetadata = ptr;
                    ptrBytes = ptrMetadata + sizeof(utils::encapsulation::shm::ShmRequestMetadata);
                }
            }

            // nothing read yet on any slot
            this->_seen.assign(this->_queue, 0);

            // increment connected counter
            this->_metadata->connected.fetch_add(1, std::memory_order_relaxed);

            // start the auto-read thread now that metadata is initialized
            this->init_();
        }

        /* communication */
        _hot inline void trigger(void) {this->read_();}; // auto in normal case
        _hot inline void send(const std::vector<std::byte>& bytes, std::size_t id, const utils::encapsulation::shm::Target& target, bool lastSending = false, bool lastTransmission = false, bool failsafe = false) // own id
        {this->send(bytes, utils::encapsulation::shm::Id{id, this->_ownership}, target, lastSending, lastTransmission, failsafe);};
        _hot _nodiscard inline bool readable(const utils::encapsulation::shm::Id& id) const {std::lock_guard<std::mutex> lock(this->_lock); return this->_data.contains(id);};

        // ------------ Operator ---------- //
        SharedMemory& operator=(const SharedMemory& other) = delete;
        SharedMemory& operator=(SharedMemory&& other) = delete;

        // ---------- Constructor --------- //
        _legacy SharedMemory(std::uint16_t ownership): _ownership{static_cast<pid_t>(ownership)} {};
        SharedMemory(pid_t ownership = ::getpid()): _ownership{ownership} {};
        SharedMemory(const SharedMemory& other) = delete;
        SharedMemory(SharedMemory&& other) = delete;

        // ----------- Destructor --------- //
        ~SharedMemory() {this->close();};
};

} // namespace end
#endif /* SHAREDMEMORY_H */
