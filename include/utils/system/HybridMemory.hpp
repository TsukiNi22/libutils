/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/10/2026 by @author Tsukini

File Name:
##  @file HybridMemory.hpp

File Description:
##  Memory loaded on the gpu when available, else in the ram
\**************************************************************/

#ifndef HYBRIDMEMORY_H
    #define HYBRIDMEMORY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::InternalCode
    #include "../attribute/Attribute.hpp"               // _hot, _cold, _nodiscard, _unlikely, _likely
    #include "HybridMemoryDefine.hpp"                   // utils::system::MemoryMode
    #include "gpu/Gpu.hpp"                              // utils::system::IGpu, utils::system::GpuBuffer
    #include <type_traits>                              // std::is_trivially_copyable_v
    #include <algorithm>                                // std::min, std::copy_n, std::fill_n, std::all_of
    #include <cstddef>                                  // std::size_t, std::byte
    #include <cstring>                                  // std::memset, std::memcpy
    #include <limits>                                   // std::numeric_limits
    #include <memory>                                   // std::shared_ptr, std::unique_ptr, std::make_unique_for_overwrite
    #include <string>                                   // std::to_string
    #include <vector>                                   // std::vector
    #include <array>                                    // std::array
    #include <span>                                     // std::span
    #include <new>                                      // std::bad_alloc
    #include <bit>                                      // std::bit_cast

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* gpu */
// Auto / Gpu: Cuda -> OpenCL -> Vulkan, nullptr = no gpu (Ram, or Auto without a gpu)
// Forced backend or Gpu without any: GpuUnavailable error, Custom: InvalidArgument (give the handler)
_nodiscard utils::system::IGpu* make_raw_gpu(const utils::system::MemoryMode mode);
_cold _nodiscard inline std::shared_ptr<utils::system::IGpu> make_shared_gpu(const utils::system::MemoryMode mode) {return std::shared_ptr<utils::system::IGpu>(utils::system::make_raw_gpu(mode));};
_cold _nodiscard inline std::unique_ptr<utils::system::IGpu> make_unique_gpu(const utils::system::MemoryMode mode) {return std::unique_ptr<utils::system::IGpu>(utils::system::make_raw_gpu(mode));};

// default redirection
_cold _nodiscard inline std::unique_ptr<utils::system::IGpu> make_gpu(const utils::system::MemoryMode mode = utils::system::MemoryMode::Auto) {return utils::system::make_unique_gpu(mode);};

//----------------------------------------------------------------//
/* CLASS */

// Store T on the gpu when one is available (or forced), else in the ram, for future computes on it
template<typename T>
class HybridMemory {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable (copied to the gpu as bytes)");

    private:
        utils::system::MemoryMode _requested = utils::system::MemoryMode::Auto; // mode asked at the construction
        std::shared_ptr<utils::system::IGpu> _gpu = nullptr; // gpu handler (nullptr = ram)
        utils::system::MemoryMode _mode = utils::system::MemoryMode::Ram; // mode really used: Ram | Cuda | OpenCL | Vulkan | Custom
        utils::system::GpuBuffer _buffer = 0; // device buffer (gpu modes, 0 = none)
        std::unique_ptr<T[]> _host = nullptr; // host buffer (ram mode)
        std::size_t _size = 0; // number of T allocated

