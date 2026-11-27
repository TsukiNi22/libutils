/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.cpp

File Description:
##  C API of utils::system (gpu, hybrid memories, id handlers, load balancer, scheduler)
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/system/HybridMemoryDefine.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/system/HybridMemory.hpp"
#include "utils/system/LoadBalancer.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/manip/smanip/CApi.hpp"
#include "utils/system/IdHandler.hpp"
#include "utils/system/Scheduler.hpp"
#include "utils/system/gpu/IGpu.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/system/CApi.hpp"
#include "utils/type/CApi.hpp"
#include "utils/system/CApi.h"
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <chrono>
#include <string>
#include <span>

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ modes must stay the same */
static_assert(C_API_MEMORY_MODE_AUTO   == static_cast<int>(utils::system::MemoryMode::Auto));
static_assert(C_API_MEMORY_MODE_GPU    == static_cast<int>(utils::system::MemoryMode::Gpu));
static_assert(C_API_MEMORY_MODE_RAM    == static_cast<int>(utils::system::MemoryMode::Ram));
static_assert(C_API_MEMORY_MODE_CUDA   == static_cast<int>(utils::system::MemoryMode::Cuda));
static_assert(C_API_MEMORY_MODE_OPENCL == static_cast<int>(utils::system::MemoryMode::OpenCL));
static_assert(C_API_MEMORY_MODE_VULKAN == static_cast<int>(utils::system::MemoryMode::Vulkan));
static_assert(C_API_MEMORY_MODE_CUSTOM == static_cast<int>(utils::system::MemoryMode::Custom));
static_assert(sizeof(uintptr_t) == sizeof(utils::system::GpuBuffer));

//----------------------------------------------------------------//
/* CLASS */

// Gpu of the C side (callbacks), the error of a callback is thrown with its info
class CGpu final: public utils::system::IGpu {
    private:
        utils_gpu_callbacks_t _callbacks = {};
        void* _data = nullptr;

        // ------------ Function ---------- //
        _hot static inline void check_(const int ret) {utils::exception::c_api::check_callback(ret);};

    public:
        // ------------ Function ---------- //
        /* getter */
        _cold _nodiscard bool isAvailable(void) const final                  {return (this->_callbacks.is_available ? this->_callbacks.is_available(this->_data) : true);};
        _cold _nodiscard utils::system::MemoryMode getMode(void) const final {return utils::system::MemoryMode::Custom;};
        _cold _nodiscard std::size_t getMemory(void) const final             {return (this->_callbacks.get_memory ? this->_callbacks.get_memory(this->_data) : 0);};
        _cold _nodiscard std::string getName(void) const final
        {
            const char* name = (this->_callbacks.get_name ? this->_callbacks.get_name(this->_data) : nullptr);

            return (name ? name : "[None]");
        };

        /* memory */
        _hot _nodiscard utils::system::GpuBuffer allocate(const std::size_t bytes) final
        {
            utils::system::GpuBuffer buffer = 0;

            utils::exception::c_api::reset_status(); // only the error of this callback
            if ((buffer = this->_callbacks.allocate(bytes, this->_data)) == 0) _unlikely {utils::exception::c_api::throw_callback_error();}
            return buffer;
        };
        _hot void free(const utils::system::GpuBuffer buffer) noexcept final {if (buffer != 0) this->_callbacks.free(buffer, this->_data);};

        /* transfer */
        _hot void store(const utils::system::GpuBuffer dst, const void* src, const std::size_t bytes, const std::size_t offset = 0) final
        {utils::exception::c_api::reset_status(); this->check_(this->_callbacks.store(dst, src, bytes, offset, this->_data));};
        _hot void fetch(void* dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t offset = 0) const final
        {utils::exception::c_api::reset_status(); this->check_(this->_callbacks.fetch(dst, src, bytes, offset, this->_data));};
        _hot void memset(const utils::system::GpuBuffer buffer, const std::byte value, const std::size_t bytes, const std::size_t offset = 0) final
        {utils::exception::c_api::reset_status(); this->check_(this->_callbacks.memset(buffer, std::to_integer<std::uint8_t>(value), bytes, offset, this->_data));};
        _hot void memcpy(const utils::system::GpuBuffer dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t dstOffset = 0, const std::size_t srcOffset = 0) final
        {utils::exception::c_api::reset_status(); this->check_(this->_callbacks.memcpy(dst, src, bytes, dstOffset, srcOffset, this->_data));};
        _hot void join(void) final
        {if (this->_callbacks.join) {utils::exception::c_api::reset_status(); this->check_(this->_callbacks.join(this->_data));}};

