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
##  Handler used for the C API on c++ exception
\**************************************************************/

#ifndef EXCEPTION_CAPI_H
    #define EXCEPTION_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _cold, _hot, _nodiscard, _unlikely
    #include "basic/ErrorException.hpp"     // utils::exception::ErrorException
    #include "ExceptionDefine.hpp"          // utils::exception::* (define / vars)
    #include "IException.hpp"               // utils::exception::IException
    #include "CApi.h"                       // UTILS_OK, UTILS_KO, utils_exception, utils_exception_status_t, utils_exception_origin_t, C_API_*
    #include <source_location>              // std::source_location
    #include <type_traits>                  // std::invoke_result_t, std::type_identity_t
    #include <algorithm>                    // std::min
    #include <exception>                    // std::exception
    #include <cstddef>                      // std::size_t
    #include <cstdint>                      // std::uint8_t
    #include <cstring>                      // std::memcpy
    #include <cstdio>                       // std::snprintf
    #include <string>                       // std::string

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ exception type must stay the same */
static_assert(C_API_EXCEPTION_TYPE_NONE    == static_cast<std::uint8_t>(utils::exception::Type::None));
static_assert(C_API_EXCEPTION_TYPE_FATAL   == static_cast<std::uint8_t>(utils::exception::Type::Fatal));
static_assert(C_API_EXCEPTION_TYPE_ERROR   == static_cast<std::uint8_t>(utils::exception::Type::Error));
static_assert(C_API_EXCEPTION_TYPE_WARNING == static_cast<std::uint8_t>(utils::exception::Type::Warning));
static_assert(sizeof(utils_exception_type_t) == sizeof(utils::exception::Type));

/* C only type, can't collide with a c++ one */
static_assert((static_cast<std::uint8_t>(C_API_EXCEPTION_TYPE_SYSTEM) & (C_API_EXCEPTION_TYPE_NONE | C_API_EXCEPTION_TYPE_FATAL | C_API_EXCEPTION_TYPE_ERROR | C_API_EXCEPTION_TYPE_WARNING)) == 0);

namespace utils::exception::c_api { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* status (utils_exception of the actual thread) */
_hot void clear_status(void) noexcept;
_cold void set_status(const utils::exception::IException& e) noexcept; // libutils exception (origin internal / external)
_cold void set_status(const char* what, const utils_exception_origin_t origin) noexcept; // other exception (C_API_OTHER_EXCEPTION_CODE)

//----------------------------------------------------------------//
/* TOOLS */

// Check a pointer given by the C side (NULL: InvalidArgument "The <name> is NULL", emplacement of the caller)
template<typename T>
_hot _nodiscard T* not_null(T* ptr, const char* name, const std::source_location loc = std::source_location::current())
{
    if (!ptr) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, std::string("The ") + name + " is NULL", loc);
    }
    return ptr;
}

// Same as not_null, returns a reference (handle of an opaque type of the C API, ...)
template<typename T>
_hot _nodiscard T& not_null_ref(T* ptr, const char* name, const std::source_location loc = std::source_location::current())
{
    return *utils::exception::c_api::not_null(ptr, name, loc);
}

// Copy in a fixed size char array (strings of utils_exception, ...), truncated with '...' on the last 3 char, always '\0' terminated
template<std::size_t N>
_cold void copy(char (&dst)[N], const char* src) noexcept
{
    static_assert(N > 3, "The buffer must be able to hold at least the '...' of a truncation");

    // Truncate to the buffer size
    const int len = std::snprintf(dst, N, "%s", (src ? src : ""));

    // Mark the truncation with '...' on the last 3 char
    if (len >= static_cast<int>(N)) {
        dst[N - 4] = '.';
        dst[N - 3] = '.';
        dst[N - 2] = '.';
    }
}

// Copy in a buffer of the user (C API _r functions), like snprintf (truncated, '\0' terminated), returns the full length
_cold inline std::size_t copy_r(const std::string& str, char* buf, const std::size_t size)
{
    if (size > 0) {
        const std::size_t len = std::min(str.size(), size - 1);
        std::memcpy(utils::exception::c_api::not_null(buf, "buffer"), str.data(), len);
        buf[len] = '\0';
    }
    return str.size();
}

// Only reset the error flag (before a C callback: the status then only holds its own error, utils_fail / utils_exception_set)
_hot inline void reset_status(void) noexcept {utils_exception.exception = false;};

// Run f, never let an exception cross the C boundary, fill utils_exception on failure (only utils_exception.exception = false on success), returns UTILS_OK / UTILS_KO
template<typename F>
_hot _nodiscard int guard(F&& f) noexcept
{
    try {
        f();
        utils_exception.exception = false;
        return UTILS_OK;
    } catch (const utils::exception::IException& e) {
        utils::exception::c_api::set_status(e);
    } catch (const std::exception& e) {
        utils::exception::c_api::set_status(e.what(), C_API_EXCEPTION_ORIGIN_STD);
    } catch (...) {
        utils::exception::c_api::set_status("Unknown exception", C_API_EXCEPTION_ORIGIN_UNKNOWN);
    }
    return UTILS_KO;
}

// Run f and return its result, fallback when it threw (status filled like guard)
template<typename F>
_hot _nodiscard std::invoke_result_t<F> guard_fallback(const std::type_identity_t<std::invoke_result_t<F>> fallback, F&& f) noexcept
{
    std::invoke_result_t<F> res = fallback;
    if (utils::exception::c_api::guard([&](void) {res = f();}) != UTILS_OK) _unlikely {return fallback;}
    return res;
}

} // namespace end
#endif /* EXCEPTION_CAPI_H */
