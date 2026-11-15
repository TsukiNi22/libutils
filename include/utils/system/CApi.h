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
##  @file CApi.h

File Description:
##  C API of utils::system: gpu handlers, hybrid memories (gpu / ram), id handlers, load balancer & scheduler (C & C++ compatible)
##  The templates are given for the fixed types of type/CApiTypes.h & as bytes (generic)
##  Every function that can fail returns UTILS_OK / UTILS_KO (or NULL / 0), details in utils_exception
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef SYSTEM_CAPI_C_H
    #define SYSTEM_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/CApi.h"  // utils_exception, utils_fail, UTILS_OK, UTILS_KO
    #include "../type/CApiTypes.h"  // C_API_*_TYPES
    #include "../type/CApi.h"       // utils_worker_t
    #include <stdbool.h>            // bool
    #include <stddef.h>             // size_t
    #include <stdint.h>             // uint8_t, uint64_t, uintptr_t

    //----------------------------------------------------------------//
    /* MACRO */

    /* hybrid memory of a fixed type (utils::system::HybridMemory<type>, size: number of type) */
    #define C_API_HYBRID_MEMORY_DECL(name, ctype) \
        typedef struct utils_hybrid_memory_##name utils_hybrid_memory_##name##_t; \
        utils_hybrid_memory_##name##_t* utils_hybrid_memory_##name##_create(size_t size, utils_memory_mode_t mode); /* size 0: nothing allocated, NULL on error */ \
        utils_hybrid_memory_##name##_t* utils_hybrid_memory_##name##_create_gpu(size_t size, const utils_gpu_t* gpu); /* custom mode with a gpu handler, NULL on error */ \
        void utils_hybrid_memory_##name##_destroy(utils_hybrid_memory_##name##_t* memory); /* NULL: nothing */ \
        int utils_hybrid_memory_##name##_allocate(utils_hybrid_memory_##name##_t* memory, size_t size); /* the previous memory is freed (uninitialized) */ \
        int utils_hybrid_memory_##name##_resize(utils_hybrid_memory_##name##_t* memory, size_t size); /* the first values are kept */ \
        int utils_hybrid_memory_##name##_free(utils_hybrid_memory_##name##_t* memory); /* size 0, mode kept */ \
        int utils_hybrid_memory_##name##_clear(utils_hybrid_memory_##name##_t* memory); /* every byte at 0 */ \
        int utils_hybrid_memory_##name##_store(utils_hybrid_memory_##name##_t* memory, const ctype* values, size_t count, size_t offset); \
        int utils_hybrid_memory_##name##_fetch(const utils_hybrid_memory_##name##_t* memory, ctype* out, size_t count, size_t offset); \
        int utils_hybrid_memory_##name##_get(const utils_hybrid_memory_##name##_t* memory, size_t index, ctype* value); \
        int utils_hybrid_memory_##name##_set(utils_hybrid_memory_##name##_t* memory, size_t index, ctype value); \
        int utils_hybrid_memory_##name##_fill(utils_hybrid_memory_##name##_t* memory, ctype value); \
        int utils_hybrid_memory_##name##_join(utils_hybrid_memory_##name##_t* memory); /* wait for the pending gpu operations */ \
        size_t utils_hybrid_memory_##name##_size(const utils_hybrid_memory_##name##_t* memory); \
        size_t utils_hybrid_memory_##name##_bytes(const utils_hybrid_memory_##name##_t* memory); \
        bool utils_hybrid_memory_##name##_empty(const utils_hybrid_memory_##name##_t* memory); \
        bool utils_hybrid_memory_##name##_is_on_gpu(const utils_hybrid_memory_##name##_t* memory); \
        utils_memory_mode_t utils_hybrid_memory_##name##_get_mode(const utils_hybrid_memory_##name##_t* memory); \
        utils_memory_mode_t utils_hybrid_memory_##name##_get_requested_mode(const utils_hybrid_memory_##name##_t* memory); \
        utils_gpu_t* utils_hybrid_memory_##name##_get_gpu(const utils_hybrid_memory_##name##_t* memory); /* a new handle sharing the gpu (destroy it), NULL in ram or on error */ \
        uintptr_t utils_hybrid_memory_##name##_native(const utils_hybrid_memory_##name##_t* memory); /* device buffer (0 in ram) */ \
        ctype* utils_hybrid_memory_##name##_data(utils_hybrid_memory_##name##_t* memory); /* host memory (NULL on the gpu) */

    /* id handler of an integer (utils::system::IdHandler<type>, 0 is never given) */
    #define C_API_ID_HANDLER_DECL(name, ctype) \
        typedef struct utils_id_handler_##name utils_id_handler_##name##_t; \
        utils_id_handler_##name##_t* utils_id_handler_##name##_create(void); /* NULL on error */ \
        void utils_id_handler_##name##_destroy(utils_id_handler_##name##_t* handler); /* NULL: nothing */ \
        ctype utils_id_handler_##name##_allocate(utils_id_handler_##name##_t* handler, bool safe_mode); /* 0 on error */ \
        int utils_id_handler_##name##_use(utils_id_handler_##name##_t* handler, ctype id, bool safe_mode); /* force an id (refused when it's used) */ \
        int utils_id_handler_##name##_free(utils_id_handler_##name##_t* handler, ctype id, bool safe_mode); \
        int utils_id_handler_##name##_clear(utils_id_handler_##name##_t* handler, bool safe_mode); /* free every id */ \
        ctype utils_id_handler_##name##_id(const utils_id_handler_##name##_t* handler); /* counter */ \
        ctype utils_id_handler_##name##_actual(const utils_id_handler_##name##_t* handler, bool safe_mode); \
        ctype utils_id_handler_##name##_preview(const utils_id_handler_##name##_t* handler, bool safe_mode); /* the next one given by allocate (nothing changed) */

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* opaque types */
typedef struct utils_gpu utils_gpu_t; // std::shared_ptr<utils::system::IGpu> (a backend or a gpu of the C side)
typedef struct utils_hybrid_memory utils_hybrid_memory_t; // utils::system::HybridMemory<std::byte> (bytes)
typedef struct utils_load_balancer utils_load_balancer_t; // utils::system::LoadBalancer of utils_worker_t
typedef struct utils_scheduler utils_scheduler_t; // utils::system::Scheduler

