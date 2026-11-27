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
##  C API of utils::verbose
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/verbose/Verbose.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/verbose/CApi.h"
#include <iostream>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <string>

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ levels must stay the same */
static_assert(C_API_VERBOSE_NONE     == static_cast<std::size_t>(utils::verbose::Verbose::None));
static_assert(C_API_VERBOSE_BASIC    == static_cast<std::size_t>(utils::verbose::Verbose::Basic));
static_assert(C_API_VERBOSE_ADVANCED == static_cast<std::size_t>(utils::verbose::Verbose::Advanced));
static_assert(C_API_VERBOSE_DEBUG    == static_cast<std::size_t>(utils::verbose::Verbose::Debug));

//----------------------------------------------------------------//
/* TOOLS */

// Level given by the C side (out of the enum: InvalidArgument)
_hot _nodiscard static utils::verbose::Verbose level_of(const utils_verbose_t level)
{
    if (static_cast<std::size_t>(level) > static_cast<std::size_t>(utils::verbose::Verbose::Debug)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The verbose level is invalid");
    }
    return static_cast<utils::verbose::Verbose>(level);
}

_hot _nodiscard static bool displayed(const utils_verbose_t level)
{
    return static_cast<std::size_t>(utils::verbose::verbose.load(std::memory_order_relaxed)) >= static_cast<std::size_t>(level_of(level));
}

// printf of the C side in a std::string
_cold _nodiscard _format(printf, 1, 0) static std::string vformat(const char* fmt, va_list args)
{
    va_list copy;
    int len = 0;

    va_copy(copy, args);
    len = std::vsnprintf(nullptr, 0, utils::exception::c_api::not_null(fmt, "format"), copy);
    va_end(copy);
    if (len < 0) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The format is invalid");
    }
    std::string str(static_cast<std::size_t>(len), '\0');
    (void)std::vsnprintf(str.data(), str.size() + 1, fmt, args);
    return str;
}

// Run the C callback with the output locked, keep its own error (utils_fail) or report a failure without info
_cold _nodiscard static int run(utils_verbose_fn_t fn, void* data)
{
    int ret = UTILS_KO;
    bool failed = false;

    if (utils::exception::c_api::guard([&](void) {
        (void)utils::exception::c_api::not_null(fn, "callback");
        utils::verbose::locked([&](void) {
            utils::exception::c_api::reset_status(); // only the error of this callback
            ret = fn(data);
            failed = utils_exception.exception;
        });
    }) != UTILS_OK) _unlikely {return UTILS_KO;}
    if (ret == UTILS_KO && failed) utils_exception.exception = true;
    else if (ret == UTILS_KO) utils::exception::c_api::set_status("C callback failed (no info given with utils_fail)", C_API_EXCEPTION_ORIGIN_C);
    return ret;
}

//----------------------------------------------------------------//
/* LEVEL */

_cold void utils_verbose_set(utils_verbose_t level)
{
    (void)utils::exception::c_api::guard([&](void) {utils::verbose::verbose.store(level_of(level));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot utils_verbose_t utils_verbose_get(void)
{
    return static_cast<utils_verbose_t>(utils::verbose::verbose.load(std::memory_order_relaxed));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot bool utils_verbose_is(utils_verbose_t level)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return displayed(level);});
}

//----------------------------------------------------------------//
/* LOG */

_hot int utils_verbose_log(utils_verbose_t level, const char* fmt, ...)
{
    va_list args;
    int ret = UTILS_KO;

    va_start(args, fmt);
    ret = utils_verbose_vlog(level, fmt, args);
    va_end(args);
    return ret;
}

_hot int utils_verbose_vlog(utils_verbose_t level, const char* fmt, va_list args)
{
    return utils::exception::c_api::guard([&](void) {
        if (!displayed(level)) return;
        const std::string str = vformat(fmt, args);
        utils::verbose::locked([&](void) {std::cout << (level == C_API_VERBOSE_DEBUG ? "debug: " : "") << str << std::endl;});
    });
}

_hot int utils_verbose_log_to(FILE* output, utils_verbose_t level, const char* fmt, ...)
{
    va_list args;
    int ret = UTILS_KO;

    va_start(args, fmt);
    ret = utils_verbose_vlog_to(output, level, fmt, args);
    va_end(args);
    return ret;
}

_hot int utils_verbose_vlog_to(FILE* output, utils_verbose_t level, const char* fmt, va_list args)
{
    return utils::exception::c_api::guard([&](void) {
        (void)utils::exception::c_api::not_null(output, "output");
        if (!displayed(level)) return;
        const std::string str = vformat(fmt, args);
        utils::verbose::locked([&](void) {
            std::fputs(str.c_str(), output);
            std::fputc('\n', output);
            std::fflush(output);
        });
    });
}

//----------------------------------------------------------------//
/* LOCKED */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_verbose_locked(utils_verbose_fn_t fn, void* data)
{
    return run(fn, data);
}

_cold int utils_verbose_on(utils_verbose_t level, utils_verbose_fn_t fn, void* data)
{
    bool show = false;

    if (utils::exception::c_api::guard([&](void) {show = displayed(level);}) != UTILS_OK) _unlikely {return UTILS_KO;}
    if (!show) return UTILS_OK;
    return run(fn, data);
}
