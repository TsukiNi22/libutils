+++
title = "Exceptions"
summary = "Exceptions with a type and a code, their formatted output, every code of the library, and the status of the C API."
keywords = "exception error code type IException ErrorException FatalException NoneException WarningException CustomException formated InternalCode ExternalCode utils_exception status"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Exception` | `_Handling` | `utils::exception` | `utils/exception/` |

Every error of the library is reported by an exception carrying a **type** (how serious it is) and a **code** (what happened), never by a return code. To add the codes of your project, see [Custom exception codes](page:custom-exceptions); the model is explained in [Errors](page:errors).

## Classes {#classes}

| Class | Type | Description |
|---|---|---|
| `IException` | - | Interface of every exception of the library (inherits `std::exception`) |
| `AException` | - | Common implementation: code, type, info, source location, formatted output |
| `NoneException` | `None` | Not an error, more like an exit (ex: `-h` handled by `ArgParser`) |
| `WarningException` | `Warning` | Something went wrong but the program can continue |
| `ErrorException` | `Error` | Error of the call |
| `CustomException` | given | Exception with the type given by the user (a `Type` combination) |
| `FatalException` | `Fatal` | Prints the exception and **ends the program with `std::abort()` from its constructor** (no RAII cleanup) |

Every class has the constructors `(code)` and `(code, info)` for an `InternalCode` (library) or an `ExternalCode` (your project); the source location is taken automatically.

| Method (`IException`) | Returns | Description |
|---|---|---|
| `what()` | `const char*` | Message of the code (from the JSON config) |
| `info()` | `const char*` | Info given at the construction, the default one of the code, or `"[None]"` |
| `getCode()` | `InternalCode` | Code of the exception (an external code is converted) |
| `getType()` | `Type` | Type of the exception |
| `isNone()` / `isFatal()` | `bool` | Shortcut on the type |
| `loc()` | `const std::source_location&` | Where the exception was built |
| `formated()` | `std::string` | Full colored output (see [Output](#output)) |

`utils::exception::format(type, file, line, module, what, function, info)` builds the same output from its fields (used by `formated()` and by the C API).

## Type {#type}

`utils::exception::Type` is a bit set, combined with `|`, `&`, `^`, `~` (and their assignment versions):

| Type | Value |
|---|---|
| `None` | `0b0001` |
| `Fatal` | `0b0010` |
| `Error` | `0b0100` |
| `Warning` | `0b1000` |

> [!IMPORTANT]
> A code can restrict the types it accepts (column **Restrictions** of the [codes](#codes)). Building an exception with a forbidden type ends with a `FatalException(ExceptionCodeRestriction)`.

## Throw and catch {#throw-catch}

```cpp
#define _Exception
#include <utils/utils.hpp>

try {
    throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "the size must be > 0");
} catch (const utils::exception::IException& e) {
    if (e.getCode() == utils::exception::InternalCode::InvalidArgument)
        std::cerr << e.formated() << std::endl;
}

throw utils::exception::NoneException(utils::exception::InternalCode::Exit);                                // not an error
throw utils::exception::CustomException(utils::exception::Type::Error | utils::exception::Type::Warning, "info");
utils::exception::FatalException(utils::exception::InternalCode::Mmap, "shm");                              // prints & aborts
```

> [!TIP]
> Many functions of the library take a `failsafe` boolean: when it is `true`, the error is printed as a `WarningException` on `std::cerr` instead of being thrown.

## Output {#output}

`std::cerr << e.formated() << std::endl;`

```text
[<type>] ...<file_path:hyperlink>:<line> (<origin>) -> <message>
-------------------------------------------
<function_demangled> = <info>
-------------------------------------------
```

## In C {#in-c}

The C API never lets an exception cross the C boundary: each call is guarded, returns `UTILS_OK` / `UTILS_KO` (or `NULL` / `0`) and, on failure, fills the thread local **status** `utils_exception` (read like `errno`). The full guide is [Use libutils from C](page:c-api).

| C++ | C |
|---|---|
| `catch (const IException& e)` | `if (ret != UTILS_OK)` then read `utils_exception` |
| `e.getType()` / `e.getCode()` | `utils_exception.type` / `utils_exception.code` (`C_API_OTHER_EXCEPTION_CODE` for a non-libutils exception) |
| `e.what()` / `e.info()` | `utils_exception.what` / `utils_exception.info` |
| `e.loc()` | `utils_exception.file`, `.line`, `.function`, `.module` |
| `e.formated()` | `utils_exception_format()` / `utils_exception_format_r(buf, size)` |
| `throw ErrorException(code, info)` in a callback | `return utils_fail(code, info);` |

`utils_exception.exception` is `true` when the **last** call failed; `origin` tells where the error comes from (`C_API_EXCEPTION_ORIGIN_INTERNAL`, `_EXTERNAL`, `_STD`, `_UNKNOWN`, `_C`).

```c
#define _Exception
#define _Verbose
#include <utils.h>
#include <stdio.h>

int main(void)
{
    if (utils_verbose_log(C_API_VERBOSE_BASIC, "%s", "hello") != UTILS_OK) {
        fprintf(stderr, "%s\n", utils_exception_format()); // same output as formated()
        return 1;
    }
    utils_fail(42, "set from C");                         // origin C_API_EXCEPTION_ORIGIN_C, returns UTILS_KO
    printf("%llu %s\n", (unsigned long long)utils_exception.code, utils_exception.info); // 42 set from C
    utils_exception_clear();
    return 0;
}
```
