+++
title = "Arguments"
summary = "ArgParser checks and parses argc / argv against the usages of the program; Settings stores typed settings and converts strings to the right type."
keywords = "arguments argparser argv argc flag option usage parse help settings cast autoCast CastType hook check utils_argparser utils_settings"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Arguments` | `_Handling` | `utils::arguments` | `utils/arguments/` |

Two tools: `ArgParser` checks and parses `argc / argv` against the usages of the program, `Settings` stores typed settings (any type) and converts strings to the right type.

## ArgParser {#argparser}

`ArgParser(binary = "[None]", description = "...")`: the `binary` and the `description` are used by the help.

### Flags {#flags}

`setFlag<force>(id, {short, flag, long, env}, options, description, unlimited = false, ignore_case = false)`

| Part | Matched as | Example |
|---|---|---|
| `short` | `-<short>` (combinable: `-va`) | `"n"` → `-n` |
| `flag` | `-<flag>` | `"num"` → `-num` |
| `long` | `--<long>` (also `--long=value`) | `"number"` → `--number` |
| `env` | environment variable read when the flag is missing | `"APP_NUMBER"` |

An empty string disables a part. `options` is the ordered list of the values of the flag, `{name, mandatory, check}`; `unlimited = true` reads values until one fails its check (the flag needs at least one option).

### Options {#options}

Positional arguments:

| Method | Description |
|---|---|
| `setOption<force>(id, name, description)` | **exact** option: the argument must be `name` (ex: a sub command `run`) |
| `setOption<force>(id, name, check, description)` | free option validated by `check` |

### Usages {#usages}

`setUsage<force>(id, name, ordered, {{id, mandatory}, ...}, description)`: a usage lists the flags / options it allows and the mandatory ones. With `ordered = true` the order of the arguments matters. `setDefaultUsage()` adds the special usage `default` that allows every flag.

| Method | Description |
|---|---|
| `removeUsage(s)`, `removeOption(s)`, `removeFlag(s)` | Remove by id (unknown id: warning on `std::cerr`) |
| `resetUsages()`, `resetOptions()`, `resetFlags()` | Remove everything of a kind |
| `getUsages()`, `getOptions()`, `getFlags()`, `getBinary()`, `getDescription()` | Getters |

> [!NOTE]
> The `set*` methods refuse to override an existing id (`Override`), `set*<true>` overrides it.

### Parsing {#parsing}

`parse(argc, argv, failsafe = false)` or `parse(std::vector<std::string>, failsafe)` returns the **usages compliant** with the arguments (`ParsedUsages` = `std::vector<ParsedUsage>`):

```cpp
struct ParsedUsage {
    std::string id; // usage id
    std::vector<std::tuple<std::string, bool, std::vector<std::string>>> arguments; // <id, option(true) | flag(false), values>
};
```

`-h`, `-help`, `--help` call the help hook then throw `NoneException(Exit)` (disabled by `disableHelp()`).

### Hooks {#hooks}

Check hooks have the signature `std::optional<std::string>(const std::string&)`: `std::nullopt` = valid, a string = the error message.

| Hook | Accepts |
|---|---|
| `defaultBoolParsingHook` | `0`, `1`, `false`, `true` |
| `defaultInt32ParsingHook` | a `std::int32_t` |
| `defaultSizetParsingHook` | a `std::size_t` |
| `defaultDoubleParsingHook` | a `double` |
| `defaultFileParsingHook` | a readable file |
| `defaultDirectoryParsingHook` | a readable directory |
| `defaultWritableParsingHook` | a readable and writable path |
| `defaultTrueParsingHook` | anything |

The help display is `defaultHelpHook(parser)`, replaced with `setHelpHook(hook)`, back with `resetHelpHook()`, displayed with `help()`.

### Example {#argparser-example}

```cpp
#define _Arguments
#include <utils/utils.hpp>

