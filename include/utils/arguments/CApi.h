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
##  C API of utils::arguments: ArgParser (usages, options, flags, parsing) & Settings (typed settings) (C & C++ compatible)
##  Every function that can fail returns UTILS_OK / UTILS_KO (or NULL / 0), details in utils_exception
##  Every check callback returns UTILS_OK (valid) or UTILS_KO (its message: the info of utils_fail / utils_exception_set)
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef ARGUMENTS_CAPI_C_H
    #define ARGUMENTS_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _nodiscard, _nothrow, _read_only, _null_terminated_string_arg
    #include "../exception/CApi.h"          // utils_exception, utils_fail, UTILS_OK, UTILS_KO
    #include "../type/CApiTypes.h"          // C_API_TYPES
    #include <stdbool.h>                    // bool
    #include <stddef.h>                     // size_t, wchar_t
    #include <stdint.h>                     // uint8_t, uint_least16_t, uint_least32_t

    //----------------------------------------------------------------//
    /* MACRO */

    /* settings of the fixed types (str: const char* copied in a std::string, given back valid until the setting is changed) */
    #define C_API_SETTINGS_DECL(name, ctype) \
        _nodiscard _nothrow _null_terminated_string_arg(2) int utils_settings_add_##name(utils_settings_t* settings, const char* id, ctype value); /* an existing one is refused */ \
        _nodiscard _nothrow _null_terminated_string_arg(2) int utils_settings_set_##name(utils_settings_t* settings, const char* id, ctype value, bool force); /* force: override an existing one */ \
        _nodiscard _nothrow _null_terminated_string_arg(2) int utils_settings_get_##name(const utils_settings_t* settings, const char* id, ctype* value); /* UTILS_KO: unknown id or another type */ \
        _nodiscard _nothrow _null_terminated_string_arg(2) bool utils_settings_is_##name(const utils_settings_t* settings, const char* id);

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* opaque types */
typedef struct utils_argparser utils_argparser_t; // utils::arguments::ArgParser
typedef struct utils_argparser_result utils_argparser_result_t; // utils::arguments::ParsedUsages (return of the parsing)
typedef struct utils_settings utils_settings_t; // utils::arguments::Settings

/* callbacks */
typedef int (*utils_argparser_check_t)(const char* option, void* data); // UTILS_OK: valid, UTILS_KO: the message is the info of utils_fail
typedef int (*utils_argparser_help_hook_t)(const utils_argparser_t* parser, void* data); // display the help

//----------------------------------------------------------------//
/* ENUM */

/* type of a setting (utils::arguments::CastType) */
typedef enum utils_settings_cast {
    C_API_SETTINGS_CAST_NONE = 0, // string (const char*)
    /* basic */
    C_API_SETTINGS_CAST_BYTE,
    C_API_SETTINGS_CAST_BOOL,
    /* integer */
    C_API_SETTINGS_CAST_INT8,
    C_API_SETTINGS_CAST_INT16,
    C_API_SETTINGS_CAST_INT32,
    C_API_SETTINGS_CAST_INT64,
    /* unsigned integer */
    C_API_SETTINGS_CAST_UINT8,
    C_API_SETTINGS_CAST_UINT16,
    C_API_SETTINGS_CAST_UINT32,
    C_API_SETTINGS_CAST_UINT64,
    /* floating */
    C_API_SETTINGS_CAST_FLOAT16,
    C_API_SETTINGS_CAST_FLOAT32,
    C_API_SETTINGS_CAST_FLOAT64,
    C_API_SETTINGS_CAST_FLOAT128,
    /* char */
    C_API_SETTINGS_CAST_CHAR8,
    C_API_SETTINGS_CAST_CHAR16,
    C_API_SETTINGS_CAST_CHAR32,
    C_API_SETTINGS_CAST_U8STRING,
    C_API_SETTINGS_CAST_U16STRING,
    C_API_SETTINGS_CAST_U32STRING,
    /* huge char */
    C_API_SETTINGS_CAST_WCHAR,
    C_API_SETTINGS_CAST_WSTRING,
    /* special */
    C_API_SETTINGS_CAST_PATH,
    /* C API only (utils_settings_get_type) */
    C_API_SETTINGS_CAST_OTHER = 1000 // a type without cast (char, bytes, size_t when it's not a uint64_t, ...)
} utils_settings_cast_t;

