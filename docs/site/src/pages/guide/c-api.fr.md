+++
title = "Utiliser libutils depuis C"
summary = "Appeler libutils depuis un programme C : le header racine utils.h, le link, le statut d'erreur utils_exception, les handles, les chaînes et les callbacks."
keywords = "c api utils.h c11 c23 extern C handle create destroy utils_exception statut UTILS_OK UTILS_KO utils_fail callback chaînes _r thread local link stdc++"
+++

Chaque section de libutils, sauf les `Concepts` de compilation, peut être appelée depuis C (C11 et C23 testés, C99 pour les attributs) : les mêmes objets derrière des handles opaques, les mêmes erreurs dans un statut qui se lit comme `errno`. Sa construction interne est sur [Conception de l'API C](page:c-api-design) ; chaque [page de référence](page:exception#in-c) a une section **En C** et l'API C complète de sa section.

## Mise en place {#setup}

L'API C est dans la même bibliothèque que l'API C++ : libutils est en C++, donc le programme est lié par un linker C++ (ou avec `-lstdc++`).

```cmake "CMakeLists.txt"
project(app LANGUAGES C CXX)                             # CXX: the link of a C++ library
find_package(utils 5.0.0 REQUIRED)
add_executable(app main.c)
set_target_properties(app PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(app PRIVATE utils::utils)          # includes, static library & OpenSSL
```

```bash "Sans CMake"
gcc -std=c11 main.c -o app -lutils -lstdc++ -lssl -lcrypto -lm
```

> [!NOTE]
> En C strict (`-std=c11` / `-std=c23`) sous Unix, les sections qui utilisent des types POSIX (`_Platform`, `_Encapsulation`, `_Network`, `_Cli`...) demandent `#define _POSIX_C_SOURCE 200809L` avant le premier include ; `-std=gnu11` / `gnu23` l'ont déjà.

## Inclusion {#include}

```c "main.c"
#define _Cli                 // the same section defines as utils.hpp (nothing defined = everything)
#include <utils.h>           // same as <utils/utils.h>
```

`utils.h` prend les mêmes defines que `utils.hpp` (`_Utils`, `_Handling`, `_Tools`, `_Exception`, `_Cli`, `_Vector`...) et `_Warning` / `_NoWarning` pour les avertissements d'obsolescence. Il vérifie avec `__has_include` que les headers installés sont présents (C23, ou l'extension GCC / Clang) et donne la version (`utils_version`, `__LIBUTILS_VERSION__`). Comme `utils.hpp`, il n'a pas de garde globale : placez les defines avant chaque include.

## Erreurs {#errors}

Aucune exception ne traverse la frontière C : chaque fonction tourne dans une garde qui attrape tout.

> [!NOTE]
> Les déclarations portent `_nothrow` et `_nodiscard` : un résultat non lu (statut, handle, valeur) est un avertissement, `(void)appel(...)` l'ignore volontairement (pas pour `utils_fail`, `utils_exception_set` et les logs du verbose). Avec gcc, les tampons (`_read_only` / `_write_only`), les chaînes (`_null_terminated_string_arg`) et les descripteurs de fichier de la plateforme (`_fd_arg`) sont aussi vérifiés.