/* callbacks */
typedef void (*utils_scheduler_task_t)(void* data); // task run in its own thread after the delay

//----------------------------------------------------------------//
/* ENUM */

/* where the memory is loaded (utils::system::MemoryMode) */
typedef enum utils_memory_mode {
    C_API_MEMORY_MODE_AUTO = 0, // first gpu found (Cuda -> OpenCL -> Vulkan), else the ram
    C_API_MEMORY_MODE_GPU, // first gpu found, error without one
    C_API_MEMORY_MODE_RAM, // host memory only
    C_API_MEMORY_MODE_CUDA, // forced: NVIDIA CUDA driver
    C_API_MEMORY_MODE_OPENCL, // forced: OpenCL ICD loader
    C_API_MEMORY_MODE_VULKAN, // forced: Vulkan loader
    C_API_MEMORY_MODE_CUSTOM // gpu handler given by the user
} utils_memory_mode_t;

//----------------------------------------------------------------//
/* STRUCT */

/* gpu of the C side (NULL: default, every other callback returns UTILS_OK / UTILS_KO with utils_fail, offsets in bytes) */
typedef struct utils_gpu_callbacks {
    bool (*is_available)(void* data); // NULL: true
    const char* (*get_name)(void* data); // NULL: "[None]"
    size_t (*get_memory)(void* data); // NULL: 0 (unknown)
    uintptr_t (*allocate)(size_t bytes, void* data); // new device buffer, 0 on error (mandatory)
    void (*free)(uintptr_t buffer, void* data); // release a buffer (mandatory)
    int (*store)(uintptr_t dst, const void* src, size_t bytes, size_t offset, void* data); // host -> device (mandatory)
    int (*fetch)(void* dst, uintptr_t src, size_t bytes, size_t offset, void* data); // device -> host (mandatory)
    int (*memset)(uintptr_t buffer, uint8_t value, size_t bytes, size_t offset, void* data); // (mandatory)
    int (*memcpy)(uintptr_t dst, uintptr_t src, size_t bytes, size_t dst_offset, size_t src_offset, void* data); // device -> device (mandatory)
    int (*join)(void* data); // NULL: nothing to wait
} utils_gpu_callbacks_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* gpu (a backend: NULL & utils_exception.exception false when there is no gpu for AUTO / RAM) */
utils_gpu_t* utils_gpu_create(utils_memory_mode_t mode); // NULL on error
utils_gpu_t* utils_gpu_create_custom(const utils_gpu_callbacks_t* callbacks, void* data); // gpu of the C side, NULL on error
void utils_gpu_destroy(utils_gpu_t* gpu); // NULL: nothing (released when the last memory using it is destroyed)
bool utils_gpu_is_available(const utils_gpu_t* gpu);
utils_memory_mode_t utils_gpu_get_mode(const utils_gpu_t* gpu); // C_API_MEMORY_MODE_RAM on error
const char* utils_gpu_get_name(const utils_gpu_t* gpu); // valid until the next call in the thread (NULL on error)
size_t utils_gpu_get_name_r(const utils_gpu_t* gpu, char* buf, size_t size); // copied in buf (truncated, always '\0' terminated), returns the full length like snprintf (0 on error)
size_t utils_gpu_get_memory(const utils_gpu_t* gpu); // in bytes (0: unknown or error)
uintptr_t utils_gpu_allocate(utils_gpu_t* gpu, size_t bytes); // 0 on error
int utils_gpu_free(utils_gpu_t* gpu, uintptr_t buffer);
int utils_gpu_store(utils_gpu_t* gpu, uintptr_t dst, const void* src, size_t bytes, size_t offset);
int utils_gpu_fetch(const utils_gpu_t* gpu, void* dst, uintptr_t src, size_t bytes, size_t offset);
int utils_gpu_memset(utils_gpu_t* gpu, uintptr_t buffer, uint8_t value, size_t bytes, size_t offset);
int utils_gpu_memcpy(utils_gpu_t* gpu, uintptr_t dst, uintptr_t src, size_t bytes, size_t dst_offset, size_t src_offset);
int utils_gpu_join(utils_gpu_t* gpu);

