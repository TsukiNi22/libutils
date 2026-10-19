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
##  @file OpenCLGpu.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/SharedObject.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/IException.hpp"
#include "utils/system/gpu/OpenCLGpu.hpp"
#include "utils/attribute/Attribute.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

//----------------------------------------------------------------//
/* DEFINE */

/* OpenCL constants */
#define CL_DEVICE_TYPE_GPU (1 << 2)
#define CL_DEVICE_GLOBAL_MEM_SIZE 0x101F
#define CL_DEVICE_NAME 0x102B
#define CL_MEM_READ_WRITE (1 << 0)
#define CL_TRUE 1

// Functions of the OpenCL 1.2 API (cl_int = std::int32_t, cl_uint = std::uint32_t, cl_ulong = std::uint64_t, objects = void*)
struct utils::system::OpenCLGpu::Api {
    std::int32_t (GPU_API_CALL* clGetPlatformIDs)(std::uint32_t, void**, std::uint32_t*) = nullptr;
    std::int32_t (GPU_API_CALL* clGetDeviceIDs)(void*, std::uint64_t, std::uint32_t, void**, std::uint32_t*) = nullptr;
    std::int32_t (GPU_API_CALL* clGetDeviceInfo)(void*, std::uint32_t, std::size_t, void*, std::size_t*) = nullptr;
    void* (GPU_API_CALL* clCreateContext)(const std::intptr_t*, std::uint32_t, void* const*, void*, void*, std::int32_t*) = nullptr; // notify callback unused (nullptr)
    void* (GPU_API_CALL* clCreateCommandQueue)(void*, void*, std::uint64_t, std::int32_t*) = nullptr;
    void* (GPU_API_CALL* clCreateBuffer)(void*, std::uint64_t, std::size_t, void*, std::int32_t*) = nullptr;
    std::int32_t (GPU_API_CALL* clReleaseMemObject)(void*) = nullptr;
    std::int32_t (GPU_API_CALL* clReleaseCommandQueue)(void*) = nullptr;
    std::int32_t (GPU_API_CALL* clReleaseContext)(void*) = nullptr;
    std::int32_t (GPU_API_CALL* clEnqueueWriteBuffer)(void*, void*, std::uint32_t, std::size_t, std::size_t, const void*, std::uint32_t, void* const*, void**) = nullptr;
    std::int32_t (GPU_API_CALL* clEnqueueReadBuffer)(void*, void*, std::uint32_t, std::size_t, std::size_t, void*, std::uint32_t, void* const*, void**) = nullptr;
    std::int32_t (GPU_API_CALL* clEnqueueFillBuffer)(void*, void*, const void*, std::size_t, std::size_t, std::size_t, std::uint32_t, void* const*, void**) = nullptr;
    std::int32_t (GPU_API_CALL* clEnqueueCopyBuffer)(void*, void*, void*, std::size_t, std::size_t, std::size_t, std::uint32_t, void* const*, void**) = nullptr;
    std::int32_t (GPU_API_CALL* clFinish)(void*) = nullptr;
};

// Throw the code with the failed call when an OpenCL call doesn't return CL_SUCCESS (0)
_cold static void check(const std::int32_t result, const char* call, const utils::exception::InternalCode code)
{
    if (result != 0) _unlikely {
        throw utils::exception::ErrorException(code, std::string(call) + ": OpenCL error " + std::to_string(result));
    }
}