        // ---------- Constructor --------- //
        CGpu(const utils_gpu_callbacks_t& callbacks, void* data): _callbacks{callbacks}, _data{data} { /* Nothing */ };
};

//----------------------------------------------------------------//
/* TOOLS */

_hot _nodiscard static utils::system::IGpu& gpu_of(const utils_gpu_t* gpu) {return *utils::exception::c_api::not_null_ref(gpu, "gpu").gpu;};

// Mode of the C side (out of the enum: InvalidArgument)
_cold _nodiscard static utils::system::MemoryMode mode_of(const utils_memory_mode_t mode)
{
    if (mode < C_API_MEMORY_MODE_AUTO || mode > C_API_MEMORY_MODE_CUSTOM) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The memory mode is invalid");
    }
    return static_cast<utils::system::MemoryMode>(mode);
}

// Gpu shared in a new handle (nullptr: no gpu)
_cold _nodiscard static utils_gpu_t* gpu_handle(const std::shared_ptr<utils::system::IGpu>& gpu)
{
    if (!gpu) return nullptr;
    return new utils_gpu_t(gpu);
}

// New hybrid memory (size 0: nothing allocated)
template<typename H>
_cold _nodiscard static H* memory_new(const std::size_t size, const utils_memory_mode_t mode)
{
    if (size == 0) return new H(mode_of(mode));
    return new H(size, mode_of(mode));
}

template<typename H>
_hot _nodiscard static H& memory(H* handle) {return utils::exception::c_api::not_null_ref(handle, "memory");};

//----------------------------------------------------------------//
/* GPU */

_cold _nothrow utils_gpu_t* utils_gpu_create(utils_memory_mode_t mode)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_gpu_t* {
        const utils::system::MemoryMode cpp = mode_of(mode);

        if (cpp == utils::system::MemoryMode::Custom) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "A custom gpu is created with utils_gpu_create_custom");
        }
        return gpu_handle(utils::system::make_shared_gpu(cpp));
    });
}

_cold _nothrow utils_gpu_t* utils_gpu_create_custom(const utils_gpu_callbacks_t* callbacks, void* data)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_gpu_t* {
        const utils_gpu_callbacks_t& given = utils::exception::c_api::not_null_ref(callbacks, "callbacks");

        (void)utils::exception::c_api::not_null(given.allocate, "allocate callback");
        (void)utils::exception::c_api::not_null(given.free, "free callback");
        (void)utils::exception::c_api::not_null(given.store, "store callback");
        (void)utils::exception::c_api::not_null(given.fetch, "fetch callback");
        (void)utils::exception::c_api::not_null(given.memset, "memset callback");
        (void)utils::exception::c_api::not_null(given.memcpy, "memcpy callback");
        return new utils_gpu_t(std::make_shared<CGpu>(given, data));
    });
}