int main(int argc, char* argv[])
{
    utils::arguments::ArgParser parser("app", "Process some numbers");
    parser.setFlag("number", {"n", "num", "number", "APP_NUMBER"}, {{"value", true, utils::arguments::defaultInt32ParsingHook}}, "A number");
    parser.setFlag("verbose", {"v", "", "verbose", ""}, {}, "Verbose mode");
    parser.setUsage("main", "main", false, {{"number", true}, {"verbose", false}}, "Main usage");

    try {
        for (const auto& [id, arguments]: parser.parse(argc, argv))
            for (const auto& [arg, isOption, values]: arguments)
                std::cout << id << ": " << arg << " " << values.size() << " value(s)" << std::endl;
    } catch (const utils::exception::IException& e) {
        if (e.isNone()) return 0; // -h
        std::cerr << e.formated() << std::endl;
        return 1;
    }
    return 0;
}
```

## Settings {#settings}

A map `id → Setting` (lookup with a `std::string_view`, without copy).

| Method | Description |
|---|---|
| `add(id, value)` | Add a setting, `Override` if the id exists |
| `set<force = true>(id, value)` | Add or replace (edit in place) |
| `get<T>(id)` | Reference on the value (`BadCast` when the type differs) |
| `get(id)`, `at(id)`, `operator[](id)` | The `Setting` itself (`UnknownId` when missing) |
| `contains(id)` | Is the id set |
| `remove<failsafe = false>(id)` | Remove (`UnknownId` when missing, nothing with `failsafe`) |
| `clear()` | Remove everything |
| `autoCast<force = false>(id, string)` | Detect the type of the string, store the converted value, return the `CastType` |
| `cast<CastType, force = false>(id, string)` | Convert the string to the given type and store it |

### Setting {#setting}

| Method | Description |
|---|---|
| `get<T>()` | Value as `T` (`BadCast` when it isn't a `T`) |
| `is<T>()` | Is the stored value a `T` |
| `assign(value)` | Replace the value (any type) |
| `operator T()` | Implicit conversion (`int n = settings["n"];`) |

### Casts {#casts}

| `CastType` | C++ type | Detected by `autoCast` |
|---|---|---|
| `None` | `std::string` | anything else |
| `Bool` | `bool` | `true`, `false`, `t`, `f` (any case) |
| `Int32` / `Int64` | `std::int32_t` / `std::int64_t` | negative integers |
| `UInt32` / `UInt64` | `std::uint32_t` / `std::uint64_t` | positive integers |
| `Float64` / `Float128` | `float64_t` / `float128_t` | decimals (`Float128` when out of the `double` range) |
| `Path` | `std::filesystem::path` | strings with a `/` |
| `Byte`, `Int8`, `Int16`, `UInt8`, `UInt16`, `Float16`, `Float32` | `std::byte`, fixed size integers, floats | only with `cast` |
| `Char8`, `Char16`, `Char32`, `WChar` | `char8_t`, `char16_t`, `char32_t`, `wchar_t` | only with `cast` (a character or a code point) |
| `U8String`, `U16String`, `U32String`, `WString` | `std::u8string`, `std::u16string`, `std::u32string`, `std::wstring` | only with `cast` (from UTF-8) |

> [!WARNING]
> The conversions check the range: `"300"` cast to `UInt8` throws `BadCast`, a value out of the `float16` range throws `OutOfBounds`.

### Example {#settings-example}

```cpp
utils::arguments::Settings settings;
settings.add("name", std::string("server"));
settings.autoCast("port", "4242");                                  // UInt32
settings.cast<utils::arguments::CastType::Path>("root", "/srv/www");
settings.get<std::uint32_t>("port") += 1;                            // edit in place
std::filesystem::path root = settings["root"];
```

## Errors {#errors}

| Code | When |
|---|---|
| `ArgumentsNumber` | `argv` is empty |
| `NoCompliantUsage` | No usage matches the arguments |
| `UnknownFlag` | Unknown flag |
| `FlagOptionsNumber`, `FlagOption` | Missing / invalid value of a flag |
| `OptionsNumber`, `Option`, `OptionIngored` | Missing / invalid / unexpected option |
| `DuplicatedFlag`, `FlagCombinaison` | Flag given twice, invalid combination |
| `ArgParserHook` | A hook threw |
| `Exit` (`NoneException`) | The help was displayed |
| `Override`, `UnknownId`, `BadCast`, `OutOfBounds`, `InvalidArgument` | `Settings` |

## In C {#in-c}

| C++ | C |
|---|---|
| `ArgParser(binary, description)` | `utils_argparser_t*`: `utils_argparser_create(binary, description)` / `utils_argparser_destroy` |
| `setFlag(id, {short, flag, long, env}, options, ...)` | `utils_argparser_set_flag(parser, id, short, flag, long, env, options, count, description, unlimited, ignore_case, force)`, options as `utils_argparser_flag_option_t {name, mandatory, check, data}` |
| `setOption`, `setUsage`, `setDefaultUsage` | `utils_argparser_set_option`, `utils_argparser_set_usage(parser, id, name, ordered, ids, mandatory, count, description, force)`, `utils_argparser_set_default_usage` |
| a check hook | `int check(const char* option, void* data)`: `UTILS_OK` = valid, `utils_fail(code, message)` = invalid; the default ones are `utils_argparser_check_*` |
| `parse(argc, argv, failsafe)` | `utils_argparser_parse(parser, argc, argv, failsafe)` → `utils_argparser_result_t*` (`utils_argparser_result_*`, destroy it) |
| `Settings` | `utils_settings_t*`: `utils_settings_add_<name>`, `_set_<name>`, `_get_<name>`, `_is_<name>` per fixed type (`str` for a string), bytes, `utils_settings_auto_cast` / `utils_settings_cast` (`C_API_SETTINGS_CAST_*`) |

The help is displayed by the help hook, then the parsing fails with the code `Exit` (type `None`): test `utils_exception.type == C_API_EXCEPTION_TYPE_NONE`.

```c
#define _Arguments
#include <utils.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    utils_argparser_t* parser = utils_argparser_create("app", "Process some numbers");
    utils_argparser_flag_option_t value = {"value", true, utils_argparser_check_int32, NULL};
    const char* ids[] = {"number", "verbose"};
    const bool mandatory[] = {true, false};
    utils_argparser_result_t* result = NULL;

    if (!parser)
        return 1;
    utils_argparser_set_flag(parser, "number", "n", "num", "number", "APP_NUMBER", &value, 1, "A number", false, false, false);
    utils_argparser_set_flag(parser, "verbose", "v", NULL, "verbose", NULL, NULL, 0, "Verbose mode", false, false, false);
    utils_argparser_set_usage(parser, "main", "main", false, ids, mandatory, 2, "Main usage", false);
    result = utils_argparser_parse(parser, argc, (const char* const*)argv, false);
    if (!result) {
        utils_argparser_destroy(parser);
        return utils_exception.type == C_API_EXCEPTION_TYPE_NONE ? 0 : 1; // -h: not an error
    }
    for (size_t i = 0; i < utils_argparser_result_count(result); ++i)
        for (size_t j = 0; j < utils_argparser_result_arguments_count(result, i); ++j)
            printf("%s: %s %zu value(s)\n", utils_argparser_result_usage(result, i),
                utils_argparser_result_argument_id(result, i, j), utils_argparser_result_values_count(result, i, j));
    utils_argparser_result_destroy(result);
    utils_argparser_destroy(parser);
    return 0;
}
```