//----------------------------------------------------------------//
/* STRUCT */

/* option of a flag (utils_argparser_set_flag) */
typedef struct utils_argparser_flag_option {
    const char* name;
    bool mandatory;
    utils_argparser_check_t check; // NULL: no check
    void* data; // given to check
} utils_argparser_flag_option_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* argparser */
_nodiscard _nothrow _null_terminated_string_arg(1) _null_terminated_string_arg(2) utils_argparser_t* utils_argparser_create(const char* binary, const char* description); // NULL: "[None]" / "...", NULL on error
_nothrow void utils_argparser_destroy(utils_argparser_t* parser); // NULL: nothing
_nodiscard _nothrow int utils_argparser_help(const utils_argparser_t* parser); // display the help (help hook)

/* parsing (the first argument is ignored), the result: NULL on error (failsafe: no throw on invalid arguments, only the valid usages) */
_nodiscard _nothrow utils_argparser_result_t* utils_argparser_parse(const utils_argparser_t* parser, int argc, const char* const* argv, bool failsafe);

/* setup (force: override an existing one, else refused) */
_nodiscard _nothrow int utils_argparser_set_default_usage(utils_argparser_t* parser); // "default": every flag & option
_nodiscard _nothrow _null_terminated_string_arg(2) _null_terminated_string_arg(3) _read_only(6, 7) _null_terminated_string_arg(8) int utils_argparser_set_usage(utils_argparser_t* parser, const char* id, const char* name, bool ordered, const char* const* ids, const bool* mandatory, size_t count, const char* description, bool force); // description: NULL = "[None]"
_nodiscard _nothrow _null_terminated_string_arg(2) _null_terminated_string_arg(3) _null_terminated_string_arg(6) int utils_argparser_set_option(utils_argparser_t* parser, const char* id, const char* name, utils_argparser_check_t check, void* data, const char* description, bool force); // check: NULL = no check
_nodiscard _nothrow _null_terminated_string_arg(2) _null_terminated_string_arg(3) _null_terminated_string_arg(4) _null_terminated_string_arg(5) _null_terminated_string_arg(6) _null_terminated_string_arg(9) int utils_argparser_set_flag(utils_argparser_t* parser, const char* id, const char* short_flag, const char* flag, const char* long_flag, const char* env, const utils_argparser_flag_option_t* options, size_t count, const char* description, bool unlimited, bool ignore_case, bool force); // NULL flag name: "" (not used)
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_remove_usage(utils_argparser_t* parser, const char* id);
_nodiscard _nothrow int utils_argparser_remove_usages(utils_argparser_t* parser, const char* const* ids, size_t count);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_remove_option(utils_argparser_t* parser, const char* id);
_nodiscard _nothrow int utils_argparser_remove_options(utils_argparser_t* parser, const char* const* ids, size_t count);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_remove_flag(utils_argparser_t* parser, const char* id);
_nodiscard _nothrow int utils_argparser_remove_flags(utils_argparser_t* parser, const char* const* ids, size_t count);
_nodiscard _nothrow int utils_argparser_reset_usages(utils_argparser_t* parser);
_nodiscard _nothrow int utils_argparser_reset_options(utils_argparser_t* parser);
_nodiscard _nothrow int utils_argparser_reset_flags(utils_argparser_t* parser);

/* hooks */
_nodiscard _nothrow int utils_argparser_set_help_hook(utils_argparser_t* parser, utils_argparser_help_hook_t hook, void* data);
_nodiscard _nothrow int utils_argparser_reset_help_hook(utils_argparser_t* parser);
_nodiscard _nothrow int utils_argparser_default_help_hook(const utils_argparser_t* parser);

