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
##  @file VulkanGpu.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/SharedObject.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/IException.hpp"
#include "utils/system/gpu/VulkanGpu.hpp"
#include "utils/attribute/Attribute.hpp"
#include <functional>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <array>
#include <mutex>

//----------------------------------------------------------------//
/* DEFINE */

/* structure types */
#define VK_STRUCTURE_TYPE_APPLICATION_INFO 0
#define VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO 1
#define VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO 2
#define VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO 3
#define VK_STRUCTURE_TYPE_SUBMIT_INFO 4
#define VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO 5
#define VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO 12
#define VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO 39
#define VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO 40
#define VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO 42

/* flags */
#define VK_API_VERSION_1_0 (1u << 22)
#define VK_QUEUE_TRANSFER_CAPABLE 0x7u // graphics | compute | transfer: every one can copy
#define VK_BUFFER_USAGE_TRANSFER_SRC 0x1u
#define VK_BUFFER_USAGE_TRANSFER_DST 0x2u
#define VK_BUFFER_USAGE_STORAGE 0x20u
#define VK_MEMORY_DEVICE_LOCAL 0x1u
#define VK_MEMORY_HOST_VISIBLE 0x2u
#define VK_MEMORY_HOST_COHERENT 0x4u
#define VK_COMMAND_POOL_RESET_BUFFER 0x2u
#define VK_COMMAND_BUFFER_ONE_TIME 0x1u
#define VK_DEVICE_TYPE_INTEGRATED 1u
#define VK_DEVICE_TYPE_DISCRETE 2u
#define VK_DEVICE_TYPE_VIRTUAL 3u

