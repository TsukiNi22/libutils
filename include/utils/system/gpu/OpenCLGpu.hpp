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
##  @file OpenCLGpu.hpp

File Description:
##  OpenCL gpu handler of the HybridMemory
\**************************************************************/

#ifndef OPENCLGPU_H
    #define OPENCLGPU_H

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

// Any gpu with an OpenCL driver (NVIDIA, AMD, Intel), ICD loader loaded at runtime
class OpenCLGpu final: public utils::system::AGpu {
    private:
        struct Api; // functions of the ICD loader (defined in the .cpp)

        std::unique_ptr<Api> _api; // functions loaded from the ICD loader
        void* _platform = nullptr; // cl_platform_id
        void* _device = nullptr; // cl_device_id (first gpu device)
        void* _context = nullptr; // cl_context
        void* _queue = nullptr; // cl_command_queue (in order)

        // ---------- Pre-Function -------- //
        void loadSymbols_(void); // clGetPlatformIDs, clCreateContext, clCreateBuffer, clEnqueue*Buffer... from the ICD loader
        void selectDevice_(void); // first gpu device of the first platform having one: context, queue, name & memory

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
        _cold _nodiscard utils::system::MemoryMode getMode(void) const final {return utils::system::MemoryMode::OpenCL;};

        // ------------ Operator ---------- //
        OpenCLGpu& operator=(const OpenCLGpu& other) = delete;
        OpenCLGpu& operator=(OpenCLGpu&& other) = delete;

        // ---------- Constructor --------- //
        OpenCLGpu(); // load libOpenCL.so.1 (OpenCL.dll on Windows), unavailable without the driver or a device (never throw)
        OpenCLGpu(const OpenCLGpu& other) = delete;
        OpenCLGpu(OpenCLGpu&& other) = delete;

        // ----------- Destructor --------- //
        ~OpenCLGpu() noexcept final; // release the device objects
};

} // namespace end
#endif /* OPENCLGPU_H */
