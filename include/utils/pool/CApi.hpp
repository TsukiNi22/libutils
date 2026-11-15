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
##  Handles of the C API of utils::pool (usable in c++ to get the object)
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef POOL_CAPI_H
    #define POOL_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "middleware/Middlewares.hpp"   // utils::pool::Middlewares
    #include "../exception/CApi.hpp"        // utils_handle
    #include "../type/CApiTypes.hpp"        // utils::type::c_api::cpp_t, C_API_*_TYPES
    #include "Cluster.hpp"                  // utils::pool::Cluster
    #include "CApi.h"                       // utils_cluster_*_t, utils_middlewares_*_t
    #include <string>                       // std::string

    //----------------------------------------------------------------//
    /* MACRO */

    /* handles of a fixed type (the object itself) */
    #define C_API_CLUSTER_HANDLE(name, ctype) \
        struct utils_cluster_##name: public utils_handle, public utils::pool::Cluster<ctype> {};
    #define C_API_MIDDLEWARES_HANDLE(name, ctype) \
        struct utils_middlewares_##name: public utils_handle, public utils::pool::Middlewares<utils::type::c_api::cpp_t<ctype>, utils::type::c_api::cpp_t<ctype>> {};

    /* cluster of a fixed type */
    #define C_API_CLUSTER_DEF(name, ctype) \
        utils_cluster_##name##_t* utils_cluster_##name##_create(void) {return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_cluster_##name##_t* {return new utils_cluster_##name##_t();});}; \
        void utils_cluster_##name##_destroy(utils_cluster_##name##_t* c) {(void)utils::exception::c_api::guard([&](void) {delete c;});}; \
        int utils_cluster_##name##_spawn(utils_cluster_##name##_t* c, size_t n, ctype value) {return utils::exception::c_api::guard([&](void) {cluster(c).spawn(n, value);});}; \
        int utils_cluster_##name##_kill(utils_cluster_##name##_t* c) {return utils::exception::c_api::guard([&](void) {cluster(c).kill();});}; \
        int utils_cluster_##name##_kill_n(utils_cluster_##name##_t* c, size_t n) {return utils::exception::c_api::guard([&](void) {cluster(c).kill(n);});}; \
        size_t utils_cluster_##name##_size(const utils_cluster_##name##_t* c) {return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return cluster(c).size();});}; \
        int utils_cluster_##name##_apply(utils_cluster_##name##_t* c, utils_cluster_##name##_fn_t fn, void* data) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                utils::pool::Cluster<ctype>& self = cluster(c); \
                (void)utils::exception::c_api::not_null(fn, "callback"); \
                self.apply([fn, data](ctype& element) {call([&](void) {return fn(&element, data);});}); \
            }); \
        }

    /* middlewares of a fixed type */
    #define C_API_MIDDLEWARES_DEF(name, ctype) \
        utils_middlewares_##name##_t* utils_middlewares_##name##_create(void) {return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_middlewares_##name##_t* {return new utils_middlewares_##name##_t();});}; \
        void utils_middlewares_##name##_destroy(utils_middlewares_##name##_t* m) {(void)utils::exception::c_api::guard([&](void) {delete m;});}; \
        int utils_middlewares_##name##_clear(utils_middlewares_##name##_t* m) {return utils::exception::c_api::guard([&](void) {middlewares(m).clear();});}; \
        int utils_middlewares_##name##_add_before(utils_middlewares_##name##_t* m, utils_middleware_##name##_t fn, void* data) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                (void)utils::exception::c_api::not_null(fn, "middleware"); \
                middlewares(m).addBefore([fn, data](utils::type::c_api::cpp_t<ctype> arg) {call([&](void) {return fn(utils::type::c_api::Storage<ctype>::c(arg), data);});}); \
            }); \
        } \
        int utils_middlewares_##name##_add_after(utils_middlewares_##name##_t* m, utils_middleware_##name##_t fn, void* data) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                (void)utils::exception::c_api::not_null(fn, "middleware"); \
                middlewares(m).addAfter([fn, data](utils::type::c_api::cpp_t<ctype> arg) {call([&](void) {return fn(utils::type::c_api::Storage<ctype>::c(arg), data);});}); \
            }); \
        } \
        int utils_middlewares_##name##_call_before(const utils_middlewares_##name##_t* m, ctype arg) \
        {return utils::exception::c_api::guard([&](void) {middlewares(m).callBefore(utils::type::c_api::Storage<ctype>::cpp(arg));});} \
        int utils_middlewares_##name##_call_after(const utils_middlewares_##name##_t* m, ctype arg) \
        {return utils::exception::c_api::guard([&](void) {middlewares(m).callAfter(utils::type::c_api::Storage<ctype>::cpp(arg));});} \
        size_t utils_middlewares_##name##_before_count(const utils_middlewares_##name##_t* m) \
        {return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {const utils_middlewares_##name##_t& self = middlewares(m); std::shared_lock lock(self._lock); return self.before.size();});} \
        size_t utils_middlewares_##name##_after_count(const utils_middlewares_##name##_t* m) \
        {return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {const utils_middlewares_##name##_t& self = middlewares(m); std::shared_lock lock(self._lock); return self.after.size();});}

//----------------------------------------------------------------//
/* STRUCT */

/* handles of the C API (the object itself) */
struct utils_cluster: public utils_handle, public utils::pool::Cluster<std::string> {};
struct utils_middlewares: public utils_handle, public utils::pool::Middlewares<void, void> {};
struct utils_middlewares_bytes: public utils_handle, public utils::pool::Middlewares<std::string, std::string> {};

C_API_NUMERIC_TYPES(C_API_CLUSTER_HANDLE)
C_API_CLUSTER_HANDLE(char, char)
C_API_TYPES(C_API_MIDDLEWARES_HANDLE)

#endif /* POOL_CAPI_H */
