/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 06/09/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"               // _cold, _hot, _nodiscard, _unlikely, std::hardware_destructive_interference_size
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Type, utils::exception::InternalCode
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../system/IdHandler.hpp"                  // utils::system::IdHandler<T>
    #include <sys/mman.h>                               // mmap, shm_open
    #include <unistd.h>                                 // close, ftruncate
    #include <fcntl.h>                                  // O_CREAT, O_RDWR
    #include <unordered_map>                            // std::unordered_map
    #include <optional>                                 // std::optional
    #include <cstring>                                  // strerror
    #include <cstddef>                                  // std::size_t, std::byte
    #include <cstdint>                                  // std::uint8_t
    #include <thread>                                   // std::jthread, std::this_thread::yield
    #include <atomic>                                   // std::atomic
    #include <vector>                                   // std::vector
    #include <string>                                   // std::string
    #include <cerrno>                                   // errno
    #include <mutex>                                    // std::mutex
    #include <set>                                      // std::set

namespace utils::encapsulation::shm { // namespace start
//----------------------------------------------------------------//
/* TYPEDEF */

struct ShmMetadata {
    //std::atomic<bool> lock{false}; // id handler lock | 0 = unlock, 1 = lock
    std::atomic<std::uint16_t> connected{0}; // number of connected
    std::atomic<std::uint8_t> readable{0}; // (signal) awake the reader
};

struct Id {
    std::size_t id = 0; // 0 == invalid/unset id
    std::uint16_t ownership = 0; // ownership of the id

    // ------------ Operator ---------- //
    bool operator==(const Id& other) const {return id == other.id && ownership == other.ownership;};
    auto operator<=>(const Id&) const = default;

    // ---------- Constructor --------- //
    Id() = default;
    Id(std::size_t id): id{id} {}
    Id(std::size_t id, std::uint16_t ownership): id{id}, ownership{ownership} {}
};

struct Target {
    std::size_t limit = 0; // number of people who will read it (0 = inf)
    std::uint16_t ownership = 0; // to only a specific reader
    bool global = false; // to all reader
    std::atomic<std::size_t> readed{0}; // number of time readed
};

struct ShmRequestMetadata {
    std::atomic<std::uint8_t> flag{0}; // request flag | 0 = no data / readed, 1 = in writting/reading (unique), 2 = data / to read (shared)
    std::atomic<std::size_t> reader{0}; // number of reader (if value need to be edited wait until reader == 1)
    utils::encapsulation::shm::Id id;
    utils::encapsulation::shm::Target target;
    bool last = false; // free the id on read if it's is ownership, otherwhise remove it from it's storage
    std::size_t size = 0; // total size of the bytes to not read the whole 'slot' with garbage from before
};

struct Slot {
    utils::encapsulation::shm::ShmRequestMetadata* metadata = nullptr;
    std::byte* bytes = nullptr;

    // ------------ Operator ---------- //
    bool operator!() const {return (!metadata || !bytes);};

    // ---------- Constructor --------- //
    Slot() = default;
    Slot(utils::encapsulation::shm::ShmRequestMetadata* metadata, std::byte* bytes): metadata{metadata}, bytes{bytes} {}
};

//static_assert(std::atomic<bool>::is_always_lock_free);
static_assert(std::atomic<std::uint8_t>::is_always_lock_free);
static_assert(std::is_standard_layout_v<utils::encapsulation::shm::ShmMetadata>);
static_assert(std::is_standard_layout_v<utils::encapsulation::shm::ShmRequestMetadata>);

enum class ReadFilter {All, ZeroOnly, NonZeroOnly};
enum class LayoutPolicy {
    Compact,             // (meta,meta,...)(byte,byte,...)  — 1 writer, N readers
    CompactSemiAligned,  // idem + alignement per groups    — some writers, some at the same time
    Interleaved,         // (meta,byte,meta,byte,...)       — N writers moderated, payload >= cache-line (advise)
    InterleavedAligned   // idem + alignas(hardware_destructive_interference_size) per slot — N writers, many at the same time
};

_cold _nodiscard inline std::size_t align_ceil(const std::size_t size)
{
    constexpr std::size_t alignment = std::hardware_destructive_interference_size;
    const std::size_t remainder = size % alignment;
    return size + (remainder ? alignment - remainder : 0);
}

} // namespace end

