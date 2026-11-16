+++
title = "Conception de l'API C"
summary = "Comment l'API C est construite sur les classes C++ : la garde, les handles et leur base commune, les callbacks, les chaînes, les types fixes des templates, et ce qui n'est pas exposé."
keywords = "api c conception garde guard_fallback not_null handle utils_handle magic handle_base dynamic_cast callback check_callback thread local copy_r X-macro types fixes CApiTypes Storage cpp_t"
+++

L'API C est une fine couche sur les classes C++, écrite en C++ : chaque section a un header C (`CApi.h`), un header C++ pour ses handles et ses macros (`CApi.hpp`) et les fonctions (`src/utils/<section>/CApi.cpp`). Cette page explique ses règles, pour un lecteur du code ou un contributeur ; le guide utilisateur est [Utiliser libutils depuis C](page:c-api).

:::html
<figure>
<svg viewBox="0 0 760 270" role="img" aria-label="A C program calls the C functions, each one runs the C++ classes in a guard; a failure fills the thread local status utils_exception; a C callback called by the C++ code returns UTILS_OK or utils_fail">
<defs>
<marker id="arrc-fig3-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
<marker id="arrca-fig3-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow-accent" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box" x="20" y="40" width="150" height="50" rx="8"/>
<text class="lbl" x="95" y="70" text-anchor="middle">C program</text>
<rect class="box-alt" x="210" y="40" width="160" height="50" rx="8"/>
<text class="lbl" x="290" y="62" text-anchor="middle">utils.h · CApi.h</text>
<text class="small" x="290" y="80" text-anchor="middle">extern "C" functions</text>
<rect class="box-alt" x="410" y="40" width="150" height="50" rx="8"/>
<text class="lbl" x="485" y="62" text-anchor="middle">guard</text>
<text class="small" x="485" y="80" text-anchor="middle">try / catch</text>
<rect class="box" x="600" y="40" width="140" height="50" rx="8"/>
<text class="lbl" x="670" y="62" text-anchor="middle">C++ classes</text>
<text class="small" x="670" y="80" text-anchor="middle">in the handles</text>
<path class="edge" d="M170 65H206" marker-end="url(#arrc-fig3-fr)"/>
<path class="edge" d="M370 65H406" marker-end="url(#arrc-fig3-fr)"/>
<path class="edge" d="M560 65H596" marker-end="url(#arrc-fig3-fr)"/>
<rect class="group" x="330" y="140" width="230" height="44" rx="8"/>
<text class="lbl" x="445" y="167" text-anchor="middle">utils_exception (thread local)</text>
<path class="edge dash" d="M485 90V136" marker-end="url(#arrc-fig3-fr)"/>
<text class="small" x="492" y="118">exception</text>
<path class="edge dash" d="M330 162H95V94" marker-end="url(#arrc-fig3-fr)"/>
<text class="small" x="105" y="154">UTILS_KO / NULL, read like errno</text>
<rect class="box" x="20" y="210" width="150" height="44" rx="8"/>
<text class="lbl" x="95" y="237" text-anchor="middle">C callback</text>
<path class="edge-accent" d="M690 90V232H174" marker-end="url(#arrca-fig3-fr)"/>
<text class="small" x="430" y="224" text-anchor="middle">fn(..., data) → UTILS_OK / utils_fail(code, info)</text>
</svg>

<figcaption>Les couches d'un appel : la fonction C, son guard, l'objet C++ tenu par le handle ; le statut et les callbacks reviennent vers le C.</figcaption>
</figure>
:::

## La garde {#guard}

`utils/exception/CApi.hpp` donne les outils utilisés par chaque fonction C (namespace `utils::exception::c_api`) :

| Outil | Rôle |
|---|---|
| `guard(f)` | Lance `f`, attrape tout, remplit `utils_exception` en cas d'échec ; renvoie `UTILS_OK` / `UTILS_KO` |
| `guard_fallback(fallback, f)` | Renvoie le résultat de `f`, ou `fallback` quand elle a levé une exception (`NULL`, `0`, `false`...) |
| `not_null(ptr, name)` / `not_null_ref(ptr, name)` | Un `NULL` venu du côté C devient `InvalidArgument` "The &lt;name&gt; is NULL", situé chez l'appelant |
| `set_status(e)` / `clear_status()` | Remplit / réinitialise le statut du thread |

```cpp "src/utils/cli/CApi.cpp"
_cold uint8_t utils_cli_get_code(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> uint8_t {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.getCode();});
}
```

En cas de succès, seul `utils_exception.exception = false` est écrit : les chaînes d'un ancien échec restent, et le chemin rapide ne copie rien. Le statut est `thread_local` (`_thread_local`), comme `errno`.

## Handles {#handles}

Un handle est une struct C++ déclarée opaque pour C (`typedef struct utils_cli utils_cli_t;`) qui contient l'objet (`utils_cli { utils::cli::Cli cli; ... }`). Chaque handle dérive de `utils_handle`, comme **première** base :

| Membre | Rôle |
|---|---|
| `magic` | `C_API_HANDLE_MAGIC` tant que le handle vit, `0` une fois détruit |
| destructeur virtuel | donne au handle un type connu à l'exécution (RTTI) |

`handle_base<T>(void* handle, name)` atteint une base partagée par plusieurs classes depuis n'importe quel handle : il vérifie le magic ("The pointer is not a handle of the C API"), puis fait un `dynamic_cast` vers `T` ("This handle has no &lt;name&gt;"). Les fonctions C génériques sont construites dessus : `utils_freezable_*` (`utils::type::Freezable`) et `utils_worker_*` (`utils::type::Worker`) acceptent le handle de n'importe quelle classe qui en dérive.

