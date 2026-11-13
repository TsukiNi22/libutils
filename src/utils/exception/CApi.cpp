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
##  @file CApi.cpp

File Description:
##  Definition of the exception status used by the C API
\**************************************************************/

#include "utils/exception/IException.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/exception/CApi.h"
#include <source_location>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

/* storage of the last exception (one per thread) */
extern "C" thread_local utils_exception_status_t utils_exception = {};

template<std::size_t N>
_cold static inline void copy(char (&dst)[N], const char* src) noexcept
{
    static_assert(N > 3, "The buffer must be able to hold at least the '...' of a truncation");

    // Truncate to the buffer size, always '\0' terminated
    const int len = std::snprintf(dst, N, "%s", (src ? src : ""));

    // Mark the truncation with '...' on the last 3 char
    if (len >= static_cast<int>(N)) {
        dst[N - 4] = '.';
        dst[N - 3] = '.';
        dst[N - 2] = '.';
    }
}

_hot void utils::exception::c_api::clear_status(void) noexcept
{
    // Only reset the first char of the strings (no full memset on each success)
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

    // Exception
    utils_exception.type = static_cast<utils_exception_type_t>(e.getType());
    utils_exception.code = static_cast<std::uint64_t>(e.getCode());
    copy(utils_exception.what, e.what());
    copy(utils_exception.info, e.info());

    // Emplacement
    utils_exception.line = static_cast<std::uint64_t>(loc.line());
    copy(utils_exception.function, loc.function_name());
    copy(utils_exception.file, loc.file_name());
    copy(utils_exception.module, module.c_str());
}

_cold void utils::exception::c_api::set_status(const char* what) noexcept
{
    utils::exception::c_api::clear_status();
    utils_exception.type = C_API_EXCEPTION_TYPE_ERROR;
    utils_exception.code = C_API_OTHER_EXCEPTION_CODE;
    copy(utils_exception.what, what);
}
