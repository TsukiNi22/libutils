+++
title = "Windows"
summary = "Build libutils and your program for Windows from Linux with MinGW-w64: requirements, packages, the toolchain, your project, the code rules and the tests."
section = "guide"
order = 60
keywords = "windows mingw mingw-w64 cross compilation toolchain mingw64-libutils sysroot wine exe winsock windows.h WIN32_LEAN_AND_MEAN static"
+++

Windows x86_64, cross compiled from Linux with MinGW-w64 (toolchain `cmake/toolchain/mingw-w64.cmake`, packages `mingw64-libutils*` since `v4.1.0`). Every class and the [Platform](page:platform) calls are implemented with Win32 / Winsock / the CRT, so a program builds for both platforms without change; only a few calls without a Windows equivalent raise a `FatalException` (`UndefinedCall`). The design is explained in [Platforms](page:platforms).

## Requirements {#requirements}

| Name | Fedora (`dnf`) | Debian / Ubuntu (`apt`) |
|---|---|---|
| MinGW-w64 (GCC >= 13) | `mingw64-gcc-c++` | `g++-mingw-w64-x86-64` |
| OpenSSL for MinGW | `mingw64-openssl` | built for MinGW (no package) |
| Wine (tests / running the `.exe` on Linux) | `wine` | `wine64` |

> [!NOTE]
> The MinGW **sysroot** holds the Windows headers and libraries of the target: `/usr/x86_64-w64-mingw32/sys-root/mingw` on Fedora, `/usr/x86_64-w64-mingw32` on Debian / Ubuntu. The packages of libutils and OpenSSL for Windows are installed there.

## Install {#install}

