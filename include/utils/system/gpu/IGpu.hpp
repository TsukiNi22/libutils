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
##  @file IGpu.hpp

File Description:
##  Interface of a gpu handler used by the HybridMemory
\**************************************************************/

#ifndef IGPU_H
    #define IGPU_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../HybridMemoryDefine.hpp"    // utils::system::MemoryMode
    #include <cstddef>                      // std::size_t, std::byte
    #include <cstdint>                      // std::uintptr_t
    #include <string>                       // std::string

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* TYPEDEF */

using GpuBuffer = std::uintptr_t; // opaque device buffer of a handler (CUdeviceptr, cl_mem, Vulkan allocation...), 0 = none

//----------------------------------------------------------------//
/* CLASS */

// Handler of one gpu backend: a custom one can be given to a HybridMemory (MemoryMode::Custom)
class IGpu {
    public:
        // ---------- Pre-Function -------- //
        /* getter */
        virtual bool isAvailable(void) const = 0; // backend loaded and a device ready
        virtual utils::system::MemoryMode getMode(void) const = 0; // Cuda | OpenCL | Vulkan | Custom
        virtual std::string getName(void) const = 0; // name of the device ("[None]" when unavailable)
        virtual std::size_t getMemory(void) const = 0; // memory of the device in bytes (0 = unknown)

        /* memory */
        virtual utils::system::GpuBuffer allocate(const std::size_t bytes) = 0; // new device buffer (GpuAllocation error)
        virtual void free(const utils::system::GpuBuffer buffer) noexcept = 0; // release a buffer (0 = nothing to do)

        /* transfer */
        // Offsets in bytes, GpuTransfer error on a failure
        virtual void store(const utils::system::GpuBuffer dst, const void* src, const std::size_t bytes, const std::size_t offset = 0) = 0; // host -> device
        virtual void fetch(void* dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t offset = 0) const = 0; // device -> host
        virtual void memset(const utils::system::GpuBuffer buffer, const std::byte value, const std::size_t bytes, const std::size_t offset = 0) = 0; // set every byte of the range
        virtual void memcpy(const utils::system::GpuBuffer dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t dstOffset = 0, const std::size_t srcOffset = 0) = 0; // device -> device
        virtual void join(void) = 0; // wait for the pending operations of the device

        // ------------ Operator ---------- //
        IGpu& operator=(const IGpu& other) = delete;
        IGpu& operator=(IGpu&& other) = delete;

        // ---------- Constructor --------- //
        IGpu() = default;
        IGpu(const IGpu& other) = delete;
        IGpu(IGpu&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~IGpu() = default;
};

} // namespace end
#endif /* IGPU_H */