// Structures (64 bits layout of vulkan_core.h) & functions of the Vulkan 1.0 API (handles: dispatchable = void*, others = std::uint64_t)
struct utils::system::VulkanGpu::Api {
    struct ApplicationInfo {
        std::int32_t sType;
        const void* pNext;
        const char* pApplicationName;
        std::uint32_t applicationVersion;
        const char* pEngineName;
        std::uint32_t engineVersion;
        std::uint32_t apiVersion;
    };
    struct InstanceCreateInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint32_t flags;
        const ApplicationInfo* pApplicationInfo;
        std::uint32_t enabledLayerCount;
        const char* const* ppEnabledLayerNames;
        std::uint32_t enabledExtensionCount;
        const char* const* ppEnabledExtensionNames;
    };
    struct QueueFamilyProperties {
        std::uint32_t queueFlags;
        std::uint32_t queueCount;
        std::uint32_t timestampValidBits;
        std::uint32_t minImageTransferGranularity[3];
    };
    struct MemoryType {
        std::uint32_t propertyFlags;
        std::uint32_t heapIndex;
    };
    struct MemoryHeap {
        std::uint64_t size;
        std::uint32_t flags;
    };
    struct MemoryProperties {
        std::uint32_t memoryTypeCount;
        MemoryType memoryTypes[32];
        std::uint32_t memoryHeapCount;
        MemoryHeap memoryHeaps[16];
    };
    struct DeviceQueueCreateInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint32_t flags;
        std::uint32_t queueFamilyIndex;
        std::uint32_t queueCount;
        const float* pQueuePriorities;
    };
    struct DeviceCreateInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint32_t flags;
        std::uint32_t queueCreateInfoCount;
        const DeviceQueueCreateInfo* pQueueCreateInfos;
        std::uint32_t enabledLayerCount;
        const char* const* ppEnabledLayerNames;
        std::uint32_t enabledExtensionCount;
        const char* const* ppEnabledExtensionNames;
        const void* pEnabledFeatures;
    };
    struct BufferCreateInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint32_t flags;
        std::uint64_t size;
        std::uint32_t usage;
        std::uint32_t sharingMode;
        std::uint32_t queueFamilyIndexCount;
        const std::uint32_t* pQueueFamilyIndices;
    };
    struct MemoryRequirements {
        std::uint64_t size;
        std::uint64_t alignment;
        std::uint32_t memoryTypeBits;
    };
    struct MemoryAllocateInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint64_t allocationSize;
        std::uint32_t memoryTypeIndex;
    };
    struct CommandPoolCreateInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint32_t flags;
        std::uint32_t queueFamilyIndex;
    };
    struct CommandBufferAllocateInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint64_t commandPool;
        std::uint32_t level;
        std::uint32_t commandBufferCount;
    };
    struct CommandBufferBeginInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint32_t flags;
        const void* pInheritanceInfo;
    };
    struct BufferCopy {
        std::uint64_t srcOffset;
        std::uint64_t dstOffset;
        std::uint64_t size;
    };
    struct SubmitInfo {
        std::int32_t sType;
        const void* pNext;
        std::uint32_t waitSemaphoreCount;
        const std::uint64_t* pWaitSemaphores;
        const std::uint32_t* pWaitDstStageMask;
        std::uint32_t commandBufferCount;
        void* const* pCommandBuffers;
        std::uint32_t signalSemaphoreCount;
        const std::uint64_t* pSignalSemaphores;
    };

    // A buffer given as GpuBuffer (VkBuffer & VkDeviceMemory)
    struct Allocation {
        std::uint64_t buffer = 0;
        std::uint64_t memory = 0;
    };

    /* loader & instance */
    void* (GPU_API_CALL* vkGetInstanceProcAddr)(void*, const char*) = nullptr;
    std::int32_t (GPU_API_CALL* vkCreateInstance)(const InstanceCreateInfo*, const void*, void**) = nullptr;
    void (GPU_API_CALL* vkDestroyInstance)(void*, const void*) = nullptr;
    std::int32_t (GPU_API_CALL* vkEnumeratePhysicalDevices)(void*, std::uint32_t*, void**) = nullptr;
    void (GPU_API_CALL* vkGetPhysicalDeviceProperties)(void*, void*) = nullptr;
    void (GPU_API_CALL* vkGetPhysicalDeviceMemoryProperties)(void*, MemoryProperties*) = nullptr;
    void (GPU_API_CALL* vkGetPhysicalDeviceQueueFamilyProperties)(void*, std::uint32_t*, QueueFamilyProperties*) = nullptr;
    std::int32_t (GPU_API_CALL* vkCreateDevice)(void*, const DeviceCreateInfo*, const void*, void**) = nullptr;
    void* (GPU_API_CALL* vkGetDeviceProcAddr)(void*, const char*) = nullptr;

    /* device */
    void (GPU_API_CALL* vkDestroyDevice)(void*, const void*) = nullptr;
    void (GPU_API_CALL* vkGetDeviceQueue)(void*, std::uint32_t, std::uint32_t, void**) = nullptr;
    std::int32_t (GPU_API_CALL* vkDeviceWaitIdle)(void*) = nullptr;
    std::int32_t (GPU_API_CALL* vkCreateBuffer)(void*, const BufferCreateInfo*, const void*, std::uint64_t*) = nullptr;
    void (GPU_API_CALL* vkDestroyBuffer)(void*, std::uint64_t, const void*) = nullptr;
    void (GPU_API_CALL* vkGetBufferMemoryRequirements)(void*, std::uint64_t, MemoryRequirements*) = nullptr;
    std::int32_t (GPU_API_CALL* vkAllocateMemory)(void*, const MemoryAllocateInfo*, const void*, std::uint64_t*) = nullptr;
    void (GPU_API_CALL* vkFreeMemory)(void*, std::uint64_t, const void*) = nullptr;
    std::int32_t (GPU_API_CALL* vkBindBufferMemory)(void*, std::uint64_t, std::uint64_t, std::uint64_t) = nullptr;
    std::int32_t (GPU_API_CALL* vkMapMemory)(void*, std::uint64_t, std::uint64_t, std::uint64_t, std::uint32_t, void**) = nullptr;
    void (GPU_API_CALL* vkUnmapMemory)(void*, std::uint64_t) = nullptr;
    std::int32_t (GPU_API_CALL* vkCreateCommandPool)(void*, const CommandPoolCreateInfo*, const void*, std::uint64_t*) = nullptr;
    void (GPU_API_CALL* vkDestroyCommandPool)(void*, std::uint64_t, const void*) = nullptr;
    std::int32_t (GPU_API_CALL* vkAllocateCommandBuffers)(void*, const CommandBufferAllocateInfo*, void**) = nullptr;
    void (GPU_API_CALL* vkFreeCommandBuffers)(void*, std::uint64_t, std::uint32_t, void* const*) = nullptr;
    std::int32_t (GPU_API_CALL* vkBeginCommandBuffer)(void*, const CommandBufferBeginInfo*) = nullptr;
    std::int32_t (GPU_API_CALL* vkEndCommandBuffer)(void*) = nullptr;
    void (GPU_API_CALL* vkCmdCopyBuffer)(void*, std::uint64_t, std::uint64_t, std::uint32_t, const BufferCopy*) = nullptr;
    void (GPU_API_CALL* vkCmdFillBuffer)(void*, std::uint64_t, std::uint64_t, std::uint64_t, std::uint32_t) = nullptr;
    std::int32_t (GPU_API_CALL* vkQueueSubmit)(void*, std::uint32_t, const SubmitInfo*, std::uint64_t) = nullptr;
    std::int32_t (GPU_API_CALL* vkQueueWaitIdle)(void*) = nullptr;

    MemoryProperties memory = {}; // memory types of the device
    std::uint32_t staging = 0; // index of the host visible & coherent memory type
    std::mutex lock; // the queue & the command pool are not thread safe

    /* tools */
    _cold _nodiscard std::int64_t findMemoryType(const std::uint32_t bits, const std::uint32_t flags) const; // index of a type allowed by bits with every flag (-1 = none)
    _hot _nodiscard Allocation createBuffer(void* device, const std::uint64_t size, const std::uint32_t usage, const std::uint32_t flags) const; // buffer bound to a new memory (GpuAllocation)
    _hot void destroyBuffer(void* device, const Allocation& allocation) const noexcept; // release a buffer & its memory
};

