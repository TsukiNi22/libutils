+++
title = "Exceptions"
summary = "Exceptions avec un type et un code, leur sortie formatée, tous les codes de la bibliothèque, et le statut de l'API C."
keywords = "exception erreur code type IException ErrorException FatalException NoneException WarningException CustomException formated InternalCode ExternalCode utils_exception statut"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_Exception` | `_Handling` | `utils::exception` | `utils/exception/` |

Chaque erreur de la bibliothèque est signalée par une exception portant un **type** (sa gravité) et un **code** (ce qui s'est passé), jamais par un code de retour. Pour ajouter les codes de votre projet, voir [Codes d'exception personnalisés](page:custom-exceptions) ; le modèle est expliqué dans [Erreurs](page:errors).

## Classes {#classes}

| Classe | Type | Description |
|---|---|---|
| `IException` | - | Interface de chaque exception de la bibliothèque (hérite de `std::exception`) |
| `AException` | - | Implémentation commune : code, type, info, emplacement dans le source, sortie formatée |
| `NoneException` | `None` | Pas une erreur, plutôt une sortie (ex : `-h` géré par `ArgParser`) |
| `WarningException` | `Warning` | Quelque chose s'est mal passé mais le programme peut continuer |
| `ErrorException` | `Error` | Erreur de l'appel |
| `CustomException` | donné | Exception avec le type donné par l'utilisateur (une combinaison de `Type`) |
| `FatalException` | `Fatal` | Affiche l'exception et **termine le programme avec `std::abort()` depuis son constructeur** (pas de nettoyage RAII) |

Chaque classe a les constructeurs `(code)` et `(code, info)` pour un `InternalCode` (bibliothèque) ou un `ExternalCode` (votre projet) ; l'emplacement dans le source est pris automatiquement.

| Méthode (`IException`) | Renvoie | Description |
|---|---|---|
| `what()` | `const char*` | Message du code (depuis la config JSON) |
| `info()` | `const char*` | Info donnée à la construction, celle par défaut du code, ou `"[None]"` |
| `getCode()` | `InternalCode` | Code de l'exception (un code externe est converti) |
| `getType()` | `Type` | Type de l'exception |
| `isNone()` / `isFatal()` | `bool` | Raccourci sur le type |
| `loc()` | `const std::source_location&` | Où l'exception a été construite |
| `formated()` | `std::string` | Sortie complète en couleur (voir [Sortie](#output)) |

`utils::exception::format(type, file, line, module, what, function, info)` construit la même sortie à partir de ses champs (utilisé par `formated()` et par l'API C).

## Type {#type}

`utils::exception::Type` est un ensemble de bits, combinés avec `|`, `&`, `^`, `~` (et leurs versions d'affectation) :

| Type | Valeur |
|---|---|
| `None` | `0b0001` |
| `Fatal` | `0b0010` |
| `Error` | `0b0100` |
| `Warning` | `0b1000` |

> [!IMPORTANT]
> Un code peut restreindre les types qu'il accepte (colonne **Restrictions** des [codes](#codes)). Construire une exception avec un type interdit se termine par une `FatalException(ExceptionCodeRestriction)`.

## Lever et attraper {#throw-catch}

```cpp
#define _Exception
#include <utils/utils.hpp>

try {
    throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "the size must be > 0");
} catch (const utils::exception::IException& e) {
    if (e.getCode() == utils::exception::InternalCode::InvalidArgument)
        std::cerr << e.formated() << std::endl;
}

throw utils::exception::NoneException(utils::exception::InternalCode::Exit);                                // not an error
throw utils::exception::CustomException(utils::exception::Type::Error | utils::exception::Type::Warning, "info");
utils::exception::FatalException(utils::exception::InternalCode::Mmap, "shm");                              // prints & aborts
```

> [!TIP]
> Beaucoup de fonctions de la bibliothèque prennent un booléen `failsafe` : quand il vaut `true`, l'erreur est affichée comme une `WarningException` sur `std::cerr` au lieu d'être levée.

## Sortie {#output}

`std::cerr << e.formated() << std::endl;`

```text
[<type>] ...<file_path:hyperlink>:<line> (<origin>) -> <message>
-------------------------------------------
<function_demangled> = <info>
-------------------------------------------
```

## En C {#in-c}

L'API C ne laisse jamais une exception traverser la frontière C : chaque appel est protégé, renvoie `UTILS_OK` / `UTILS_KO` (ou `NULL` / `0`) et, en cas d'échec, remplit le **statut** thread local `utils_exception` (lu comme `errno`). Le guide complet est [Utiliser libutils depuis C](page:c-api).

| C++ | C |
|---|---|
| `catch (const IException& e)` | `if (ret != UTILS_OK)` puis lire `utils_exception` |
| `e.getType()` / `e.getCode()` | `utils_exception.type` / `utils_exception.code` (`C_API_OTHER_EXCEPTION_CODE` pour une exception qui ne vient pas de libutils) |
| `e.what()` / `e.info()` | `utils_exception.what` / `utils_exception.info` |
| `e.loc()` | `utils_exception.file`, `.line`, `.function`, `.module` |
| `e.formated()` | `utils_exception_format()` / `utils_exception_format_r(buf, size)` |
| `throw ErrorException(code, info)` dans un callback | `return utils_fail(code, info);` |

`utils_exception.exception` vaut `true` quand le **dernier** appel a échoué ; `origin` indique d'où vient l'erreur (`C_API_EXCEPTION_ORIGIN_INTERNAL`, `_EXTERNAL`, `_STD`, `_UNKNOWN`, `_C`).

```c
#define _Exception
#define _Verbose
#include <utils.h>
#include <stdio.h>

int main(void)
{
    if (utils_verbose_log(C_API_VERBOSE_BASIC, "%s", "hello") != UTILS_OK) {
        fprintf(stderr, "%s\n", utils_exception_format()); // same output as formated()
        return 1;
    }
    utils_fail(42, "set from C");                         // origin C_API_EXCEPTION_ORIGIN_C, returns UTILS_KO
    printf("%llu %s\n", (unsigned long long)utils_exception.code, utils_exception.info); // 42 set from C
    utils_exception_clear();
    return 0;
}
```
