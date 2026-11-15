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
##  C API of utils::pool (clusters, middlewares)
\**************************************************************/

#include "utils/pool/middleware/Middlewares.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/manip/smanip/CApi.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/type/CApiTypes.hpp"
#include "utils/pool/Cluster.hpp"
#include "utils/pool/CApi.hpp"
#include "utils/pool/CApi.h"
#include <shared_mutex>
#include <cstddef>
#include <string>

//----------------------------------------------------------------//
/* TOOLS */

template<typename H>
_hot _nodiscard static H& cluster(H* handle) {return utils::exception::c_api::not_null_ref(handle, "cluster");};

template<typename H>
_hot _nodiscard static H& middlewares(H* handle) {return utils::exception::c_api::not_null_ref(handle, "middlewares");};

// Call of a C callback (only its own error, thrown with its info)
template<typename F>
_hot static void call(F&& f)
{
    utils::exception::c_api::reset_status();
    utils::exception::c_api::check_callback(f());
}

//----------------------------------------------------------------//
/* CLUSTER (bytes) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_cluster_t* utils_cluster_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_cluster_t* {return new utils_cluster_t();});
}

_cold void utils_cluster_destroy(utils_cluster_t* c)
{
    (void)utils::exception::c_api::guard([&](void) {delete c;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_cluster_spawn(utils_cluster_t* c, size_t n, const void* value, size_t size)
{
    return utils::exception::c_api::guard([&](void) {cluster(c).spawn(n, utils::smanip::c_api::bytes(value, size));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_cluster_kill(utils_cluster_t* c)
{
    return utils::exception::c_api::guard([&](void) {cluster(c).kill();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_cluster_kill_n(utils_cluster_t* c, size_t n)
{
    return utils::exception::c_api::guard([&](void) {cluster(c).kill(n);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot size_t utils_cluster_size(const utils_cluster_t* c)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return cluster(c).size();});
}

_hot int utils_cluster_apply(utils_cluster_t* c, utils_cluster_fn_t fn, void* data)
{
    return utils::exception::c_api::guard([&](void) {
        utils::pool::Cluster<std::string>& self = cluster(c);

        (void)utils::exception::c_api::not_null(fn, "callback");
        self.apply([fn, data](std::string& element) {call([&](void) {return fn(element.data(), element.size(), data);});});
    });
}

//----------------------------------------------------------------//
/* CLUSTER (fixed types) */

C_API_NUMERIC_TYPES(C_API_CLUSTER_DEF)
C_API_CLUSTER_DEF(char, char)

//----------------------------------------------------------------//
/* MIDDLEWARES (void) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_middlewares_t* utils_middlewares_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_middlewares_t* {return new utils_middlewares_t();});
}

_cold void utils_middlewares_destroy(utils_middlewares_t* m)
{
    (void)utils::exception::c_api::guard([&](void) {delete m;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_middlewares_clear(utils_middlewares_t* m)
{
    return utils::exception::c_api::guard([&](void) {middlewares(m).clear();});
}

_cold int utils_middlewares_add_before(utils_middlewares_t* m, utils_middleware_t fn, void* data)
{
    return utils::exception::c_api::guard([&](void) {
        (void)utils::exception::c_api::not_null(fn, "middleware");
        middlewares(m).addBefore([fn, data](void) {call([&](void) {return fn(data);});});
    });
}

_cold int utils_middlewares_add_after(utils_middlewares_t* m, utils_middleware_t fn, void* data)
{
    return utils::exception::c_api::guard([&](void) {
        (void)utils::exception::c_api::not_null(fn, "middleware");
        middlewares(m).addAfter([fn, data](void) {call([&](void) {return fn(data);});});
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_middlewares_call_before(const utils_middlewares_t* m)
{
    return utils::exception::c_api::guard([&](void) {middlewares(m).callBefore();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_middlewares_call_after(const utils_middlewares_t* m)
{
    return utils::exception::c_api::guard([&](void) {middlewares(m).callAfter();});
}

_cold size_t utils_middlewares_before_count(const utils_middlewares_t* m)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const utils_middlewares_t& self = middlewares(m);
        std::shared_lock lock(self._lock);

        return self.before.size();
    });
}

_cold size_t utils_middlewares_after_count(const utils_middlewares_t* m)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const utils_middlewares_t& self = middlewares(m);
        std::shared_lock lock(self._lock);

        return self.after.size();
    });
}

//----------------------------------------------------------------//
/* MIDDLEWARES (bytes) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_middlewares_bytes_t* utils_middlewares_bytes_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_middlewares_bytes_t* {return new utils_middlewares_bytes_t();});
}

_cold void utils_middlewares_bytes_destroy(utils_middlewares_bytes_t* m)
{
    (void)utils::exception::c_api::guard([&](void) {delete m;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_middlewares_bytes_clear(utils_middlewares_bytes_t* m)
{
    return utils::exception::c_api::guard([&](void) {middlewares(m).clear();});
}

_cold int utils_middlewares_bytes_add_before(utils_middlewares_bytes_t* m, utils_middleware_bytes_t fn, void* data)
{
    return utils::exception::c_api::guard([&](void) {
        (void)utils::exception::c_api::not_null(fn, "middleware");
        middlewares(m).addBefore([fn, data](std::string arg) {call([&](void) {return fn(arg.data(), arg.size(), data);});});
    });
}

_cold int utils_middlewares_bytes_add_after(utils_middlewares_bytes_t* m, utils_middleware_bytes_t fn, void* data)
{
    return utils::exception::c_api::guard([&](void) {
        (void)utils::exception::c_api::not_null(fn, "middleware");
        middlewares(m).addAfter([fn, data](std::string arg) {call([&](void) {return fn(arg.data(), arg.size(), data);});});
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_middlewares_bytes_call_before(const utils_middlewares_bytes_t* m, const void* arg, size_t size)
{
    return utils::exception::c_api::guard([&](void) {middlewares(m).callBefore(utils::smanip::c_api::bytes(arg, size));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_middlewares_bytes_call_after(const utils_middlewares_bytes_t* m, const void* arg, size_t size)
{
    return utils::exception::c_api::guard([&](void) {middlewares(m).callAfter(utils::smanip::c_api::bytes(arg, size));});
}

_cold size_t utils_middlewares_bytes_before_count(const utils_middlewares_bytes_t* m)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const utils_middlewares_bytes_t& self = middlewares(m);
        std::shared_lock lock(self._lock);

        return self.before.size();
    });
}

_cold size_t utils_middlewares_bytes_after_count(const utils_middlewares_bytes_t* m)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const utils_middlewares_bytes_t& self = middlewares(m);
        std::shared_lock lock(self._lock);

        return self.after.size();
    });
}

//----------------------------------------------------------------//
/* MIDDLEWARES (fixed types) */

C_API_TYPES(C_API_MIDDLEWARES_DEF)
