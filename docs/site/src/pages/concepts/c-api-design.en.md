+++
title = "The C API design"
summary = "How the C API is built over the C++ classes: the guard, the handles and their common base, the callbacks, the strings, the fixed types of the templates, and what is not exposed."
section = "concepts"
order = 30
keywords = "c api design guard guard_fallback not_null handle utils_handle magic handle_base dynamic_cast callback check_callback thread local copy_r X-macro fixed types CApiTypes Storage cpp_t"
+++

The C API is a thin layer over the C++ classes, written in C++: each section has a C header (`CApi.h`), a C++ header for its handles and its macros (`CApi.hpp`) and the functions (`src/utils/<section>/CApi.cpp`). This page explains its rules, for a reader of the code or a contributor; the user guide is [Use libutils from C](page:c-api).

:::html
<figure>
<svg viewBox="0 0 760 270" role="img" aria-label="A C program calls the C functions, each one runs the C++ classes in a guard; a failure fills the thread local status utils_exception; a C callback called by the C++ code returns UTILS_OK or utils_fail">
<defs>
<marker id="arrc-fig3-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
<marker id="arrca-fig3-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
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
<path class="edge" d="M170 65H206" marker-end="url(#arrc-fig3-en)"/>
<path class="edge" d="M370 65H406" marker-end="url(#arrc-fig3-en)"/>
<path class="edge" d="M560 65H596" marker-end="url(#arrc-fig3-en)"/>
<rect class="group" x="330" y="140" width="230" height="44" rx="8"/>
<text class="lbl" x="445" y="167" text-anchor="middle">utils_exception (thread local)</text>
<path class="edge dash" d="M485 90V136" marker-end="url(#arrc-fig3-en)"/>
<text class="small" x="492" y="118">exception</text>
<path class="edge dash" d="M330 162H95V94" marker-end="url(#arrc-fig3-en)"/>
<text class="small" x="105" y="154">UTILS_KO / NULL, read like errno</text>
<rect class="box" x="20" y="210" width="150" height="44" rx="8"/>
<text class="lbl" x="95" y="237" text-anchor="middle">C callback</text>
<path class="edge-accent" d="M690 90V232H174" marker-end="url(#arrca-fig3-en)"/>
<text class="small" x="430" y="224" text-anchor="middle">fn(..., data) → UTILS_OK / utils_fail(code, info)</text>
</svg>

<figcaption>The layers of a call: the C function, its guard, the C++ object held by the handle; the status and the callbacks go back to C.</figcaption>
</figure>
:::

## The guard {#guard}

`utils/exception/CApi.hpp` gives the tools every C function uses (namespace `utils::exception::c_api`):

| Tool | Role |
|---|---|
| `guard(f)` | Runs `f`, catches everything, fills `utils_exception` on failure; returns `UTILS_OK` / `UTILS_KO` |
| `guard_fallback(fallback, f)` | Returns the result of `f`, or `fallback` when it threw (`NULL`, `0`, `false`...) |
| `not_null(ptr, name)` / `not_null_ref(ptr, name)` | A `NULL` from the C side becomes `InvalidArgument` "The &lt;name&gt; is NULL", located at the caller |
| `set_status(e)` / `clear_status()` | Fill / reset the status of the thread |

```cpp "src/utils/cli/CApi.cpp"
_cold uint8_t utils_cli_get_code(const utils_cli_t* cli)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> uint8_t {return utils::exception::c_api::not_null_ref(cli, "cli handle").cli.getCode();});
}
```

On success only `utils_exception.exception = false` is written: the strings of an old failure stay, and the fast path copies nothing. The status is `thread_local` (`_thread_local`), like `errno`.

## Handles {#handles}

A handle is a C++ struct declared opaque to C (`typedef struct utils_cli utils_cli_t;`) that holds the object (`utils_cli { utils::cli::Cli cli; ... }`). Every handle derives `utils_handle`, as its **first** base:

| Member | Role |
|---|---|
| `magic` | `C_API_HANDLE_MAGIC` while the handle lives, `0` once destroyed |
| virtual destructor | gives the handle a type known at runtime (RTTI) |

`handle_base<T>(void* handle, name)` reaches a base shared by many classes from any handle: it checks the magic ("The pointer is not a handle of the C API"), then `dynamic_cast`s to `T` ("This handle has no &lt;name&gt;"). The generic C functions are built on it: `utils_freezable_*` (`utils::type::Freezable`) and `utils_worker_*` (`utils::type::Worker`) accept the handle of any class that derives them.

`utils_<name>_destroy(NULL)` does nothing (like `free`). A handle that keeps a thread (the cli with `THREAD`) joins it in its destroy.

## Callbacks {#callbacks}

A C callback is a function pointer and a `void* data`, wrapped in a `std::function` on the C++ side. Around each call:

