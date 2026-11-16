+++
title = "Arguments"
summary = "ArgParser vérifie et analyse argc / argv selon les usages du programme ; Settings stocke des réglages typés et convertit les chaînes dans le bon type."
keywords = "arguments argparser argv argc flag option usage parse aide settings cast autoCast CastType hook check utils_argparser utils_settings"
+++

| Define | Groupe | Espace de noms | En-têtes |
|---|---|---|---|
| `_Arguments` | `_Handling` | `utils::arguments` | `utils/arguments/` |

Deux outils : `ArgParser` vérifie et analyse `argc / argv` selon les usages du programme, `Settings` stocke des réglages typés (de n'importe quel type) et convertit les chaînes dans le bon type.

## ArgParser {#argparser}

`ArgParser(binary = "[None]", description = "...")` : le `binary` et la `description` sont utilisés par l'aide.

### Flags {#flags}

`setFlag<force>(id, {short, flag, long, env}, options, description, unlimited = false, ignore_case = false)`

| Partie | Reconnue comme | Exemple |
|---|---|---|
| `short` | `-<short>` (combinables : `-va`) | `"n"` → `-n` |
| `flag` | `-<flag>` | `"num"` → `-num` |
| `long` | `--<long>` (aussi `--long=value`) | `"number"` → `--number` |
| `env` | variable d'environnement lue quand le flag est absent | `"APP_NUMBER"` |

Une chaîne vide désactive une partie. `options` est la liste ordonnée des valeurs du flag, `{name, mandatory, check}` ; `unlimited = true` lit des valeurs jusqu'à ce que l'une d'elles échoue à son check (le flag doit alors avoir au moins une option).

### Options {#options}

Arguments positionnels :

| Méthode | Description |
|---|---|
| `setOption<force>(id, name, description)` | option **exacte** : l'argument doit être `name` (ex : une sous-commande `run`) |
| `setOption<force>(id, name, check, description)` | option libre validée par `check` |

### Usages {#usages}

`setUsage<force>(id, name, ordered, {{id, mandatory}, ...}, description)` : un usage liste les flags / options qu'il autorise et ceux qui sont obligatoires. Avec `ordered = true`, l'ordre des arguments compte. `setDefaultUsage()` ajoute l'usage spécial `default`, qui autorise tous les flags.

| Méthode | Description |
|---|---|
| `removeUsage(s)`, `removeOption(s)`, `removeFlag(s)` | Supprime par id (id inconnu : avertissement sur `std::cerr`) |
| `resetUsages()`, `resetOptions()`, `resetFlags()` | Supprime tous les éléments d'un genre |
| `getUsages()`, `getOptions()`, `getFlags()`, `getBinary()`, `getDescription()` | Accesseurs |

> [!NOTE]
> Les méthodes `set*` refusent d'écraser un id existant (`Override`), `set*<true>` l'écrase.

### Analyse {#parsing}

`parse(argc, argv, failsafe = false)` ou `parse(std::vector<std::string>, failsafe)` renvoie les **usages conformes** aux arguments (`ParsedUsages` = `std::vector<ParsedUsage>`) :

```cpp
struct ParsedUsage {
    std::string id; // usage id
    std::vector<std::tuple<std::string, bool, std::vector<std::string>>> arguments; // <id, option(true) | flag(false), values>
};
```

`-h`, `-help`, `--help` appellent le hook d'aide puis lancent `NoneException(Exit)` (désactivé par `disableHelp()`).

### Hooks {#hooks}

Les hooks de vérification ont la signature `std::optional<std::string>(const std::string&)` : `std::nullopt` = valide, une chaîne = le message d'erreur.

| Hook | Accepte |
|---|---|
| `defaultBoolParsingHook` | `0`, `1`, `false`, `true` |
| `defaultInt32ParsingHook` | un `std::int32_t` |
| `defaultSizetParsingHook` | un `std::size_t` |
| `defaultDoubleParsingHook` | un `double` |
| `defaultFileParsingHook` | un fichier lisible |
| `defaultDirectoryParsingHook` | un dossier lisible |
| `defaultWritableParsingHook` | un chemin lisible et accessible en écriture |
| `defaultTrueParsingHook` | n'importe quoi |

L'affichage de l'aide est `defaultHelpHook(parser)`, remplacé avec `setHelpHook(hook)`, rétabli avec `resetHelpHook()`, affiché avec `help()`.

### Exemple {#argparser-example}

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

Une map `id → Setting` (recherche avec un `std::string_view`, sans copie).

| Méthode | Description |
|---|---|
| `add(id, value)` | Ajoute un réglage, `Override` si l'id existe |
| `set<force = true>(id, value)` | Ajoute ou remplace (modification sur place) |
| `get<T>(id)` | Référence sur la valeur (`BadCast` quand le type diffère) |
| `get(id)`, `at(id)`, `operator[](id)` | Le `Setting` lui-même (`UnknownId` s'il est absent) |
| `contains(id)` | L'id est-il défini |
| `remove<failsafe = false>(id)` | Supprime (`UnknownId` s'il est absent, rien avec `failsafe`) |
| `clear()` | Supprime tout |
| `autoCast<force = false>(id, string)` | Détecte le type de la chaîne, stocke la valeur convertie, renvoie le `CastType` |
| `cast<CastType, force = false>(id, string)` | Convertit la chaîne dans le type donné et la stocke |

### Setting {#setting}

| Méthode | Description |
|---|---|
| `get<T>()` | Valeur en `T` (`BadCast` quand ce n'est pas un `T`) |
| `is<T>()` | La valeur stockée est-elle un `T` |
| `assign(value)` | Remplace la valeur (n'importe quel type) |
| `operator T()` | Conversion implicite (`int n = settings["n"];`) |

### Conversions {#casts}

| `CastType` | Type C++ | Détecté par `autoCast` |
|---|---|---|
| `None` | `std::string` | tout le reste |
| `Bool` | `bool` | `true`, `false`, `t`, `f` (toute casse) |
| `Int32` / `Int64` | `std::int32_t` / `std::int64_t` | entiers négatifs |
| `UInt32` / `UInt64` | `std::uint32_t` / `std::uint64_t` | entiers positifs |
| `Float64` / `Float128` | `float64_t` / `float128_t` | décimaux (`Float128` hors de la plage de `double`) |
| `Path` | `std::filesystem::path` | chaînes contenant un `/` |
| `Byte`, `Int8`, `Int16`, `UInt8`, `UInt16`, `Float16`, `Float32` | `std::byte`, entiers de taille fixe, flottants | seulement avec `cast` |
| `Char8`, `Char16`, `Char32`, `WChar` | `char8_t`, `char16_t`, `char32_t`, `wchar_t` | seulement avec `cast` (un caractère ou un point de code) |
| `U8String`, `U16String`, `U32String`, `WString` | `std::u8string`, `std::u16string`, `std::u32string`, `std::wstring` | seulement avec `cast` (depuis de l'UTF-8) |

> [!WARNING]
> Les conversions vérifient la plage : `"300"` converti en `UInt8` lance `BadCast`, une valeur hors de la plage de `float16` lance `OutOfBounds`.

### Exemple {#settings-example}

```cpp
utils::arguments::Settings settings;
settings.add("name", std::string("server"));
settings.autoCast("port", "4242");                                  // UInt32
settings.cast<utils::arguments::CastType::Path>("root", "/srv/www");
settings.get<std::uint32_t>("port") += 1;                            // edit in place
std::filesystem::path root = settings["root"];
```

## Erreurs {#errors}

| Code | Quand |
|---|---|
| `ArgumentsNumber` | `argv` est vide |
| `NoCompliantUsage` | Aucun usage ne correspond aux arguments |
| `UnknownFlag` | Flag inconnu |
| `FlagOptionsNumber`, `FlagOption` | Valeur d'un flag manquante / invalide |
| `OptionsNumber`, `Option`, `OptionIngored` | Option manquante / invalide / inattendue |
| `DuplicatedFlag`, `FlagCombinaison` | Flag donné deux fois, combinaison invalide |
| `ArgParserHook` | Un hook a lancé une exception |
| `Exit` (`NoneException`) | L'aide a été affichée |
| `Override`, `UnknownId`, `BadCast`, `OutOfBounds`, `InvalidArgument` | `Settings` |

## En C {#in-c}

| C++ | C |
|---|---|
| `ArgParser(binary, description)` | `utils_argparser_t*` : `utils_argparser_create(binary, description)` / `utils_argparser_destroy` |
| `setFlag(id, {short, flag, long, env}, options, ...)` | `utils_argparser_set_flag(parser, id, short, flag, long, env, options, count, description, unlimited, ignore_case, force)`, options en `utils_argparser_flag_option_t {name, mandatory, check, data}` |
| `setOption`, `setUsage`, `setDefaultUsage` | `utils_argparser_set_option`, `utils_argparser_set_usage(parser, id, name, ordered, ids, mandatory, count, description, force)`, `utils_argparser_set_default_usage` |
| un hook de vérification | `int check(const char* option, void* data)` : `UTILS_OK` = valide, `utils_fail(code, message)` = invalide ; ceux par défaut sont `utils_argparser_check_*` |
| `parse(argc, argv, failsafe)` | `utils_argparser_parse(parser, argc, argv, failsafe)` → `utils_argparser_result_t*` (`utils_argparser_result_*`, à détruire) |
| `Settings` | `utils_settings_t*` : `utils_settings_add_<name>`, `_set_<name>`, `_get_<name>`, `_is_<name>` par type fixe (`str` pour une chaîne), octets, `utils_settings_auto_cast` / `utils_settings_cast` (`C_API_SETTINGS_CAST_*`) |

L'aide est affichée par le hook d'aide, puis l'analyse échoue avec le code `Exit` (type `None`) : testez `utils_exception.type == C_API_EXCEPTION_TYPE_NONE`.

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
