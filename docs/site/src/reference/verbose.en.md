+++
title = "Verbose"
summary = "A global verbose level and output macros that only print when the level is high enough, under one lock shared by every thread."
keywords = "verbose log level debug set_verbose onBasicVerbose onDebugVerbose LOCK_OUTPUT locked output lock utils_verbose_log"
+++

| Define | Group | Namespace | Header |
|---|---|---|---|
| `_Verbose` | `_Handling` | `utils::verbose` | `utils/verbose/Verbose.hpp` |

A global verbose level and macros that only print when the level is high enough. Every output is done under one recursive lock (`utils::verbose::output_lock`), so the lines of several threads never mix.

## Levels {#levels}

`utils::verbose::verbose` is a `std::atomic<utils::verbose::Verbose>` (default `Basic`): it can be changed while other threads log.

| Level | Value | Displayed |
|---|---|---|
| `None` | 0 | nothing |
| `Basic` | 1 | `onBasicVerbose` |
| `Advanced` | 2 | `onBasicVerbose`, `onAdvancedVerbose` |
| `Debug` | 3 | everything, the `onDebugVerbose` lines start with `debug: ` |

## Macros {#macros}

| Macro | Description |
|---|---|
| `set_verbose(level)` | Set the global level (`set_verbose(Debug)`) |
| `onBasicVerbose(info)` / `onAdvancedVerbose(info)` / `onDebugVerbose(info)` | Print `info` on `std::cout` (a stream expression: `"x = " << x`) |
| `onVerbose(level, info)` | Same with a level given at runtime |
| `onBasicVerboseC(output, info)` ... `onVerboseC(output, level, info)` | Same on another stream (`std::cerr`, a file...) |
| `onBasicVerboseFn(code)` ... `onVerboseFn(level, code)` | Run a block of code under the output lock when the level is reached |
| `LOCK_OUTPUT` | Lock the output for the current scope (raw writes mixed with the verbose ones) |
| `utils::verbose::locked(fn)` | Run `fn` with the output locked (used by every macro) |

> [!NOTE]
> The lock is recursive: a verbose macro can be called inside a `*Fn` block.

## Example {#example}

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

## In C {#in-c}

The same global level, with `printf` style logs locked like the C++ macros (the C and C++ logs of one program never mix).

| C++ | C |
|---|---|
| `set_verbose(Advanced)` / `verbose.load()` | `utils_verbose_set(C_API_VERBOSE_ADVANCED)` / `utils_verbose_get()` |
| `onVerbose(level, info)` | `utils_verbose_log(level, fmt, ...)` (`utils_verbose_vlog` with a `va_list`) |
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