// Throw the code with the failed call when a Vulkan call doesn't return VK_SUCCESS (0)
_cold static inline void check(const std::int32_t result, const char* call, const utils::exception::InternalCode code)
{
    if (result != 0) _unlikely {
        throw utils::exception::ErrorException(code, std::string(call) + ": Vulkan error " + std::to_string(result));
    }
}

_cold std::int64_t utils::system::VulkanGpu::Api::findMemoryType(const std::uint32_t bits, const std::uint32_t flags) const
{
    for (std::uint32_t i = 0; i < this->memory.memoryTypeCount; ++i) {
        if ((bits & (1u << i)) && (this->memory.memoryTypes[i].propertyFlags & flags) == flags) return i;
    }
    return -1;
}

_hot utils::system::VulkanGpu::Api::Allocation utils::system::VulkanGpu::Api::createBuffer(void* device, const std::uint64_t size, const std::uint32_t usage, const std::uint32_t flags) const
{
    utils::system::VulkanGpu::Api::BufferCreateInfo info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, size, usage, 0, 0, nullptr};
    utils::system::VulkanGpu::Api::MemoryRequirements requirements = {};
    utils::system::VulkanGpu::Api::Allocation allocation = {};
    std::int64_t type = -1;

    check(this->vkCreateBuffer(device, &info, nullptr, &allocation.buffer), "vkCreateBuffer", utils::exception::InternalCode::GpuAllocation);
    try {
        this->vkGetBufferMemoryRequirements(device, allocation.buffer, &requirements);
        type = this->findMemoryType(requirements.memoryTypeBits, flags);
        if (type < 0) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::GpuAllocation, "no memory type for the buffer");

        // Memory of the buffer
        utils::system::VulkanGpu::Api::MemoryAllocateInfo allocate = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, requirements.size, static_cast<std::uint32_t>(type)};
        check(this->vkAllocateMemory(device, &allocate, nullptr, &allocation.memory), "vkAllocateMemory", utils::exception::InternalCode::GpuAllocation);
        check(this->vkBindBufferMemory(device, allocation.buffer, allocation.memory, 0), "vkBindBufferMemory", utils::exception::InternalCode::GpuAllocation);
    } catch (const utils::exception::IException&) {
        this->destroyBuffer(device, allocation);
        throw;
    }
    return allocation;
}

_hot void utils::system::VulkanGpu::Api::destroyBuffer(void* device, const utils::system::VulkanGpu::Api::Allocation& allocation) const noexcept
{
    if (allocation.buffer) this->vkDestroyBuffer(device, allocation.buffer, nullptr);
    if (allocation.memory) this->vkFreeMemory(device, allocation.memory, nullptr);
}

