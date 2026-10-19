/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 17/10/2026 by @author Tsukini

File Name:
##  @file HybridMemoryDefine.hpp

File Description:
##  Modes of the HybridMemory (gpu / ram)
\**************************************************************/

#ifndef HYBRIDMEMORYDEFINE_H
    #define HYBRIDMEMORYDEFINE_H

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* ENUM */

// Where the memory of a HybridMemory is loaded
enum class MemoryMode {
    Auto, // first gpu found (Cuda -> OpenCL -> Vulkan), else the ram
    Gpu, // first gpu found (Cuda -> OpenCL -> Vulkan), GpuUnavailable error without one
    Ram, // host memory only
    Cuda, // forced: NVIDIA CUDA driver (libcuda / nvcuda.dll)
    OpenCL, // forced: OpenCL ICD loader (libOpenCL / OpenCL.dll)
    Vulkan, // forced: Vulkan loader (libvulkan / vulkan-1.dll)
    Custom, // gpu handler given by the user (utils::system::IGpu)
};

} // namespace end
#endif /* HYBRIDMEMORYDEFINE_H */