_cold utils::system::OpenCLGpu::OpenCLGpu()
: _api{std::make_unique<utils::system::OpenCLGpu::Api>()}
{
    #ifdef UTILS_WINDOWS
        if (!this->load_({"OpenCL.dll"})) return;
    #else
        if (!this->load_({"libOpenCL.so.1", "libOpenCL.so"})) return;
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

_cold utils::system::OpenCLGpu::~OpenCLGpu() noexcept
{
    if (this->_queue && this->_api->clReleaseCommandQueue) (void)this->_api->clReleaseCommandQueue(this->_queue);
    if (this->_context && this->_api->clReleaseContext) (void)this->_api->clReleaseContext(this->_context);
}

_cold void utils::system::OpenCLGpu::loadSymbols_(void)
{
    this->_api->clGetPlatformIDs = this->_library->loadFunction<decltype(this->_api->clGetPlatformIDs)>("clGetPlatformIDs");
    this->_api->clGetDeviceIDs = this->_library->loadFunction<decltype(this->_api->clGetDeviceIDs)>("clGetDeviceIDs");
    this->_api->clGetDeviceInfo = this->_library->loadFunction<decltype(this->_api->clGetDeviceInfo)>("clGetDeviceInfo");
    this->_api->clCreateContext = this->_library->loadFunction<decltype(this->_api->clCreateContext)>("clCreateContext");
    this->_api->clCreateCommandQueue = this->_library->loadFunction<decltype(this->_api->clCreateCommandQueue)>("clCreateCommandQueue");
    this->_api->clCreateBuffer = this->_library->loadFunction<decltype(this->_api->clCreateBuffer)>("clCreateBuffer");
    this->_api->clReleaseMemObject = this->_library->loadFunction<decltype(this->_api->clReleaseMemObject)>("clReleaseMemObject");
    this->_api->clReleaseCommandQueue = this->_library->loadFunction<decltype(this->_api->clReleaseCommandQueue)>("clReleaseCommandQueue");
    this->_api->clReleaseContext = this->_library->loadFunction<decltype(this->_api->clReleaseContext)>("clReleaseContext");
    this->_api->clEnqueueWriteBuffer = this->_library->loadFunction<decltype(this->_api->clEnqueueWriteBuffer)>("clEnqueueWriteBuffer");
    this->_api->clEnqueueReadBuffer = this->_library->loadFunction<decltype(this->_api->clEnqueueReadBuffer)>("clEnqueueReadBuffer");
    this->_api->clEnqueueFillBuffer = this->_library->loadFunction<decltype(this->_api->clEnqueueFillBuffer)>("clEnqueueFillBuffer");
    this->_api->clEnqueueCopyBuffer = this->_library->loadFunction<decltype(this->_api->clEnqueueCopyBuffer)>("clEnqueueCopyBuffer");
    this->_api->clFinish = this->_library->loadFunction<decltype(this->_api->clFinish)>("clFinish");
}

_cold void utils::system::OpenCLGpu::selectDevice_(void)
{
    std::vector<void*> platforms = {};
    std::uint32_t count = 0;
    std::uint64_t memory = 0;
    std::size_t length = 0;
    std::int32_t result = 0;

    check(this->_api->clGetPlatformIDs(0, nullptr, &count), "clGetPlatformIDs", utils::exception::InternalCode::GpuUnavailable);
    if (count == 0) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::GpuUnavailable, "no OpenCL platform");
    platforms.resize(count);
    check(this->_api->clGetPlatformIDs(count, platforms.data(), nullptr), "clGetPlatformIDs", utils::exception::InternalCode::GpuUnavailable);

    // First gpu device of the first platform having one (a platform without gpu returns an error)
    for (void* platform: platforms) {
        if (this->_api->clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &this->_device, nullptr) != 0) continue;
        this->_platform = platform;
        break;
    }
    if (!this->_platform) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::GpuUnavailable, "no OpenCL gpu device");

    // Name & memory of the device
    check(this->_api->clGetDeviceInfo(this->_device, CL_DEVICE_NAME, 0, nullptr, &length), "clGetDeviceInfo", utils::exception::InternalCode::GpuUnavailable);
    std::string name(length, '\0');
    check(this->_api->clGetDeviceInfo(this->_device, CL_DEVICE_NAME, length, name.data(), nullptr), "clGetDeviceInfo", utils::exception::InternalCode::GpuUnavailable);
    check(this->_api->clGetDeviceInfo(this->_device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(memory), &memory, nullptr), "clGetDeviceInfo", utils::exception::InternalCode::GpuUnavailable);
    this->_name = name.c_str(); // without the final '\0'
    this->_memory = static_cast<std::size_t>(memory);

    // Context & in order queue of the device
    this->_context = this->_api->clCreateContext(nullptr, 1, &this->_device, nullptr, nullptr, &result);
    check(result, "clCreateContext", utils::exception::InternalCode::GpuUnavailable);
    this->_queue = this->_api->clCreateCommandQueue(this->_context, this->_device, 0, &result);
    check(result, "clCreateCommandQueue", utils::exception::InternalCode::GpuUnavailable);
}

_hot utils::system::GpuBuffer utils::system::OpenCLGpu::allocate(const std::size_t bytes)
{
    std::int32_t result = 0;
    void* buffer = nullptr;

    if (bytes == 0) return 0;
    buffer = this->_api->clCreateBuffer(this->_context, CL_MEM_READ_WRITE, bytes, nullptr, &result);
    check(result, "clCreateBuffer", utils::exception::InternalCode::GpuAllocation);
    return reinterpret_cast<utils::system::GpuBuffer>(buffer);
}

_hot void utils::system::OpenCLGpu::free(const utils::system::GpuBuffer buffer) noexcept
{
    if (buffer == 0 || !this->_available) return;
    (void)this->_api->clReleaseMemObject(reinterpret_cast<void*>(buffer));
}

_hot void utils::system::OpenCLGpu::store(const utils::system::GpuBuffer dst, const void* src, const std::size_t bytes, const std::size_t offset)
{
    if (bytes == 0) return;
    check(this->_api->clEnqueueWriteBuffer(this->_queue, reinterpret_cast<void*>(dst), CL_TRUE, offset, bytes, src, 0, nullptr, nullptr), "clEnqueueWriteBuffer", utils::exception::InternalCode::GpuTransfer);
}

_hot void utils::system::OpenCLGpu::fetch(void* dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t offset) const
{
    if (bytes == 0) return;
    check(this->_api->clEnqueueReadBuffer(this->_queue, reinterpret_cast<void*>(src), CL_TRUE, offset, bytes, dst, 0, nullptr, nullptr), "clEnqueueReadBuffer", utils::exception::InternalCode::GpuTransfer);
}

_hot void utils::system::OpenCLGpu::memset(const utils::system::GpuBuffer buffer, const std::byte value, const std::size_t bytes, const std::size_t offset)
{
    if (bytes == 0) return;
    check(this->_api->clEnqueueFillBuffer(this->_queue, reinterpret_cast<void*>(buffer), &value, sizeof(value), offset, bytes, 0, nullptr, nullptr), "clEnqueueFillBuffer", utils::exception::InternalCode::GpuTransfer);
}

_hot void utils::system::OpenCLGpu::memcpy(const utils::system::GpuBuffer dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t dstOffset, const std::size_t srcOffset)
{
    if (bytes == 0) return;
    check(this->_api->clEnqueueCopyBuffer(this->_queue, reinterpret_cast<void*>(src), reinterpret_cast<void*>(dst), srcOffset, dstOffset, bytes, 0, nullptr, nullptr), "clEnqueueCopyBuffer", utils::exception::InternalCode::GpuTransfer);
}

_hot void utils::system::OpenCLGpu::join(void)
{
    if (!this->_available) return;
    check(this->_api->clFinish(this->_queue), "clFinish", utils::exception::InternalCode::GpuTransfer);
}
