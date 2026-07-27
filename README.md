# Utils
> [!NOTE]
> To have more information look directly in the files

## Dependencies

| Name + Link | Status | Last Update |
| ----------- | ------ | ----------- |
| [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) | ![CD - Algorithm](https://github.com/TsukiNi22/c2dmp-hsm/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/c2dmp-hsm) |
| [s.o.s](https://github.com/TsukiNi22/s.o.s) | ![CD - Algorithm](https://github.com/TsukiNi22/s.o.s/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/s.o.s) |

## Utils - Content

### Table of Contents
 - [Exception](#exception)
 - [Write](#write)
 - [Verbose](#verbose)
 - [Arguments](#arguments)
 - [Cli](#cli)
 - [BLT](#blt)
 - [Vector](#vector)
 - [Attribute](#attribute)
 - [Concepts](#concepts)
 - [Middleware](#middleware)
 - [Algorithms](#algorithms)
 - [Warning](#warning)

### Usage
> [!NOTE]
> Define used to include only part of the Utils lib (by default everything is included)
>
> Included with `utils/utils.hpp` & the `libutils.a`

| Category | Define to include | Content |
| -------- | ----------------- | ------- |
| Utils | `_Utils` | handling, tools and attribute |
| Handling | `_Handling` | exception, write, verbose, arguments and cli |
| Exception | `_Exception` | customized exception |
| Write | `_Write` | different handling for writing edition |
| Verbose | `_Verbose` | some tools used for verbose handling |
| Algorithms | `_Algorithms` | definition of home made algorithms such as the c2dmp-hsm (`_C2DMP`, `_SOS`) |
| Cli | `_Cli` | customizable command line interface |
| Tools | `_Tools` | BLT, vector, concepts, middleware and algorithms |
| BLT | `_Blt` | implementation of a bidirectional lookup table |
| Vector | `_Vector` | definition of vector2<T> and vector3<T> |
| Concepts | `_Concepts` | definition of different concepts |
| Middleware | `_Middleware` | definition of middlwares |
| Arguments | `_Arguments` | utils for arguments (argc/argv) handling |
| Attribute | `_Attribute` | auto select of attribute definition for `fallback`, `c++14`, `c++17` and `c++20` |

## Exception
> [!NOTE]
> Customizable exception (None, Error, Warning, Fatal)

Included from:
```cpp
// Namespace used
using utils::exception

/* define */
#define OK 0
#define KO -1
Type::* // None, Error, Warning, Fatal
Code::* // defined in the json

/* interface */
IException

/* basic */
NoneException
ErrorException
WarningException

/* custom */
CustomException
```

Exception config `cmake/config/exception/*.json` (empty exemple):
> The `.json` are converted in a `.hpp` by the py script located at `cmake/script/generate_exception_header.py`

```json
{
    "errors": [
        {
            "code": "<code_name>", // used with utils::exception::InternalCode::<code_name>
            "message": "<error_message>", // Returned by what()
            "info": "<sub_info>", // Return by info(), can be removed and will be equal to "[None]" by default
            "restrictions": ["<restriction1>", "<restriction2>", ...] // Restrict the <code_name> to certain type of error, can be removed and there will be no restriction
        }
    ]
}
```

## Write
> [!NOTE]
> Writing macro and define for ouput edition

Included from:
```cpp
// Namespace used
using utils::write

/* format */
format // Format a string using <style1|style2|...> to insert ANSI in a string (look at the file `format.hpp` for more details)

/* color */
Color
BackColor

/* char */
Char // Special ascii char such as ESC, ETB, ...

/* style */
Style
ResetStyle

/* ANSI */
// Many functions for ANSI escape sequence (look at the file `ANSI.hpp` for more details)

```

## Verbose
> [!NOTE]
> Definition of tools for verbose

Included from:
```cpp
// Namespace used
using utils::verbose

/* macro */
set_verbose(v) {utils::verbose::verbose = v;}

/* macro verbose display */
// info = str
// level = verbose value
onBasicVerbose(info)
onAdvancedVerbose(info)
onDebugVerbose(info)
onVerbose(level, info)

/* macro verbose execution */
// fn = instructions
// level = verbose value
onBasicVerboseFn(fn)
onAdvancedVerboseFn(fn)
onDebugVerboseFn(fn)
onVerboseFn(level, fn)

/* variables */
verbose // define the global level of verbosity
stdout_lock // mutex that lock the verbose writing (auto handled)
```

## Arguments
> [!NOTE]
> Definition of utils to handle arguments argc/argv

Included from:
```cpp
// Namespace used
using utils::arguments

/* class */
ArgParser // argc/argv parsing and dispatching
Settings // Handle settings extracted from argc/argv
```

## Cli
> [!NOTE]
> Command line interface customizable using flags, hooks and middlewares

Included from:
```cpp
// Namespace used
using utils::cli

/* type */
ParsedData

/* class */
Cli

/* flag */
enum Flag {
    DEBUG           = 1 << 0, // Active verbose for internal action (Nothing for now)
    NOECHO          = 1 << 1, // Disable echo of the input
    CATCH           = 1 << 2, // Enable error catching on execution
    EMPTY_INPUT     = 1 << 3, // Ingore empty input (default: error)
    TRIM            = 1 << 4, // Enable trim on input
    PARSED          = 1 << 5, // Active parser for the input and send vector<std::string> (default: std::string)
    PROMPT          = 1 << 6, // Active the prompt
    LOGIC           = 1 << 7, // Enable logic with '&&', '||' and ';'
    ARROW           = 1 << 8, // Activate left, right
    HISTORY         = 1 << 9, // Activate history, up and down arrow
    PERSISTENT      = 1 << 10, // Allow persistent memory between session (history, ...)
    HINT            = 1 << 11, // Display hint when a command fail
    AUTO_COMPLETION = 1 << 12, // Active auto completion with `\t` (only work on the first command for now)
    MANUAL          = 1 << 13, // Enable manual call for each new input handling
    THREAD          = 1 << 14, // Run in a thread
    DETACHED        = 1 << 15, // Detach the thread execution (by default return the thread at start)
    NO_TTY          = 1 << 16, // Allow usage even without tty
};

// Namespace used
using utils::cli::Flags

/* flags */
constexpr std::uint32_t ALL     = DEBUG | CATCH | NOECHO | TRIM | EMPTY_INPUT | PARSED | PROMPT | LOGIC | ARROW | HISTORY | HINT | AUTO_COMPLETION | MANUAL | THREAD | DETACHED;
constexpr std::uint32_t DEFAULT = CATCH | EMPTY_INPUT | TRIM | PROMPT | ARROW;
constexpr std::uint32_t DUMB    = 0;
constexpr std::uint32_t TERM1   = CATCH | EMPTY_INPUT | TRIM | PARSED | PROMPT | EMPTY_INPUT | LOGIC | ARROW | HISTORY;
constexpr std::uint32_t TERM2   = TERM1 | HINT | AUTO_COMPLETION;
constexpr std::uint32_t TERM3   = TERM2 | THREAD;
constexpr std::uint32_t LOG     = TERM3 | DETACHED | NO_TTY;
constexpr std::uint32_t DEV     = TERM2 | DEBUG;
constexpr std::uint32_t MULTI_THREADING = THREAD | DETACHED;
/*
 * DEFAULT -> Basic term
 * TERM1   -> Advenced term
 * TERM2   -> Completion on advenced term
 * TERM3   -> Multi threading advenced term
*/
```

## BLT
> [!NOTE]
> Bidirectional Lookup Table definition

Included from:
```cpp
// Namespace used
using utils::blt

/* class */
utils::btl::BidirectionalLookupTable<L, R, ...>
utils::btl::BidirectionalLookupTable<T, ...>
```

## Vector
> [!NOTE]
> Definition of vector2 and vector3

Included from:
```cpp
// Namespace used
using utils::vector

/* interface */
IVector<T>

/* vectorX */
Vector2<T>
Vector3<T>

/* optimized vectorX */
// Unsafe~ vector, no type check
OVector2<T>
OVector3<T>
```

## Attribute
> [!NOTE]
> Auto selection of attribute define for `fallback`, `c++14`, `c++17` and `c++20`

## Concepts
> [!NOTE]
> Different concept global, operation, ...

## Middleware
> [!NOTE]
> Definition of middlwares

Included from:
```cpp
// Namespace used
using utils::middleware

/* type */
// Store a function
Middleware<T>
Middleware<void>

/* class */
// Store Middleware (before & after)
Middlewares<T, U>
Middlewares<T, void>
Middlewares<void, T>
Middlewares<void, void>
```

## Algorithms

### [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm)
Included from:
```cpp
// Namespace used
using utils::algorithms

/* c2dmp-hsm */
c2dmp::c2dmp // You should call this one, automatic redirection to the best one
c2dmp::c2dmp_optimized // Optimized version of the c2dmp-hsm and the one used by default
c2dmp::c2dmp_simplified // Semi Optimized version of the c2dmp-hsm (deprecated)
```

### [s.o.s](https://github.com/TsukiNi22/s.o.s)
Included from:
```cpp
// Namespace used
using utils::algorithms

/* s.o.s */
sos::sos_embed & sos::sos_extract // You should call these, automatic redirection to the best one
sos::sos_embed_optimized & sos::sos_extract_optimized // Optimized versions of the s.o.s and the one used by default

/* tools */
sos::to_bytes // Convert any range type into an array of byte
sos::bytes_to // Convert any array of byte into a range type
```

## Warning
> [!NOTE]
> Used for internal automatic warning

> [!CAUTION]
> Not mean to be used with external class (should probably work)
> 
> Can be hard included with `utils/warning/Observer.hpp` & `utils/warning/UnsafeObserver.hpp`

Included from:
```cpp
// Namespace used
using utils::warning

/* class */
// Used as a parent for a class to be observed, ex: class Cli: private utils::warning::Observer
Observer // Thread-safe
UnsafeObserver // NOT Thread-safe

/* class */
// DO NOT USE IT MANUALY!!!
SharedObject // Auto warning for unfreed class (mostly used for shared object)
```
