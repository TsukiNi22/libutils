/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  15/11/2026 by Tsukini

File Name:
##  @file CApi.h

File Description:
##  C side of the exception status filled by the C API (C & C++ compatible)
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (EXCEPTION_CAPI_H is the guard of CApi.hpp)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef EXCEPTION_CAPI_C_H
    #define EXCEPTION_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _thread_local
    #include "Common.hpp"                   // UTILS_OK, UTILS_KO, OK, KO
    #include <stdbool.h>                    // bool, true, false
    #include <stddef.h>                     // size_t
    #include <stdint.h>                     // uint8_t, uint64_t, UINT64_MAX

    //----------------------------------------------------------------//
    /* DEFINE */

    /* limits */
    #define C_API_MAX_CHAR_EXCEPTION_INFORMATION 512 // limits for the string in the exception status
    #define C_API_NO_EXCEPTION 0                     // default value, no error (utils::exception::InternalCode::Undefined)
    #define C_API_OTHER_EXCEPTION_CODE UINT64_MAX    // not a libutils exception (std::exception, ...)

    /* fill the status from the C side, with the emplacement of the call (always returns UTILS_KO, ignorable: no unused result warning) */
    #define utils_exception_set(type, code, what, info) utils_exception_set_at((type), (code), (what), (info), __FILE__, __LINE__, __func__)
    #define utils_fail(code, info) utils_exception_set(C_API_EXCEPTION_TYPE_ERROR, (code), "C error", (info)) // error of a C function

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

typedef uint8_t utils_exception_type_t; // utils::exception::Type (bit flags)
typedef uint8_t utils_exception_origin_t; // where the exception comes from (C_API_EXCEPTION_ORIGIN_*)

//----------------------------------------------------------------//
/* ENUM */

/* values of utils_exception_type_t */
enum {
    C_API_EXCEPTION_TYPE_NONE    = 0b00001, // Exception like Exit, does nothing (ignore other type)
    C_API_EXCEPTION_TYPE_FATAL   = 0b00010,
    C_API_EXCEPTION_TYPE_ERROR   = 0b00100,
    C_API_EXCEPTION_TYPE_WARNING = 0b01000,
    C_API_EXCEPTION_TYPE_SYSTEM  = 0b10000, // custom type for std::exception
};

/* values of utils_exception_origin_t */
enum {
    C_API_EXCEPTION_ORIGIN_NONE     = 0, // no exception (default value)
    C_API_EXCEPTION_ORIGIN_INTERNAL = 1, // libutils exception, utils::exception::InternalCode
    C_API_EXCEPTION_ORIGIN_EXTERNAL = 2, // libutils exception, utils::exception::ExternalCode (codes of the project)
    C_API_EXCEPTION_ORIGIN_STD      = 3, // std::exception (code: C_API_OTHER_EXCEPTION_CODE)
    C_API_EXCEPTION_ORIGIN_UNKNOWN  = 4, // anything else thrown (code: C_API_OTHER_EXCEPTION_CODE)
    C_API_EXCEPTION_ORIGIN_C        = 5, // set by the C side (utils_exception_set / utils_fail, code of the user)
};

//----------------------------------------------------------------//
/* STRUCT */

typedef struct utils_exception_status {
    /* status */
    bool exception; // true: the last call failed (fields below), false: it succeeded (fields below of an old failure)
    utils_exception_origin_t origin;

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

//----------------------------------------------------------------//
/* PROTOTYPE */

/* status */
void utils_exception_clear(void); // reset the whole status of the thread
int utils_exception_set_at(utils_exception_type_t type, uint64_t code, const char* what, const char* info, const char* file, uint64_t line, const char* function); // origin C, returns UTILS_KO (see utils_exception_set / utils_fail)

/* format (like utils::exception::IException::formated, "" when there is no exception, never changes the status) */
const char* utils_exception_format(void); // valid until the next call in the thread (NULL on error)
size_t utils_exception_format_r(char* buf, size_t size); // copied in buf (truncated, always '\0' terminated), returns the full length like snprintf (0 on error)

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* EXCEPTION_CAPI_C_H */
