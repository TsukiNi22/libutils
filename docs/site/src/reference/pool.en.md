+++
title = "Pool"
summary = "Groups of entities: a Cluster of objects of the same type, and the Middlewares chains called before and after an action."
keywords = "pool cluster spawn apply kill middleware middlewares addBefore addAfter callBefore callAfter hook chain utils_cluster utils_middlewares"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Pool` | `_Handling` | `utils::pool` | `utils/pool/` |
| `_Middleware` (from `_Pool`) | `_Pool` | `utils::pool` | `utils/pool/middleware/Middlewares.hpp` |

Handling of groups of entities: a `Cluster` of objects of the same type, and the `Middlewares` chains.

| Name | Header | Description |
|---|---|---|
| `Cluster<T>` | `utils/pool/Cluster.hpp` | Group of objects of the same type, built, used and destroyed together |
| `Middlewares<T, U>` | `utils/pool/middleware/Middlewares.hpp` | Chains of functions called before / after an action |

## Cluster {#cluster}

| Method | Description |
|---|---|
| `spawn(args...)` | Build one element from the arguments (`T` must be constructible from them) |
| `spawn(std::size_t n, args...)` | Build `n` elements, each one from a copy of the arguments |
| `apply(std::function<void(T&)> fn)` | Call `fn` on every element, in the creation order |
| `size()` | Number of elements |
| `kill()` | Destroy every element |
| `kill(std::size_t n)` | Destroy the `n` last elements, `ErrorException(OutOfBounds)` if `n > size()` |

> [!NOTE]
> A `Cluster` can be moved, not copied.

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

A `Middlewares<T, U>` stores two chains of functions: the `before` ones, called with a `T`, and the `after` ones, called with a `U` (`void` = no argument). The [Cli](page:cli#middlewares) uses them (`cliMiddlewares`, `inputMiddlewares`, `promptMiddlewares`...) to hook code around its actions.

| Type | `before` called with | `after` called with |
|---|---|---|
| `Middlewares<T, U>` | `T` | `U` |
| `Middlewares<T, void>` | `T` | nothing |
| `Middlewares<void, U>` | nothing | `U` |
| `Middlewares<void, void>` | nothing | nothing |

`utils::pool::Middleware<T>` is `std::function<void(T)>` (`std::function<void(void)>` for `void`).

| Method | Description |
|---|---|
| `addBefore(middleware)` / `addBefore(std::vector<...>)` | Append one or several functions to the `before` chain |
| `addAfter(middleware)` / `addAfter(std::vector<...>)` | Append one or several functions to the `after` chain |
| `callBefore(arg)` / `callAfter(arg)` | Call the chain in the adding order (no argument for `void`) |
| `clear()` | Remove every middleware |

> [!NOTE]
> The chains are called on a snapshot, without the internal lock: a middleware can add / remove middlewares, the change applies to the next call.

> [!WARNING]
> A middleware throwing a `std::exception` is reported as `ErrorException(MiddlewareCall)` with the original message as info.

```cpp
#define _Middleware
#include <utils/utils.hpp>

utils::pool::Middlewares<const std::string&, int> m;
m.addBefore([](const std::string& cmd) {std::cout << "> " << cmd << std::endl;});
m.addAfter([](int code) {std::cout << "exit code " << code << std::endl;});

m.callBefore("build");
m.callAfter(0);
```

## In C {#in-c}

A template becomes one set of functions per fixed type (`<name>` = `i8` ... `u64`, `size`, `f32`, `f64`, `f128`, see [the fixed types](page:c-api-design#fixed-types)), plus a generic version in bytes (data + size).

| C++ | C |
|---|---|
| `Cluster<T>` (numeric `T`) | `utils_cluster_<name>_t*`: `utils_cluster_<name>_create`, `_spawn(cluster, n, value)`, `_apply(cluster, fn, data)`, `_kill`, `_kill_n`, `_size` |
| `Cluster` of any data | `utils_cluster_t*` (bytes: `utils_cluster_spawn(cluster, n, &value, sizeof(value))`) |
| `Middlewares<void, void>` | `utils_middlewares_t*` |
| `Middlewares<T, T>` (`T` a fixed type or `str`) | `utils_middlewares_<name>_t*` |
| `Middlewares` of any data | `utils_middlewares_bytes_t*` |

The callbacks return `UTILS_OK`, or `UTILS_KO` / `utils_fail(code, info)` to stop the chain: the call then fails with `MiddlewareCall` and the info of the callback. Middlewares with a different type before and after are not generated.

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
