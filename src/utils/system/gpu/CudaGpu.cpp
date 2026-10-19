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
##  @file CudaGpu.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/SharedObject.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/IException.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/system/gpu/CudaGpu.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <array>

// Functions of the CUDA driver API (CUresult = int, CUdevice = int, CUdeviceptr = unsigned long long, CUcontext = void*)
struct utils::system::CudaGpu::Api {
    int (GPU_API_CALL* cuInit)(unsigned int) = nullptr;
    int (GPU_API_CALL* cuDeviceGetCount)(int*) = nullptr;
    int (GPU_API_CALL* cuDeviceGet)(int*, int) = nullptr;
    int (GPU_API_CALL* cuDeviceGetName)(char*, int, int) = nullptr;
    int (GPU_API_CALL* cuDeviceTotalMem)(std::size_t*, int) = nullptr;
    int (GPU_API_CALL* cuCtxCreate)(void**, unsigned int, int) = nullptr;
    int (GPU_API_CALL* cuCtxDestroy)(void*) = nullptr;
    int (GPU_API_CALL* cuCtxSetCurrent)(void*) = nullptr;
    int (GPU_API_CALL* cuCtxSynchronize)(void) = nullptr;
    int (GPU_API_CALL* cuMemAlloc)(unsigned long long*, std::size_t) = nullptr;
    int (GPU_API_CALL* cuMemFree)(unsigned long long) = nullptr;
    int (GPU_API_CALL* cuMemcpyHtoD)(unsigned long long, const void*, std::size_t) = nullptr;
    int (GPU_API_CALL* cuMemcpyDtoH)(void*, unsigned long long, std::size_t) = nullptr;
    int (GPU_API_CALL* cuMemcpyDtoD)(unsigned long long, unsigned long long, std::size_t) = nullptr;
    int (GPU_API_CALL* cuMemsetD8)(unsigned long long, unsigned char, std::size_t) = nullptr;
};

// Throw the code with the failed call when a CUDA call doesn't return CUDA_SUCCESS (0)
_cold static inline void check(const int result, const char* call, const utils::exception::InternalCode code)
{
    if (result != 0) _unlikely {
        throw utils::exception::ErrorException(code, std::string(call) + ": CUDA error " + std::to_string(result));
    }
}

_cold utils::system::CudaGpu::CudaGpu()
: _api{std::make_unique<utils::system::CudaGpu::Api>()}
{
    #ifdef UTILS_WINDOWS
        if (!this->load_({"nvcuda.dll"})) return;
    #else
        if (!this->load_({"libcuda.so.1", "libcuda.so"})) return;
    #endif

    try {
        this->loadSymbols_();
        this->selectDevice_();
        this->_available = true;
    } catch (const utils::exception::IException&) {
        // Not an error: the backend is only unavailable
        this->_available = false;
    }
}

_cold utils::system::CudaGpu::~CudaGpu() noexcept
{
    if (this->_context && this->_api->cuCtxDestroy) (void)this->_api->cuCtxDestroy(this->_context);
}

_cold void utils::system::CudaGpu::loadSymbols_(void)
{
    // The _v2 symbols are the 64 bits versions of the driver API
    this->_api->cuInit = this->_library->loadFunction<decltype(this->_api->cuInit)>("cuInit");
    this->_api->cuDeviceGetCount = this->_library->loadFunction<decltype(this->_api->cuDeviceGetCount)>("cuDeviceGetCount");
    this->_api->cuDeviceGet = this->_library->loadFunction<decltype(this->_api->cuDeviceGet)>("cuDeviceGet");
    this->_api->cuDeviceGetName = this->_library->loadFunction<decltype(this->_api->cuDeviceGetName)>("cuDeviceGetName");
    this->_api->cuDeviceTotalMem = this->_library->loadFunction<decltype(this->_api->cuDeviceTotalMem)>("cuDeviceTotalMem_v2");
    this->_api->cuCtxCreate = this->_library->loadFunction<decltype(this->_api->cuCtxCreate)>("cuCtxCreate_v2");
    this->_api->cuCtxDestroy = this->_library->loadFunction<decltype(this->_api->cuCtxDestroy)>("cuCtxDestroy_v2");
    this->_api->cuCtxSetCurrent = this->_library->loadFunction<decltype(this->_api->cuCtxSetCurrent)>("cuCtxSetCurrent");
    this->_api->cuCtxSynchronize = this->_library->loadFunction<decltype(this->_api->cuCtxSynchronize)>("cuCtxSynchronize");
    this->_api->cuMemAlloc = this->_library->loadFunction<decltype(this->_api->cuMemAlloc)>("cuMemAlloc_v2");
    this->_api->cuMemFree = this->_library->loadFunction<decltype(this->_api->cuMemFree)>("cuMemFree_v2");
    this->_api->cuMemcpyHtoD = this->_library->loadFunction<decltype(this->_api->cuMemcpyHtoD)>("cuMemcpyHtoD_v2");
    this->_api->cuMemcpyDtoH = this->_library->loadFunction<decltype(this->_api->cuMemcpyDtoH)>("cuMemcpyDtoH_v2");
    this->_api->cuMemcpyDtoD = this->_library->loadFunction<decltype(this->_api->cuMemcpyDtoD)>("cuMemcpyDtoD_v2");
    this->_api->cuMemsetD8 = this->_library->loadFunction<decltype(this->_api->cuMemsetD8)>("cuMemsetD8_v2");
}