_cold utils::system::VulkanGpu::VulkanGpu()
: _api{std::make_unique<utils::system::VulkanGpu::Api>()}
{
    #ifdef UTILS_WINDOWS
        if (!this->load_({"vulkan-1.dll"})) return;
    #else
        if (!this->load_({"libvulkan.so.1", "libvulkan.so"})) return;
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

_cold utils::system::VulkanGpu::~VulkanGpu() noexcept
{
    if (this->_device) {
        (void)this->_api->vkDeviceWaitIdle(this->_device);
        if (this->_commandPool) this->_api->vkDestroyCommandPool(this->_device, this->_commandPool, nullptr);
        this->_api->vkDestroyDevice(this->_device, nullptr);
    }
    if (this->_instance && this->_api->vkDestroyInstance) this->_api->vkDestroyInstance(this->_instance, nullptr);
}

_cold void utils::system::VulkanGpu::loadSymbols_(void)
{
    utils::system::VulkanGpu::Api::ApplicationInfo application = {VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "libutils", 1, "libutils", 1, VK_API_VERSION_1_0};
    utils::system::VulkanGpu::Api::InstanceCreateInfo info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &application, 0, nullptr, 0, nullptr};
    utils::system::VulkanGpu::Api& api = *this->_api;

    // Only vkGetInstanceProcAddr is taken from the loader, the rest is given by it
    api.vkGetInstanceProcAddr = this->_library->loadFunction<decltype(api.vkGetInstanceProcAddr)>("vkGetInstanceProcAddr");
    api.vkCreateInstance = reinterpret_cast<decltype(api.vkCreateInstance)>(api.vkGetInstanceProcAddr(nullptr, "vkCreateInstance"));
    if (!api.vkCreateInstance) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::Dlsym, "vkCreateInstance");
    check(api.vkCreateInstance(&info, nullptr, &this->_instance), "vkCreateInstance", utils::exception::InternalCode::GpuUnavailable);

    // Functions of the instance
    std::array<std::pair<const char*, void**>, 7> instance = {{
        {"vkDestroyInstance", reinterpret_cast<void**>(&api.vkDestroyInstance)},
        {"vkEnumeratePhysicalDevices", reinterpret_cast<void**>(&api.vkEnumeratePhysicalDevices)},
        {"vkGetPhysicalDeviceProperties", reinterpret_cast<void**>(&api.vkGetPhysicalDeviceProperties)},
        {"vkGetPhysicalDeviceMemoryProperties", reinterpret_cast<void**>(&api.vkGetPhysicalDeviceMemoryProperties)},
        {"vkGetPhysicalDeviceQueueFamilyProperties", reinterpret_cast<void**>(&api.vkGetPhysicalDeviceQueueFamilyProperties)},
        {"vkCreateDevice", reinterpret_cast<void**>(&api.vkCreateDevice)},
        {"vkGetDeviceProcAddr", reinterpret_cast<void**>(&api.vkGetDeviceProcAddr)},
    }};
    for (const auto &[name, function]: instance) {
        *function = api.vkGetInstanceProcAddr(this->_instance, name);
        if (!*function) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::Dlsym, name);
    }
}

