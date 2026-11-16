+++
title = "Verbose"
summary = "Un niveau de verbosité global et des macros de sortie qui n'affichent que si le niveau est assez élevé, sous un seul verrou partagé par tous les threads."
keywords = "verbose log niveau debug set_verbose onBasicVerbose onDebugVerbose LOCK_OUTPUT locked sortie verrou utils_verbose_log"
+++

| Define | Groupe | Namespace | Header |
|---|---|---|---|
| `_Verbose` | `_Handling` | `utils::verbose` | `utils/verbose/Verbose.hpp` |

Un niveau de verbosité global et des macros qui n'affichent que si le niveau est assez élevé. Chaque sortie se fait sous un seul verrou récursif (`utils::verbose::output_lock`), donc les lignes de plusieurs threads ne se mélangent jamais.

## Niveaux {#levels}

`utils::verbose::verbose` est un `std::atomic<utils::verbose::Verbose>` (`Basic` par défaut) : il peut être changé pendant que d'autres threads écrivent des logs.

| Niveau | Valeur | Affiché |
|---|---|---|
| `None` | 0 | rien |
| `Basic` | 1 | `onBasicVerbose` |
| `Advanced` | 2 | `onBasicVerbose`, `onAdvancedVerbose` |
| `Debug` | 3 | tout, les lignes `onDebugVerbose` commencent par `debug: ` |

## Macros {#macros}

| Macro | Description |
|---|---|
| `set_verbose(level)` | Définit le niveau global (`set_verbose(Debug)`) |
| `onBasicVerbose(info)` / `onAdvancedVerbose(info)` / `onDebugVerbose(info)` | Affiche `info` sur `std::cout` (une expression de flux : `"x = " << x`) |
| `onVerbose(level, info)` | Pareil avec un niveau donné à l'exécution |
| `onBasicVerboseC(output, info)` ... `onVerboseC(output, level, info)` | Pareil sur un autre flux (`std::cerr`, un fichier...) |
| `onBasicVerboseFn(code)` ... `onVerboseFn(level, code)` | Exécute un bloc de code sous le verrou de sortie quand le niveau est atteint |
| `LOCK_OUTPUT` | Verrouille la sortie pour la portée courante (écritures brutes mêlées à celles du verbose) |
| `utils::verbose::locked(fn)` | Exécute `fn` avec la sortie verrouillée (utilisé par chaque macro) |

> [!NOTE]
> Le verrou est récursif : une macro verbose peut être appelée dans un bloc `*Fn`.

## Exemple {#example}

```cpp
#define _Verbose
#include <utils/utils.hpp>

int main(void)
{
    set_verbose(Advanced);
    onBasicVerbose("Starting " << 3 << " workers...");
    onDebugVerbose("never displayed (level Advanced)");
    onAdvancedVerboseC(std::cerr, "on the error output");
    onBasicVerboseFn(
        for (int i = 0; i < 3; ++i)
            std::cout << i << std::endl;
    );
    return 0;
}
```

## En C {#in-c}

Le même niveau global, avec des logs à la `printf` verrouillés comme les macros C++ (les logs C et C++ d'un même programme ne se mélangent jamais).

| C++ | C |
|---|---|
| `set_verbose(Advanced)` / `verbose.load()` | `utils_verbose_set(C_API_VERBOSE_ADVANCED)` / `utils_verbose_get()` |
| `onVerbose(level, info)` | `utils_verbose_log(level, fmt, ...)` (`utils_verbose_vlog` avec une `va_list`) |
| `onVerboseC(output, level, info)` | `utils_verbose_log_to(FILE*, level, fmt, ...)` |
| `utils::verbose::locked(fn)` | `utils_verbose_locked(fn, data)` |
| `onVerboseFn(level, code)` | `utils_verbose_on(level, fn, data)` |

```c
#define _Verbose
#include <utils.h>
#include <stdio.h>

static int dump(void* data)
{
    printf("%d workers\n", *(int*)data); // the output is locked while fn runs
    return UTILS_OK;
}

int main(void)
{
    int workers = 3;

    utils_verbose_set(C_API_VERBOSE_ADVANCED);
    utils_verbose_log(C_API_VERBOSE_BASIC, "Starting %d workers...", workers);
    utils_verbose_log(C_API_VERBOSE_DEBUG, "never displayed (level ADVANCED)");
    utils_verbose_log_to(stderr, C_API_VERBOSE_ADVANCED, "on the error output");
    return utils_verbose_on(C_API_VERBOSE_BASIC, dump, &workers) == UTILS_OK ? 0 : 1;
}
```
