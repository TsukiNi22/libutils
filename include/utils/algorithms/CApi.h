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
##  C API of the algorithms of libutils: c2dmp-hsm (string similarity) & s.o.s (steganography) (C & C++ compatible)
##  Wrapper of the synced sources (c2dmp-hsm/, sos/), their template parameters given at runtime
##  Every function that can fail returns UTILS_OK / UTILS_KO (or -1 / 0), details in utils_exception
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef ALGORITHMS_CAPI_C_H
    #define ALGORITHMS_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _nodiscard, _nothrow, _read_only, _write_only, _read_write, _null_terminated_string_arg
    #include "../exception/CApi.h"          // utils_exception, UTILS_OK, UTILS_KO
    #include <stdbool.h>                    // bool
    #include <stddef.h>                     // size_t
    #include <stdint.h>                     // uint8_t ... uint64_t

    //----------------------------------------------------------------//
    /* DEFINE */

    /* s.o.s (utils/algorithms/sos/sosDefine.hpp) */
    #define C_API_SOS_MAGIC 0x22 // magic of the embedded payloads (fixed in the C API)

    //----------------------------------------------------------------//
    /* MACRO */

    /* s.o.s of an unsigned integer (carrier changed in place, key: NULL / 0 = no key) */
    #define C_API_SOS_DECL(name, ctype) \
        _nodiscard _nothrow _read_write(1, 2) _read_only(3, 4) _read_only(5, 6) int utils_sos_embed_##name(ctype* carrier, size_t count, const ctype* payload, size_t payload_count, const ctype* key, size_t key_count, int options); /* options: C_API_SOS_OPTION_* */ \
        _nodiscard _nothrow _read_only(1, 2) _read_only(3, 4) _write_only(5, 6) size_t utils_sos_extract_##name(const ctype* carrier, size_t count, const ctype* key, size_t key_count, ctype* out, size_t max); /* payload copied in out (max at most), returns its full count (0 on error) */

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* ENUM */

/* options of an embedding (utils::algorithms::sos::Option, combined with |) */
typedef enum utils_sos_option {
    C_API_SOS_OPTION_NONE = 0,
    C_API_SOS_OPTION_NOISE = 1 << 0,
    C_API_SOS_OPTION_GLOBAL_NOISE = 1 << 1
} utils_sos_option_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* c2dmp-hsm: similarity of b to a (prefix_depth: 1 to 5, counter_size: 1, 2, 4 or 8 bytes, full: count the misplaced chars of the whole a), -1 on error */
_nodiscard _nothrow _read_only(1, 2) _read_only(3, 4) float utils_c2dmp(const char* a, size_t a_size, const char* b, size_t b_size, uint8_t prefix_depth, size_t counter_size, bool full);
_nodiscard _nothrow _null_terminated_string_arg(1) _null_terminated_string_arg(2) float utils_c2dmp_default(const char* a, const char* b); // '\0' terminated strings, prefix depth 3, 1 byte counter, not full

/* s.o.s of the unsigned integers */
C_API_SOS_DECL(u8, uint8_t)
C_API_SOS_DECL(u16, uint16_t)
C_API_SOS_DECL(u32, uint32_t)
C_API_SOS_DECL(u64, uint64_t)

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* ALGORITHMS_CAPI_C_H */