_cold void utils::system::VulkanGpu::selectDevice_(void)
{
    utils::system::VulkanGpu::Api& api = *this->_api;
    std::vector<utils::system::VulkanGpu::Api::QueueFamilyProperties> families = {};
    std::vector<void*> devices = {};
    std::uint32_t rank = 0;
    std::uint32_t count = 0;

    check(api.vkEnumeratePhysicalDevices(this->_instance, &count, nullptr), "vkEnumeratePhysicalDevices", utils::exception::InternalCode::GpuUnavailable);
    devices.resize(count);
    check(api.vkEnumeratePhysicalDevices(this->_instance, &count, devices.data()), "vkEnumeratePhysicalDevices", utils::exception::InternalCode::GpuUnavailable);

    // Best gpu: discrete > integrated > virtual (a cpu device is not a gpu)
    for (void* device: devices) {
        alignas(8) std::array<unsigned char, 2048> properties = {}; // VkPhysicalDeviceProperties: deviceType at 16, deviceName[256] at 20
        std::uint32_t type = 0;
        std::uint32_t score = 0;

        api.vkGetPhysicalDeviceProperties(device, properties.data());
        std::memcpy(&type, properties.data() + 16, sizeof(type));
        switch (type) {
            case VK_DEVICE_TYPE_DISCRETE: score = 3;   break;
            case VK_DEVICE_TYPE_INTEGRATED: score = 2; break;
            case VK_DEVICE_TYPE_VIRTUAL: score = 1;    break;
            default: score = 0;                        break;
        }
        if (score <= rank) continue;
        rank = score;
        this->_physical = device;
        this->_name = reinterpret_cast<const char*>(properties.data() + 20);
    }
    if (!this->_physical) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::GpuUnavailable, "no Vulkan gpu device");

    // Memory: device local type for the buffers, host visible & coherent one for the transfers
    api.vkGetPhysicalDeviceMemoryProperties(this->_physical, &api.memory);
    std::int64_t local = api.findMemoryType(~0u, VK_MEMORY_DEVICE_LOCAL);
    std::int64_t staging = api.findMemoryType(~0u, VK_MEMORY_HOST_VISIBLE | VK_MEMORY_HOST_COHERENT);
    if (local < 0 || staging < 0) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::GpuUnavailable, "no device local or host visible memory");
    this->_memoryType = static_cast<std::uint32_t>(local);
    api.staging = static_cast<std::uint32_t>(staging);
    for (std::uint32_t i = 0; i < api.memory.memoryHeapCount; ++i) {
        if (api.memory.memoryHeaps[i].flags & VK_MEMORY_DEVICE_LOCAL) this->_memory = std::max<std::size_t>(this->_memory, api.memory.memoryHeaps[i].size);
    }

    // Queue family able to copy
    api.vkGetPhysicalDeviceQueueFamilyProperties(this->_physical, &count, nullptr);
    families.resize(count);
    api.vkGetPhysicalDeviceQueueFamilyProperties(this->_physical, &count, families.data());
    for (this->_queueFamily = 0; this->_queueFamily < count && !(families[this->_queueFamily].queueFlags & VK_QUEUE_TRANSFER_CAPABLE); ++this->_queueFamily);
    if (this->_queueFamily == count) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::GpuUnavailable, "no Vulkan queue able to copy");

    // Device, its queue & the command pool of the transfers
    const float priority = 1.0f;
    utils::system::VulkanGpu::Api::DeviceQueueCreateInfo queue = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, this->_queueFamily, 1, &priority};
    utils::system::VulkanGpu::Api::DeviceCreateInfo device = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1, &queue, 0, nullptr, 0, nullptr, nullptr};
    check(api.vkCreateDevice(this->_physical, &device, nullptr, &this->_device), "vkCreateDevice", utils::exception::InternalCode::GpuUnavailable);

    std::array<std::pair<const char*, void**>, 21> functions = {{
        {"vkDestroyDevice", reinterpret_cast<void**>(&api.vkDestroyDevice)},
        {"vkGetDeviceQueue", reinterpret_cast<void**>(&api.vkGetDeviceQueue)},
        {"vkDeviceWaitIdle", reinterpret_cast<void**>(&api.vkDeviceWaitIdle)},
        {"vkCreateBuffer", reinterpret_cast<void**>(&api.vkCreateBuffer)},
        {"vkDestroyBuffer", reinterpret_cast<void**>(&api.vkDestroyBuffer)},
        {"vkGetBufferMemoryRequirements", reinterpret_cast<void**>(&api.vkGetBufferMemoryRequirements)},
        {"vkAllocateMemory", reinterpret_cast<void**>(&api.vkAllocateMemory)},
        {"vkFreeMemory", reinterpret_cast<void**>(&api.vkFreeMemory)},
        {"vkBindBufferMemory", reinterpret_cast<void**>(&api.vkBindBufferMemory)},
        {"vkMapMemory", reinterpret_cast<void**>(&api.vkMapMemory)},
        {"vkUnmapMemory", reinterpret_cast<void**>(&api.vkUnmapMemory)},
        {"vkCreateCommandPool", reinterpret_cast<void**>(&api.vkCreateCommandPool)},
        {"vkDestroyCommandPool", reinterpret_cast<void**>(&api.vkDestroyCommandPool)},
        {"vkAllocateCommandBuffers", reinterpret_cast<void**>(&api.vkAllocateCommandBuffers)},
        {"vkFreeCommandBuffers", reinterpret_cast<void**>(&api.vkFreeCommandBuffers)},
        {"vkBeginCommandBuffer", reinterpret_cast<void**>(&api.vkBeginCommandBuffer)},
        {"vkEndCommandBuffer", reinterpret_cast<void**>(&api.vkEndCommandBuffer)},
        {"vkCmdCopyBuffer", reinterpret_cast<void**>(&api.vkCmdCopyBuffer)},
        {"vkCmdFillBuffer", reinterpret_cast<void**>(&api.vkCmdFillBuffer)},
        {"vkQueueSubmit", reinterpret_cast<void**>(&api.vkQueueSubmit)},
        {"vkQueueWaitIdle", reinterpret_cast<void**>(&api.vkQueueWaitIdle)},
    }};
    for (const auto &[name, function]: functions) {
        *function = api.vkGetDeviceProcAddr(this->_device, name);
        if (!*function) _unlikely throw utils::exception::ErrorException(utils::exception::InternalCode::Dlsym, name);
    }
    api.vkGetDeviceQueue(this->_device, this->_queueFamily, 0, &this->_queue);

    utils::system::VulkanGpu::Api::CommandPoolCreateInfo pool = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_RESET_BUFFER, this->_queueFamily};
    check(api.vkCreateCommandPool(this->_device, &pool, nullptr, &this->_commandPool), "vkCreateCommandPool", utils::exception::InternalCode::GpuUnavailable);
}

