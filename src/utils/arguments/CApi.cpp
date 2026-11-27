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
##  C API of utils::arguments (ArgParser, Settings)
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/arguments/SettingsDefine.hpp"
#include "utils/arguments/ArgParserType.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/arguments/ArgParser.hpp"
#include "utils/arguments/Settings.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/arguments/CApi.hpp"
#include "utils/type/CApiTypes.hpp"
#include "utils/arguments/CApi.h"
#include <filesystem>
#include <functional>
#include <optional>
#include <iterator>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <tuple>

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ casts must stay the same */
static_assert(C_API_SETTINGS_CAST_NONE      == static_cast<std::size_t>(utils::arguments::CastType::None));
static_assert(C_API_SETTINGS_CAST_BYTE      == static_cast<std::size_t>(utils::arguments::CastType::Byte));
static_assert(C_API_SETTINGS_CAST_BOOL      == static_cast<std::size_t>(utils::arguments::CastType::Bool));
static_assert(C_API_SETTINGS_CAST_INT8      == static_cast<std::size_t>(utils::arguments::CastType::Int8));
static_assert(C_API_SETTINGS_CAST_INT64     == static_cast<std::size_t>(utils::arguments::CastType::Int64));
static_assert(C_API_SETTINGS_CAST_UINT8     == static_cast<std::size_t>(utils::arguments::CastType::UInt8));
static_assert(C_API_SETTINGS_CAST_UINT64    == static_cast<std::size_t>(utils::arguments::CastType::UInt64));
static_assert(C_API_SETTINGS_CAST_FLOAT16   == static_cast<std::size_t>(utils::arguments::CastType::Float16));
static_assert(C_API_SETTINGS_CAST_FLOAT128  == static_cast<std::size_t>(utils::arguments::CastType::Float128));
static_assert(C_API_SETTINGS_CAST_CHAR8     == static_cast<std::size_t>(utils::arguments::CastType::Char8));
static_assert(C_API_SETTINGS_CAST_U32STRING == static_cast<std::size_t>(utils::arguments::CastType::U32String));
static_assert(C_API_SETTINGS_CAST_WCHAR     == static_cast<std::size_t>(utils::arguments::CastType::WChar));
static_assert(C_API_SETTINGS_CAST_WSTRING   == static_cast<std::size_t>(utils::arguments::CastType::WString));
static_assert(C_API_SETTINGS_CAST_PATH      == static_cast<std::size_t>(utils::arguments::CastType::Path));

//----------------------------------------------------------------//
/* TOOLS */

// String of the C side (NULL: the default)
_cold _nodiscard static std::string text(const char* str, const char* fallback)
{
    return (str ? str : fallback);
}

// List of strings of the C side (count of them, NULL only with a count of 0)
_cold _nodiscard static std::vector<std::string> strings(const char* const* list, const std::size_t count)
{
    std::vector<std::string> res;

    if (count > 0) (void)utils::exception::c_api::not_null(list, "list");
    for (std::size_t i = 0; i < count; ++i)
        res.emplace_back(utils::exception::c_api::not_null(list[i], "string"));
    return res;
}

// Check of the C side in a check of the c++ (UTILS_OK: valid, else the info of utils_fail as the message)
_cold _nodiscard static std::function<std::optional<std::string>(const std::string&)> check_of(utils_argparser_check_t check, void* data)
{
    if (!check) return utils::arguments::defaultTrueParsingHook;
    return [check, data](const std::string& option) -> std::optional<std::string> {
        utils::exception::c_api::reset_status(); // only the error of this callback
        if (check(option.c_str(), data) == UTILS_OK) return std::nullopt;
        if (!utils_exception.exception) return "Invalid option (no info given with utils_fail)";
        return std::string(utils_exception.info[0] != '\0' ? utils_exception.info : utils_exception.what);
    };
}

