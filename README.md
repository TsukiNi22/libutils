# Utils

> [!TIP]
> Documentation [TsukiNi22/libutils/wiki](https://github.com/TsukiNi22/libutils/wiki) (v4.1.0).
> Technical documentation and project graph: [tsukini22.github.io/libutils/docs](https://tsukini22.github.io/libutils/docs/).

C++20 utility library (`utils::`): exceptions, verbose, CLI, arguments, network, IPC, encryption, custom types, algorithms.
Built as a static library and distributed as RPM/DEB packages (optimized, debug and asan variants, plus the `mingw64-libutils*` packages for Windows), or installed from the sources with CMake.
Runs on Unix (Linux, BSD, macOS) and on Windows (cross compiled with MinGW-w64), see [Windows](#windows-cross-compilation).

### Table of Contents
 - [Dependencies](#dependencies)
 - [Packages](#packages)
 - [Quick Setup 1 (All)](#quick-setup---1-all)
 - [Quick Setup 2 (Limited)](#quick-setup---2-limited)
 - [Windows (cross compilation)](#windows-cross-compilation)
 - [Usage](#usage)
 - [Unit tests](#unit-tests)
 - [Workflows/Release](#workflowsrelease)

## Dependencies

> [!CAUTION]
> The MIT license of this project ([LICENSE](LICENSE)) only applies to the code of `libutils` itself.
> It does **not** apply to the dependencies, included or linked: each one keeps its own license (see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)).
>
> The algorithms synced in `include/utils/algorithms/` are under **CC BY-NC-SA 4.0** ([LICENSE.md](include/utils/algorithms/LICENSE.md)): no commercial use without an agreement with the author.

| Name + Link | License | Status | Last Update |
| ----------- | ------- | ------ | ----------- |
| [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) | CC BY-NC-SA 4.0 | ![CD - Algorithm](https://github.com/TsukiNi22/c2dmp-hsm/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/c2dmp-hsm) |
| [s.o.s](https://github.com/TsukiNi22/s.o.s) | CC BY-NC-SA 4.0 | ![CD - Algorithm](https://github.com/TsukiNi22/s.o.s/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/s.o.s) |
| [OpenSSL](https://www.openssl.org) | Apache-2.0 | linked (system package) | — |

| Name | Version | Fedora (`dnf`) | Debian/Ubuntu (`apt`) |
| ---- | ------- | -------------- | --------------------- |
| `clang++` | C++20 | `clang` | `clang` |
| `cmake` | >= 3.20 | `cmake` | `cmake` |
| OpenSSL | >= 3.0 | `openssl-devel` | `libssl-dev` |
| GoogleTest (tests only) | — | `gtest-devel` | `libgtest-dev` |
| MinGW-w64 (Windows only) | GCC >= 13 | `mingw64-gcc-c++ mingw64-openssl` | `g++-mingw-w64-x86-64` + OpenSSL built for MinGW (no package) |
| Wine (Windows tests only) | — | `wine` | `wine64` |

## Packages

> [!NOTE]
> Most packages have a pre-release/unstable version named `<package_name>-pre`.
> The `-pre` packages are marked as obsolete by any release/stable package (without `-pre`) of the same version or higher.

| File Name | Content |
| --------- | ------- |
| `libutils` | Install the default packages (`libutils-dev`, `libutils-op`) |
| `libutils-dev` | All the includes and the global CMake configuration (requires `openssl-devel` / `libssl-dev`) |
| `libutils-dev-op` | CMake configuration for `libutils-op` |
| `libutils-dev-db` | CMake configuration for `libutils-db` |
| `libutils-dev-as` | CMake configuration for `libutils-as` |
| `libutils-op` | Library compiled with the optimization options |
| `libutils-db` | Library compiled with the debug options (intended for debugging only) |
| `libutils-as` | Library compiled with the asan options (intended for debugging only) |

**Windows** (MinGW-w64 cross compilation, installed in the MinGW sysroot: `/usr/x86_64-w64-mingw32/sys-root/mingw` on Fedora, `/usr/x86_64-w64-mingw32` on Debian/Ubuntu)

| File Name | Content |
| --------- | ------- |
| `mingw64-libutils` | Install the default Windows packages (`mingw64-libutils-dev`, `mingw64-libutils-op`) |
| `mingw64-libutils-dev` | All the includes and the global CMake configuration (requires `mingw64-openssl` on Fedora, OpenSSL for MinGW built by hand on Debian) |
| `mingw64-libutils-dev-op` | CMake configuration for `mingw64-libutils-op` |
| `mingw64-libutils-dev-db` | CMake configuration for `mingw64-libutils-db` |
| `mingw64-libutils-op` | Windows library compiled with the optimization options |
| `mingw64-libutils-db` | Windows library compiled with the debug options (no asan version with MinGW) |

## Quick Setup - 1 (all)
> Setup into `/usr/local`

### Clone the repository
```bash
git clone https://github.com/TsukiNi22/libutils.git
cd libutils
```

### Install the lib
```bash
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR
sudo cmake --build $BUILD_DIR --target install --parallel $(nproc)
```

## Quick Setup - 2 (Limited)
> Setup into `/usr`

> [!WARNING]
> If during the installation using tools like `dnf`, `apt`... you see an `invalid hash` error, reset the cache of the tool or relaunch the `setup.sh` script.
>
> If the error still persists, it might be due to an error in the repository.

> [!WARNING]
> Restriction: `fedora-based (rpm)`, `debian-based (deb)`

> [!NOTE]
> The usage of `sudo` in the script can be removed with the `--no-sudo` argument.

Run the setup script directly, without cloning the repository. It registers the libutils package repository so the system package manager can install the packages:
1. Detect the distribution from `/etc/os-release` (`rpm` or `deb` based).
2. `rpm`: add `/etc/yum.repos.d/libutils.repo` and import the GPG key.
3. `deb`: add the GPG key in `/etc/apt/keyrings/libutils.gpg`, the source in `/etc/apt/sources.list.d/libutils.list`, then `apt-get update`.

```bash
wget -qO- https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s
```

or with `curl`:

```bash
curl -fsSL https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s
```

Then install the packages: `sudo dnf install libutils` or `sudo apt install libutils`.

| Argument | Effect |
| -------- | ------ |
| `--no-sudo` | Run without `sudo` (the script must already run as root) |
| `-h`, `--help` | Display the usage |

## Windows (cross compilation)
> Build `libutils.a` for Windows (x86_64) from Linux with the MinGW-w64 toolchain

From the packages (after the `setup.sh` of [Quick Setup - 2](#quick-setup---2-limited)), installed in the MinGW sysroot:

```bash
sudo dnf install mingw64-libutils-pre       # Fedora: + mingw64-libutils-db-pre for Debug (requires mingw64-openssl)
sudo apt install mingw64-libutils-pre       # Debian/Ubuntu: OpenSSL for MinGW must be built by hand
```

From the sources:

```bash
export BUILD_DIR=build-windows
cmake -S . -B $BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Optimized
cmake --build $BUILD_DIR --parallel $(nproc)
cmake --install $BUILD_DIR --prefix <windows_prefix>          # include/, lib/libutils.a, lib/cmake/utils/
cmake --build $BUILD_DIR --target package                       # mingw64-libutils-* RPM/DEB (MinGW sysroot)
```

> [!NOTE]
> Every class has the same API on Unix and Windows: a program builds for both without any change, only the implementation used by the library differs.

| Part | Windows implementation |
| ---- | ---------------------- |
| `utils::network::TCPSocket` (and `is_ip`, `resolve_*`) | Winsock 2 (`ws2_32`, linked by `utils::utils`), started on the first use |
| `utils::network::Client` / `Server` | `utils::platform::epoll_*` (emulation over `WSAPoll`) |
| `utils::encapsulation::Poll` | Same `struct epoll_event` / `EPOLL*` API, emulated over `WSAPoll` |
| `utils::encapsulation::Process` | `CreateProcess` / `WaitForSingleObject` / `TerminateProcess`, `replace` with the CRT `_execvp` |
| `utils::encapsulation::SharedMemory` | `CreateFileMapping` / `MapViewOfFile` (`Local\utils-shm-<name>`), named event instead of the futex |
| `utils::encapsulation::SharedObject` | `LoadLibrary` / `GetProcAddress` / `FreeLibrary` (`.dll`) |
| `utils::encapsulation::Pipe` / `Dup` | CRT `_pipe` (binary mode) / `_dup` / `_dup2` |
| `utils::cli::Cli` | `utils::platform::tcgetattr` / `tcsetattr` on the console (VT input), `utils::platform::poll` / `read` on the console input |
| `utils::iomanip` reports (`read_cursor_position`, mouse) | CRT `_read` on the standard input |
| `utils::iomanip::enable_ansi()` | Enables the virtual terminal mode of the console (call it once at the start, `true` on Unix) |
| Exceptions / observers module name | `utils::platform::dladdr` / `readlink("/proc/self/exe")` (`GetModuleHandleEx` / `GetModuleFileName`) |

> [!TIP]
> The library only calls the system through `utils::platform::` (section `_Platform`), the same calls are usable by a program:
> same names and contract as the POSIX functions (-1 & `errno`, the Winsock errors are translated in `errno`), so the same code builds on Unix and Windows.
>
> | Header | Calls |
> | ------ | ----- |
> | `utils/platform/Unistd.hpp` | `close`, `read`, `write`, `pipe`, `dup`, `dup2`, `isatty`, `poll`, `getpid`, `readlink`, `strerror` |
> | `utils/platform/Socket.hpp` | `socket`, `connect`, `bind`, `listen`, `accept`, `recv`, `send`, `setsockopt`, `getsockname`, `getpeername`, `getaddrinfo`, `freeaddrinfo`, `gai_strerror` |
> | `utils/platform/Epoll.hpp` | `epoll_create1`, `epoll_ctl`, `epoll_wait` (closed by `close`) |
> | `utils/platform/Dlfcn.hpp` | `dlopen`, `dlsym`, `dlclose`, `dlerror`, `dladdr` |
> | `utils/platform/Termios.hpp` | `tcgetattr`, `tcsetattr` |
>
> On Windows the structures of Winsock (`sockaddr_in`, `addrinfo`, `pollfd`...) come from `<winsock2.h>` / `<ws2tcpip.h>`, included by the program (not by libutils).

> [!WARNING]
> Without equivalent on Windows, `FatalException` with `UndefinedCall` (like the raw calls of `ASocket`):
> - `Process::spawn(void)`: a `fork`, use `spawn(path, args)`
> - `Process::dup(origin, clone)` with a `clone` other than `0`, `1`, `2`: only the standard fds are given to the child
>
> Different behaviors: `Poll` / `Client` / `Server` only handle sockets (`ENOTSOCK` -> `EPoll` error for a pipe), `EPOLLET` is refused (level triggered only), a `Process` has no signal (`Status::sig` = `0`, killed = exit code `137`), ctrl+z is a char for the `Cli` (no `SIGTSTP`), a `SharedMemory` is destroyed with its last handle (no `shm_unlink` persistence).

> [!IMPORTANT]
> The complete `<windows.h>` declares a member named `_unused` (`winioctl.h`), which is a libutils attribute macro.
> Include `<windows.h>` **before** `<utils/utils.hpp>`, or define `WIN32_LEAN_AND_MEAN` before including it.

> [!NOTE]
> No AddressSanitizer with MinGW: the `Asan` mode is built like `Debug`. The tests (`-DBUILD_TESTS=ON`) download GoogleTest and run with `wine` through `ctest`.

## Usage

```cmake
find_package(utils 4.1.0 REQUIRED)                      # installed CMake configuration
target_link_libraries(${TARGET} PRIVATE utils::utils)   # includes, OpenSSL & library
```

The same CMake code works for Linux and Windows: for Windows, configure the project with a MinGW toolchain whose
`CMAKE_FIND_ROOT_PATH` is the MinGW sysroot (`-DCMAKE_TOOLCHAIN_FILE=`: the `cmake/toolchain/mingw-w64.cmake` of libutils,
or `/usr/share/mingw/toolchain-mingw64.cmake` / `mingw64-cmake` on Fedora), `find_package` then finds the `mingw64-libutils*`
packages and `utils::utils` also links `ws2_32`. The library follows the `CMAKE_BUILD_TYPE` of the project:

| Build type | Library | Package (Linux) | Package (Windows) |
| ---------- | ------- | --------------- | ----------------- |
| `Debug` | `libutils_debug.a` | `libutils-db` | `mingw64-libutils-db` |
| `Asan` | `libutils_asan.a` | `libutils-as` | none (no AddressSanitizer with MinGW, use `Debug`) |
| anything else | `libutils.a` | `libutils-op` (in `libutils`) | `mingw64-libutils-op` (in `mingw64-libutils`) |

A missing package stops the configuration with the name of the package to install.

```cpp
#define _Utils          // every section (or only the needed ones: _Network, _Cli, ...)
#include <utils.hpp>    // same as <utils/utils.hpp>
```

> [!WARNING]
> Everything is defined within the namespace `utils::`.

| Macro | Content |
| ----- | ------- |
| `_Utils` | Every section below (`_Handling`, `_Tools`, `_Attribute`) |
| `_Handling` | `_Exception`, `_Verbose`, `_Pool`, `_Cli`, `_Arguments`, `_Network` |
| `_Tools` | `_Math`, `_Concepts`, `_Encapsulation`, `_System`, `_CustomType`, `_Manip`, `_Algorithms`, `_Security`, `_Platform` |
| `_Exception` | `utils::exception::` error / warning / fatal exceptions with codes |
| `_Network` | `utils::network::` sockets, `Client`, `Server` |
| `_CustomType` | `utils::type::` `BidirectionalLookupTable`, vectors, matrices |
| `_Algorithms` | `utils::algorithms::` `c2dmp-hsm`, `s.o.s` |
| `_Security` | `utils::security::` encryption (AES, RSA), observers |
| `_Warning` / `_NoWarning` | Enable / disable every deprecation, linker and usage warning |
| `_Platform` | `utils::platform::` POSIX calls for Unix & Windows (`close`, `socket`, `epoll_*`, `dlopen`, `tcgetattr`...) |
| `UTILS_UNIX` / `UTILS_WINDOWS` | Platform detected by `<utils/platform/Platform.hpp>` (always included), `UTILS_PLATFORM` = `"unix"` / `"windows"` |

## Unit tests

```bash
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build $BUILD_DIR --parallel $(nproc)
ctest --test-dir $BUILD_DIR --output-on-failure --timeout 30   # or ./unit_tests (written at the root)
```

> [!NOTE]
> The tests use GoogleTest (`tests/`, one file per module) and run in the `Unit Tests - Libraries (CI)` workflow.

## Workflows/Release
### Workflows
- `Dispatch (CI/CD)`: runs on every push (branch `main` or tag `v*`, not for `*.md` / `*.txt` only changes) and decides what to trigger from the ref / commit message
- `Unit Tests - Libraries (CI)`: builds and runs the unit tests, always first
- `Build - Packages (CI/CD)`: builds the packages (RPM/DEB: Linux in the `ci` image, Windows `mingw64-libutils*` in the `build-packages-windows` job on `mingw64-ci`), signs them and publishes them on the `gh-pages` repository
- `Build - Libraries (CI)`: only builds the libraries and checks the compilation, without producing or publishing packages

> [!NOTE]
> `Build - Libraries (CI)` only runs when `Dispatch (CI/CD)` decides **not** to build the packages, to check that the code still compiles.

> [!TIP]
> A commit containing `[ignore]` (preferably in the description) skips the whole CI.

### Pre-Release (unstable)
The pre-release of the packages (`<package_name>-pre`) is triggered by:
- a tag with a suffix, matching `vx.x.x-<suffix>` (ex: `v4.0.1-pre`, `x` = `major`, `minor`, `fix`)
- or a commit containing `[build]`, preferably in the description

> [!NOTE]
> Only the 5 last versions of each `-pre` package are kept in the package repository, every release is kept.

### Release (stable)
The release of the packages (channel `stable`) is only triggered by a tag matching exactly `vx.x.x` (ex: `v4.0.0`).

### Downstream notification
`Dispatch (CI/CD)` notifies the `docker-image` repository (`repository_dispatch`) after the packages are published:
- on every package build (release or pre-release)
- or on a commit containing `[release]`, preferably in the description
