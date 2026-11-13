/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 13/11/2026 by @author Tsukini

File Name:
##  @file CApi.hpp

File Description:
##  Handler used for the C API on c++ exception
\**************************************************************/

#ifndef CAPI_H
    #define CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _cold, _hot, _nodiscard
    #include "ExceptionDefine.hpp"          // utils::exception::* (define / vars)
    #include "IException.hpp"               // utils::exception::IException
    #include "CApi.h"                       // utils_exception, utils_exception_status_t, C_API_*, C_API_EXCEPTION_TYPE_*
    #include <exception>                    // std::exception
    #include <cstdint>                      // std::uint8_t

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ exception type must stay the same */
static_assert(C_API_EXCEPTION_TYPE_NONE    == static_cast<std::uint8_t>(utils::exception::Type::None));
static_assert(C_API_EXCEPTION_TYPE_FATAL   == static_cast<std::uint8_t>(utils::exception::Type::Fatal));
static_assert(C_API_EXCEPTION_TYPE_ERROR   == static_cast<std::uint8_t>(utils::exception::Type::Error));
static_assert(C_API_EXCEPTION_TYPE_WARNING == static_cast<std::uint8_t>(utils::exception::Type::Warning));
static_assert(sizeof(utils_exception_type_t) == sizeof(utils::exception::Type));

namespace utils::exception::c_api { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* status (utils_exception of the actual thread) */
_hot void clear_status(void) noexcept;
_cold void set_status(const utils::exception::IException& e) noexcept; // libutils exception
_cold void set_status(const char* what) noexcept; // other exception (C_API_OTHER_EXCEPTION_CODE)

//----------------------------------------------------------------//
/* TOOLS */

// Run f, never let an exception cross the C boundary, fill utils_exception on failure (untouched on success)
template<typename F>
_hot _nodiscard bool guard(F&& f) noexcept
{
    try {
        f();
        return true;
    } catch (const utils::exception::IException& e) {
        utils::exception::c_api::set_status(e);
    } catch (const std::exception& e) {
        utils::exception::c_api::set_status(e.what());
    } catch (...) {
        utils::exception::c_api::set_status("Unknown exception");
    }
    return false;
}

} // namespace end
#endif /* CAPI_H */
