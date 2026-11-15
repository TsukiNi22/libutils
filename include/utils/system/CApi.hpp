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
##  @file CApi.hpp

File Description:
##  Handles of the C API of utils::system (usable in c++ to get the object)
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef SYSTEM_CAPI_H
    #define SYSTEM_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/CApi.hpp"    // utils_handle
    #include "../type/CApiTypes.hpp"    // C_API_*_TYPES
    #include "../type/CApi.hpp"         // utils_worker
    #include "HybridMemory.hpp"         // utils::system::HybridMemory
    #include "LoadBalancer.hpp"         // utils::system::LoadBalancer
    #include "IdHandler.hpp"            // utils::system::IdHandler
    #include "Scheduler.hpp"            // utils::system::Scheduler
    #include "gpu/IGpu.hpp"             // utils::system::IGpu
    #include "CApi.h"                   // utils_gpu_t, utils_hybrid_memory_*_t, utils_id_handler_*_t, utils_load_balancer_t, utils_scheduler_t
    #include <cstddef>                  // std::byte
    #include <utility>                  // std::move
    #include <chrono>                   // std::chrono::milliseconds
    #include <memory>                   // std::shared_ptr

    //----------------------------------------------------------------//
    /* MACRO */

    /* handle of a hybrid memory (the memory itself) */
    #define C_API_HYBRID_MEMORY_HANDLE(name, ctype) \
        struct utils_hybrid_memory_##name: public utils_handle, public utils::system::HybridMemory<ctype> { \
            using utils::system::HybridMemory<ctype>::HybridMemory; \
        };

    /* handle of an id handler (the handler itself) */
    #define C_API_ID_HANDLER_HANDLE(name, ctype) \
        struct utils_id_handler_##name: public utils_handle, public utils::system::IdHandler<ctype> {};

    /* hybrid memory of a fixed type */
    #define C_API_HYBRID_MEMORY_DEF(name, ctype) \
        utils_hybrid_memory_##name##_t* utils_hybrid_memory_##name##_create(size_t size, utils_memory_mode_t mode) \
        {return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_hybrid_memory_##name##_t* {return memory_new<utils_hybrid_memory_##name##_t>(size, mode);});} \
        utils_hybrid_memory_##name##_t* utils_hybrid_memory_##name##_create_gpu(size_t size, const utils_gpu_t* gpu) \
        {return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_hybrid_memory_##name##_t* {return new utils_hybrid_memory_##name##_t(size, utils::exception::c_api::not_null_ref(gpu, "gpu").gpu);});} \
        void utils_hybrid_memory_##name##_destroy(utils_hybrid_memory_##name##_t* m) {(void)utils::exception::c_api::guard([&](void) {delete m;});}; \
        int utils_hybrid_memory_##name##_allocate(utils_hybrid_memory_##name##_t* m, size_t size) {return utils::exception::c_api::guard([&](void) {memory(m).allocate(size);});}; \
        int utils_hybrid_memory_##name##_resize(utils_hybrid_memory_##name##_t* m, size_t size) {return utils::exception::c_api::guard([&](void) {memory(m).resize(size);});}; \
        int utils_hybrid_memory_##name##_free(utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard([&](void) {memory(m).free();});}; \
        int utils_hybrid_memory_##name##_clear(utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard([&](void) {memory(m).clear();});}; \
        int utils_hybrid_memory_##name##_store(utils_hybrid_memory_##name##_t* m, const ctype* values, size_t count, size_t offset) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                if (count > 0) (void)utils::exception::c_api::not_null(values, "values"); \
                memory(m).store(std::span<const ctype>(values, count), offset); \
            }); \
        } \
        int utils_hybrid_memory_##name##_fetch(const utils_hybrid_memory_##name##_t* m, ctype* out, size_t count, size_t offset) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                if (count > 0) (void)utils::exception::c_api::not_null(out, "output"); \
                memory(m).fetch(std::span<ctype>(out, count), offset); \
            }); \
        } \
        int utils_hybrid_memory_##name##_get(const utils_hybrid_memory_##name##_t* m, size_t index, ctype* value) \
        {return utils::exception::c_api::guard([&](void) {ctype& res = utils::exception::c_api::not_null_ref(value, "value"); res = memory(m).get(index);});} \
        int utils_hybrid_memory_##name##_set(utils_hybrid_memory_##name##_t* m, size_t index, ctype value) {return utils::exception::c_api::guard([&](void) {memory(m).set(index, value);});}; \
        int utils_hybrid_memory_##name##_fill(utils_hybrid_memory_##name##_t* m, ctype value) {return utils::exception::c_api::guard([&](void) {memory(m).fill(value);});}; \
        int utils_hybrid_memory_##name##_join(utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard([&](void) {memory(m).join();});}; \
        size_t utils_hybrid_memory_##name##_size(const utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return memory(m).size();});}; \
        size_t utils_hybrid_memory_##name##_bytes(const utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return memory(m).bytes();});}; \
        bool utils_hybrid_memory_##name##_empty(const utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard_fallback(true, [&](void) -> bool {return memory(m).empty();});}; \
        bool utils_hybrid_memory_##name##_is_on_gpu(const utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return memory(m).isOnGpu();});}; \
        utils_memory_mode_t utils_hybrid_memory_##name##_get_mode(const utils_hybrid_memory_##name##_t* m) \
        {return utils::exception::c_api::guard_fallback(C_API_MEMORY_MODE_RAM, [&](void) -> utils_memory_mode_t {return static_cast<utils_memory_mode_t>(memory(m).getMode());});} \
        utils_memory_mode_t utils_hybrid_memory_##name##_get_requested_mode(const utils_hybrid_memory_##name##_t* m) \
        {return utils::exception::c_api::guard_fallback(C_API_MEMORY_MODE_RAM, [&](void) -> utils_memory_mode_t {return static_cast<utils_memory_mode_t>(memory(m).getRequestedMode());});} \
        utils_gpu_t* utils_hybrid_memory_##name##_get_gpu(const utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_gpu_t* {return gpu_handle(memory(m).getGpu());});}; \
        uintptr_t utils_hybrid_memory_##name##_native(const utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard_fallback(static_cast<uintptr_t>(0), [&](void) -> uintptr_t {return memory(m).native();});}; \
        ctype* utils_hybrid_memory_##name##_data(utils_hybrid_memory_##name##_t* m) {return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> ctype* {return memory(m).data();});}

    /* id handler of an integer */
    #define C_API_ID_HANDLER_DEF(name, ctype) \
        utils_id_handler_##name##_t* utils_id_handler_##name##_create(void) {return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_id_handler_##name##_t* {return new utils_id_handler_##name##_t();});}; \
        void utils_id_handler_##name##_destroy(utils_id_handler_##name##_t* handler) {(void)utils::exception::c_api::guard([&](void) {delete handler;});}; \
        ctype utils_id_handler_##name##_allocate(utils_id_handler_##name##_t* handler, bool safe_mode) \
        {return utils::exception::c_api::guard_fallback(static_cast<ctype>(0), [&](void) -> ctype {return utils::exception::c_api::not_null_ref(handler, "handler").allocate(safe_mode);});} \
        int utils_id_handler_##name##_use(utils_id_handler_##name##_t* handler, ctype id, bool safe_mode) \
        {return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(handler, "handler").use(id, safe_mode);});} \
        int utils_id_handler_##name##_free(utils_id_handler_##name##_t* handler, ctype id, bool safe_mode) \
        {return utils::exception::c_api::guard([&](void) {const ctype given = id; utils::exception::c_api::not_null_ref(handler, "handler").free(given, safe_mode);});} \
        int utils_id_handler_##name##_clear(utils_id_handler_##name##_t* handler, bool safe_mode) \
        {return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(handler, "handler").clear(safe_mode);});} \
        ctype utils_id_handler_##name##_id(const utils_id_handler_##name##_t* handler) \
        {return utils::exception::c_api::guard_fallback(static_cast<ctype>(0), [&](void) -> ctype {return utils::exception::c_api::not_null_ref(handler, "handler").id();});} \
        ctype utils_id_handler_##name##_actual(const utils_id_handler_##name##_t* handler, bool safe_mode) \
        {return utils::exception::c_api::guard_fallback(static_cast<ctype>(0), [&](void) -> ctype {return utils::exception::c_api::not_null_ref(handler, "handler").actual(safe_mode);});} \
        ctype utils_id_handler_##name##_preview(const utils_id_handler_##name##_t* handler, bool safe_mode) \
        {return utils::exception::c_api::guard_fallback(static_cast<ctype>(0), [&](void) -> ctype {return utils::exception::c_api::not_null_ref(handler, "handler").preview(safe_mode);});}

//----------------------------------------------------------------//
/* STRUCT */

/* handles of the C API */
struct utils_gpu: public utils_handle {
    std::shared_ptr<utils::system::IGpu> gpu;

    // ---------- Constructor --------- //
    utils_gpu(std::shared_ptr<utils::system::IGpu> g): gpu{std::move(g)} { /* Nothing */ };
};

struct utils_hybrid_memory: public utils_handle, public utils::system::HybridMemory<std::byte> {
    using utils::system::HybridMemory<std::byte>::HybridMemory;
};

C_API_VALUE_TYPES(C_API_HYBRID_MEMORY_HANDLE)
C_API_INT_TYPES(C_API_ID_HANDLER_HANDLE)

struct utils_load_balancer: public utils_handle, public utils::system::LoadBalancer<utils_worker> {
    // ---------- Constructor --------- //
    utils_load_balancer(const std::size_t limit, const std::chrono::milliseconds lifespan): utils::system::LoadBalancer<utils_worker>(limit, lifespan) { /* Nothing */ };
};

struct utils_scheduler: public utils_handle, public utils::system::Scheduler {};

#endif /* SYSTEM_CAPI_H */