// Simple hash function for Id class
namespace std {
    template <>
    struct hash<utils::encapsulation::shm::Id> {
        std::size_t operator()(const utils::encapsulation::shm::Id& id) const {
            std::size_t h1 = std::hash<std::size_t>{}(id.id);
            std::size_t h2 = std::hash<std::uint16_t>{}(id.ownership);
            return h1 ^ (h2 << 1);
        }
    };
}

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class SharedMemory: private utils::security::observer::Observer<"SharedMemory"> {
    private:
        mutable std::mutex _lock;
        utils::system::IdHandler<std::size_t> _idHandler;
        std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>> _data;
        std::set<utils::encapsulation::shm::Id> _ownerships;
        std::set<std::size_t> _await; // id that await to be free on user read

        /* shm */
        std::size_t _queue = 1;
        std::size_t _size = 0;
        std::uint16_t _ownership = 0; // 0 = creator, 1 = user, n = custom
        int _fd = -1;
        void* _ptr = nullptr;
        utils::encapsulation::shm::ShmMetadata* _metadata;
        std::vector<utils::encapsulation::shm::Slot> _slots;

        /* internal */
        std::jthread _thread; // auto read

        // ---------- Pre-Function -------- //
        void init_(void); // create the internal thread
        void read_(void); // function call by the thread
        void send_(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Id& id, bool last, bool failsafe);

    public:
        // ---------- Pre-Function -------- //
        /* setup */
        void close(void);

        /* communication */
        void send(const std::vector<std::byte>& bytes, const utils::encapsulation::shm::Id& id, bool last = true, bool failsafe = false); // force an id for the request
        utils::encapsulation::shm::Id send(const std::vector<std::byte>& bytes, bool last = false, bool failsafe = false); // allocate an id for this request
        std::optional<std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>>> read(const utils::encapsulation::shm::ReadFilter filter = utils::encapsulation::shm::ReadFilter::All);
        std::optional<std::vector<std::vector<std::byte>>> read(const utils::encapsulation::shm::Id& id);
        bool readable(const utils::encapsulation::shm::ReadFilter filter = utils::encapsulation::shm::ReadFilter::All) const;
        void join(void); // wait for anything to be readed
        void join(utils::encapsulation::shm::Id id); // for a specifc id to be readed

        // ------------ Function ---------- //
        _cold inline void ownership(std::uint16_t ownership) {this->_ownership = ownership;};
        _hot _nodiscard inline std::uint16_t ownership(void) const {return this->_ownership;};

        /* setup */
        template<bool create = true, utils::encapsulation::shm::LayoutPolicy policy = utils::encapsulation::shm::LayoutPolicy::Compact, std::size_t group_size = 2, bool forced = false>
        void init(const std::string& name, std::size_t size, std::size_t queue = 1) // size is only for a single section, real size: sizeof(IdHandler lock) + (size + sizeof(metadata)) * queue
        {
            if constexpr (!forced)
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "Why? Do you want to create a empty memory??? (required: size > 0)");

            // setup
            this->_queue = queue;
            this->_size = size;

            // open the shm
            this->_fd = ::shm_open(name.data(), (create ? (O_CREAT | O_RDWR) : O_RDWR), 0600);
            if (this->_fd == -1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::ShmOpen, ::strerror(errno));
            }

            // setup the memory in create mode
            if constexpr (create) {
                if (::ftruncate(this->_fd, static_cast<off_t>(this->_size)) == -1) _unlikely {
                    ::close(this->_fd);
                    this->_fd = -1;
                    throw utils::exception::ErrorException(utils::exception::InternalCode::Ftruncate, ::strerror(errno));
                }
            }

            // compute size: metadata + (size + metadata) * queue
            std::size_t group_count = (this->_queue + group_size - 1) / group_size;
            std::size_t mem_size = utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmMetadata));
            if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Compact || policy == utils::encapsulation::shm::LayoutPolicy::Interleaved) {
                mem_size += (this->_size + sizeof(utils::encapsulation::shm::ShmRequestMetadata)) * this->_queue;
            } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::CompactSemiAligned) {
                mem_size += utils::encapsulation::shm::align_ceil(this->_size * group_size) * group_count + utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmRequestMetadata) * group_size) * group_count;
            } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::InterleavedAligned) {
                mem_size += utils::encapsulation::shm::align_ceil(this->_size + sizeof(utils::encapsulation::shm::ShmRequestMetadata)) * this->_queue;
            }

            // allocate the memory
            this->_ptr = ::mmap(
                nullptr,
                mem_size,
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

            // init global metadata and starting address
            utils::encapsulation::shm::ShmRequestMetadata* metadata = nullptr;
            std::byte *ptr_metadata, *ptr_bytes, *last_ptr_metadata, *last_ptr_bytes;
            std::byte* ptr = (std::byte*) this->_ptr;
            if constexpr (create) this->_metadata = std::construct_at((utils::encapsulation::shm::ShmMetadata*) ptr);
            else this->_metadata = (utils::encapsulation::shm::ShmMetadata*) ptr;
            ptr += utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmMetadata));
            ptr_metadata = ptr; ptr_bytes = ptr;
            if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Compact) {
                ptr_bytes += sizeof(utils::encapsulation::shm::ShmRequestMetadata) * this->_queue;
            } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::CompactSemiAligned) {
                ptr_bytes += utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmRequestMetadata) * group_size) * group_count;
            }
            last_ptr_metadata = ptr_metadata; last_ptr_bytes = ptr_bytes;

            // for each slot init memory and store address
            for (std::size_t i = 0; i < this->_queue; ++i) {
                // setup address at actual emplacement
                metadata = (utils::encapsulation::shm::ShmRequestMetadata*) ptr_metadata;
                if constexpr (create) std::construct_at(metadata);
                this->_slots.emplace_back(metadata, ptr_bytes);

                // setup next emplacement
                if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Compact) {
                    ptr_metadata += sizeof(utils::encapsulation::shm::ShmRequestMetadata);
                    ptr_bytes += this->_size;
                } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::CompactSemiAligned) {
                    if (group_size == 1 || ((i + 1) % group_size == 0)) {
                        last_ptr_metadata += utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmRequestMetadata) * group_size);
                        last_ptr_bytes += utils::encapsulation::shm::align_ceil(this->_size * group_size);
                        ptr_metadata = last_ptr_metadata;
                        ptr_bytes = last_ptr_bytes;
                    } else {
                        ptr_metadata += sizeof(utils::encapsulation::shm::ShmRequestMetadata);
                        ptr_bytes += this->_size;
                    }
                } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::Interleaved) {
                    ptr_metadata = ptr;
                    ptr_bytes = ptr_metadata + sizeof(utils::encapsulation::shm::ShmRequestMetadata);
                    ptr = ptr_bytes + this->_size;
                } else if constexpr (policy == utils::encapsulation::shm::LayoutPolicy::InterleavedAligned) {
                    ptr_metadata = ptr;
                    ptr_bytes = ptr_metadata + sizeof(utils::encapsulation::shm::ShmRequestMetadata);
                    ptr += utils::encapsulation::shm::align_ceil(sizeof(utils::encapsulation::shm::ShmRequestMetadata) + this->_size);
                }
            }

            enum class LayoutPolicy {
                Compact,             // (meta,meta,...)(byte,byte,...)  — 1 writer, N readers
                CompactSemiAligned,  // idem + alignement per groups    — some writers, some at the same time
                Interleaved,         // (meta,byte,meta,byte,...)       — N writers moderated, payload >= cache-line (advise)
                InterleavedAligned   // idem + alignas(hardware_destructive_interference_size) per slot — N writers, many at the same time
            };

            // increment connected counter
            this->_metadata->connected.fetch_add(1, std::memory_order_relaxed);
        }

        /* communication */
        _hot inline void send(const std::vector<std::byte>& bytes, std::size_t id, bool last, bool failsafe) {this->send(bytes, {id, this->_ownership}, last, failsafe);}
        _hot _nodiscard inline bool readable(const utils::encapsulation::shm::Id& id) const {std::lock_guard<std::mutex> lock(this->_lock); return this->_data.contains(id);};

        // ------------ Operator ---------- //
        SharedMemory& operator=(const SharedMemory& other) = delete;
        SharedMemory& operator=(SharedMemory&& other) = delete;

        // ---------- Constructor --------- //
        SharedMemory(std::uint16_t ownership = 0): _ownership{ownership} {this->init_();};
        SharedMemory(const SharedMemory& other) = delete;
        SharedMemory(SharedMemory&& other) = delete;

        // ----------- Destructor --------- //
        ~SharedMemory() {this->close();};
};

} // namespace end
#endif /* SHAREDMEMORY_H */
