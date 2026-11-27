+++
title = "Use libutils from C"
summary = "Call libutils from a C program: the root header utils.h, the link, the error status utils_exception, the handles, the strings and the callbacks."
section = "guide"
order = 50
keywords = "c api utils.h c11 c23 extern C handle create destroy utils_exception status UTILS_OK UTILS_KO utils_fail callback strings _r thread local link stdc++"
+++

Every section of libutils, except the compile time `Concepts`, can be called from C (C11 and C23 tested, C99 for the attributes): the same objects behind opaque handles, the same errors in a status read like `errno`. How it is built inside is on [The C API design](page:c-api-design); each [reference page](page:exception#in-c) has a section **In C** and the full C API of its section.

## Setup {#setup}

The C API is in the same library as the C++ one: libutils is C++, so the program is linked by a C++ linker (or with `-lstdc++`).

```cmake "CMakeLists.txt"
project(app LANGUAGES C CXX)                             # CXX: the link of a C++ library
find_package(utils 5.0.0 REQUIRED)
add_executable(app main.c)
set_target_properties(app PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(app PRIVATE utils::utils)          # includes, static library & OpenSSL
```

```bash "Without CMake"
gcc -std=c11 main.c -o app -lutils -lstdc++ -lssl -lcrypto -lm
```

> [!NOTE]
> In strict C (`-std=c11` / `-std=c23`) on Unix, the sections that use POSIX types (`_Platform`, `_Encapsulation`, `_Network`, `_Cli`...) need `#define _POSIX_C_SOURCE 200809L` before the first include; `-std=gnu11` / `gnu23` already have it.

## Include {#include}

```c "main.c"
#define _Cli                 // the same section defines as utils.hpp (nothing defined = everything)
#include <utils.h>           // same as <utils/utils.h>
```

`utils.h` takes the same defines as `utils.hpp` (`_Utils`, `_Handling`, `_Tools`, `_Exception`, `_Cli`, `_Vector`...) and `_Warning` / `_NoWarning` for the deprecation warnings. It checks with `__has_include` that the installed headers are there (C23, or the GCC / Clang extension) and gives the version (`utils_version`, `__LIBUTILS_VERSION__`). Like `utils.hpp`, it has no global guard: put the defines before every include.

## Errors {#errors}

No exception crosses the C boundary: each function runs in a guard that catches everything.

> [!NOTE]
> The declarations carry `_nothrow` and `_nodiscard`: a result that is not read (status, handle, value) is a warning, `(void)call(...)` ignores it on purpose (not for `utils_fail`, `utils_exception_set` and the logs of the verbose). With gcc, the buffers (`_read_only` / `_write_only`), the strings (`_null_terminated_string_arg`) and the file descriptors of the platform (`_fd_arg`) are checked too.

| A function returns | Failure |
|---|---|
| `int` (`UTILS_OK` / `UTILS_KO`) | `UTILS_KO` |
| a handle or a pointer | `NULL` |
| a size, a count, an id | `0` (when `0` is not a valid result, see the comment of the function) |
| `bool`, a value | the comment of the function says it |

On failure the thread local status `utils_exception` holds the error; on success only `utils_exception.exception` is set back to `false`.

| Field | Content |
|---|---|
| `exception` | `true` when the last call failed |
| `origin` | `C_API_EXCEPTION_ORIGIN_INTERNAL` / `_EXTERNAL` (a libutils code, of the library / of your project), `_STD` (a `std::exception`), `_UNKNOWN`, `_C` (set from C) |
| `type` | `C_API_EXCEPTION_TYPE_NONE`, `_FATAL`, `_ERROR`, `_WARNING` (the C++ `Type`), `_SYSTEM` (not a libutils exception) |
| `code` | The `InternalCode` / `ExternalCode` value, `C_API_OTHER_EXCEPTION_CODE` when it isn't a libutils exception |
| `what`, `info` | Message and info (truncated to `C_API_MAX_CHAR_EXCEPTION_INFORMATION`, 512) |
| `file`, `line`, `function`, `module` | Where it was raised |

| Function | Use |
|---|---|
| `utils_exception_format()` / `utils_exception_format_r(buf, size)` | The same output as `formated()` (the status is kept) |
| `utils_exception_clear()` | Reset the status of the thread |
| `utils_fail(code, info)` | Set an error from C (origin `C`, type `ERROR`, with `__FILE__` / `__LINE__` / `__func__`) and return `UTILS_KO` |
| `utils_exception_set(type, code, what, info)` | Same with every field |

```c
if (utils_cli_start(cli, NULL, 0, 1) != UTILS_OK) {
    if (utils_exception.type & C_API_EXCEPTION_TYPE_NONE)
        return 0;                                        // not an error (ex: the code Exit)
    fprintf(stderr, "%s\n", utils_exception_format());   // [Error] file:line (origin) -> message ...
    return 1;
}
```

## Handles {#handles}

A class is an opaque handle: `utils_<name>_t*`, created by `utils_<name>_create(...)` (`NULL` on error) and released by `utils_<name>_destroy(handle)` (`NULL` does nothing). A function given a `NULL` handle fails with `InvalidArgument`.

- A handle that **holds** another one documents it (`utils_eetp_parser_set_codec` takes the codec: never destroy it after); a result returned as a new handle is yours to destroy (`utils_matrix_f64_invert`, `utils_hybrid_memory_get_gpu`...).
- The bases shared by many classes are reached on any handle: `utils_freezable_*(void* handle)`, `utils_worker_*(void* handle)`. A handle of another class fails with `InvalidArgument` ("This handle has no ...").
- An object of a template is generated for the [fixed types](page:c-api-design#fixed-types): `utils_cluster_i32_t*`, `utils_matrix_f64_t*`, `utils_hybrid_memory_u8_t*`..., plus a generic version in bytes (`data` + `size`).

## Strings {#strings}

| Form | Lifetime |
|---|---|
| `const char* utils_x(...)` without argument | A static string |
| `const char* utils_x(...)` built from its arguments | A thread local buffer, valid until the next call of the same function in the thread |
| `size_t utils_x_r(..., char* buf, size_t size)` | Copied in your buffer, truncated and always `'\0'` terminated, returns the full length like `snprintf` (`0` on error) |
| a string read from a handle | Valid until the handle is changed (said by the comment of the function) |
| bytes (can hold `'\0'`) | A pointer + a size, or a `utils_string_t*` ([SManip](page:smanip#in-c)) |

## Callbacks {#callbacks}

Every callback takes a `void* data` given at its registration, and returns `UTILS_OK` on success. To fail, return `UTILS_KO`, or `utils_fail(code, info)` to give the message: the C++ side turns it into an exception with that info, and the call that ran the callback fails with it. Some callbacks have their own extra values: `C_API_CLI_EXIT` (stop the cli), `C_API_CLI_NO_INPUT` (getc hook).

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

`utils_exception` and the thread local buffers are per thread: read the status in the thread that made the call. The objects keep the thread safety of their C++ class (`safe_mode` arguments, the thread of a `THREAD` cli kept in its handle and joined with `utils_cli_join`).

## Example {#example}

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