From the packages (after the `setup.sh` of [Installation](page:installation#packages)), installed in the MinGW sysroot:

```bash "Terminal"
sudo dnf install mingw64-libutils       # Fedora: headers, CMake config & optimized library
sudo dnf install mingw64-libutils-db    # Debug library (optional)
sudo apt install mingw64-libutils       # Debian / Ubuntu: OpenSSL for MinGW must be built by hand
```

| Package | Content |
|---|---|
| `mingw64-libutils` | `mingw64-libutils-dev` + `mingw64-libutils-op` |
| `mingw64-libutils-dev` | Headers and global CMake configuration (requires `mingw64-openssl` on Fedora) |
| `mingw64-libutils-dev-op` / `-dev-db` | CMake configuration of the optimized / debug library |
| `mingw64-libutils-op` / `-db` | `libutils.a` / `libutils_debug.a` for Windows (no asan with MinGW) |

From the sources (any prefix):

```bash "Terminal"
export BUILD_DIR=build-windows
cmake -S . -B $BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Optimized
cmake --build $BUILD_DIR --parallel $(nproc)
cmake --install $BUILD_DIR --prefix <windows_prefix>    # include/, lib/libutils.a, lib/cmake/utils/
cmake --build $BUILD_DIR --target package                 # mingw64-libutils-* RPM/DEB (MinGW sysroot)
```

## The toolchain {#toolchain}

`cmake/toolchain/mingw-w64.cmake` (in the repository, usable by any project):

| Setting | Value |
|---|---|
| `CMAKE_SYSTEM_NAME` | `Windows` (`WIN32` and `CMAKE_CROSSCOMPILING` are true) |
| Compilers | `${MINGW_PREFIX}-gcc` / `-g++` / `-windres`, `MINGW_PREFIX` = `x86_64-w64-mingw32` by default (`-DMINGW_PREFIX=<triplet>`) |
| `CMAKE_FIND_ROOT_PATH` | The sysroot found: `/usr/<triplet>/sys-root/mingw` (Fedora) and / or `/usr/<triplet>` (Debian) |
| Find modes | Programs on the host (`python3`, `ccache`), libraries / headers / packages only in the sysroot |
| `CMAKE_CROSSCOMPILING_EMULATOR` | `wine64` or `wine` when installed, with `WINEPATH=<sysroot>/bin` and `WINEDEBUG=-all`: `ctest` runs the `.exe` |

> [!TIP]
> Any MinGW toolchain whose `CMAKE_FIND_ROOT_PATH` is the sysroot works the same: `/usr/share/mingw/toolchain-mingw64.cmake` (or the `mingw64-cmake` wrapper) on Fedora.

## Your project {#your-project}

**The same CMake code works for Linux and Windows**, only the configuration command changes:

```cmake "CMakeLists.txt"
find_package(utils 5.0.0 REQUIRED)
target_link_libraries(${TARGET} PRIVATE utils::utils)   # includes, OpenSSL, ws2_32 on Windows & library
```

```bash "Terminal"
cmake -S . -B build                                                                           # Linux
cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=<libutils>/cmake/toolchain/mingw-w64.cmake  # Windows
```

`Debug` links `mingw64-libutils-db`, anything else `mingw64-libutils-op`; there is no Asan with MinGW (build in `Debug`). A missing package stops the configuration with its name.

> [!TIP]
> A standalone `.exe` (without the dll of the MinGW runtime next to it): `if(WIN32) target_link_options(${TARGET} PRIVATE -static-libgcc -static-libstdc++ -static) endif()`. OpenSSL stays a dll (`libcrypto-3-x64.dll`, `libssl-3-x64.dll` from the sysroot `bin/`) unless its static libraries are installed.

## Code {#code}

| Rule | Why |
|---|---|
| Include `<windows.h>` **before** `<utils/utils.hpp>` (or define `WIN32_LEAN_AND_MEAN`) | The complete `<windows.h>` declares a member named `_unused`, a libutils attribute macro |
| Call `utils::iomanip::enable_ansi()` once at the start | Enables the virtual terminal mode of the console (colors, cursor); returns `true` on Unix |
| Use `utils::platform::` for the system calls | Same names and contract as POSIX (`-1` and `errno`), see [Platform](page:platform) |
| Test the platform with `UTILS_WINDOWS` / `UTILS_UNIX` | Defined by `utils/platform/Platform.hpp` (always included), `UTILS_PLATFORM` = `"windows"` / `"unix"` |
| The Winsock structures (`sockaddr_in`, `addrinfo`, `pollfd`...) | Come from `<winsock2.h>` / `<ws2tcpip.h>`, included by the program (not by libutils) |

Different behaviors on Windows (details in [Platform](page:platform#windows-behavior), [Encapsulation](page:encapsulation#process), [Cli](page:cli#start-stop)):

- `Poll`, `Client` and `Server` only handle sockets (a pipe → `EPoll` error), `EPOLLET` is refused (level triggered only);
- a `Process` has no signal (`Status::sig` = `0`, killed = exit code `137`), only the fds `0`, `1`, `2` are given to a child;
- a `SharedMemory` is destroyed with its last handle (no `shm_unlink` persistence);
- <kbd>Ctrl</kbd>+<kbd>Z</kbd> is a char for the `Cli` (no `SIGTSTP`).

The C API works the same on Windows (the C structures of the POSIX calls come from `utils/platform/Types.h`).

## Tests {#tests}

```bash "Terminal"
cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build-windows --parallel $(nproc)     # unit_tests.exe at the root of the repository
ctest --test-dir build-windows --output-on-failure --parallel $(nproc)
```

- GoogleTest: the one built for MinGW in the sysroot (`find_package(GTest CONFIG)`, ex: the `mingw64-unit-tests` image), else downloaded and built from the sources (`FetchContent`).
- `ctest` runs `unit_tests.exe` with `wine`; the tests are listed when `ctest` starts (`DISCOVERY_MODE PRE_TEST`), and the first start of wine creates its prefix (`~/.wine`), which can take a few minutes.
- `tests/platform/Platform.cpp` runs on both platforms, `tests/platform/Windows.cpp` only on Windows.

## Example {#example}

```cpp "main.cpp"
#define _Platform
#define _Network
#include <utils/utils.hpp>
#include <iostream>

int main(void)
{
    std::cout << utils::version << " " << UTILS_PLATFORM          // "v5.0.0 windows"
        << " " << utils::network::resolve_hostname("localhost")   // "127.0.0.1" (Winsock)
        << std::endl;
    return 0;
}
```

```bash "Terminal"
cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=<libutils>/cmake/toolchain/mingw-w64.cmake
cmake --build build-windows
WINEPATH=/usr/x86_64-w64-mingw32/sys-root/mingw/bin wine build-windows/<program>.exe
```

| Code | Raised when |
|---|---|
| `UndefinedCall` | A call without Windows equivalent: `Process::spawn()` (fork), `Process::dup` on a fd other than `0`, `1`, `2`, the raw calls of `ASocket` (`FatalException`) |
| `EPoll` | `Poll` / `Client` / `Server` on a non socket fd, or `EPOLLET` |