_cold _nothrow void utils_gpu_destroy(utils_gpu_t* gpu)
{
    (void)utils::exception::c_api::guard([&](void) {delete gpu;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_gpu_is_available(const utils_gpu_t* gpu)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return gpu_of(gpu).isAvailable();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_memory_mode_t utils_gpu_get_mode(const utils_gpu_t* gpu)
{
    return utils::exception::c_api::guard_fallback(C_API_MEMORY_MODE_RAM, [&](void) -> utils_memory_mode_t {return static_cast<utils_memory_mode_t>(gpu_of(gpu).getMode());});
}

_cold _nothrow const char* utils_gpu_get_name(const utils_gpu_t* gpu)
{
    // xstyle: ignore-next CPP-THIS (false positive: _thread_local is the attribute macro, not a member)
    static _thread_local std::string name;

    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {name = gpu_of(gpu).getName(); return name.c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_gpu_get_name_r(const utils_gpu_t* gpu, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(gpu_of(gpu).getName(), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_gpu_get_memory(const utils_gpu_t* gpu)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return gpu_of(gpu).getMemory();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow uintptr_t utils_gpu_allocate(utils_gpu_t* gpu, size_t bytes)
{
    return utils::exception::c_api::guard_fallback(static_cast<uintptr_t>(0), [&](void) -> uintptr_t {return gpu_of(gpu).allocate(bytes);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_gpu_free(utils_gpu_t* gpu, uintptr_t buffer)
{
    return utils::exception::c_api::guard([&](void) {gpu_of(gpu).free(buffer);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_gpu_store(utils_gpu_t* gpu, uintptr_t dst, const void* src, size_t bytes, size_t offset)
{
    return utils::exception::c_api::guard([&](void) {gpu_of(gpu).store(dst, utils::exception::c_api::not_null(src, "source"), bytes, offset);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_gpu_fetch(const utils_gpu_t* gpu, void* dst, uintptr_t src, size_t bytes, size_t offset)
{
    return utils::exception::c_api::guard([&](void) {gpu_of(gpu).fetch(utils::exception::c_api::not_null(dst, "destination"), src, bytes, offset);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_gpu_memset(utils_gpu_t* gpu, uintptr_t buffer, uint8_t value, size_t bytes, size_t offset)
{
    return utils::exception::c_api::guard([&](void) {gpu_of(gpu).memset(buffer, static_cast<std::byte>(value), bytes, offset);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_gpu_memcpy(utils_gpu_t* gpu, uintptr_t dst, uintptr_t src, size_t bytes, size_t dst_offset, size_t src_offset)
{
    return utils::exception::c_api::guard([&](void) {gpu_of(gpu).memcpy(dst, src, bytes, dst_offset, src_offset);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_gpu_join(utils_gpu_t* gpu)
{
    return utils::exception::c_api::guard([&](void) {gpu_of(gpu).join();});
}

//----------------------------------------------------------------//
/* HYBRID MEMORY (bytes) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_hybrid_memory_t* utils_hybrid_memory_create(size_t bytes, utils_memory_mode_t mode)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_hybrid_memory_t* {return memory_new<utils_hybrid_memory_t>(bytes, mode);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_hybrid_memory_t* utils_hybrid_memory_create_gpu(size_t bytes, const utils_gpu_t* gpu)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_hybrid_memory_t* {return new utils_hybrid_memory_t(bytes, utils::exception::c_api::not_null_ref(gpu, "gpu").gpu);});
}

_cold _nothrow void utils_hybrid_memory_destroy(utils_hybrid_memory_t* m)
{
    (void)utils::exception::c_api::guard([&](void) {delete m;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_hybrid_memory_allocate(utils_hybrid_memory_t* m, size_t bytes)
{
    return utils::exception::c_api::guard([&](void) {memory(m).allocate(bytes);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_hybrid_memory_resize(utils_hybrid_memory_t* m, size_t bytes)
{
    return utils::exception::c_api::guard([&](void) {memory(m).resize(bytes);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_hybrid_memory_free(utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard([&](void) {memory(m).free();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_hybrid_memory_clear(utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard([&](void) {memory(m).clear();});
}

_hot _nothrow int utils_hybrid_memory_store(utils_hybrid_memory_t* m, const void* data, size_t bytes, size_t offset)
{
    return utils::exception::c_api::guard([&](void) {
        if (bytes > 0) (void)utils::exception::c_api::not_null(data, "data");
        memory(m).store(std::span<const std::byte>(static_cast<const std::byte*>(data), bytes), offset);
    });
}

_hot _nothrow int utils_hybrid_memory_fetch(const utils_hybrid_memory_t* m, void* out, size_t bytes, size_t offset)
{
    return utils::exception::c_api::guard([&](void) {
        if (bytes > 0) (void)utils::exception::c_api::not_null(out, "output");
        memory(m).fetch(std::span<std::byte>(static_cast<std::byte*>(out), bytes), offset);
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_hybrid_memory_fill(utils_hybrid_memory_t* m, uint8_t value)
{
    return utils::exception::c_api::guard([&](void) {memory(m).fill(static_cast<std::byte>(value));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_hybrid_memory_join(utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard([&](void) {memory(m).join();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_hybrid_memory_bytes(const utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return memory(m).bytes();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_hybrid_memory_empty(const utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard_fallback(true, [&](void) -> bool {return memory(m).empty();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_hybrid_memory_is_on_gpu(const utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return memory(m).isOnGpu();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_memory_mode_t utils_hybrid_memory_get_mode(const utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard_fallback(C_API_MEMORY_MODE_RAM, [&](void) -> utils_memory_mode_t {return static_cast<utils_memory_mode_t>(memory(m).getMode());});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_memory_mode_t utils_hybrid_memory_get_requested_mode(const utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard_fallback(C_API_MEMORY_MODE_RAM, [&](void) -> utils_memory_mode_t {return static_cast<utils_memory_mode_t>(memory(m).getRequestedMode());});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_gpu_t* utils_hybrid_memory_get_gpu(const utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_gpu_t* {return gpu_handle(memory(m).getGpu());});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow uintptr_t utils_hybrid_memory_native(const utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard_fallback(static_cast<uintptr_t>(0), [&](void) -> uintptr_t {return memory(m).native();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow void* utils_hybrid_memory_data(utils_hybrid_memory_t* m)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> void* {return memory(m).data();});
}

//----------------------------------------------------------------//
/* HYBRID MEMORY (fixed types) / ID HANDLER */

C_API_VALUE_TYPES(C_API_HYBRID_MEMORY_DEF)
C_API_INT_TYPES(C_API_ID_HANDLER_DEF)

//----------------------------------------------------------------//
/* LOAD BALANCER */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_load_balancer_t* utils_load_balancer_create(size_t limit, uint64_t lifespan_ms)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_load_balancer_t* {return new utils_load_balancer_t(limit, std::chrono::milliseconds{lifespan_ms});});
}

_cold _nothrow void utils_load_balancer_destroy(utils_load_balancer_t* balancer)
{
    (void)utils::exception::c_api::guard([&](void) {delete balancer;}); // like free(NULL)
}

_hot _nothrow utils_worker_t* utils_load_balancer_get_worker(utils_load_balancer_t* balancer)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_worker_t* {
        std::future<utils_worker&> worker = utils::exception::c_api::not_null_ref(balancer, "balancer").getWorker();

        return &worker.get();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_load_balancer_spawn(utils_load_balancer_t* balancer, size_t n)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(balancer, "balancer").spawn(n);});
}

_cold _nothrow int utils_load_balancer_kill(utils_load_balancer_t* balancer, bool forced)
{
    return utils::exception::c_api::guard([&](void) {
        utils::system::LoadBalancer<utils_worker>& self = utils::exception::c_api::not_null_ref(balancer, "balancer");

        if (forced) self.kill<true>();
        else self.kill<false>();
    });
}

_cold _nothrow int utils_load_balancer_kill_n(utils_load_balancer_t* balancer, size_t n, bool forced)
{
    return utils::exception::c_api::guard([&](void) {
        utils::system::LoadBalancer<utils_worker>& self = utils::exception::c_api::not_null_ref(balancer, "balancer");

        if (forced) self.kill<true>(n);
        else self.kill<false>(n);
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_load_balancer_set_limit(utils_load_balancer_t* balancer, size_t limit)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(balancer, "balancer").setLimit(limit);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_load_balancer_set_lifespan(utils_load_balancer_t* balancer, uint64_t lifespan_ms)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(balancer, "balancer").setLifespan(std::chrono::milliseconds{lifespan_ms});});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_load_balancer_get_limit(const utils_load_balancer_t* balancer)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(balancer, "balancer").getLimit();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow uint64_t utils_load_balancer_get_lifespan(const utils_load_balancer_t* balancer)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::uint64_t {return static_cast<std::uint64_t>(utils::exception::c_api::not_null_ref(balancer, "balancer").getLifespan().count());});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_load_balancer_size(const utils_load_balancer_t* balancer)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(balancer, "balancer").size();});
}

//----------------------------------------------------------------//
/* SCHEDULER */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_scheduler_t* utils_scheduler_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_scheduler_t* {return new utils_scheduler_t();});
}

_cold _nothrow void utils_scheduler_destroy(utils_scheduler_t* scheduler)
{
    (void)utils::exception::c_api::guard([&](void) {delete scheduler;}); // like free(NULL)
}

_cold _nothrow size_t utils_scheduler_schedule(utils_scheduler_t* scheduler, uint64_t delay_ms, utils_scheduler_task_t task, void* data)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        utils::system::Scheduler& self = utils::exception::c_api::not_null_ref(scheduler, "scheduler");

        (void)utils::exception::c_api::not_null(task, "task");
        return self.schedule(std::chrono::milliseconds{delay_ms}, [task, data](void) {task(data);});
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_scheduler_cancel(utils_scheduler_t* scheduler, size_t id)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(scheduler, "scheduler").cancel(id);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_scheduler_cancel_all(utils_scheduler_t* scheduler)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(scheduler, "scheduler").cancel();});
}
