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
##  @file CApi.hpp

File Description:
##  Handles of the C API of utils::arguments (usable in c++ to get the object)
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef ARGUMENTS_CAPI_H
    #define ARGUMENTS_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/CApi.hpp"    // utils_handle
    #include "ArgParserType.hpp"        // utils::arguments::ParsedUsages
    #include "ArgParser.hpp"            // utils::arguments::ArgParser
    #include "Settings.hpp"             // utils::arguments::Settings
    #include "CApi.h"                   // utils_argparser_t, utils_argparser_result_t, utils_settings_t
    #include <string>                   // std::string

    //----------------------------------------------------------------//
    /* MACRO */

    /* settings of the fixed types */
    #define C_API_SETTINGS_DEF(name, ctype) \
        _nothrow int utils_settings_add_##name(utils_settings_t* settings, const char* id, ctype value) \
        {return utils::exception::c_api::guard([&](void) {store(settings_of(settings), id, utils::type::c_api::Storage<ctype>::cpp(value), true, false);});} \
        _nothrow int utils_settings_set_##name(utils_settings_t* settings, const char* id, ctype value, bool force) \
        {return utils::exception::c_api::guard([&](void) {store(settings_of(settings), id, utils::type::c_api::Storage<ctype>::cpp(value), false, force);});} \
        _nothrow int utils_settings_get_##name(const utils_settings_t* settings, const char* id, ctype* value) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                ctype& res = utils::exception::c_api::not_null_ref(value, "value"); \
                res = utils::type::c_api::Storage<ctype>::c(settings_of(settings).template get<utils::type::c_api::cpp_t<ctype>>(utils::exception::c_api::not_null(id, "id"))); \
            }); \
        } \
        _nothrow bool utils_settings_is_##name(const utils_settings_t* settings, const char* id) \
        {return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return setting_is<utils::type::c_api::cpp_t<ctype>>(settings, id);});}

    /* settings of a type of the casts (value: the C side, stored: the c++ side) */
    #define C_API_SETTINGS_CAST_DEF(name, ctype, cpptype, to_cpp, to_c) \
        _nothrow int utils_settings_add_##name(utils_settings_t* settings, const char* id, ctype value) \
        {return utils::exception::c_api::guard([&](void) {store(settings_of(settings), id, cpptype(to_cpp), true, false);});} \
        _nothrow int utils_settings_set_##name(utils_settings_t* settings, const char* id, ctype value, bool force) \
        {return utils::exception::c_api::guard([&](void) {store(settings_of(settings), id, cpptype(to_cpp), false, force);});} \
        _nothrow int utils_settings_get_##name(const utils_settings_t* settings, const char* id, ctype* value) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                ctype& res = utils::exception::c_api::not_null_ref(value, "value"); \
                const cpptype& stored = settings_of(settings).template get<cpptype>(utils::exception::c_api::not_null(id, "id")); \
                res = (to_c); \
            }); \
        } \
        _nothrow bool utils_settings_is_##name(const utils_settings_t* settings, const char* id) \
        {return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return setting_is<cpptype>(settings, id);});}

    /* settings of a string of the casts (data + size) */
    #define C_API_SETTINGS_STRING_DEF(name, ctype, cpptype) \
        _nothrow int utils_settings_set_##name(utils_settings_t* settings, const char* id, const ctype* str, size_t size, bool force) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                if (size > 0) (void)utils::exception::c_api::not_null(str, "string"); \
                store(settings_of(settings), id, (size > 0 ? cpptype(reinterpret_cast<const cpptype::value_type*>(str), size) : cpptype()), false, force); \
            }); \
        } \
        _nothrow int utils_settings_get_##name(const utils_settings_t* settings, const char* id, const ctype** str, size_t* size) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                const ctype*& res = utils::exception::c_api::not_null_ref(str, "string"); \
                const cpptype& stored = settings_of(settings).template get<cpptype>(utils::exception::c_api::not_null(id, "id")); \
                res = reinterpret_cast<const ctype*>(stored.c_str()); \
                if (size) *size = stored.size(); \
            }); \
        } \
        _nothrow bool utils_settings_is_##name(const utils_settings_t* settings, const char* id) \
        {return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return setting_is<cpptype>(settings, id);});}

    // Path of a setting as a C string (its native string is not a char* on Windows), valid until the next call in the thread
    _cold _nodiscard static const char* path_c(const std::filesystem::path& path)
    {
        static _thread_local std::string buffer;

        buffer = path.string();
        return buffer.c_str();
    }

//----------------------------------------------------------------//
/* STRUCT */

/* handles of the C API (the object itself) */
struct utils_argparser: public utils_handle, public utils::arguments::ArgParser {
    // ---------- Constructor --------- //
    utils_argparser(const std::string& binary, const std::string& description): utils::arguments::ArgParser(binary, description) { /* Nothing */ };
};

struct utils_argparser_result: public utils_handle {
    utils::arguments::ParsedUsages usages;
};

struct utils_settings: public utils_handle, public utils::arguments::Settings {};

#endif /* ARGUMENTS_CAPI_H */