/* default checks (usable as a check callback, data ignored) */
_nodiscard _nothrow _null_terminated_string_arg(1) int utils_argparser_check_true(const char* option, void* data); // always valid
_nodiscard _nothrow _null_terminated_string_arg(1) int utils_argparser_check_bool(const char* option, void* data); // 0, 1, false, true
_nodiscard _nothrow _null_terminated_string_arg(1) int utils_argparser_check_int32(const char* option, void* data);
_nodiscard _nothrow _null_terminated_string_arg(1) int utils_argparser_check_sizet(const char* option, void* data);
_nodiscard _nothrow _null_terminated_string_arg(1) int utils_argparser_check_double(const char* option, void* data);
_nodiscard _nothrow _null_terminated_string_arg(1) int utils_argparser_check_file(const char* option, void* data); // readable file
_nodiscard _nothrow _null_terminated_string_arg(1) int utils_argparser_check_directory(const char* option, void* data); // readable directory
_nodiscard _nothrow _null_terminated_string_arg(1) int utils_argparser_check_writable(const char* option, void* data); // readable & writable path

/* setter / getter (strings: valid until the parser is changed) */
_nodiscard _nothrow int utils_argparser_disable_help(utils_argparser_t* parser); // no default -h
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_set_binary(utils_argparser_t* parser, const char* binary);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_set_description(utils_argparser_t* parser, const char* description);
_nodiscard _nothrow const char* utils_argparser_get_binary(const utils_argparser_t* parser);
_nodiscard _nothrow const char* utils_argparser_get_description(const utils_argparser_t* parser);

/* usages / options / flags of the parser (index: any order, valid until the parser is changed) */
_nodiscard _nothrow size_t utils_argparser_usages_count(const utils_argparser_t* parser);
_nodiscard _nothrow const char* utils_argparser_usage_id(const utils_argparser_t* parser, size_t i);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_get_usage(const utils_argparser_t* parser, const char* id, const char** name, bool* ordered, const char** description); // each output: optional
_nodiscard _nothrow _null_terminated_string_arg(2) size_t utils_argparser_usage_ids_count(const utils_argparser_t* parser, const char* id);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_usage_get_id(const utils_argparser_t* parser, const char* id, size_t i, const char** option_id, bool* mandatory);
_nodiscard _nothrow size_t utils_argparser_options_count(const utils_argparser_t* parser);
_nodiscard _nothrow const char* utils_argparser_option_id(const utils_argparser_t* parser, size_t i);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_get_option(const utils_argparser_t* parser, const char* id, const char** name, bool* exact, const char** description); // each output: optional
_nodiscard _nothrow size_t utils_argparser_flags_count(const utils_argparser_t* parser);
_nodiscard _nothrow const char* utils_argparser_flag_id(const utils_argparser_t* parser, size_t i);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_get_flag(const utils_argparser_t* parser, const char* id, const char** short_flag, const char** flag, const char** long_flag, const char** env, const char** description, bool* unlimited); // each output: optional
_nodiscard _nothrow _null_terminated_string_arg(2) size_t utils_argparser_flag_options_count(const utils_argparser_t* parser, const char* id);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_argparser_flag_get_option(const utils_argparser_t* parser, const char* id, size_t i, const char** name, bool* mandatory);

/* result of the parsing (one usage = an id & its arguments: an option or a flag with its values) */
_nothrow void utils_argparser_result_destroy(utils_argparser_result_t* result); // NULL: nothing
_nodiscard _nothrow size_t utils_argparser_result_count(const utils_argparser_result_t* result); // number of usages
_nodiscard _nothrow const char* utils_argparser_result_usage(const utils_argparser_result_t* result, size_t i); // id of the usage i ("": nothing), NULL on error
_nodiscard _nothrow size_t utils_argparser_result_arguments_count(const utils_argparser_result_t* result, size_t i);
_nodiscard _nothrow const char* utils_argparser_result_argument_id(const utils_argparser_result_t* result, size_t i, size_t j); // NULL on error
_nodiscard _nothrow bool utils_argparser_result_argument_is_option(const utils_argparser_result_t* result, size_t i, size_t j); // option (true) or flag (false)
_nodiscard _nothrow size_t utils_argparser_result_values_count(const utils_argparser_result_t* result, size_t i, size_t j);
_nodiscard _nothrow const char* utils_argparser_result_value(const utils_argparser_result_t* result, size_t i, size_t j, size_t k); // NULL on error

