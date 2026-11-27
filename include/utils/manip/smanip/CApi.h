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
##  C API of utils::smanip: byte strings, format, codecs & EETP parser (C & C++ compatible)
##  Every function that can fail returns UTILS_OK / UTILS_KO (or NULL / 0), details in utils_exception
##  Every callback returns UTILS_OK or UTILS_KO (info: utils_fail / utils_exception_set)
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef SMANIP_CAPI_C_H
    #define SMANIP_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"   // _nodiscard, _nothrow, _read_only, _write_only, _null_terminated_string_arg
    #include "../../exception/CApi.h"          // utils_exception, utils_fail, UTILS_OK, UTILS_KO
    #include <stdbool.h>                       // bool
    #include <stddef.h>                        // size_t

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* opaque types */
typedef struct utils_string utils_string_t; // std::string (bytes, can hold '\0')
typedef struct utils_codec utils_codec_t; // utils::smanip::codec::ICodec (Base64Codec or a codec of the C side)
typedef struct utils_eetp_content utils_eetp_content_t; // utils::smanip::parser::EETPContent
typedef struct utils_eetp_parser utils_eetp_parser_t; // utils::smanip::parser::EETPParser

/* codec of the C side: fill out (empty at the call) from the size bytes of in */
typedef int (*utils_codec_fn_t)(const char* in, size_t size, utils_string_t* out, void* data);

//----------------------------------------------------------------//
/* PROTOTYPE */

/* byte string (data: valid until the string is changed, always '\0' terminated) */
_nodiscard _nothrow _read_only(1, 2) utils_string_t* utils_string_create(const char* data, size_t size); // NULL / 0: empty, NULL on error
_nothrow void utils_string_destroy(utils_string_t* str); // NULL: nothing
_nodiscard _nothrow _read_only(2, 3) int utils_string_set(utils_string_t* str, const char* data, size_t size);
_nodiscard _nothrow _read_only(2, 3) int utils_string_append(utils_string_t* str, const char* data, size_t size);
_nodiscard _nothrow int utils_string_clear(utils_string_t* str);
_nodiscard _nothrow const char* utils_string_data(const utils_string_t* str); // NULL on error
_nodiscard _nothrow size_t utils_string_size(const utils_string_t* str);
_nodiscard _nothrow _write_only(2, 3) size_t utils_string_copy_r(const utils_string_t* str, char* buf, size_t size); // copied in buf (truncated, always '\0' terminated), returns the full length like snprintf (0 on error)

/* format (utils::smanip::format: "<strong|italic>text<>" -> ANSI styles) */
_nodiscard _nothrow _null_terminated_string_arg(1) const char* utils_smanip_format(const char* str); // valid until the next call in the thread (NULL on error)
_nodiscard _nothrow _null_terminated_string_arg(1) _write_only(2, 3) size_t utils_smanip_format_r(const char* str, char* buf, size_t size); // copied in buf (truncated, always '\0' terminated), returns the full length like snprintf (0 on error)

/* codec */
_nodiscard _nothrow utils_codec_t* utils_codec_create_base64(void); // utils::smanip::codec::Base64Codec, NULL on error
_nodiscard _nothrow utils_codec_t* utils_codec_create(utils_codec_fn_t encode, utils_codec_fn_t decode, void* data); // codec of the C side, NULL on error
_nothrow void utils_codec_destroy(utils_codec_t* codec); // NULL: nothing
_nodiscard _nothrow _read_only(2, 3) int utils_codec_encode(const utils_codec_t* codec, const char* in, size_t size, utils_string_t* out); // out replaced
_nodiscard _nothrow _read_only(2, 3) int utils_codec_decode(const utils_codec_t* codec, const char* in, size_t size, utils_string_t* out); // out replaced

/* EETP content (<type> + data parts) */
_nodiscard _nothrow _null_terminated_string_arg(1) utils_eetp_content_t* utils_eetp_content_create(const char* type); // NULL on error
_nothrow void utils_eetp_content_destroy(utils_eetp_content_t* content); // NULL: nothing
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_eetp_content_set_type(utils_eetp_content_t* content, const char* type);
_nodiscard _nothrow const char* utils_eetp_content_get_type(const utils_eetp_content_t* content); // valid until content is changed (NULL on error)
_nodiscard _nothrow _read_only(2, 3) int utils_eetp_content_add_data(utils_eetp_content_t* content, const char* data, size_t size);
_nodiscard _nothrow int utils_eetp_content_clear_data(utils_eetp_content_t* content);
_nodiscard _nothrow size_t utils_eetp_content_data_count(const utils_eetp_content_t* content);
_nodiscard _nothrow const char* utils_eetp_content_get_data(const utils_eetp_content_t* content, size_t i, size_t* size); // valid until content is changed, size: optional (NULL on error)

/* EETP parser (the codec given is owned by the parser on success: destroyed with it, never destroy / use it after) */
_nodiscard _nothrow utils_eetp_parser_t* utils_eetp_parser_create(void); // Base64Codec, type size 1 (needs the common RSA key ~/.ssh/common), NULL on error
_nodiscard _nothrow utils_eetp_parser_t* utils_eetp_parser_create_with(utils_codec_t* codec, size_t type_size); // NULL on error (the codec is still owned by the caller)
_nothrow void utils_eetp_parser_destroy(utils_eetp_parser_t* parser); // NULL: nothing
_nodiscard _nothrow int utils_eetp_parser_set_codec(utils_eetp_parser_t* parser, utils_codec_t* codec);
_nodiscard _nothrow int utils_eetp_parser_set_type_size(utils_eetp_parser_t* parser, size_t type_size); // at least 1
_nodiscard _nothrow bool utils_eetp_parser_has_id_overload(const utils_eetp_parser_t* parser);
_nodiscard _nothrow bool utils_eetp_parser_has_no_id_overload(const utils_eetp_parser_t* parser);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_eetp_parser_format(utils_eetp_parser_t* parser, const char* id, const utils_eetp_content_t* content, utils_string_t* out); // out replaced
_nodiscard _nothrow _null_terminated_string_arg(2) _read_only(3, 4) int utils_eetp_parser_parse(utils_eetp_parser_t* parser, const char* id, const char* in, size_t size, utils_eetp_content_t* out); // out replaced

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* SMANIP_CAPI_C_H */