/* hybrid memory of bytes (generic: sizes & offsets in bytes) */
utils_hybrid_memory_t* utils_hybrid_memory_create(size_t bytes, utils_memory_mode_t mode); // NULL on error
utils_hybrid_memory_t* utils_hybrid_memory_create_gpu(size_t bytes, const utils_gpu_t* gpu); // NULL on error
void utils_hybrid_memory_destroy(utils_hybrid_memory_t* memory); // NULL: nothing
int utils_hybrid_memory_allocate(utils_hybrid_memory_t* memory, size_t bytes);
int utils_hybrid_memory_resize(utils_hybrid_memory_t* memory, size_t bytes);
int utils_hybrid_memory_free(utils_hybrid_memory_t* memory);
int utils_hybrid_memory_clear(utils_hybrid_memory_t* memory);
int utils_hybrid_memory_store(utils_hybrid_memory_t* memory, const void* data, size_t bytes, size_t offset);
int utils_hybrid_memory_fetch(const utils_hybrid_memory_t* memory, void* out, size_t bytes, size_t offset);
int utils_hybrid_memory_fill(utils_hybrid_memory_t* memory, uint8_t value);
int utils_hybrid_memory_join(utils_hybrid_memory_t* memory);
size_t utils_hybrid_memory_bytes(const utils_hybrid_memory_t* memory);
bool utils_hybrid_memory_empty(const utils_hybrid_memory_t* memory);
bool utils_hybrid_memory_is_on_gpu(const utils_hybrid_memory_t* memory);
utils_memory_mode_t utils_hybrid_memory_get_mode(const utils_hybrid_memory_t* memory);
utils_memory_mode_t utils_hybrid_memory_get_requested_mode(const utils_hybrid_memory_t* memory);
utils_gpu_t* utils_hybrid_memory_get_gpu(const utils_hybrid_memory_t* memory); // a new handle sharing the gpu (destroy it), NULL in ram or on error
uintptr_t utils_hybrid_memory_native(const utils_hybrid_memory_t* memory); // device buffer (0 in ram)
void* utils_hybrid_memory_data(utils_hybrid_memory_t* memory); // host memory (NULL on the gpu)

/* hybrid memories of the fixed types */
C_API_VALUE_TYPES(C_API_HYBRID_MEMORY_DECL)

/* id handlers of the integers */
C_API_INT_TYPES(C_API_ID_HANDLER_DECL)

/* load balancer (workers: utils_worker_t owned by the balancer, set them as not working with utils_worker_set_working_status when the work is done) */
utils_load_balancer_t* utils_load_balancer_create(size_t limit, uint64_t lifespan_ms); // limit 0: unlimited, lifespan 0: never killed, NULL on error
void utils_load_balancer_destroy(utils_load_balancer_t* balancer); // NULL: nothing (every worker destroyed)
utils_worker_t* utils_load_balancer_get_worker(utils_load_balancer_t* balancer); // wait for a free worker (set as working), NULL on error
int utils_load_balancer_spawn(utils_load_balancer_t* balancer, size_t n); // n new workers
int utils_load_balancer_kill(utils_load_balancer_t* balancer, bool forced); // every not working worker (forced: the working ones too, their pointer become invalid)
int utils_load_balancer_kill_n(utils_load_balancer_t* balancer, size_t n, bool forced); // n workers (the not working ones first)
int utils_load_balancer_set_limit(utils_load_balancer_t* balancer, size_t limit);
int utils_load_balancer_set_lifespan(utils_load_balancer_t* balancer, uint64_t lifespan_ms);
size_t utils_load_balancer_get_limit(const utils_load_balancer_t* balancer);
uint64_t utils_load_balancer_get_lifespan(const utils_load_balancer_t* balancer); // in ms
size_t utils_load_balancer_size(const utils_load_balancer_t* balancer); // number of workers

/* scheduler */
utils_scheduler_t* utils_scheduler_create(void); // NULL on error
void utils_scheduler_destroy(utils_scheduler_t* scheduler); // NULL: nothing (the waiting tasks are canceled)
size_t utils_scheduler_schedule(utils_scheduler_t* scheduler, uint64_t delay_ms, utils_scheduler_task_t task, void* data); // id of the task (0 on error)
int utils_scheduler_cancel(utils_scheduler_t* scheduler, size_t id);
int utils_scheduler_cancel_all(utils_scheduler_t* scheduler);

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* SYSTEM_CAPI_C_H */
