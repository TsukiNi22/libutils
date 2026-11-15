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
##  C API of utils::verbose (level of the global verbose, printf style logs on the locked output) (C & C++ compatible)
##  The logs go through std::cout like the c++ macros (onVerbose, ...), so the C & C++ logs stay in order
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef VERBOSE_CAPI_C_H
    #define VERBOSE_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _format
    #include "../exception/CApi.h"          // utils_exception, utils_fail, UTILS_OK, UTILS_KO
    #include <stdbool.h>                    // bool
    #include <stdarg.h>                     // va_list
    #include <stdio.h>                      // FILE

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* callbacks */
typedef int (*utils_verbose_fn_t)(void* data); // UTILS_OK / UTILS_KO (info: utils_fail / utils_exception_set)

//----------------------------------------------------------------//
/* ENUM */

/* level (utils::verbose::Verbose), less (0) -> most */
typedef enum utils_verbose {
    C_API_VERBOSE_NONE = 0,
    C_API_VERBOSE_BASIC,
    C_API_VERBOSE_ADVANCED,
    C_API_VERBOSE_DEBUG
} utils_verbose_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* level (utils::verbose::verbose, global & atomic) */
void utils_verbose_set(utils_verbose_t level);
utils_verbose_t utils_verbose_get(void);
bool utils_verbose_is(utils_verbose_t level); // true when the level is displayed (level <= the global one)

/* logs on stdout (std::cout) with a new line, only when the level is displayed (like onVerbose / onDebugVerbose: "debug: " prefix for C_API_VERBOSE_DEBUG) */
_format(printf, 2, 3) int utils_verbose_log(utils_verbose_t level, const char* fmt, ...);
_format(printf, 2, 0) int utils_verbose_vlog(utils_verbose_t level, const char* fmt, va_list args);

/* logs on a custom output (like onVerboseC), locked like the stdout ones */
_format(printf, 3, 4) int utils_verbose_log_to(FILE* output, utils_verbose_t level, const char* fmt, ...);
_format(printf, 3, 0) int utils_verbose_vlog_to(FILE* output, utils_verbose_t level, const char* fmt, va_list args);

/* run fn with the output locked (utils::verbose::locked, recursive: the logs can be called inside), returns what fn returned */
int utils_verbose_locked(utils_verbose_fn_t fn, void* data);
int utils_verbose_on(utils_verbose_t level, utils_verbose_fn_t fn, void* data); // only when the level is displayed (like onVerboseFn), UTILS_OK when not run

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* VERBOSE_CAPI_C_H */
