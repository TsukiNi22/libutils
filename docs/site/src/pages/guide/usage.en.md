+++
title = "Link and include"
summary = "Link libutils with CMake, include its root header and choose the sections you need with defines."
section = "guide"
order = 20
keywords = "usage cmake find_package utils::utils include utils.hpp define _Utils _Handling _Tools _Network sections build type debug asan version"
+++

Once [installed](page:installation), a project links the CMake target `utils::utils` and includes one root header, with the sections it needs chosen by defines.

## CMake {#cmake}

```cmake "CMakeLists.txt"
find_package(utils 5.0.0 REQUIRED)                       # minimal version
target_link_libraries(${TARGET} PRIVATE utils::utils)    # includes, static library & OpenSSL
```

The library linked follows the `CMAKE_BUILD_TYPE` of your project:

| Your build type | Library | Package (Linux) | Package (Windows) |
|---|---|---|---|
| `Debug` | `libutils_debug.a` | `libutils-db` | `mingw64-libutils-db` |
| `Asan` | `libutils_asan.a` (build your project with `-fsanitize=address` too) | `libutils-as` | none (no AddressSanitizer with MinGW, use `Debug`) |
| anything else | `libutils.a` (optimized) | `libutils-op` (in `libutils`) | `mingw64-libutils-op` (in `mingw64-libutils`) |

A missing package stops the configuration with the name of the package to install (same names for RPM, DEB and Arch). The same CMake code works for Windows: see [Windows](page:windows#your-project).

## Include {#include}

```cpp "main.cpp"
#define _Network        // only the needed sections (nothing defined = everything)
#include <utils/utils.hpp>

int main(void)
{
    std::cout << utils::version << std::endl; // "v5.0.0"
    return 0;
}
```

`#include <utils.hpp>` is the same as `<utils/utils.hpp>` (a redirection, with a message when `utils/utils.hpp` can't be found). From C the root header is `<utils.h>`: see [Use libutils from C](page:c-api).

> [!IMPORTANT]
> Always include the root header, never a libutils header one by one. The only exception: the [observers](page:security#observer) (`<utils/security/observer/Observer.hpp>`...).

## Sections {#sections}

A define placed before the include enables a section; nothing defined means everything (`_Utils`). How the sections fit together is drawn in [Architecture](page:overview#sections).

| Define | Content |
|---|---|
| `_Utils` | Everything (defined when no other define is set): `_Handling`, `_Tools`, `_Attribute` |
| `_Handling` | `_Exception`, `_Verbose`, `_Pool`, `_Cli`, `_Arguments`, `_Network` |
| `_Tools` | `_Math`, `_Concepts`, `_Encapsulation`, `_System`, `_CustomType`, `_Manip`, `_Algorithms`, `_Security`, `_Platform` |

| Define | Content | Page |
|---|---|---|
| `_Exception` | Exceptions with a type and a code, formatted output | [Exceptions](page:exception) |
| `_Verbose` | Global verbose level and output macros | [Verbose](page:verbose) |
| `_Pool` (`_Middleware`) | Clusters of entities, middleware chains | [Pool](page:pool) |
| `_Cli` | Customizable interactive command line | [Cli](page:cli) |
| `_Arguments` | `argc / argv` parsing and typed settings | [Arguments](page:arguments) |
| `_Network` (`_Socket`) | TCP client / server, sockets | [Network](page:network) |
| `_Math` (`_Geometry`, `_Trigo`) | Coordinates, angles, rotations | [Math](page:math) |
| `_Concepts` | C++20 concepts of the operations | [Concepts](page:concepts) |
| `_Encapsulation` | Pipe, dup, process, poll, shared object, shared memory | [Encapsulation](page:encapsulation) |
| `_System` | Id handler, scheduler, load balancer, GPU memory | [System](page:system) |
| `_CustomType` (`_Vector`, `_Matrix`, `_BLT`) | Vectors, matrices, bidirectional lookup table, freezable, worker | [Type](page:type) |
| `_Manip` (`_IOManip`, `_SManip`) | ANSI sequences, string tools, codecs, parsers | [IOManip](page:iomanip), [SManip](page:smanip) |
| `_Algorithms` (`_C2DMP`, `_SOS`) | String distance, steganography | [Algorithms](page:algorithms) |
| `_Security` (`_Encryption`, `_Observer`) | AES / RSA, observers | [Security](page:security) |
| `_Platform` | POSIX calls for Unix and Windows | [Platform](page:platform) |
| `_Attribute` | Compiler attribute macros | [Attribute](page:attribute) |

The warnings of the library are chosen with defines too: see [Warnings](page:warnings).

## Rules {#rules}

> [!WARNING]
> `utils.hpp` has no global include guard and the defines stay set for the rest of the file: put the defines **before every** include of `utils.hpp` (in a `.hpp` as in a `.cpp`). A bare include after a partial one only gets the sections already defined.

Everything is in the namespace `utils::`. Most methods that can fail take a `failsafe` boolean (warn instead of throwing) or a `safe_mode` boolean (lock, thread safe); see [Architecture](page:overview#conventions).

## Version {#version}

`utils::version` (`"vX.Y.Z"`) and the macro `__LIBUTILS_VERSION__` give the installed version (`"[unknown]"` when `utils/version.hpp` is missing). In C: `utils_version` and `__LIBUTILS_VERSION__` from `utils/version.h`.
