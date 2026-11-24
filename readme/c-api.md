# C API

Guide: [Use libutils from C](https://tsukini22.github.io/libutils/docs/c-api.html), design: [The C API design](https://tsukini22.github.io/libutils/docs/c-api-design.html).

Every section except `_Concepts` has a C API (C11 / C23) in the same library: `utils/<section>/CApi.h`.
`<utils.h>` includes them with the same section defines as `utils.hpp`.

| Rule | Detail |
| ---- | ------ |
| Link | a C++ linker (`LINKER_LANGUAGE CXX`) or `-lstdc++`; CMake: `project(app LANGUAGES C CXX)` + `utils::utils` |
| Errors | no exception crosses the C boundary: `UTILS_KO` / `NULL` / `0`, then the status `utils_exception` |
| Status | type, code, what, info, location, origin; `utils_exception_format()`; `utils_fail(code, info)` from a callback |
| Objects | `utils_<name>_t*` handles (`_create` / `_destroy`), `utils_freezable_*` / `utils_worker_*` on any handle |
| Templates | one set of functions per fixed type, plus a version in bytes |
| Types | `i8` ... `u64`, `size`, `f32`, `f64`, `f128`, `bool`, `char`, `str` |
| Strings | a thread local result, or `_r` writing in a buffer (full length, like `snprintf`) |
| Strict C | `#define _POSIX_C_SOURCE 200809L` for the sections with POSIX types (`_Platform`, `_Encapsulation`, `_Network`, `_Cli`...) |

The tests: `tests/<section>/CApi.cpp` (GoogleTest), and `tests/c/CApi.c`, a pure C program built in C11 and C23 (`ctest`: `CApi.C11`, `CApi.C23`).