1. `reset_status()` clears the error flag, so the status only holds the error of the callback;
2. the callback runs and returns an `int`;
3. `check_callback(ret)`: anything other than `UTILS_OK` throws, with the info given by `utils_fail` (or "C callback failed (no info given with utils_fail)").

The exception then goes up the C++ code like any other (a hook error of the cli, `MiddlewareCall` of a middleware, the message of an argument check) and ends in the status of the C call that ran it.

## Strings {#strings}

`utils/manip/smanip/CApi.hpp` gives `copy` (into a fixed buffer, truncated) and `copy_r` (the `_r` convention: truncated, always `'\0'` terminated, returns the full length like `snprintf`), and `bytes` the bytes of a pointer + size of the C side in a `std::string` (`NULL` only with a size of 0). A function that builds a string has two forms: the plain one returns a `thread_local std::string` buffer of the function (valid until its next call in the thread), the `_r` one writes in a buffer of the caller. Bytes that can hold `'\0'` use a pointer + size, or a `utils_string_t*`.

## Fixed types {#fixed-types}

C has no templates: a template is generated for a list of **fixed types** by X-macros of `utils/type/CApiTypes.h`, and has a generic version in bytes (data + size) for anything else.

| X-macro | Types (`name`: C type) |
|---|---|
| `C_API_INT_TYPES` | `i8`, `i16`, `i32`, `i64` (`int8_t` ... `int64_t`), `u8`, `u16`, `u32`, `u64`, `size` (`size_t`) |
| `C_API_FLOAT_TYPES` | `f32` (`float`), `f64` (`double`), `f128` (`long double`) |
| `C_API_NUMERIC_TYPES` | the integers + the floating points |
| `C_API_VALUE_TYPES` | the numeric types + `bool`, `char` |
| `C_API_TYPES` | the value types + `str` (`const char*`, copied in a `std::string`) |

Each section unrolls the list it needs: `C_API_NUMERIC_TYPES(C_API_CLUSTER_DECL)` declares `utils_cluster_i8_*` ... `utils_cluster_f128_*`. Every macro is in a header and named `C_API_*`: the declarations (`*_DECL`) in the `CApi.h`, the handles (`*_HANDLE`) and the definitions (`*_DEF`, expanded in the `CApi.cpp` with its own helpers) in the `CApi.hpp`. On the C++ side, `utils/type/CApiTypes.hpp` gives `Storage<C>` and `cpp_t<C>`: how a C type is stored in C++ (`const char*` → `std::string`, the value types as they are) and converted back.

| Template | Generated for |
|---|---|
| `Cluster<T>` | numeric types (+ bytes) |
| `Middlewares<T, T>` | every type (+ `void`, bytes) |
| `Settings` values | every type (+ the cast types, bytes) |
| `HybridMemory<T>` | value types (+ bytes) |
| `IdHandler<T>` | integers |
| `OVector2<T>` / `OVector3<T>` | numeric types (by value) |
| `OMatrix<T>` | numeric types (`div` / `invert` for the floating points) |
| `BidirectionalLookupTable` | `str_str`, `size_size`, `i64_i64`, string ↔ each integer (both orders), bytes |
| `sos_embed` / `sos_extract` | `u8`, `u16`, `u32`, `u64` |

## Not exposed {#not-exposed}

| Not in C | Why |
|---|---|
| `Concepts` | compile time only |
| The C++ interfaces to derive (`IParser`, `IKey`...) | a C side version exists where it makes sense instead: a socket (`utils_socket_create`), a codec (`utils_codec_create`), a GPU (`utils_gpu_create_custom`), an observer (`utils_observer_create`) |
| `Middlewares<T, U>` with `T ≠ U` | one set per pair of types: n² functions |
| `Cluster<bool>` | `std::vector<bool>` can't give a `bool&` |
| `c2dmp_simplified`, `c2dmp_fsimplified` | deprecated |
| the magic of s.o.s | fixed to `C_API_SOS_MAGIC` (`0x22`) |

## Adding a C API {#adding}

For a new class of a section:

1. `include/utils/<section>/CApi.h`: the opaque type, the enums / structs as C types (`static_assert` their values against the C++ ones in `CApi.hpp`), the prototypes with a comment saying the error value and the lifetime of what they return;
2. `include/utils/<section>/CApi.hpp`: `struct utils_<name>: public utils_handle { <Class> object; };`, and for a fixed type its `C_API_<NAME>_HANDLE` / `C_API_<NAME>_DEF` macros;
3. `src/utils/<section>/CApi.cpp`: each function in `guard` / `guard_fallback`, every pointer through `not_null`; add it to the `SRC` of the root `CMakeLists.txt`;
4. the block of the section in `include/utils/utils.h`;
5. the tests: `tests/<section>/CApi.cpp` (GoogleTest) and a function of `tests/c/CApi.c` (pure C, C11 and C23).
