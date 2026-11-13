/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  13/11/2026 by Tsukini

File Name:
##  @file CApi.h

File Description:
##  C side of the exception status filled by the C API (C & C++ compatible)
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (CAPI_H is the guard of CApi.hpp)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef CAPI_C_H
    #define CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <stdint.h> // uint8_t, uint64_t, UINT64_MAX

    //----------------------------------------------------------------//
    /* DEFINE */

    /* limits */
    #define C_API_MAX_CHAR_EXCEPTION_INFORMATION 512 // limits for the string in the exception status
    #define C_API_NO_EXCEPTION 0                     // default value, no error (utils::exception::InternalCode::Undefined)
    #define C_API_OTHER_EXCEPTION_CODE UINT64_MAX    // not a libutils exception (std::exception, ...)

    /* thread local storage */
    #ifdef __cplusplus
        #define _thread_local thread_local
    #else
        #define _thread_local _Thread_local // C11 (still valid in C23)
    #endif

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

typedef uint8_t utils_exception_type_t; // utils::exception::Type (bit flags)

//----------------------------------------------------------------//
/* ENUM */

/* values of utils_exception_type_t */
enum {
    C_API_EXCEPTION_TYPE_NONE    = 0x1, // Exception like Exit, does nothing (ignore other type)
    C_API_EXCEPTION_TYPE_FATAL   = 0x2,
    C_API_EXCEPTION_TYPE_ERROR   = 0x4,
    C_API_EXCEPTION_TYPE_WARNING = 0x8,
};

//----------------------------------------------------------------//
/* STRUCT */

typedef struct utils_exception_status {
    /* exception */
    utils_exception_type_t type;
    uint64_t code;
    char what[C_API_MAX_CHAR_EXCEPTION_INFORMATION];
    char info[C_API_MAX_CHAR_EXCEPTION_INFORMATION];

    /* emplacement */
    uint64_t line;
    char function[C_API_MAX_CHAR_EXCEPTION_INFORMATION];
    char file[C_API_MAX_CHAR_EXCEPTION_INFORMATION];
    char module[C_API_MAX_CHAR_EXCEPTION_INFORMATION];
} utils_exception_status_t;

//----------------------------------------------------------------//
/* GLOBAL */

// Last exception caught by the C API (one per thread, strings truncated to the max size)
extern _thread_local utils_exception_status_t utils_exception;

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* CAPI_C_H */