| Une fonction renvoie | Échec |
|---|---|
| `int` (`UTILS_OK` / `UTILS_KO`) | `UTILS_KO` |
| un handle ou un pointeur | `NULL` |
| une taille, un nombre, un id | `0` (quand `0` n'est pas un résultat valide, voir le commentaire de la fonction) |
| `bool`, une valeur | le commentaire de la fonction le précise |

En cas d'échec, le statut thread local `utils_exception` contient l'erreur ; en cas de succès, seul `utils_exception.exception` est remis à `false`.

| Champ | Contenu |
|---|---|
| `exception` | `true` quand le dernier appel a échoué |
| `origin` | `C_API_EXCEPTION_ORIGIN_INTERNAL` / `_EXTERNAL` (un code libutils, de la bibliothèque / de votre projet), `_STD` (une `std::exception`), `_UNKNOWN`, `_C` (posé depuis C) |
| `type` | `C_API_EXCEPTION_TYPE_NONE`, `_FATAL`, `_ERROR`, `_WARNING` (le `Type` C++), `_SYSTEM` (pas une exception libutils) |
| `code` | La valeur `InternalCode` / `ExternalCode`, `C_API_OTHER_EXCEPTION_CODE` quand ce n'est pas une exception libutils |
| `what`, `info` | Message et info (tronqués à `C_API_MAX_CHAR_EXCEPTION_INFORMATION`, 512) |
| `file`, `line`, `function`, `module` | L'endroit où elle a été levée |

| Fonction | Usage |
|---|---|
| `utils_exception_format()` / `utils_exception_format_r(buf, size)` | La même sortie que `formated()` (le statut est conservé) |
| `utils_exception_clear()` | Réinitialise le statut du thread |
| `utils_fail(code, info)` | Pose une erreur depuis C (origine `C`, type `ERROR`, avec `__FILE__` / `__LINE__` / `__func__`) et renvoie `UTILS_KO` |
| `utils_exception_set(type, code, what, info)` | Pareil avec tous les champs |

```c
if (utils_cli_start(cli, NULL, 0, 1) != UTILS_OK) {
    if (utils_exception.type & C_API_EXCEPTION_TYPE_NONE)
        return 0;                                        // not an error (ex: the code Exit)
    fprintf(stderr, "%s\n", utils_exception_format());   // [Error] file:line (origin) -> message ...
    return 1;
}
```

## Handles {#handles}

Une classe est un handle opaque : `utils_<name>_t*`, créé par `utils_<name>_create(...)` (`NULL` en cas d'erreur) et libéré par `utils_<name>_destroy(handle)` (`NULL` ne fait rien). Une fonction qui reçoit un handle `NULL` échoue avec `InvalidArgument`.

- Un handle qui **détient** un autre handle le documente (`utils_eetp_parser_set_codec` prend le codec : ne le détruisez jamais ensuite) ; un résultat renvoyé comme nouveau handle est à vous de détruire (`utils_matrix_f64_invert`, `utils_hybrid_memory_get_gpu`...).
- Les bases partagées par plusieurs classes s'atteignent depuis n'importe quel handle : `utils_freezable_*(void* handle)`, `utils_worker_*(void* handle)`. Un handle d'une autre classe échoue avec `InvalidArgument` ("This handle has no ...").
- Un objet d'un template est généré pour les [types fixes](page:c-api-design#fixed-types) : `utils_cluster_i32_t*`, `utils_matrix_f64_t*`, `utils_hybrid_memory_u8_t*`..., plus une version générique en octets (`data` + `size`).

## Chaînes {#strings}

| Forme | Durée de vie |
|---|---|
| `const char* utils_x(...)` sans argument | Une chaîne statique |
| `const char* utils_x(...)` construite depuis ses arguments | Un buffer thread local, valide jusqu'au prochain appel de la même fonction dans le thread |
| `size_t utils_x_r(..., char* buf, size_t size)` | Copiée dans votre buffer, tronquée et toujours terminée par `'\0'`, renvoie la longueur complète comme `snprintf` (`0` en cas d'erreur) |
| une chaîne lue depuis un handle | Valide jusqu'à la modification du handle (précisé par le commentaire de la fonction) |
| des octets (peuvent contenir `'\0'`) | Un pointeur + une taille, ou un `utils_string_t*` ([SManip](page:smanip#in-c)) |

## Callbacks {#callbacks}

Chaque callback prend un `void* data` donné à son enregistrement, et renvoie `UTILS_OK` en cas de succès. Pour échouer, renvoyez `UTILS_KO`, ou `utils_fail(code, info)` pour donner le message : le côté C++ le transforme en exception avec cette info, et l'appel qui a lancé le callback échoue avec elle. Certains callbacks ont leurs propres valeurs supplémentaires : `C_API_CLI_EXIT` (arrête la cli), `C_API_CLI_NO_INPUT` (hook getc).

```c
static int check_port(const char* option, void* data)
{
    long port = strtol(option, NULL, 10);

    (void)data;
    if (port < 1 || port > 65535)
        return utils_fail(1, "the port must be between 1 and 65535"); // the message of the parsing error
    return UTILS_OK;
}
```

## Threads {#threads}

`utils_exception` et les buffers thread local sont propres à chaque thread : lisez le statut dans le thread qui a fait l'appel. Les objets gardent la thread safety de leur classe C++ (arguments `safe_mode`, le thread d'une cli `THREAD` gardé dans son handle et joint avec `utils_cli_join`).

## Exemple {#example}

```c "main.c"
#define _Verbose
#define _BLT
#include <utils.h>
#include <stdio.h>

int main(void)
{
    utils_blt_str_str_t* table = utils_blt_str_str_create();
    const char* other = NULL;

    if (!table)
        return 1;
    utils_verbose_set(C_API_VERBOSE_DEBUG);
    utils_blt_str_str_add(table, "fr", "France");
    if (utils_blt_str_str_get(table, "France", &other) == UTILS_OK)
        utils_verbose_log(C_API_VERBOSE_BASIC, "France -> %s", other);    // fr
    if (utils_blt_str_str_get(table, "de", &other) != UTILS_OK)             // unknown key
        utils_verbose_log(C_API_VERBOSE_DEBUG, "%s", utils_exception_format());
    utils_blt_str_str_destroy(table);
    return 0;
}
```