/* settings */
_nodiscard _nothrow utils_settings_t* utils_settings_create(void); // NULL on error
_nothrow void utils_settings_destroy(utils_settings_t* settings); // NULL: nothing
_nodiscard _nothrow _null_terminated_string_arg(2) _null_terminated_string_arg(3) int utils_settings_auto_cast(utils_settings_t* settings, const char* id, const char* setting, bool force, utils_settings_cast_t* type); // type found & stored (type: optional)
_nodiscard _nothrow _null_terminated_string_arg(2) _null_terminated_string_arg(3) int utils_settings_cast(utils_settings_t* settings, const char* id, const char* setting, utils_settings_cast_t type, bool force);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_settings_remove(utils_settings_t* settings, const char* id, bool failsafe); // failsafe: an unknown id is not an error
_nodiscard _nothrow int utils_settings_clear(utils_settings_t* settings);
_nodiscard _nothrow _null_terminated_string_arg(2) bool utils_settings_contains(const utils_settings_t* settings, const char* id);
_nodiscard _nothrow _null_terminated_string_arg(2) utils_settings_cast_t utils_settings_get_type(const utils_settings_t* settings, const char* id); // C_API_SETTINGS_CAST_OTHER: unknown id (error) or a type without cast

/* settings of the fixed types */
C_API_TYPES(C_API_SETTINGS_DECL)

/* settings of the types of the casts (strings: valid until the setting is changed, size: optional) */
C_API_SETTINGS_DECL(byte, uint8_t) // std::byte
C_API_SETTINGS_DECL(char8, uint8_t) // char8_t
C_API_SETTINGS_DECL(char16, uint_least16_t) // char16_t
C_API_SETTINGS_DECL(char32, uint_least32_t) // char32_t
C_API_SETTINGS_DECL(wchar, wchar_t)
C_API_SETTINGS_DECL(u8string, const char*) // std::u8string (utf-8)
C_API_SETTINGS_DECL(path, const char*) // std::filesystem::path (the path given back: valid until the next utils_settings_get_path of the thread)
_nodiscard _nothrow _null_terminated_string_arg(2) _read_only(3, 4) int utils_settings_set_u16string(utils_settings_t* settings, const char* id, const uint_least16_t* str, size_t size, bool force); // std::u16string
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_settings_get_u16string(const utils_settings_t* settings, const char* id, const uint_least16_t** str, size_t* size);
_nodiscard _nothrow _null_terminated_string_arg(2) bool utils_settings_is_u16string(const utils_settings_t* settings, const char* id);
_nodiscard _nothrow _null_terminated_string_arg(2) _read_only(3, 4) int utils_settings_set_u32string(utils_settings_t* settings, const char* id, const uint_least32_t* str, size_t size, bool force); // std::u32string
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_settings_get_u32string(const utils_settings_t* settings, const char* id, const uint_least32_t** str, size_t* size);
_nodiscard _nothrow _null_terminated_string_arg(2) bool utils_settings_is_u32string(const utils_settings_t* settings, const char* id);
_nodiscard _nothrow _null_terminated_string_arg(2) _read_only(3, 4) int utils_settings_set_wstring(utils_settings_t* settings, const char* id, const wchar_t* str, size_t size, bool force); // std::wstring
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_settings_get_wstring(const utils_settings_t* settings, const char* id, const wchar_t** str, size_t* size);
_nodiscard _nothrow _null_terminated_string_arg(2) bool utils_settings_is_wstring(const utils_settings_t* settings, const char* id);

/* settings of bytes (generic: data + size, copied) */
_nodiscard _nothrow _null_terminated_string_arg(2) _read_only(3, 4) int utils_settings_set_bytes(utils_settings_t* settings, const char* id, const void* data, size_t size, bool force);
_nodiscard _nothrow _null_terminated_string_arg(2) int utils_settings_get_bytes(const utils_settings_t* settings, const char* id, const void** data, size_t* size);
_nodiscard _nothrow _null_terminated_string_arg(2) bool utils_settings_is_bytes(const utils_settings_t* settings, const char* id);

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* ARGUMENTS_CAPI_C_H */