// Default check of the c++ as a check of the C side
_cold _nodiscard static int check_with(std::optional<std::string> (*hook)(const std::string&), const char* option)
{
    std::optional<std::string> res;

    if (utils::exception::c_api::guard([&](void) {res = hook(utils::exception::c_api::not_null(option, "option"));}) != UTILS_OK) _unlikely {return UTILS_KO;}
    if (!res) return UTILS_OK;
    return utils_fail(static_cast<std::uint64_t>(utils::exception::InternalCode::InvalidArgument), res->c_str());
}

// Element i of a map of the parser (any order)
template<typename M>
_cold _nodiscard static const std::string& key_at(const M& map, const std::size_t i)
{
    if (i >= map.size()) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid index: " + std::to_string(i));
    }
    return std::next(map.begin(), static_cast<std::ptrdiff_t>(i))->first;
}

template<typename M>
_cold _nodiscard static const typename M::mapped_type& value_of(const M& map, const char* id)
{
    auto it = map.find(utils::exception::c_api::not_null(id, "id"));

    if (it == map.end()) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownId, id);
    }
    return it->second;
}

/* parsed result */
_cold _nodiscard static const utils::arguments::ParsedUsage& usage_at(const utils_argparser_result_t* result, const std::size_t i)
{
    return utils::exception::c_api::not_null_ref(result, "result").usages.at(i);
}

_cold _nodiscard static const std::tuple<std::string, bool, std::vector<std::string>>& argument_at(const utils_argparser_result_t* result, const std::size_t i, const std::size_t j)
{
    return usage_at(result, i).arguments.at(j);
}

/* settings */
_hot _nodiscard static utils::arguments::Settings& settings_of(utils_settings_t* settings)             {return utils::exception::c_api::not_null_ref(settings, "settings");}
_hot _nodiscard static const utils::arguments::Settings& settings_of(const utils_settings_t* settings) {return utils::exception::c_api::not_null_ref(settings, "settings");};

// Store a setting (add: an existing one is refused, else set with force or not)
template<typename T>
_cold static void store(utils::arguments::Settings& settings, const char* id, T&& value, const bool add, const bool force)
{
    const std::string key = utils::exception::c_api::not_null(id, "id");

    if (add) settings.add(key, std::forward<T>(value));
    else if (force) settings.template set<true>(key, std::forward<T>(value));
    else settings.template set<false>(key, std::forward<T>(value));
}

// Is the setting a T (unknown id: false)
template<typename T>
_hot _nodiscard static bool setting_is(const utils_settings_t* settings, const char* id)
{
    const utils::arguments::Settings& self = settings_of(settings);
    const char* key = utils::exception::c_api::not_null(id, "id");

    return self.contains(key) && self.get(key).template is<T>();
}