_hot void utils::system::VulkanGpu::submit_(const std::function<void(void*)>& record) const
{
    utils::system::VulkanGpu::Api& api = *this->_api;
    utils::system::VulkanGpu::Api::CommandBufferAllocateInfo info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, this->_commandPool, 0, 1};
    utils::system::VulkanGpu::Api::CommandBufferBeginInfo begin = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_ONE_TIME, nullptr};
    std::lock_guard lock(api.lock);
    void* command = nullptr;

    check(api.vkAllocateCommandBuffers(this->_device, &info, &command), "vkAllocateCommandBuffers", utils::exception::InternalCode::GpuTransfer);
    try {
        utils::system::VulkanGpu::Api::SubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &command, 0, nullptr};

        // Record, submit & wait (one transfer at a time)
        check(api.vkBeginCommandBuffer(command, &begin), "vkBeginCommandBuffer", utils::exception::InternalCode::GpuTransfer);
        record(command);
        check(api.vkEndCommandBuffer(command), "vkEndCommandBuffer", utils::exception::InternalCode::GpuTransfer);
        check(api.vkQueueSubmit(this->_queue, 1, &submit, 0), "vkQueueSubmit", utils::exception::InternalCode::GpuTransfer);
        check(api.vkQueueWaitIdle(this->_queue), "vkQueueWaitIdle", utils::exception::InternalCode::GpuTransfer);
    } catch (const utils::exception::IException&) {
        api.vkFreeCommandBuffers(this->_device, this->_commandPool, 1, &command);
        throw;
    }
    api.vkFreeCommandBuffers(this->_device, this->_commandPool, 1, &command);
}

_hot utils::system::GpuBuffer utils::system::VulkanGpu::allocate(const std::size_t bytes)
{
    const std::uint32_t usage = VK_BUFFER_USAGE_TRANSFER_SRC | VK_BUFFER_USAGE_TRANSFER_DST | VK_BUFFER_USAGE_STORAGE;

    if (bytes == 0) return 0;
    std::unique_ptr<utils::system::VulkanGpu::Api::Allocation> allocation = std::make_unique<utils::system::VulkanGpu::Api::Allocation>(this->_api->createBuffer(this->_device, bytes, usage, VK_MEMORY_DEVICE_LOCAL));
    return reinterpret_cast<utils::system::GpuBuffer>(allocation.release());
}

_hot void utils::system::VulkanGpu::free(const utils::system::GpuBuffer buffer) noexcept
{
    std::unique_ptr<utils::system::VulkanGpu::Api::Allocation> allocation(reinterpret_cast<utils::system::VulkanGpu::Api::Allocation*>(buffer));

    if (!allocation || !this->_available) return;
    this->_api->destroyBuffer(this->_device, *allocation);
}

