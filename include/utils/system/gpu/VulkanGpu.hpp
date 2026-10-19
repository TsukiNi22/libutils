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
##  @file VulkanGpu.hpp

File Description:
##  Vulkan gpu handler of the HybridMemory
\**************************************************************/

#ifndef VULKANGPU_H
    #define VULKANGPU_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _hot, _cold, _nodiscard
    #include "AGpu.hpp"                         // utils::system::AGpu, utils::system::GpuBuffer
    #include <functional>                       // std::function
    #include <cstddef>                          // std::size_t, std::byte
    #include <cstdint>                          // std::uint32_t, std::uint64_t
    #include <memory>                           // std::unique_ptr

namespace utils::system { // namespace start
//----------------------------------------------------------------//
/* CLASS */

// Any gpu with a Vulkan driver, loader loaded at runtime (buffers in device local memory, transfers through a staging buffer)
class VulkanGpu final: public utils::system::AGpu {
    private:
        struct Api; // functions of the loader & the Vulkan structures (defined in the .cpp)

        std::unique_ptr<Api> _api; // functions loaded from the loader
        void* _instance = nullptr; // VkInstance
        void* _physical = nullptr; // VkPhysicalDevice
        void* _device = nullptr; // VkDevice
        void* _queue = nullptr; // VkQueue (transfer capable)
        std::uint64_t _commandPool = 0; // VkCommandPool of the transfers
        std::uint32_t _queueFamily = 0; // family of the queue
        std::uint32_t _memoryType = 0; // index of the device local memory type

        // ---------- Pre-Function -------- //
        void loadSymbols_(void); // vkGetInstanceProcAddr then the instance / device functions
        void selectDevice_(void); // first discrete (else integrated) device: device, queue, command pool, name & memory
        void submit_(const std::function<void(void*)>& record) const; // record a transfer command (given the VkCommandBuffer), submit it and wait for it

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
        _cold _nodiscard utils::system::MemoryMode getMode(void) const final {return utils::system::MemoryMode::Vulkan;};

        // ------------ Operator ---------- //
        VulkanGpu& operator=(const VulkanGpu& other) = delete;
        VulkanGpu& operator=(VulkanGpu&& other) = delete;

        // ---------- Constructor --------- //
        VulkanGpu(); // load libvulkan.so.1 (vulkan-1.dll on Windows), unavailable without the driver or a device (never throw)
        VulkanGpu(const VulkanGpu& other) = delete;
        VulkanGpu(VulkanGpu&& other) = delete;

        // ----------- Destructor --------- //
        ~VulkanGpu() noexcept final; // release the device objects
};

} // namespace end
#endif /* VULKANGPU_H */