// Runtime cast of the C side (the template cast of the c++)
template<bool force>
_cold static void cast_runtime(utils::arguments::Settings& settings, const std::string& id, const std::string& setting, const utils_settings_cast_t type)
{
    switch (type) {
        case C_API_SETTINGS_CAST_NONE:      settings.template set<force>(id, setting); break;
        case C_API_SETTINGS_CAST_BYTE:      settings.template cast<utils::arguments::CastType::Byte, force>(id, setting); break;
        case C_API_SETTINGS_CAST_BOOL:      settings.template cast<utils::arguments::CastType::Bool, force>(id, setting); break;
        case C_API_SETTINGS_CAST_INT8:      settings.template cast<utils::arguments::CastType::Int8, force>(id, setting); break;
        case C_API_SETTINGS_CAST_INT16:     settings.template cast<utils::arguments::CastType::Int16, force>(id, setting); break;
        case C_API_SETTINGS_CAST_INT32:     settings.template cast<utils::arguments::CastType::Int32, force>(id, setting); break;
        case C_API_SETTINGS_CAST_INT64:     settings.template cast<utils::arguments::CastType::Int64, force>(id, setting); break;
        case C_API_SETTINGS_CAST_UINT8:     settings.template cast<utils::arguments::CastType::UInt8, force>(id, setting); break;
        case C_API_SETTINGS_CAST_UINT16:    settings.template cast<utils::arguments::CastType::UInt16, force>(id, setting); break;
        case C_API_SETTINGS_CAST_UINT32:    settings.template cast<utils::arguments::CastType::UInt32, force>(id, setting); break;
        case C_API_SETTINGS_CAST_UINT64:    settings.template cast<utils::arguments::CastType::UInt64, force>(id, setting); break;
        case C_API_SETTINGS_CAST_FLOAT16:   settings.template cast<utils::arguments::CastType::Float16, force>(id, setting); break;
        case C_API_SETTINGS_CAST_FLOAT32:   settings.template cast<utils::arguments::CastType::Float32, force>(id, setting); break;
        case C_API_SETTINGS_CAST_FLOAT64:   settings.template cast<utils::arguments::CastType::Float64, force>(id, setting); break;
        case C_API_SETTINGS_CAST_FLOAT128:  settings.template cast<utils::arguments::CastType::Float128, force>(id, setting); break;
        case C_API_SETTINGS_CAST_CHAR8:     settings.template cast<utils::arguments::CastType::Char8, force>(id, setting); break;
        case C_API_SETTINGS_CAST_CHAR16:    settings.template cast<utils::arguments::CastType::Char16, force>(id, setting); break;
        case C_API_SETTINGS_CAST_CHAR32:    settings.template cast<utils::arguments::CastType::Char32, force>(id, setting); break;
        case C_API_SETTINGS_CAST_U8STRING:  settings.template cast<utils::arguments::CastType::U8String, force>(id, setting); break;
        case C_API_SETTINGS_CAST_U16STRING: settings.template cast<utils::arguments::CastType::U16String, force>(id, setting); break;
        case C_API_SETTINGS_CAST_U32STRING: settings.template cast<utils::arguments::CastType::U32String, force>(id, setting); break;
        case C_API_SETTINGS_CAST_WCHAR:     settings.template cast<utils::arguments::CastType::WChar, force>(id, setting); break;
        case C_API_SETTINGS_CAST_WSTRING:   settings.template cast<utils::arguments::CastType::WString, force>(id, setting); break;
        case C_API_SETTINGS_CAST_PATH:      settings.template cast<utils::arguments::CastType::Path, force>(id, setting); break;
        default: throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownCast);
    }
}

//----------------------------------------------------------------//
/* ARGPARSER */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_argparser_t* utils_argparser_create(const char* binary, const char* description)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_argparser_t* {return new utils_argparser_t(text(binary, "[None]"), text(description, "..."));});
}

