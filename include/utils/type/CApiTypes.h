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
##  @file CApiTypes.h

File Description:
##  Fixed types of the C API (X-macros: X(name, type) unrolled by each C API for its templates) (C & C++ compatible)
##  name: suffix of the functions & types (utils_vector2_f64_t, utils_blt_str_size_*), str: const char* in C (std::string in c++, copied)
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_*_C_H like the other C API)

#ifndef TYPE_CAPITYPES_C_H
    #define TYPE_CAPITYPES_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <stdbool.h>    // bool
    #include <stddef.h>     // size_t
    #include <stdint.h>     // int8_t ... uint64_t

    //----------------------------------------------------------------//
    /* MACRO */

    /* integers (signed, unsigned, size) */
    #define C_API_INT_TYPES(X) \
        X(i8, int8_t) \
        X(i16, int16_t) \
        X(i32, int32_t) \
        X(i64, int64_t) \
        X(u8, uint8_t) \
        X(u16, uint16_t) \
        X(u32, uint32_t) \
        X(u64, uint64_t) \
        X(size, size_t)

    /* floating points (f128: long double, the utils::arguments::float128_t of libutils) */
    #define C_API_FLOAT_TYPES(X) \
        X(f32, float) \
        X(f64, double) \
        X(f128, long double)

    /* numeric (math: vector, matrix) */
    #define C_API_NUMERIC_TYPES(X) \
        C_API_INT_TYPES(X) \
        C_API_FLOAT_TYPES(X)

    /* every value type (containers) */
    #define C_API_VALUE_TYPES(X) \
        C_API_NUMERIC_TYPES(X) \
        X(bool, bool) \
        X(char, char)

    /* every type (containers), str: const char* (copied in a std::string) */
    #define C_API_TYPES(X) \
        C_API_VALUE_TYPES(X) \
        X(str, const char*)

#endif /* TYPE_CAPITYPES_C_H */
