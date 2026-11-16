+++
title = "Pool"
summary = "Groupes d'entités : un Cluster d'objets du même type, et les chaînes Middlewares appelées avant et après une action."
keywords = "pool cluster spawn apply kill middleware middlewares addBefore addAfter callBefore callAfter hook chaîne utils_cluster utils_middlewares"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_Pool` | `_Handling` | `utils::pool` | `utils/pool/` |
| `_Middleware` (depuis `_Pool`) | `_Pool` | `utils::pool` | `utils/pool/middleware/Middlewares.hpp` |

Gestion de groupes d'entités : un `Cluster` d'objets du même type, et les chaînes `Middlewares`.

| Nom | Header | Description |
|---|---|---|
| `Cluster<T>` | `utils/pool/Cluster.hpp` | Groupe d'objets du même type, construits, utilisés et détruits ensemble |
| `Middlewares<T, U>` | `utils/pool/middleware/Middlewares.hpp` | Chaînes de fonctions appelées avant / après une action |

## Cluster {#cluster}

| Méthode | Description |
|---|---|
| `spawn(args...)` | Construit un élément à partir des arguments (`T` doit être constructible à partir d'eux) |
| `spawn(std::size_t n, args...)` | Construit `n` éléments, chacun à partir d'une copie des arguments |
| `apply(std::function<void(T&)> fn)` | Appelle `fn` sur chaque élément, dans l'ordre de création |
| `size()` | Nombre d'éléments |
| `kill()` | Détruit chaque élément |
| `kill(std::size_t n)` | Détruit les `n` derniers éléments, `ErrorException(OutOfBounds)` si `n > size()` |

> [!NOTE]
> Un `Cluster` peut être déplacé, pas copié.

```cpp
#define _Pool
#include <utils/utils.hpp>

utils::pool::Cluster<std::string> cluster;
cluster.spawn(std::size_t{3}, "worker");      // 3 x "worker"
cluster.spawn("leader");                       // 1 more
cluster.apply([](std::string& name) {name += "!";});
cluster.kill(1);                               // removes "leader!"
```

## Middleware {#middleware}

Un `Middlewares<T, U>` stocke deux chaînes de fonctions : celles `before`, appelées avec un `T`, et celles `after`, appelées avec un `U` (`void` = pas d'argument). Le [Cli](page:cli#middlewares) les utilise (`cliMiddlewares`, `inputMiddlewares`, `promptMiddlewares`...) pour accrocher du code autour de ses actions.

| Type | `before` appelé avec | `after` appelé avec |
|---|---|---|
| `Middlewares<T, U>` | `T` | `U` |
| `Middlewares<T, void>` | `T` | rien |
| `Middlewares<void, U>` | rien | `U` |
| `Middlewares<void, void>` | rien | rien |

`utils::pool::Middleware<T>` est `std::function<void(T)>` (`std::function<void(void)>` pour `void`).

| Méthode | Description |
|---|---|
| `addBefore(middleware)` / `addBefore(std::vector<...>)` | Ajoute une ou plusieurs fonctions à la chaîne `before` |
| `addAfter(middleware)` / `addAfter(std::vector<...>)` | Ajoute une ou plusieurs fonctions à la chaîne `after` |
| `callBefore(arg)` / `callAfter(arg)` | Appelle la chaîne dans l'ordre d'ajout (pas d'argument pour `void`) |
| `clear()` | Retire chaque middleware |

> [!NOTE]
> Les chaînes sont appelées sur un instantané, sans le verrou interne : un middleware peut ajouter / retirer des middlewares, le changement s'applique à l'appel suivant.

> [!WARNING]
> Un middleware qui lève une `std::exception` est signalé comme `ErrorException(MiddlewareCall)` avec le message d'origine en info.

```cpp
#define _Middleware
#include <utils/utils.hpp>

utils::pool::Middlewares<const std::string&, int> m;
m.addBefore([](const std::string& cmd) {std::cout << "> " << cmd << std::endl;});
m.addAfter([](int code) {std::cout << "exit code " << code << std::endl;});

m.callBefore("build");
m.callAfter(0);
```

## En C {#in-c}

Un template devient un ensemble de fonctions par type fixe (`<name>` = `i8` ... `u64`, `size`, `f32`, `f64`, `f128`, voir [les types fixes](page:c-api-design#fixed-types)), plus une version générique en octets (données + taille).

| C++ | C |
|---|---|
| `Cluster<T>` (`T` numérique) | `utils_cluster_<name>_t*` : `utils_cluster_<name>_create`, `_spawn(cluster, n, value)`, `_apply(cluster, fn, data)`, `_kill`, `_kill_n`, `_size` |
| `Cluster` de données quelconques | `utils_cluster_t*` (octets : `utils_cluster_spawn(cluster, n, &value, sizeof(value))`) |
| `Middlewares<void, void>` | `utils_middlewares_t*` |
| `Middlewares<T, T>` (`T` un type fixe ou `str`) | `utils_middlewares_<name>_t*` |
| `Middlewares` de données quelconques | `utils_middlewares_bytes_t*` |

Les callbacks renvoient `UTILS_OK`, ou `UTILS_KO` / `utils_fail(code, info)` pour arrêter la chaîne : l'appel échoue alors avec `MiddlewareCall` et l'info du callback. Les middlewares avec un type différent avant et après ne sont pas générés.

```c
#define _Pool
#include <utils.h>
#include <stdio.h>

static int twice(int32_t* element, void* data)
{
    (void)data;
    *element *= 2;
    return UTILS_OK;
}

static int log_before(const char* cmd, void* data)
{
    (void)data;
    printf("> %s\n", cmd);
    return UTILS_OK;
}

int main(void)
{
    utils_cluster_i32_t* cluster = utils_cluster_i32_create();
    utils_middlewares_str_t* m = utils_middlewares_str_create();

    if (!cluster || !m)
        return 1;
    utils_cluster_i32_spawn(cluster, 3, 21);            // 3 x 21
    utils_cluster_i32_apply(cluster, twice, NULL);      // 3 x 42
    utils_middlewares_str_add_before(m, log_before, NULL);
    utils_middlewares_str_call_before(m, "build");      // "> build"
    printf("%zu elements\n", utils_cluster_i32_size(cluster));
    utils_middlewares_str_destroy(m);
    utils_cluster_i32_destroy(cluster);
    return 0;
}
```
