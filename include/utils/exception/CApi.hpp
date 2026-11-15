/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/11/2026 by @author Tsukini

File Name:
##  @file CApi.hpp

File Description:
##  Handler used for the C API on c++ exception
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef EXCEPTION_CAPI_H
    #define EXCEPTION_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _cold, _hot, _nodiscard, _unused, _unlikely, _noreturn
    #include "basic/ErrorException.hpp"     // utils::exception::ErrorException
    #include "ExceptionDefine.hpp"          // utils::exception::* (define / vars)
    #include "IException.hpp"               // utils::exception::IException
    #include "CApi.h"                       // UTILS_OK, UTILS_KO, utils_exception, utils_exception_status_t, utils_exception_origin_t, C_API_*
    #include <source_location>              // std::source_location
    #include <type_traits>                  // std::invoke_result_t, std::type_identity_t
    #include <stdexcept>                    // std::runtime_error
    #include <exception>                    // std::exception
    #include <cstdint>                      // std::uint8_t
    #include <string>                       // std::string

    //----------------------------------------------------------------//
    /* DEFINE */

    /* marks a live handle of the C API ("handle" in ASCII) */
    #define C_API_HANDLE_MAGIC 0x68616E646C65ull

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

//----------------------------------------------------------------//
/* STRUCT */

// Base of every handle of the C API: always the first base (a void* of the C side is a utils_handle*), the magic marks a live handle
struct utils_handle {
    std::uint64_t magic = C_API_HANDLE_MAGIC;

    // ------------ Operator ---------- //
    utils_handle& operator=(_unused const utils_handle& other) {return *this;};
    utils_handle& operator=(_unused utils_handle&& other)      {return *this;};

    // ---------- Constructor --------- //
    utils_handle() = default;
    utils_handle(_unused const utils_handle& other): utils_handle() { /* Nothing */ };
    utils_handle(_unused utils_handle&& other): utils_handle()      { /* Nothing */ };

    // ----------- Destructor --------- //
    virtual ~utils_handle() noexcept {this->magic = 0;};
};

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

// Base T (utils::type::Freezable, utils::type::Worker...) of any handle of the C side: InvalidArgument when it's not a live handle or its class has no T
template<typename T>
_hot _nodiscard T& handle_base(void* handle, const char* name, const std::source_location loc = std::source_location::current())
{
    utils_handle* base = static_cast<utils_handle*>(utils::exception::c_api::not_null(handle, "handle", loc));
    T* object = nullptr;

    if (base->magic != C_API_HANDLE_MAGIC) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The pointer is not a handle of the C API", loc);
    }
    if (!(object = dynamic_cast<T*>(base))) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, std::string("This handle has no ") + name, loc);
    }
    return *object;
}

template<typename T>
_hot _nodiscard const T& handle_base(const void* handle, const char* name, const std::source_location loc = std::source_location::current())
{
    return utils::exception::c_api::handle_base<T>(const_cast<void*>(handle), name, loc);
}

// Error of a C callback (UTILS_KO or an unknown value) as a c++ exception: its info (utils_fail / utils_exception_set) in the message
_cold _noreturn inline void throw_callback_error(void)
{
    throw std::runtime_error(utils_exception.exception ? (utils_exception.info[0] != '\0' ? utils_exception.info : utils_exception.what) : "C callback failed (no info given with utils_fail)");
}

// Check the return of a C callback (called after reset_status): anything else than UTILS_OK throws
_hot inline void check_callback(const int ret) {if (ret != UTILS_OK) _unlikely {utils::exception::c_api::throw_callback_error();}};

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
