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
##  Definition of the exception status used by the C API
\**************************************************************/

#include "utils/exception/IException.hpp"
#include "utils/exception/AException.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/manip/smanip/CApi.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/exception/CApi.h"
#include <source_location>
#include <cstddef>
#include <cstdint>
#include <string>

/* storage of the last exception (one per thread) */
extern "C" _thread_local utils_exception_status_t utils_exception = {};

// Formated status (like formated() of the c++ exceptions), empty when there is no exception
_cold static inline std::string format_status(void)
{
    if (!utils_exception.exception) return "";
    return utils::exception::format(static_cast<utils::exception::Type>(utils_exception.type), utils_exception.file, static_cast<std::size_t>(utils_exception.line), utils_exception.module, utils_exception.what, utils_exception.function, utils_exception.info);
}

//----------------------------------------------------------------//
/* C++ SIDE */

_hot void utils::exception::c_api::clear_status(void) noexcept
{
    // Only reset the first char of the strings (no full memset on each success)
    utils_exception.exception = false;
    utils_exception.origin = C_API_EXCEPTION_ORIGIN_NONE;
    utils_exception.type = 0;
    utils_exception.code = C_API_NO_EXCEPTION;
    utils_exception.what[0] = '\0';
    utils_exception.info[0] = '\0';
    utils_exception.line = 0;
    utils_exception.function[0] = '\0';
    utils_exception.file[0] = '\0';
    utils_exception.module[0] = '\0';
}

_cold void utils::exception::c_api::set_status(const utils::exception::IException& e) noexcept
{
    const std::source_location& loc = e.loc();
    const std::string module = e.module();

    // Status (a code unknown to libutils is one of the project: ExternalCode)
    utils_exception.exception = true;
    utils_exception.origin = (utils::exception::InternalMessages.contains(e.getCode()) ? C_API_EXCEPTION_ORIGIN_INTERNAL : C_API_EXCEPTION_ORIGIN_EXTERNAL);

    // Exception
    utils_exception.type = static_cast<utils_exception_type_t>(e.getType());
    utils_exception.code = static_cast<std::uint64_t>(e.getCode());
    utils::smanip::c_api::copy(utils_exception.what, e.what());
    utils::smanip::c_api::copy(utils_exception.info, e.info());

    // Emplacement
    utils_exception.line = static_cast<std::uint64_t>(loc.line());
    utils::smanip::c_api::copy(utils_exception.function, loc.function_name());
    utils::smanip::c_api::copy(utils_exception.file, loc.file_name());
    utils::smanip::c_api::copy(utils_exception.module, module.c_str());
}

_cold void utils::exception::c_api::set_status(const char* what, const utils_exception_origin_t origin) noexcept
{
    utils::exception::c_api::clear_status();
    utils_exception.exception = true;
    utils_exception.origin = origin;
    utils_exception.type = (origin == C_API_EXCEPTION_ORIGIN_C ? C_API_EXCEPTION_TYPE_ERROR : C_API_EXCEPTION_TYPE_SYSTEM); // SYSTEM: std::exception, ...
    utils_exception.code = C_API_OTHER_EXCEPTION_CODE;
    utils::smanip::c_api::copy(utils_exception.what, what);
}

//----------------------------------------------------------------//
/* C SIDE */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow void utils_exception_clear(void)
{
    utils::exception::c_api::clear_status();
}

_cold _nothrow int utils_exception_set_at(utils_exception_type_t type, uint64_t code, const char* what, const char* info, const char* file, uint64_t line, const char* function)
{
    utils::exception::c_api::clear_status();

    // Status
    utils_exception.exception = true;
    utils_exception.origin = C_API_EXCEPTION_ORIGIN_C;

    // Exception
    utils_exception.type = type;
    utils_exception.code = code;
    utils::smanip::c_api::copy(utils_exception.what, what);
    utils::smanip::c_api::copy(utils_exception.info, info);

    // Emplacement (no module: the C side)
    utils_exception.line = line;
    utils::smanip::c_api::copy(utils_exception.function, function);
    utils::smanip::c_api::copy(utils_exception.file, file);
    return UTILS_KO;
}

// Not through guard: reading the status never changes it
_cold _nothrow const char* utils_exception_format(void)
{
    // xstyle: ignore-next CPP-THIS (false positive: _thread_local is the attribute macro, not a member)
    static _thread_local std::string str;

    try {
        str = format_status();
        return str.c_str();
    } catch (...) {return nullptr;}
}

_cold _nothrow size_t utils_exception_format_r(char* buf, size_t size)
{
    if (!buf && size > 0) _unlikely {return 0;}
    try {
        return utils::smanip::c_api::copy_r(format_status(), buf, size);
    } catch (...) {return 0;}
}