_hot void utils::system::VulkanGpu::store(const utils::system::GpuBuffer dst, const void* src, const std::size_t bytes, const std::size_t offset)
{
    const utils::system::VulkanGpu::Api::Allocation& target = *reinterpret_cast<const utils::system::VulkanGpu::Api::Allocation*>(dst);
    utils::system::VulkanGpu::Api& api = *this->_api;
    utils::system::VulkanGpu::Api::Allocation staging = {};
    void* mapped = nullptr;

    if (bytes == 0) return;
    staging = api.createBuffer(this->_device, bytes, VK_BUFFER_USAGE_TRANSFER_SRC, VK_MEMORY_HOST_VISIBLE | VK_MEMORY_HOST_COHERENT);
    try {
        // Host -> staging buffer -> device buffer
        check(api.vkMapMemory(this->_device, staging.memory, 0, bytes, 0, &mapped), "vkMapMemory", utils::exception::InternalCode::GpuTransfer);
        std::memcpy(mapped, src, bytes);
        api.vkUnmapMemory(this->_device, staging.memory);
        utils::system::VulkanGpu::Api::BufferCopy region = {0, offset, bytes};
        this->submit_([&](void* command) {api.vkCmdCopyBuffer(command, staging.buffer, target.buffer, 1, &region);});
    } catch (const utils::exception::IException&) {
        api.destroyBuffer(this->_device, staging);
        throw;
    }
    api.destroyBuffer(this->_device, staging);
}

_hot void utils::system::VulkanGpu::fetch(void* dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t offset) const
{
    const utils::system::VulkanGpu::Api::Allocation& source = *reinterpret_cast<const utils::system::VulkanGpu::Api::Allocation*>(src);
    utils::system::VulkanGpu::Api& api = *this->_api;
    utils::system::VulkanGpu::Api::Allocation staging = {};
    void* mapped = nullptr;

    if (bytes == 0) return;
    staging = api.createBuffer(this->_device, bytes, VK_BUFFER_USAGE_TRANSFER_DST, VK_MEMORY_HOST_VISIBLE | VK_MEMORY_HOST_COHERENT);
    try {
        // Device buffer -> staging buffer -> host
        utils::system::VulkanGpu::Api::BufferCopy region = {offset, 0, bytes};
        this->submit_([&](void* command) {api.vkCmdCopyBuffer(command, source.buffer, staging.buffer, 1, &region);});
        check(api.vkMapMemory(this->_device, staging.memory, 0, bytes, 0, &mapped), "vkMapMemory", utils::exception::InternalCode::GpuTransfer);
        std::memcpy(dst, mapped, bytes);
        api.vkUnmapMemory(this->_device, staging.memory);
    } catch (const utils::exception::IException&) {
        api.destroyBuffer(this->_device, staging);
        throw;
    }
    api.destroyBuffer(this->_device, staging);
}

_hot void utils::system::VulkanGpu::memset(const utils::system::GpuBuffer buffer, const std::byte value, const std::size_t bytes, const std::size_t offset)
{
    const utils::system::VulkanGpu::Api::Allocation& target = *reinterpret_cast<const utils::system::VulkanGpu::Api::Allocation*>(buffer);
    const std::uint32_t pattern = static_cast<std::uint32_t>(value) * 0x01010101u;

    if (bytes == 0) return;

    // vkCmdFillBuffer only takes a range aligned on 4 bytes, else the bytes go through a store
    if (offset % 4 == 0 && bytes % 4 == 0) {
        this->submit_([&](void* command) {this->_api->vkCmdFillBuffer(command, target.buffer, offset, bytes, pattern);});
    } else {
        std::vector<std::byte> bytesValue(bytes, value);
        this->store(buffer, bytesValue.data(), bytes, offset);
    }
}

_hot void utils::system::VulkanGpu::memcpy(const utils::system::GpuBuffer dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t dstOffset, const std::size_t srcOffset)
{
    const utils::system::VulkanGpu::Api::Allocation& target = *reinterpret_cast<const utils::system::VulkanGpu::Api::Allocation*>(dst);
    const utils::system::VulkanGpu::Api::Allocation& source = *reinterpret_cast<const utils::system::VulkanGpu::Api::Allocation*>(src);
    const utils::system::VulkanGpu::Api::BufferCopy region = {srcOffset, dstOffset, bytes};

    if (bytes == 0) return;
    this->submit_([&](void* command) {this->_api->vkCmdCopyBuffer(command, source.buffer, target.buffer, 1, &region);});
}

_hot void utils::system::VulkanGpu::join(void)
{
    if (!this->_available) return;
    std::lock_guard lock(this->_api->lock);
    check(this->_api->vkQueueWaitIdle(this->_queue), "vkQueueWaitIdle", utils::exception::InternalCode::GpuTransfer);
}
