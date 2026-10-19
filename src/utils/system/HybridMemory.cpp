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
##  @file HybridMemory.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/system/HybridMemory.hpp"
#include <memory>
#include <vector>

_cold utils::system::IGpu* utils::system::make_raw_gpu(const utils::system::MemoryMode mode)
{
    std::vector<utils::system::MemoryMode> candidates = {};

    switch (mode) {
        case utils::system::MemoryMode::Ram: return nullptr;
        case utils::system::MemoryMode::Custom: throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "MemoryMode::Custom needs the gpu handler (HybridMemory(size, gpu))");
        case utils::system::MemoryMode::Auto: _fallthrough;
        case utils::system::MemoryMode::Gpu: candidates = {utils::system::MemoryMode::Cuda, utils::system::MemoryMode::OpenCL, utils::system::MemoryMode::Vulkan}; break;
        default: candidates = {mode}; break;
    }

    // First available backend of the candidates
    for (const utils::system::MemoryMode candidate: candidates) {
        utils::system::IGpu* gpu = nullptr;
        switch (candidate) {
            case utils::system::MemoryMode::Cuda:   gpu = new utils::system::CudaGpu();   break;
            case utils::system::MemoryMode::OpenCL: gpu = new utils::system::OpenCLGpu(); break;
            default:                                gpu = new utils::system::VulkanGpu(); break;
        }
        if (gpu->isAvailable()) return gpu;
        delete gpu; // destroy ptr, the invalid gpu won't be used
    }

    // Auto falls back on the ram, a gpu mode can't
    if (mode == utils::system::MemoryMode::Auto) return nullptr;
    throw utils::exception::ErrorException(utils::exception::InternalCode::GpuUnavailable, (mode == utils::system::MemoryMode::Gpu) ? "no Cuda, OpenCL or Vulkan device" : "the forced backend has no driver or device");
}
