+++
title = "Architecture"
summary = "What libutils is made of: one namespace, sections chosen with defines, a C++ API and a C API over the same objects, and the conventions shared by every class."
section = "concepts"
order = 10
keywords = "architecture overview namespace utils sections groups defines static library conventions failsafe safe_mode observer movable"
+++

libutils is one static library (`libutils.a`, plus a debug and an asan build) for C++20 programs, with a C API over the same objects. Everything is in the namespace `utils::`; a program includes one root header and chooses the sections it needs.

## In short {#in-short}

| Point | Value |
|---|---|
| Language | C++20, built with `clang++` and CMake (>= 3.20); C API for C11 / C23 |
| Library | static: `libutils.a` (optimized), `libutils_debug.a`, `libutils_asan.a` |
| Platforms | Unix (Linux, BSD, macOS) and Windows x86_64 (cross compiled with MinGW-w64), same API ([Platforms](page:platforms)) |
| Dependency | OpenSSL >= 3.0 (encryption, Base64) |
| Entry points | `<utils/utils.hpp>` (C++), `<utils/utils.h>` (C) |
| Version | `utils::version` / `utils_version`, `__LIBUTILS_VERSION__` |

## Sections {#sections}

A section is a folder of `include/utils/`, a namespace and a define. A **group** define enables its sections, a section define enables its sub-sections; nothing defined means everything (`_Utils`). The list with their pages is on [Link and include](page:usage#sections).

:::html
<figure>
<svg viewBox="0 0 760 300" role="img" aria-label="utils.hpp and utils.h include the Handling and Tools groups and their sections">
<defs>
<marker id="arr-fig1-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box-alt" x="300" y="16" width="160" height="44" rx="8"/>
<text class="lbl" x="380" y="43" text-anchor="middle">utils.hpp · utils.h</text>
<path class="edge" d="M330 60L190 98" marker-end="url(#arr-fig1-en)"/>
<path class="edge" d="M430 60L570 98" marker-end="url(#arr-fig1-en)"/>
<path class="edge dash" d="M380 60V250" marker-end="url(#arr-fig1-en)"/>
<rect class="group" x="20" y="100" width="330" height="140" rx="10"/>
<text class="title" x="36" y="124">_Handling</text>
<text class="small" x="36" y="150">_Exception · _Verbose · _Pool (_Middleware)</text>
<text class="small" x="36" y="174">_Cli · _Arguments</text>
<text class="small" x="36" y="198">_Network (_Socket)</text>
<text class="small" x="36" y="222">utils::exception, verbose, pool, cli, arguments, network</text>
<rect class="group" x="410" y="100" width="330" height="140" rx="10"/>
<text class="title" x="426" y="124">_Tools</text>
<text class="small" x="426" y="150">_Math (_Geometry, _Trigo) · _Concepts</text>
<text class="small" x="426" y="174">_Encapsulation · _System</text>
<text class="small" x="426" y="198">_CustomType (_Vector, _Matrix, _BLT)</text>
<text class="small" x="426" y="222">_Manip · _Algorithms · _Security · _Platform</text>
<rect class="box" x="300" y="252" width="160" height="38" rx="8"/>
<text class="lbl" x="380" y="276" text-anchor="middle">_Attribute</text>
</svg>
<figcaption>The defines of <code>utils.hpp</code> and <code>utils.h</code>: a group enables its sections, a section its sub-sections.</figcaption>
</figure>
:::

| Define | Namespace | Content |
|---|---|---|
| `_Exception` | `utils::exception` | exceptions with a type and a code, formatted output |
| `_Verbose` | `utils::verbose` | atomic global level, locked output macros |
| `_Pool` | `utils::pool` | `Cluster<T>`, `Middlewares<T, U>` |
| `_Cli` | `utils::cli` | interactive command line: commands, history, hints, completion, hooks, middlewares |
| `_Arguments` | `utils::arguments` | `ArgParser` (usages, flags, options, env), `Settings` (typed values, casts) |
| `_Network` | `utils::network` | `Server`, `Client`, `ISocket` / `ASocket` / `TCPSocket` |
| `_Math` | `utils::math` | coordinates, `CFrame`, point rotation, look vector, deg / rad |
| `_Concepts` | `utils::concepts` | operation concepts (`AddableWith`, `Comparable`...) |
| `_Encapsulation` | `utils::encapsulation` | `Pipe`, `Dup`, `Process`, `Poll`, `SharedObject`, `SharedMemory` |
| `_System` | `utils::system` | `IdHandler`, `Scheduler`, `LoadBalancer`, `HybridMemory` |
| `_CustomType` | `utils::type` | `Vector2/3`, `OVector2/3`, `Matrix`, `OMatrix`, `BidirectionalLookupTable`, `Freezable`, `Worker` |
| `_Manip` | `utils::iomanip`, `utils::smanip` | ANSI sequences, `format`, `FixedString`, `Base64Codec`, `EETPParser` |
| `_Algorithms` | `utils::algorithms` | c2dmp-hsm (string distance), s.o.s (steganography) |
| `_Security` | `utils::security` | `AESKey`, `RSAKey`, `CommonRSAKey`, observers / memory leak notifier |
| `_Platform` | `utils::platform` | POSIX calls for Unix and Windows, `UTILS_UNIX` / `UTILS_WINDOWS` |
| `_Attribute` | macros | `_hot`, `_nodiscard`, `_likely`, `_migration(x, y, z)`... |

The dependencies between the headers, the classes and the calls are drawn on the [project graph](page:project-graph).

## Two APIs, one library {#apis}

The C API is not a second implementation: each `CApi.h` declares C functions (`extern "C"`) whose `.cpp` calls the C++ classes inside a guard. The objects are the same, so a C program gets the same behavior, the same threads and the same error codes. How the boundary works: [The C API design](page:c-api-design).

| | C++ | C |
|---|---|---|
| Root header | `<utils/utils.hpp>` | `<utils/utils.h>` |
| An object | a class (`utils::cli::Cli`) | an opaque handle (`utils_cli_t*`) |
| An error | an exception (`IException`) | `UTILS_KO` / `NULL` + the status `utils_exception` |
| A template | `Cluster<T>` | one set of functions per fixed type (`utils_cluster_i32_*`) + a version in bytes |
| A callback | `std::function` | a function pointer + `void* data` |

## Conventions {#conventions}

- Every error is an exception (`utils::exception::*Exception`) with a code of [Exception](page:exception#codes); catch `const utils::exception::IException&`. The model is on [Errors](page:errors).
- Many methods take a `failsafe` boolean (print a warning instead of throwing) or a `safe_mode` boolean (lock, thread safe).
- The check hooks return `std::optional<std::string>`: `std::nullopt` = valid, a string = the error message.
- Most classes are movable but not copyable, and are tracked by an [observer](page:security#observer) (memory leak report at exit).
- Every class has the same API on Unix and Windows: only a call without Windows equivalent raises a `FatalException` (`UndefinedCall`).
- The names follow one style: classes in `PascalCase`, methods in `camelCase`, tool functions in `snake_case`, members `_camelCase`, macros `UPPER_CASE`; the C API is `utils_<section>_<name>` and `C_API_<SECTION>_<NAME>`.
