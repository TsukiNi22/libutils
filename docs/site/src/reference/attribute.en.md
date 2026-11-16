+++
title = "Attribute"
summary = "Short macros for the compiler attributes, defined for each C++ (11 to 23) and C (99, 11, 23) version: an attribute the version can't use becomes empty."
keywords = "attribute macro nodiscard unused hot cold likely unlikely noinline deprecated migration legacy alignas packed thread_local restrict export import format FORCED_CXX_STANDARD FORCED_C_STANDARD"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Attribute` | `_Utils` | macros (no namespace) | `utils/attribute/` |

Short macros for the compiler attributes, defined from the language and its version, so the code stays the same everywhere: an attribute not supported by the version becomes empty.

## Versions {#versions}

`utils/attribute/Attribute.hpp` only chooses the file of the language:

| Compiler | File | Versions |
|---|---|---|
| C++ | `c++.hpp` | `c++11.hpp`, `c++14.hpp`, `c++17.hpp`, `c++20.hpp`, `c++23.hpp` (forced with `FORCED_CXX_STANDARD`) |
| C | `c.h` | `c99.h`, `c11.h`, `c23.h` (forced with `FORCED_C_STANDARD`) |
| neither / no GNU compiler | `fallback.hpp` | every macro empty |

`common.hpp` / `common.h` hold what every version shares: `_thread_local`, `_restrict`, `_alignas` and the fallback of `std::hardware_destructive_interference_size` / `std::hardware_constructive_interference_size` (`64`) when the standard library has none. From C++11 / C23 the macros use the `[[gnu::...]]` syntax.

## Groups {#groups}

| Purpose | Macros |
|---|---|
| attributes | `_nodiscard`, `_unused`, `_noreturn`, `_fallthrough`, `_deprecated(info)` |
| visibility | `_hidden`, `_export`, `_import` (`dllexport` / `dllimport` on Windows) |
| lifetime | `_ctor`, `_dtor` (run before / after `main`), `_cleanup(fn)` |
| inlining | `_always_inline`, `_noinline`, `_flatten` |
| optimisation | `_hot`, `_cold`, `_const`, `_pure`, `_assume(expr)`, `_unreachable()` |
| branch prediction | `_likely`, `_unlikely` (after the condition: `if (error) _unlikely {...}`), `_likely_c(c)`, `_unlikely_c(c)`, `_expect(c, v)` |
| diagnostics | `_format(archetype, i_fmt, i_args)`, `_sentinel` |
| binary layout | `_noaddress`, `_packed`, `_alignas(n)` |
| memory | `_malloc`, `_alloc_size(i)`, `_alloc_size_mul(i, j)`, `_nonnull(i)`, `_returns_nonnull`, `_read_only(ptr, size)`, `_write_only(ptr, size)`, `_restrict` |
| storage | `_thread_local` |

## Deprecation {#deprecation}

| Macro | Description |
|---|---|
| `_deprecated(info)` | `[[deprecated(info)]]` |
| `_migration(major, minor, fix)` | Renamed API kept until the given version (`~vX.Y.Z` in the warning) |
| `_legacy` | Kept for backward compatibility, no removal planned |

The deprecation warnings follow the [warning defines](page:warnings) (`_Warning` / `_NoWarning`). `_legacy` / `_migration` use the GNU syntax in every C and C++ version.

## Example {#example}

```cpp
#define _Attribute
#include <utils/utils.hpp>

_hot _nodiscard int parse(const char* s)
{
    if (!s) _unlikely {return -1;}
    return 0;
}

_migration(4, 0, 0) inline int oldParse(const char* s) {return parse(s);}
```

## In C {#in-c}

The same macros exist in C: `<utils.h>` with `_Attribute` (or `_Utils`) includes `utils/attribute/Attribute.hpp`, which picks `c.h`. An attribute of C++ only (`_likely`, `_unlikely`, `_noaddress`) is empty in C; use `_likely_c(c)` / `_unlikely_c(c)` in an expression.

```c
#define _Attribute
#include <utils.h>

_hot _nodiscard static int parse(const char* s)
{
    if (_unlikely_c(!s))
        return -1;
    return 0;
}

int main(void)
{
    return parse("x");
}
```
