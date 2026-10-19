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
##  @file CudaGpu.hpp

File Description:
##  Cuda gpu handler of the HybridMemory
\**************************************************************/

#ifndef CUDAGPU_H
    #define CUDAGPU_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _hot, _cold, _nodiscard
    #include "AGpu.hpp"                         // utils::system::AGpu, utils::system::GpuBuffer
    #include <cstddef>                          // std::size_t, std::byte
    #include <memory>                           // std::unique_ptr

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* CLASS */

// NVIDIA gpu through the CUDA driver API (loaded at runtime, no toolkit needed at the build)
class CudaGpu final: public utils::system::AGpu {
    private:
        struct Api; // functions of the driver (defined in the .cpp)

        std::unique_ptr<Api> _api; // functions loaded from the driver
        void* _context = nullptr; // CUcontext of the device
        int _device = -1; // CUdevice (-1 = none)

        // ---------- Pre-Function -------- //
        void loadSymbols_(void); // cuInit, cuDeviceGet, cuCtxCreate, cuMemAlloc, cuMemcpy*, cuMemsetD8... from the driver
        void selectDevice_(void); // first device: context, name & memory
        void bind_(void) const; // make the context current on the calling thread (GpuTransfer)

    public:
        // ---------- Pre-Function -------- //
        /* memory */
        utils::system::GpuBuffer allocate(const std::size_t bytes) final;
        void free(const utils::system::GpuBuffer buffer) noexcept final;

        /* transfer */
        void store(const utils::system::GpuBuffer dst, const void* src, const std::size_t bytes, const std::size_t offset = 0) final;
        void fetch(void* dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t offset = 0) const final;
        void memset(const utils::system::GpuBuffer buffer, const std::byte value, const std::size_t bytes, const std::size_t offset = 0) final;
        void memcpy(const utils::system::GpuBuffer dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t dstOffset = 0, const std::size_t srcOffset = 0) final;
        void join(void) final;

        // ------------ Function ---------- //
        /* getter */
        _cold _nodiscard utils::system::MemoryMode getMode(void) const final {return utils::system::MemoryMode::Cuda;};

        // ------------ Operator ---------- //
        CudaGpu& operator=(const CudaGpu& other) = delete;
        CudaGpu& operator=(CudaGpu&& other) = delete;

        // ---------- Constructor --------- //
        CudaGpu(); // load libcuda.so.1 (nvcuda.dll on Windows), unavailable without the driver or a device (never throw)
        CudaGpu(const CudaGpu& other) = delete;
        CudaGpu(CudaGpu&& other) = delete;

        // ----------- Destructor --------- //
        ~CudaGpu() noexcept final; // release the device objects
};

} // namespace end
#endif /* CUDAGPU_H */
