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
##  C API of utils::pool: clusters & middlewares (C & C++ compatible)
##  The templates are given for the fixed types of type/CApiTypes.h & as bytes (generic)
##  Every function that can fail returns UTILS_OK / UTILS_KO (or NULL / 0), details in utils_exception
##  Every callback returns UTILS_OK or UTILS_KO (info: utils_fail / utils_exception_set)
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef POOL_CAPI_C_H
    #define POOL_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _nodiscard, _nothrow, _read_only
    #include "../exception/CApi.h"          // utils_exception, utils_fail, UTILS_OK, UTILS_KO
    #include "../type/CApiTypes.h"          // C_API_*_TYPES
    #include <stdbool.h>                    // bool
    #include <stddef.h>                     // size_t

    //----------------------------------------------------------------//
    /* MACRO */

    /* cluster of a fixed type (utils::pool::Cluster<type>, the elements can be changed in apply) */
    #define C_API_CLUSTER_DECL(name, ctype) \
        typedef struct utils_cluster_##name utils_cluster_##name##_t; \
        typedef int (*utils_cluster_##name##_fn_t)(ctype* element, void* data); \
        _nodiscard _nothrow utils_cluster_##name##_t* utils_cluster_##name##_create(void); /* NULL on error */ \
        _nothrow void utils_cluster_##name##_destroy(utils_cluster_##name##_t* cluster); /* NULL: nothing */ \
        _nodiscard _nothrow int utils_cluster_##name##_spawn(utils_cluster_##name##_t* cluster, size_t n, ctype value); /* n new elements set to value */ \
        _nodiscard _nothrow int utils_cluster_##name##_kill(utils_cluster_##name##_t* cluster); /* every element */ \
        _nodiscard _nothrow int utils_cluster_##name##_kill_n(utils_cluster_##name##_t* cluster, size_t n); /* the n last ones */ \
        _nodiscard _nothrow size_t utils_cluster_##name##_size(const utils_cluster_##name##_t* cluster); \
        _nodiscard _nothrow int utils_cluster_##name##_apply(utils_cluster_##name##_t* cluster, utils_cluster_##name##_fn_t fn, void* data); /* fn on each element (stops at the first error) */

    /* middlewares of a fixed type (utils::pool::Middlewares<type, type>, called in the order of addition) */
    #define C_API_MIDDLEWARES_DECL(name, ctype) \
        typedef struct utils_middlewares_##name utils_middlewares_##name##_t; \
        typedef int (*utils_middleware_##name##_t)(ctype arg, void* data); \
        _nodiscard _nothrow utils_middlewares_##name##_t* utils_middlewares_##name##_create(void); /* NULL on error */ \
        _nothrow void utils_middlewares_##name##_destroy(utils_middlewares_##name##_t* middlewares); /* NULL: nothing */ \
        _nodiscard _nothrow int utils_middlewares_##name##_clear(utils_middlewares_##name##_t* middlewares); \
        _nodiscard _nothrow int utils_middlewares_##name##_add_before(utils_middlewares_##name##_t* middlewares, utils_middleware_##name##_t fn, void* data); \
        _nodiscard _nothrow int utils_middlewares_##name##_add_after(utils_middlewares_##name##_t* middlewares, utils_middleware_##name##_t fn, void* data); \
        _nodiscard _nothrow int utils_middlewares_##name##_call_before(const utils_middlewares_##name##_t* middlewares, ctype arg); /* error: MiddlewareCall with the info of the callback */ \
        _nodiscard _nothrow int utils_middlewares_##name##_call_after(const utils_middlewares_##name##_t* middlewares, ctype arg); \
        _nodiscard _nothrow size_t utils_middlewares_##name##_before_count(const utils_middlewares_##name##_t* middlewares); \
        _nodiscard _nothrow size_t utils_middlewares_##name##_after_count(const utils_middlewares_##name##_t* middlewares);

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* opaque types */
typedef struct utils_cluster utils_cluster_t; // utils::pool::Cluster of bytes (an element = data + size)
typedef struct utils_middlewares utils_middlewares_t; // utils::pool::Middlewares<void, void>
typedef struct utils_middlewares_bytes utils_middlewares_bytes_t; // utils::pool::Middlewares of bytes (an argument = data + size)

/* callbacks */
typedef int (*utils_cluster_fn_t)(void* element, size_t size, void* data); // the bytes of the element can be changed (not its size)
typedef int (*utils_middleware_t)(void* data);
typedef int (*utils_middleware_bytes_t)(const void* arg, size_t size, void* data);

//----------------------------------------------------------------//
/* PROTOTYPE */

/* cluster of bytes */
_nodiscard _nothrow utils_cluster_t* utils_cluster_create(void); // NULL on error
_nothrow void utils_cluster_destroy(utils_cluster_t* cluster); // NULL: nothing
_nodiscard _nothrow _read_only(3, 4) int utils_cluster_spawn(utils_cluster_t* cluster, size_t n, const void* value, size_t size); // n new elements: a copy of value (size bytes)
_nodiscard _nothrow int utils_cluster_kill(utils_cluster_t* cluster); // every element
_nodiscard _nothrow int utils_cluster_kill_n(utils_cluster_t* cluster, size_t n); // the n last ones
_nodiscard _nothrow size_t utils_cluster_size(const utils_cluster_t* cluster);
_nodiscard _nothrow int utils_cluster_apply(utils_cluster_t* cluster, utils_cluster_fn_t fn, void* data); // fn on each element (stops at the first error)

/* clusters of the fixed types (bool: no c++ cluster, std::vector<bool> can't give a bool&) */
C_API_NUMERIC_TYPES(C_API_CLUSTER_DECL)
C_API_CLUSTER_DECL(char, char)

/* middlewares without argument */
_nodiscard _nothrow utils_middlewares_t* utils_middlewares_create(void); // NULL on error
_nothrow void utils_middlewares_destroy(utils_middlewares_t* middlewares); // NULL: nothing
_nodiscard _nothrow int utils_middlewares_clear(utils_middlewares_t* middlewares);
_nodiscard _nothrow int utils_middlewares_add_before(utils_middlewares_t* middlewares, utils_middleware_t fn, void* data);
_nodiscard _nothrow int utils_middlewares_add_after(utils_middlewares_t* middlewares, utils_middleware_t fn, void* data);
_nodiscard _nothrow int utils_middlewares_call_before(const utils_middlewares_t* middlewares); // error: MiddlewareCall with the info of the callback
_nodiscard _nothrow int utils_middlewares_call_after(const utils_middlewares_t* middlewares);
_nodiscard _nothrow size_t utils_middlewares_before_count(const utils_middlewares_t* middlewares);
_nodiscard _nothrow size_t utils_middlewares_after_count(const utils_middlewares_t* middlewares);

/* middlewares of bytes */
_nodiscard _nothrow utils_middlewares_bytes_t* utils_middlewares_bytes_create(void); // NULL on error
_nothrow void utils_middlewares_bytes_destroy(utils_middlewares_bytes_t* middlewares); // NULL: nothing
_nodiscard _nothrow int utils_middlewares_bytes_clear(utils_middlewares_bytes_t* middlewares);
_nodiscard _nothrow int utils_middlewares_bytes_add_before(utils_middlewares_bytes_t* middlewares, utils_middleware_bytes_t fn, void* data);
_nodiscard _nothrow int utils_middlewares_bytes_add_after(utils_middlewares_bytes_t* middlewares, utils_middleware_bytes_t fn, void* data);
_nodiscard _nothrow _read_only(2, 3) int utils_middlewares_bytes_call_before(const utils_middlewares_bytes_t* middlewares, const void* arg, size_t size);
_nodiscard _nothrow _read_only(2, 3) int utils_middlewares_bytes_call_after(const utils_middlewares_bytes_t* middlewares, const void* arg, size_t size);
_nodiscard _nothrow size_t utils_middlewares_bytes_before_count(const utils_middlewares_bytes_t* middlewares);
_nodiscard _nothrow size_t utils_middlewares_bytes_after_count(const utils_middlewares_bytes_t* middlewares);

/* middlewares of the fixed types (the same type before & after, str: const char*) */
C_API_TYPES(C_API_MIDDLEWARES_DECL)

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* POOL_CAPI_C_H */