_cold _nothrow void utils_argparser_destroy(utils_argparser_t* parser)
{
    (void)utils::exception::c_api::guard([&](void) {delete parser;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_help(const utils_argparser_t* parser)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").help();});
}

_cold _nothrow utils_argparser_result_t* utils_argparser_parse(const utils_argparser_t* parser, int argc, const char* const* argv, bool failsafe)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_argparser_result_t* {
        const utils::arguments::ArgParser& self = utils::exception::c_api::not_null_ref(parser, "parser");
        utils_argparser_result_t* res = new utils_argparser_result_t();

        try {
            res->usages = self.parse(argc, utils::exception::c_api::not_null(argv, "argv"), failsafe);
        } catch (...) {
            delete res;
            throw;
        }
        return res;
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_set_default_usage(utils_argparser_t* parser)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").setDefaultUsage();});
}

_cold _nothrow int utils_argparser_set_usage(utils_argparser_t* parser, const char* id, const char* name, bool ordered, const char* const* ids, const bool* mandatory, size_t count, const char* description, bool force)
{
    return utils::exception::c_api::guard([&](void) {
        utils::arguments::ArgParser& self = utils::exception::c_api::not_null_ref(parser, "parser");
        const std::vector<std::string> list = strings(ids, count);
        std::vector<std::pair<std::string, bool>> usage;

        for (std::size_t i = 0; i < list.size(); ++i)
            usage.emplace_back(list[i], (mandatory ? mandatory[i] : false));
        if (force) self.setUsage<true>(utils::exception::c_api::not_null(id, "id"), utils::exception::c_api::not_null(name, "name"), ordered, usage, text(description, "[None]"));
        else self.setUsage<false>(utils::exception::c_api::not_null(id, "id"), utils::exception::c_api::not_null(name, "name"), ordered, usage, text(description, "[None]"));
    });
}

_cold _nothrow int utils_argparser_set_option(utils_argparser_t* parser, const char* id, const char* name, utils_argparser_check_t check, void* data, const char* description, bool force)
{
    return utils::exception::c_api::guard([&](void) {
        utils::arguments::ArgParser& self = utils::exception::c_api::not_null_ref(parser, "parser");
        const std::string key = utils::exception::c_api::not_null(id, "id");
        const std::string option = utils::exception::c_api::not_null(name, "name");

        if (!check && force) self.setOption<true>(key, option, text(description, "[None]"));
        else if (!check) self.setOption<false>(key, option, text(description, "[None]"));
        else if (force) self.setOption<true>(key, option, check_of(check, data), text(description, "[None]"));
        else self.setOption<false>(key, option, check_of(check, data), text(description, "[None]"));
    });
}

_cold _nothrow int utils_argparser_set_flag(utils_argparser_t* parser, const char* id, const char* short_flag, const char* flag, const char* long_flag, const char* env, const utils_argparser_flag_option_t* options, size_t count, const char* description, bool unlimited, bool ignore_case, bool force)
{
    return utils::exception::c_api::guard([&](void) {
        utils::arguments::ArgParser& self = utils::exception::c_api::not_null_ref(parser, "parser");
        const std::tuple<std::string, std::string, std::string, std::string> names = {text(short_flag, ""), text(flag, ""), text(long_flag, ""), text(env, "")};
        std::vector<std::tuple<std::string, bool, std::function<std::optional<std::string>(const std::string&)>>> list;

        if (count > 0) (void)utils::exception::c_api::not_null(options, "options");
        for (std::size_t i = 0; i < count; ++i)
            list.emplace_back(utils::exception::c_api::not_null(options[i].name, "option name"), options[i].mandatory, check_of(options[i].check, options[i].data));
        if (force) self.setFlag<true>(utils::exception::c_api::not_null(id, "id"), names, list, text(description, "[None]"), unlimited, ignore_case);
        else self.setFlag<false>(utils::exception::c_api::not_null(id, "id"), names, list, text(description, "[None]"), unlimited, ignore_case);
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_remove_usage(utils_argparser_t* parser, const char* id)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").removeUsage(utils::exception::c_api::not_null(id, "id"));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_remove_usages(utils_argparser_t* parser, const char* const* ids, size_t count)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").removeUsages(strings(ids, count));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_remove_option(utils_argparser_t* parser, const char* id)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").removeOption(utils::exception::c_api::not_null(id, "id"));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_remove_options(utils_argparser_t* parser, const char* const* ids, size_t count)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").removeOptions(strings(ids, count));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_remove_flag(utils_argparser_t* parser, const char* id)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").removeFlag(utils::exception::c_api::not_null(id, "id"));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_remove_flags(utils_argparser_t* parser, const char* const* ids, size_t count)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").removeFlags(strings(ids, count));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_reset_usages(utils_argparser_t* parser)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").resetUsages();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_reset_options(utils_argparser_t* parser)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").resetOptions();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_reset_flags(utils_argparser_t* parser)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").resetFlags();});
}

//----------------------------------------------------------------//
/* ARGPARSER HOOKS */

