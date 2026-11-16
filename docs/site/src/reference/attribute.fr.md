+++
title = "Attribute"
summary = "Macros courtes pour les attributs du compilateur, définies pour chaque version de C++ (11 à 23) et de C (99, 11, 23) : un attribut que la version ne peut pas utiliser devient vide."
keywords = "attribut macro nodiscard unused hot cold likely unlikely noinline deprecated migration legacy alignas packed thread_local restrict export import format FORCED_CXX_STANDARD FORCED_C_STANDARD"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_Attribute` | `_Utils` | macros (pas de namespace) | `utils/attribute/` |

Macros courtes pour les attributs du compilateur, définies selon le langage et sa version, pour que le code reste le même partout : un attribut non pris en charge par la version devient vide.

## Versions {#versions}

`utils/attribute/Attribute.hpp` choisit seulement le fichier du langage :

| Compilateur | Fichier | Versions |
|---|---|---|
| C++ | `c++.hpp` | `c++11.hpp`, `c++14.hpp`, `c++17.hpp`, `c++20.hpp`, `c++23.hpp` (forcée avec `FORCED_CXX_STANDARD`) |
| C | `c.h` | `c99.h`, `c11.h`, `c23.h` (forcée avec `FORCED_C_STANDARD`) |
| aucun des deux / pas de compilateur GNU | `fallback.hpp` | chaque macro vide |

`common.hpp` / `common.h` contiennent ce que toutes les versions partagent : `_thread_local`, `_restrict`, `_alignas` et le repli de `std::hardware_destructive_interference_size` / `std::hardware_constructive_interference_size` (`64`) quand la bibliothèque standard n'en a pas. À partir de C++11 / C23, les macros utilisent la syntaxe `[[gnu::...]]`.

## Groupes {#groups}

| Rôle | Macros |
|---|---|
| attributs | `_nodiscard`, `_unused`, `_noreturn`, `_fallthrough`, `_deprecated(info)` |
| visibilité | `_hidden`, `_export`, `_import` (`dllexport` / `dllimport` sous Windows) |
| durée de vie | `_ctor`, `_dtor` (exécutées avant / après `main`), `_cleanup(fn)` |
| inlining | `_always_inline`, `_noinline`, `_flatten` |
| optimisation | `_hot`, `_cold`, `_const`, `_pure`, `_assume(expr)`, `_unreachable()` |
| prédiction de branchement | `_likely`, `_unlikely` (après la condition : `if (error) _unlikely {...}`), `_likely_c(c)`, `_unlikely_c(c)`, `_expect(c, v)` |
| diagnostics | `_format(archetype, i_fmt, i_args)`, `_sentinel` |
| disposition binaire | `_noaddress`, `_packed`, `_alignas(n)` |
| mémoire | `_malloc`, `_alloc_size(i)`, `_alloc_size_mul(i, j)`, `_nonnull(i)`, `_returns_nonnull`, `_read_only(ptr, size)`, `_write_only(ptr, size)`, `_restrict` |
| stockage | `_thread_local` |

## Dépréciation {#deprecation}

| Macro | Description |
|---|---|
| `_deprecated(info)` | `[[deprecated(info)]]` |
| `_migration(major, minor, fix)` | API renommée gardée jusqu'à la version donnée (`~vX.Y.Z` dans l'avertissement) |
| `_legacy` | Gardée pour la compatibilité, aucune suppression prévue |

Les avertissements de dépréciation suivent les [defines d'avertissement](page:warnings) (`_Warning` / `_NoWarning`). `_legacy` / `_migration` utilisent la syntaxe GNU dans toutes les versions de C et de C++.

## Exemple {#example}

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

## En C {#in-c}

Les mêmes macros existent en C : `<utils.h>` avec `_Attribute` (ou `_Utils`) inclut `utils/attribute/Attribute.hpp`, qui choisit `c.h`. Un attribut propre au C++ (`_likely`, `_unlikely`, `_noaddress`) est vide en C ; utilisez `_likely_c(c)` / `_unlikely_c(c)` dans une expression.

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
