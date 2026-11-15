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
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef SMANIP_CAPI_C_H
    #define SMANIP_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../exception/CApi.h"   // utils_exception, utils_fail, UTILS_OK, UTILS_KO
    #include <stdbool.h>                // bool
    #include <stddef.h>                 // size_t

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
utils_string_t* utils_string_create(const char* data, size_t size); // NULL / 0: empty, NULL on error
void utils_string_destroy(utils_string_t* str); // NULL: nothing
int utils_string_set(utils_string_t* str, const char* data, size_t size);
int utils_string_append(utils_string_t* str, const char* data, size_t size);
int utils_string_clear(utils_string_t* str);
const char* utils_string_data(const utils_string_t* str); // NULL on error
size_t utils_string_size(const utils_string_t* str);
size_t utils_string_copy_r(const utils_string_t* str, char* buf, size_t size); // copied in buf (truncated, always '\0' terminated), returns the full length like snprintf (0 on error)

/* format (utils::smanip::format: "<strong|italic>text<>" -> ANSI styles) */
const char* utils_smanip_format(const char* str); // valid until the next call in the thread (NULL on error)
size_t utils_smanip_format_r(const char* str, char* buf, size_t size); // copied in buf (truncated, always '\0' terminated), returns the full length like snprintf (0 on error)

/* codec */
utils_codec_t* utils_codec_create_base64(void); // utils::smanip::codec::Base64Codec, NULL on error
utils_codec_t* utils_codec_create(utils_codec_fn_t encode, utils_codec_fn_t decode, void* data); // codec of the C side, NULL on error
void utils_codec_destroy(utils_codec_t* codec); // NULL: nothing
int utils_codec_encode(const utils_codec_t* codec, const char* in, size_t size, utils_string_t* out); // out replaced
int utils_codec_decode(const utils_codec_t* codec, const char* in, size_t size, utils_string_t* out); // out replaced

/* EETP content (<type> + data parts) */
utils_eetp_content_t* utils_eetp_content_create(const char* type); // NULL on error
void utils_eetp_content_destroy(utils_eetp_content_t* content); // NULL: nothing
int utils_eetp_content_set_type(utils_eetp_content_t* content, const char* type);
const char* utils_eetp_content_get_type(const utils_eetp_content_t* content); // valid until content is changed (NULL on error)
int utils_eetp_content_add_data(utils_eetp_content_t* content, const char* data, size_t size);
int utils_eetp_content_clear_data(utils_eetp_content_t* content);
size_t utils_eetp_content_data_count(const utils_eetp_content_t* content);
const char* utils_eetp_content_get_data(const utils_eetp_content_t* content, size_t i, size_t* size); // valid until content is changed, size: optional (NULL on error)

/* EETP parser (the codec given is owned by the parser on success: destroyed with it, never destroy / use it after) */
utils_eetp_parser_t* utils_eetp_parser_create(void); // Base64Codec, type size 1 (needs the common RSA key ~/.ssh/common), NULL on error
utils_eetp_parser_t* utils_eetp_parser_create_with(utils_codec_t* codec, size_t type_size); // NULL on error (the codec is still owned by the caller)
void utils_eetp_parser_destroy(utils_eetp_parser_t* parser); // NULL: nothing
int utils_eetp_parser_set_codec(utils_eetp_parser_t* parser, utils_codec_t* codec);
int utils_eetp_parser_set_type_size(utils_eetp_parser_t* parser, size_t type_size); // at least 1
bool utils_eetp_parser_has_id_overload(const utils_eetp_parser_t* parser);
bool utils_eetp_parser_has_no_id_overload(const utils_eetp_parser_t* parser);
int utils_eetp_parser_format(utils_eetp_parser_t* parser, const char* id, const utils_eetp_content_t* content, utils_string_t* out); // out replaced
int utils_eetp_parser_parse(utils_eetp_parser_t* parser, const char* id, const char* in, size_t size, utils_eetp_content_t* out); // out replaced

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* SMANIP_CAPI_C_H */