        // ------------ Function ---------- //
        _hot static std::size_t bytesOf_(const std::size_t count) // count * sizeof(T) (OutOfMemory on an overflow)
        {
            if (count > std::numeric_limits<std::size_t>::max() / sizeof(T)) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfMemory, std::to_string(count) + " elements overflow the size");
            }
            return count * sizeof(T);
        };

        _hot static std::unique_ptr<T[]> allocateHost_(const std::size_t count) // uninitialized host buffer of count T (OutOfMemory)
        {
            if (count == 0) return nullptr;
            try {
                return std::make_unique_for_overwrite<T[]>(count);
            } catch (const std::bad_alloc&) {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfMemory, std::to_string(bytesOf_(count)) + " bytes in the ram");
            }
        };

        _hot void allocate_(const std::size_t size) // allocate size T on the gpu or in the ram (OutOfMemory | GpuAllocation)
        {
            if (size == 0) return;
            if (this->_gpu) this->_buffer = this->_gpu->allocate(this->bytesOf_(size));
            else this->_host = this->allocateHost_(size);
            this->_size = size;
        };

        _cold void release_(void) noexcept // free the buffer of the current mode
        {
            if (this->_gpu && this->_buffer) this->_gpu->free(this->_buffer);
            this->_buffer = 0;
            this->_host.reset();
            this->_size = 0;
        };

        _hot void check_(const std::size_t index, const std::size_t count) const // OutOfBounds when [index, index + count) is outside the memory
        {
            if (index > this->_size || count > this->_size - index) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "[" + std::to_string(index) + ", " + std::to_string(index + count) + ") outside of " + std::to_string(this->_size) + " elements");
            }
        };

    public:
        // ------------ Function ---------- //
        /* memory */
        _cold void allocate(const std::size_t size) // free the previous memory then allocate size T (uninitialized)
        {
            this->release_();
            this->allocate_(size);
        };

        _cold void resize(const std::size_t size) // new size, the first min(size, old size) T kept
        {
            const std::size_t kept = std::min(size, this->_size);

            if (size == this->_size) return;
            if (this->_gpu) {
                utils::system::GpuBuffer buffer = (size == 0) ? 0 : this->_gpu->allocate(this->bytesOf_(size));
                try {
                    if (kept) this->_gpu->memcpy(buffer, this->_buffer, this->bytesOf_(kept));
                } catch (...) {
                    this->_gpu->free(buffer); // the old memory is kept on a failure
                    throw;
                }
                if (this->_buffer) this->_gpu->free(this->_buffer);
                this->_buffer = buffer;
            } else {
                std::unique_ptr<T[]> host = this->allocateHost_(size);
                std::copy_n(this->_host.get(), kept, host.get());
                this->_host = std::move(host);
            }
            this->_size = size;
        };

        _cold inline void free(void) noexcept {this->release_();}; // release the memory (size 0, mode kept)

        _hot void clear(void) // every byte set to 0, allocation kept
        {
            if (this->_size == 0) return;
            if (this->_gpu) this->_gpu->memset(this->_buffer, std::byte{0}, this->bytes());
            else std::memset(static_cast<void*>(this->_host.get()), 0, this->bytes());
        };

        /* access */
        _hot void store(std::span<const T> values, const std::size_t offset = 0) // copy the values from the index offset (OutOfBounds)
        {
            this->check_(offset, values.size());
            if (values.empty()) return;
            if (this->_gpu) this->_gpu->store(this->_buffer, values.data(), values.size_bytes(), this->bytesOf_(offset));
            else std::copy_n(values.data(), values.size(), this->_host.get() + offset);
        };

        _hot void fetch(std::span<T> out, const std::size_t offset = 0) const // copy out.size() T from the index offset (OutOfBounds)
        {
            this->check_(offset, out.size());
            if (out.empty()) return;
            if (this->_gpu) this->_gpu->fetch(out.data(), this->_buffer, out.size_bytes(), this->bytesOf_(offset));
            else std::copy_n(this->_host.get() + offset, out.size(), out.data());
        };

        _hot _nodiscard T get(const std::size_t index) const // one T (a transfer on the gpu: prefer fetch for many)
        {
            std::array<std::byte, sizeof(T)> raw = {};

            this->check_(index, 1);
            if (!this->_gpu) _likely return this->_host[index];
            this->_gpu->fetch(raw.data(), this->_buffer, sizeof(T), this->bytesOf_(index));
            return std::bit_cast<T>(raw);
        };

        _hot void set(const std::size_t index, const T& value) // one T (a transfer on the gpu: prefer store for many)
        {
            this->check_(index, 1);
            if (this->_gpu) this->_gpu->store(this->_buffer, &value, sizeof(T), this->bytesOf_(index));
            else this->_host[index] = value;
        };

        _hot void fill(const T& value) // every T set to value
        {
            const std::array<std::byte, sizeof(T)> raw = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);

            if (this->_size == 0) return;
            if (!this->_gpu) {
                std::fill_n(this->_host.get(), this->_size, value);
                return;
            }

            // Same byte everywhere: filled by the gpu, else uploaded by chunks of the repeated value
            if (std::all_of(raw.begin(), raw.end(), [&](const std::byte b) {return b == raw[0];})) {
                this->_gpu->memset(this->_buffer, raw[0], this->bytes());
                return;
            }
            const std::vector<T> chunk(std::min<std::size_t>(this->_size, (1 << 20) / sizeof(T) + 1), value);
            for (std::size_t index = 0; index < this->_size; index += chunk.size())
                this->_gpu->store(this->_buffer, chunk.data(), this->bytesOf_(std::min(chunk.size(), this->_size - index)), this->bytesOf_(index));
        };

        _cold _nodiscard std::vector<T> toVector(void) const // whole memory copied in the ram
        {
            std::vector<T> values(this->_size);
            this->fetch(values);
            return values;
        };

        _hot inline void join(void) {if (this->_gpu) this->_gpu->join();}; // wait for the pending gpu operations (nothing in ram)

        /* getter */
        _cold _nodiscard inline std::size_t bytes(void) const                             {return this->_size * sizeof(T);}; // size in bytes
        _cold _nodiscard inline bool empty(void) const                                    {return this->_size == 0;}; // nothing allocated
        _cold _nodiscard inline bool isOnGpu(void) const                                  {return this->_gpu != nullptr;}; // memory stored by a gpu handler
        _cold _nodiscard inline std::size_t size(void) const                              {return this->_size;};
        _cold _nodiscard inline utils::system::MemoryMode getMode(void) const             {return this->_mode;};
        _cold _nodiscard inline utils::system::MemoryMode getRequestedMode(void) const    {return this->_requested;};
        _cold _nodiscard inline std::shared_ptr<utils::system::IGpu> getGpu(void) const   {return this->_gpu;};
        _hot _nodiscard inline utils::system::GpuBuffer native(void) const                {return this->_buffer;}; // device buffer for the computes (0 in ram)
        _hot _nodiscard inline T* data(void)                                              {return this->_host.get();}; // host memory (nullptr on the gpu)
        _hot _nodiscard inline const T* data(void) const                                  {return this->_host.get();};

        // ------------ Operator ---------- //
        _hot _nodiscard T operator[](const std::size_t index) const {return this->get(index);};
        HybridMemory& operator=(const HybridMemory& other) = delete;
        HybridMemory& operator=(HybridMemory&& other) noexcept // take the memory and the handler of other
        {
            if (this == &other) return *this;
            this->release_();
            this->_requested = other._requested;
            this->_gpu = std::move(other._gpu);
            this->_mode = other._mode;
            this->_buffer = other._buffer;
            this->_host = std::move(other._host);
            this->_size = other._size;
            other._mode = utils::system::MemoryMode::Ram;
            other._buffer = 0;
            other._size = 0;
            return *this;
        };

        // ---------- Constructor --------- //
        HybridMemory(const utils::system::MemoryMode mode = utils::system::MemoryMode::Auto): _requested{mode}, _gpu{utils::system::make_shared_gpu(mode)}, _mode{this->_gpu ? this->_gpu->getMode() : utils::system::MemoryMode::Ram} { /* Nothing */ }; // select the backend (make_gpu), nothing allocated
        HybridMemory(const std::size_t size, const utils::system::MemoryMode mode = utils::system::MemoryMode::Auto): HybridMemory(mode) {this->allocate_(size);}; // select the backend & allocate size T
        HybridMemory(const std::size_t size, std::shared_ptr<utils::system::IGpu> gpu) // Custom mode with a user handler (InvalidArgument when nullptr or unavailable)
        : _requested{utils::system::MemoryMode::Custom}, _gpu{gpu}, _mode{gpu ? gpu->getMode() : utils::system::MemoryMode::Custom}
        {
            if (!this->_gpu || !this->_gpu->isAvailable()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "the custom gpu handler is null or unavailable");
            }
            this->allocate_(size);
        };
        HybridMemory(const HybridMemory& other) = delete;
        HybridMemory(HybridMemory&& other) noexcept // take the memory and the handler of other
        : _requested{other._requested}, _gpu{std::move(other._gpu)}, _mode{other._mode}, _buffer{other._buffer}, _host{std::move(other._host)}, _size{other._size}
        {
            other._mode = utils::system::MemoryMode::Ram;
            other._buffer = 0;
            other._size = 0;
        };

        // ----------- Destructor --------- //
        ~HybridMemory() noexcept {this->release_();}; // release the memory
};

} // namespace end
#endif /* HYBRIDMEMORY_H */