_cold void utils::system::CudaGpu::selectDevice_(void)
{
    std::array<char, 256> name = {};
    int count = 0;

    check(this->_api->cuInit(0), "cuInit", utils::exception::InternalCode::GpuUnavailable);
    check(this->_api->cuDeviceGetCount(&count), "cuDeviceGetCount", utils::exception::InternalCode::GpuUnavailable);
    if (count <= 0) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::GpuUnavailable, "no CUDA device");

    // First device, its context is made current by every call (bind_)
    check(this->_api->cuDeviceGet(&this->_device, 0), "cuDeviceGet", utils::exception::InternalCode::GpuUnavailable);
    check(this->_api->cuDeviceGetName(name.data(), static_cast<int>(name.size()), this->_device), "cuDeviceGetName", utils::exception::InternalCode::GpuUnavailable);
    check(this->_api->cuDeviceTotalMem(&this->_memory, this->_device), "cuDeviceTotalMem", utils::exception::InternalCode::GpuUnavailable);
    check(this->_api->cuCtxCreate(&this->_context, 0, this->_device), "cuCtxCreate", utils::exception::InternalCode::GpuUnavailable);
    this->_name = name.data();
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (needs check() and the Api of the .cpp)
_hot void utils::system::CudaGpu::bind_(void) const
{
    check(this->_api->cuCtxSetCurrent(this->_context), "cuCtxSetCurrent", utils::exception::InternalCode::GpuTransfer);
}

_hot utils::system::GpuBuffer utils::system::CudaGpu::allocate(const std::size_t bytes)
{
    unsigned long long buffer = 0;

    if (bytes == 0) return 0;
    this->bind_();
    check(this->_api->cuMemAlloc(&buffer, bytes), "cuMemAlloc", utils::exception::InternalCode::GpuAllocation);
    return static_cast<utils::system::GpuBuffer>(buffer);
}

_hot void utils::system::CudaGpu::free(const utils::system::GpuBuffer buffer) noexcept
{
    if (buffer == 0 || !this->_available) return;
    if (this->_api->cuCtxSetCurrent(this->_context) != 0) _unlikely return; // nothing to release without the context
    (void)this->_api->cuMemFree(static_cast<unsigned long long>(buffer));
}

_hot void utils::system::CudaGpu::store(const utils::system::GpuBuffer dst, const void* src, const std::size_t bytes, const std::size_t offset)
{
    if (bytes == 0) return;
    this->bind_();
    check(this->_api->cuMemcpyHtoD(static_cast<unsigned long long>(dst + offset), src, bytes), "cuMemcpyHtoD", utils::exception::InternalCode::GpuTransfer);
}

_hot void utils::system::CudaGpu::fetch(void* dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t offset) const
{
    if (bytes == 0) return;
    this->bind_();
    check(this->_api->cuMemcpyDtoH(dst, static_cast<unsigned long long>(src + offset), bytes), "cuMemcpyDtoH", utils::exception::InternalCode::GpuTransfer);
}

_hot void utils::system::CudaGpu::memset(const utils::system::GpuBuffer buffer, const std::byte value, const std::size_t bytes, const std::size_t offset)
{
    if (bytes == 0) return;
    this->bind_();
    check(this->_api->cuMemsetD8(static_cast<unsigned long long>(buffer + offset), static_cast<unsigned char>(value), bytes), "cuMemsetD8", utils::exception::InternalCode::GpuTransfer);
}

_hot void utils::system::CudaGpu::memcpy(const utils::system::GpuBuffer dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t dstOffset, const std::size_t srcOffset)
{
    if (bytes == 0) return;
    this->bind_();
    check(this->_api->cuMemcpyDtoD(static_cast<unsigned long long>(dst + dstOffset), static_cast<unsigned long long>(src + srcOffset), bytes), "cuMemcpyDtoD", utils::exception::InternalCode::GpuTransfer);
}

_hot void utils::system::CudaGpu::join(void)
{
    if (!this->_available) return;
    this->bind_();
    check(this->_api->cuCtxSynchronize(), "cuCtxSynchronize", utils::exception::InternalCode::GpuTransfer);
}