`utils_<name>_destroy(NULL)` ne fait rien (comme `free`). Un handle qui garde un thread (la cli avec `THREAD`) le joint dans son destroy.

## Callbacks {#callbacks}

Un callback C est un pointeur de fonction et un `void* data`, enveloppés dans une `std::function` côté C++. Autour de chaque appel :

1. `reset_status()` efface le flag d'erreur, pour que le statut ne contienne que l'erreur du callback ;
2. le callback tourne et renvoie un `int` ;
3. `check_callback(ret)` : tout autre chose que `UTILS_OK` lève une exception, avec l'info donnée par `utils_fail` (ou "C callback failed (no info given with utils_fail)").

L'exception remonte ensuite le code C++ comme n'importe quelle autre (une erreur de hook de la cli, `MiddlewareCall` d'un middleware, le message d'une vérification d'argument) et finit dans le statut de l'appel C qui l'a lancé.

## Chaînes {#strings}

`utils/manip/smanip/CApi.hpp` donne `copy` (dans un buffer fixe, tronqué) et `copy_r` (la convention `_r` : tronqué, toujours terminé par `'\0'`, renvoie la longueur complète comme `snprintf`), et `bytes` les octets d'un pointeur + une taille du côté C dans un `std::string` (`NULL` seulement avec une taille de 0). Une fonction qui construit une chaîne a deux formes : la forme simple renvoie un buffer `thread_local std::string` de la fonction (valide jusqu'à son prochain appel dans le thread), la forme `_r` écrit dans un buffer de l'appelant. Les octets qui peuvent contenir `'\0'` utilisent un pointeur + une taille, ou un `utils_string_t*`.

## Types fixes {#fixed-types}

C n'a pas de templates : un template est généré pour une liste de **types fixes** par les X-macros de `utils/type/CApiTypes.h`, et a une version générique en octets (data + size) pour tout le reste.

| X-macro | Types (`name` : type C) |
|---|---|
| `C_API_INT_TYPES` | `i8`, `i16`, `i32`, `i64` (`int8_t` ... `int64_t`), `u8`, `u16`, `u32`, `u64`, `size` (`size_t`) |
| `C_API_FLOAT_TYPES` | `f32` (`float`), `f64` (`double`), `f128` (`long double`) |
| `C_API_NUMERIC_TYPES` | les entiers + les flottants |
| `C_API_VALUE_TYPES` | les types numériques + `bool`, `char` |
| `C_API_TYPES` | les types valeur + `str` (`const char*`, copié dans une `std::string`) |

Chaque section déroule la liste dont elle a besoin : `C_API_NUMERIC_TYPES(C_API_CLUSTER_DECL)` déclare `utils_cluster_i8_*` ... `utils_cluster_f128_*`. Chaque macro est dans un header et nommée `C_API_*` : les déclarations (`*_DECL`) dans le `CApi.h`, les handles (`*_HANDLE`) et les définitions (`*_DEF`, développées dans le `CApi.cpp` avec ses propres helpers) dans le `CApi.hpp`. Côté C++, `utils/type/CApiTypes.hpp` donne `Storage<C>` et `cpp_t<C>` : comment un type C est stocké en C++ (`const char*` → `std::string`, les types valeur tels quels) et reconverti.

| Template | Généré pour |
|---|---|
| `Cluster<T>` | types numériques (+ octets) |
| `Middlewares<T, T>` | tous les types (+ `void`, octets) |
| Valeurs de `Settings` | tous les types (+ les types de cast, octets) |
| `HybridMemory<T>` | types valeur (+ octets) |
| `IdHandler<T>` | entiers |
| `OVector2<T>` / `OVector3<T>` | types numériques (par valeur) |
| `OMatrix<T>` | types numériques (`div` / `invert` pour les flottants) |
| `BidirectionalLookupTable` | `str_str`, `size_size`, `i64_i64`, chaîne ↔ chaque entier (dans les deux sens), octets |
| `sos_embed` / `sos_extract` | `u8`, `u16`, `u32`, `u64` |

## Non exposé {#not-exposed}

| Absent en C | Pourquoi |
|---|---|
| `Concepts` | compilation uniquement |
| Les interfaces C++ à dériver (`IParser`, `IKey`...) | une version côté C existe à la place quand elle a du sens : un socket (`utils_socket_create`), un codec (`utils_codec_create`), un GPU (`utils_gpu_create_custom`), un observer (`utils_observer_create`) |
| `Middlewares<T, U>` avec `T ≠ U` | un jeu par paire de types : n² fonctions |
| `Cluster<bool>` | `std::vector<bool>` ne peut pas donner un `bool&` |
| `c2dmp_simplified`, `c2dmp_fsimplified` | obsolètes |
| le magic de s.o.s | fixé à `C_API_SOS_MAGIC` (`0x22`) |

## Ajouter une API C {#adding}

Pour une nouvelle classe d'une section :

1. `include/utils/<section>/CApi.h` : le type opaque, les enums / structs en types C (`static_assert` de leurs valeurs contre celles du C++ dans `CApi.hpp`), les prototypes avec un commentaire qui donne la valeur d'erreur et la durée de vie de ce qu'ils renvoient ;
2. `include/utils/<section>/CApi.hpp` : `struct utils_<name>: public utils_handle { <Class> object; };`, et pour un type fixe ses macros `C_API_<NAME>_HANDLE` / `C_API_<NAME>_DEF` ;
3. `src/utils/<section>/CApi.cpp` : chaque fonction dans `guard` / `guard_fallback`, chaque pointeur passé par `not_null` ; l'ajouter au `SRC` du `CMakeLists.txt` racine ;
4. le bloc de la section dans `include/utils/utils.h` ;
5. les tests : `tests/<section>/CApi.cpp` (GoogleTest) et une fonction de `tests/c/CApi.c` (C pur, C11 et C23).