_cold _nothrow int utils_argparser_set_help_hook(utils_argparser_t* parser, utils_argparser_help_hook_t hook, void* data)
{
    return utils::exception::c_api::guard([&](void) {
        utils::arguments::ArgParser& self = utils::exception::c_api::not_null_ref(parser, "parser");

        (void)utils::exception::c_api::not_null(hook, "hook");
        self.setHelpHook([hook, data](const utils::arguments::ArgParser& p) {
            utils::exception::c_api::reset_status(); // only the error of this callback
            utils::exception::c_api::check_callback(hook(static_cast<const utils_argparser_t*>(&p), data)); // p is always the parser of a handle
        });
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_reset_help_hook(utils_argparser_t* parser)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").resetHelpHook();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_default_help_hook(const utils_argparser_t* parser)
{
    return utils::exception::c_api::guard([&](void) {utils::arguments::defaultHelpHook(utils::exception::c_api::not_null_ref(parser, "parser"));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_check_true(const char* option, _unused void* data)
{
    return check_with(utils::arguments::defaultTrueParsingHook, option);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_check_bool(const char* option, _unused void* data)
{
    return check_with(utils::arguments::defaultBoolParsingHook, option);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_check_int32(const char* option, _unused void* data)
{
    return check_with(utils::arguments::defaultInt32ParsingHook, option);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_check_sizet(const char* option, _unused void* data)
{
    return check_with(utils::arguments::defaultSizetParsingHook, option);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_check_double(const char* option, _unused void* data)
{
    return check_with(utils::arguments::defaultDoubleParsingHook, option);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_check_file(const char* option, _unused void* data)
{
    return check_with(utils::arguments::defaultFileParsingHook, option);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_check_directory(const char* option, _unused void* data)
{
    return check_with(utils::arguments::defaultDirectoryParsingHook, option);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_check_writable(const char* option, _unused void* data)
{
    return check_with(utils::arguments::defaultWritableParsingHook, option);
}

//----------------------------------------------------------------//
/* ARGPARSER SETTER / GETTER */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_disable_help(utils_argparser_t* parser)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").disableHelp();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_set_binary(utils_argparser_t* parser, const char* binary)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").setBinary(utils::exception::c_api::not_null(binary, "binary"));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_argparser_set_description(utils_argparser_t* parser, const char* description)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(parser, "parser").setDescription(utils::exception::c_api::not_null(description, "description"));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_argparser_get_binary(const utils_argparser_t* parser)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return utils::exception::c_api::not_null_ref(parser, "parser").getBinary().c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_argparser_get_description(const utils_argparser_t* parser)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return utils::exception::c_api::not_null_ref(parser, "parser").getDescription().c_str();});
}

//----------------------------------------------------------------//
/* ARGPARSER USAGES / OPTIONS / FLAGS */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_argparser_usages_count(const utils_argparser_t* parser)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(parser, "parser").getUsages().size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_argparser_usage_id(const utils_argparser_t* parser, size_t i)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return key_at(utils::exception::c_api::not_null_ref(parser, "parser").getUsages(), i).c_str();});
}

_cold _nothrow int utils_argparser_get_usage(const utils_argparser_t* parser, const char* id, const char** name, bool* ordered, const char** description)
{
    return utils::exception::c_api::guard([&](void) {
        const utils::arguments::Usage& usage = value_of(utils::exception::c_api::not_null_ref(parser, "parser").getUsages(), id);

        if (name) *name = usage.name.c_str();
        if (ordered) *ordered = usage.ordered;
        if (description) *description = usage.description.c_str();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_argparser_usage_ids_count(const utils_argparser_t* parser, const char* id)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return value_of(utils::exception::c_api::not_null_ref(parser, "parser").getUsages(), id).ids.size();});
}

_cold _nothrow int utils_argparser_usage_get_id(const utils_argparser_t* parser, const char* id, size_t i, const char** option_id, bool* mandatory)
{
    return utils::exception::c_api::guard([&](void) {
        const std::pair<std::string, bool>& entry = value_of(utils::exception::c_api::not_null_ref(parser, "parser").getUsages(), id).ids.at(i);

        if (option_id) *option_id = entry.first.c_str();
        if (mandatory) *mandatory = entry.second;
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_argparser_options_count(const utils_argparser_t* parser)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(parser, "parser").getOptions().size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_argparser_option_id(const utils_argparser_t* parser, size_t i)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return key_at(utils::exception::c_api::not_null_ref(parser, "parser").getOptions(), i).c_str();});
}

_cold _nothrow int utils_argparser_get_option(const utils_argparser_t* parser, const char* id, const char** name, bool* exact, const char** description)
{
    return utils::exception::c_api::guard([&](void) {
        const utils::arguments::Option& option = value_of(utils::exception::c_api::not_null_ref(parser, "parser").getOptions(), id);

        if (name) *name = option.name.c_str();
        if (exact) *exact = option.exact;
        if (description) *description = option.description.c_str();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_argparser_flags_count(const utils_argparser_t* parser)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(parser, "parser").getFlags().size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_argparser_flag_id(const utils_argparser_t* parser, size_t i)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return key_at(utils::exception::c_api::not_null_ref(parser, "parser").getFlags(), i).c_str();});
}

_cold _nothrow int utils_argparser_get_flag(const utils_argparser_t* parser, const char* id, const char** short_flag, const char** flag, const char** long_flag, const char** env, const char** description, bool* unlimited)
{
    return utils::exception::c_api::guard([&](void) {
        const utils::arguments::Flag& found = value_of(utils::exception::c_api::not_null_ref(parser, "parser").getFlags(), id);

        if (short_flag) *short_flag = std::get<0>(found.flag).c_str();
        if (flag) *flag = std::get<1>(found.flag).c_str();
        if (long_flag) *long_flag = std::get<2>(found.flag).c_str();
        if (env) *env = std::get<3>(found.flag).c_str();
        if (description) *description = found.description.c_str();
        if (unlimited) *unlimited = found.unlimited.first;
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_argparser_flag_options_count(const utils_argparser_t* parser, const char* id)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return value_of(utils::exception::c_api::not_null_ref(parser, "parser").getFlags(), id).options.size();});
}

_cold _nothrow int utils_argparser_flag_get_option(const utils_argparser_t* parser, const char* id, size_t i, const char** name, bool* mandatory)
{
    return utils::exception::c_api::guard([&](void) {
        const auto &[option, required, _] = value_of(utils::exception::c_api::not_null_ref(parser, "parser").getFlags(), id).options.at(i);

        if (name) *name = option.c_str();
        if (mandatory) *mandatory = required;
    });
}

//----------------------------------------------------------------//
/* ARGPARSER RESULT */

_cold _nothrow void utils_argparser_result_destroy(utils_argparser_result_t* result)
{
    (void)utils::exception::c_api::guard([&](void) {delete result;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_argparser_result_count(const utils_argparser_result_t* result)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(result, "result").usages.size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_argparser_result_usage(const utils_argparser_result_t* result, size_t i)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return usage_at(result, i).id.c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_argparser_result_arguments_count(const utils_argparser_result_t* result, size_t i)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return usage_at(result, i).arguments.size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_argparser_result_argument_id(const utils_argparser_result_t* result, size_t i, size_t j)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return std::get<0>(argument_at(result, i, j)).c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_argparser_result_argument_is_option(const utils_argparser_result_t* result, size_t i, size_t j)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return std::get<1>(argument_at(result, i, j));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_argparser_result_values_count(const utils_argparser_result_t* result, size_t i, size_t j)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return std::get<2>(argument_at(result, i, j)).size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow const char* utils_argparser_result_value(const utils_argparser_result_t* result, size_t i, size_t j, size_t k)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return std::get<2>(argument_at(result, i, j)).at(k).c_str();});
}

//----------------------------------------------------------------//
/* SETTINGS */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_settings_t* utils_settings_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_settings_t* {return new utils_settings_t();});
}

_cold _nothrow void utils_settings_destroy(utils_settings_t* settings)
{
    (void)utils::exception::c_api::guard([&](void) {delete settings;}); // like free(NULL)
}

_cold _nothrow int utils_settings_auto_cast(utils_settings_t* settings, const char* id, const char* setting, bool force, utils_settings_cast_t* type)
{
    return utils::exception::c_api::guard([&](void) {
        utils::arguments::Settings& self = settings_of(settings);
        const std::string key = utils::exception::c_api::not_null(id, "id");
        const std::string value = utils::exception::c_api::not_null(setting, "setting");
        const utils::arguments::CastType res = (force ? self.autoCast<true>(key, value) : self.autoCast<false>(key, value));

        if (type) *type = static_cast<utils_settings_cast_t>(res);
    });
}

_cold _nothrow int utils_settings_cast(utils_settings_t* settings, const char* id, const char* setting, utils_settings_cast_t type, bool force)
{
    return utils::exception::c_api::guard([&](void) {
        utils::arguments::Settings& self = settings_of(settings);
        const std::string key = utils::exception::c_api::not_null(id, "id");
        const std::string value = utils::exception::c_api::not_null(setting, "setting");

        if (force) cast_runtime<true>(self, key, value, type);
        else cast_runtime<false>(self, key, value, type);
    });
}

_cold _nothrow int utils_settings_remove(utils_settings_t* settings, const char* id, bool failsafe)
{
    return utils::exception::c_api::guard([&](void) {
        utils::arguments::Settings& self = settings_of(settings);
        const char* key = utils::exception::c_api::not_null(id, "id");

        if (failsafe) self.remove<true>(key);
        else self.remove<false>(key);
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_settings_clear(utils_settings_t* settings)
{
    return utils::exception::c_api::guard([&](void) {settings_of(settings).clear();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow bool utils_settings_contains(const utils_settings_t* settings, const char* id)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return settings_of(settings).contains(utils::exception::c_api::not_null(id, "id"));});
}

_cold _nothrow utils_settings_cast_t utils_settings_get_type(const utils_settings_t* settings, const char* id)
{
    return utils::exception::c_api::guard_fallback(C_API_SETTINGS_CAST_OTHER, [&](void) -> utils_settings_cast_t {
        const utils::arguments::Setting& setting = settings_of(settings).get(utils::exception::c_api::not_null(id, "id"));

        // the floats first (float16_t can be a float when <cstdfloat> is missing)
        if (setting.is<std::string>()) return C_API_SETTINGS_CAST_NONE;
        if (setting.is<std::byte>()) return C_API_SETTINGS_CAST_BYTE;
        if (setting.is<bool>()) return C_API_SETTINGS_CAST_BOOL;
        if (setting.is<std::int8_t>()) return C_API_SETTINGS_CAST_INT8;
        if (setting.is<std::int16_t>()) return C_API_SETTINGS_CAST_INT16;
        if (setting.is<std::int32_t>()) return C_API_SETTINGS_CAST_INT32;
        if (setting.is<std::int64_t>()) return C_API_SETTINGS_CAST_INT64;
        if (setting.is<std::uint8_t>()) return C_API_SETTINGS_CAST_UINT8;
        if (setting.is<std::uint16_t>()) return C_API_SETTINGS_CAST_UINT16;
        if (setting.is<std::uint32_t>()) return C_API_SETTINGS_CAST_UINT32;
        if (setting.is<std::uint64_t>()) return C_API_SETTINGS_CAST_UINT64;
        if (setting.is<utils::arguments::float32_t>()) return C_API_SETTINGS_CAST_FLOAT32;
        if (setting.is<utils::arguments::float64_t>()) return C_API_SETTINGS_CAST_FLOAT64;
        if (setting.is<utils::arguments::float128_t>()) return C_API_SETTINGS_CAST_FLOAT128;
        if (setting.is<utils::arguments::float16_t>()) return C_API_SETTINGS_CAST_FLOAT16;
        if (setting.is<char8_t>()) return C_API_SETTINGS_CAST_CHAR8;
        if (setting.is<char16_t>()) return C_API_SETTINGS_CAST_CHAR16;
        if (setting.is<char32_t>()) return C_API_SETTINGS_CAST_CHAR32;
        if (setting.is<std::u8string>()) return C_API_SETTINGS_CAST_U8STRING;
        if (setting.is<std::u16string>()) return C_API_SETTINGS_CAST_U16STRING;
        if (setting.is<std::u32string>()) return C_API_SETTINGS_CAST_U32STRING;
        if (setting.is<wchar_t>()) return C_API_SETTINGS_CAST_WCHAR;
        if (setting.is<std::wstring>()) return C_API_SETTINGS_CAST_WSTRING;
        if (setting.is<std::filesystem::path>()) return C_API_SETTINGS_CAST_PATH;
        return C_API_SETTINGS_CAST_OTHER;
    });
}

/* fixed types */
C_API_TYPES(C_API_SETTINGS_DEF)

/* types of the casts */
C_API_SETTINGS_CAST_DEF(byte, uint8_t, std::byte, static_cast<std::byte>(value), std::to_integer<std::uint8_t>(stored))
C_API_SETTINGS_CAST_DEF(char8, uint8_t, char8_t, static_cast<char8_t>(value), static_cast<std::uint8_t>(stored))
C_API_SETTINGS_CAST_DEF(char16, uint_least16_t, char16_t, static_cast<char16_t>(value), static_cast<uint_least16_t>(stored))
C_API_SETTINGS_CAST_DEF(char32, uint_least32_t, char32_t, static_cast<char32_t>(value), static_cast<uint_least32_t>(stored))
C_API_SETTINGS_CAST_DEF(wchar, wchar_t, wchar_t, value, stored)
C_API_SETTINGS_CAST_DEF(u8string, const char*, std::u8string, reinterpret_cast<const char8_t*>(utils::exception::c_api::not_null(value, "string")), reinterpret_cast<const char*>(stored.c_str()))
C_API_SETTINGS_CAST_DEF(path, const char*, std::filesystem::path, utils::exception::c_api::not_null(value, "path"), path_c(stored))
C_API_SETTINGS_STRING_DEF(u16string, uint_least16_t, std::u16string)
C_API_SETTINGS_STRING_DEF(u32string, uint_least32_t, std::u32string)
C_API_SETTINGS_STRING_DEF(wstring, wchar_t, std::wstring)

/* bytes */
_cold _nothrow int utils_settings_set_bytes(utils_settings_t* settings, const char* id, const void* data, size_t size, bool force)
{
    return utils::exception::c_api::guard([&](void) {
        const std::byte* begin = (size > 0 ? static_cast<const std::byte*>(utils::exception::c_api::not_null(data, "data")) : nullptr);

        store(settings_of(settings), id, std::vector<std::byte>(begin, begin + size), false, force);
    });
}

_cold _nothrow int utils_settings_get_bytes(const utils_settings_t* settings, const char* id, const void** data, size_t* size)
{
    return utils::exception::c_api::guard([&](void) {
        const void*& res = utils::exception::c_api::not_null_ref(data, "data");
        const std::vector<std::byte>& stored = settings_of(settings).get<std::vector<std::byte>>(utils::exception::c_api::not_null(id, "id"));

        res = stored.data();
        if (size) *size = stored.size();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_settings_is_bytes(const utils_settings_t* settings, const char* id)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return setting_is<std::vector<std::byte>>(settings, id);});
}
