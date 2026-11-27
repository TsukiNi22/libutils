# Utils

<p align="center">
  <img alt="Version" src="https://img.shields.io/github/v/release/TsukiNi22/libutils">
  <img alt="CI" src="https://github.com/TsukiNi22/libutils/actions/workflows/dispatch.yml/badge.svg">
  <img alt="Last commit" src="https://img.shields.io/github/last-commit/TsukiNi22/libutils">
  <img alt="License" src="https://img.shields.io/github/license/TsukiNi22/libutils">
</p>

> [!TIP]
> Documentation [tsukini22.github.io/libutils](https://tsukini22.github.io/libutils/docs/) (v5.0.0, EN / FR), [wiki](https://github.com/TsukiNi22/libutils/wiki), [project graph](https://tsukini22.github.io/libutils/docs/graph.html).

C++20 utility library (`utils::`): exceptions, verbose, CLI, arguments, network, IPC, GPU memory, encryption, types.
Also callable from C (`<utils.h>`), on Unix and on Windows (MinGW-w64).

### Table of Contents
 - [Dependencies](#dependencies)
 - [Packages](#packages)
 - [Quick Setup 1 (All)](#quick-setup---1-all)
 - [Quick Setup 2 (Limited)](#quick-setup---2-limited)
 - [Windows (cross compilation)](#windows-cross-compilation)
 - [Usage](#usage)
 - [C API](#c-api)
 - [Unit tests](#unit-tests)
 - [Pre-commit hook](#pre-commit-hook)
 - [Documentation](#documentation)
 - [Workflows/Release](#workflowsrelease)
 - [License](#license)

## Dependencies

> [!CAUTION]
> This project's license does not apply to the content of the dependencies used by `libutils`.

| Name + Link | License | Status | Last Update |
| ----------- | ------- | ------ | ----------- |
| [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) | CC BY-NC-SA 4.0 | ![CD - Algorithm](https://github.com/TsukiNi22/c2dmp-hsm/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/c2dmp-hsm) |
| [s.o.s](https://github.com/TsukiNi22/s.o.s) | CC BY-NC-SA 4.0 | ![CD - Algorithm](https://github.com/TsukiNi22/s.o.s/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/s.o.s) |
| [OpenSSL](https://www.openssl.org) | Apache-2.0 | linked (system package) | - |

Build requirements (only needed for the [Quick Setup 1](#quick-setup---1-all)):

| Name | Version | Fedora (`dnf`) | Debian/Ubuntu (`apt`) | Arch (`pacman`) |
| ---- | ------- | -------------- | --------------------- | --------------- |
| `clang++` / `cmake` | C++20 / `>= 3.20` | `clang cmake` | `clang cmake` | `clang cmake` |
| OpenSSL | `>= 3.0` | `openssl-devel` | `libssl-dev` | `openssl` |
| GoogleTest (tests) | - | `gtest-devel` | `libgtest-dev` | `gtest` |
| `bsdtar` (Arch packages) | - | `bsdtar` | `libarchive-tools` | `libarchive` |
| MinGW-w64 (Windows) | GCC `>= 13` | `mingw64-gcc-c++ mingw64-openssl` | `g++-mingw-w64-x86-64` (OpenSSL built by hand) | - |
| Wine (Windows tests) | - | `wine` | `wine64` | `wine` |
| GPU driver (`HybridMemory`, runtime) | - | `libcuda` / `ocl-icd` / `vulkan-loader` | `libcuda1` / `ocl-icd-libopencl1` / `libvulkan1` | `nvidia-utils` / `ocl-icd` / `vulkan-icd-loader` |

## Packages

> [!NOTE]
> `<package>-pre` is the pre-release: a stable package of the same version or newer replaces it.

Same names and content as RPM (Fedora), DEB (Debian/Ubuntu) and Arch (pacman).

| File Name | Content |
| --------- | ------- |
| `libutils` | `libutils-dev` + `libutils-op` |
| `libutils-dev` | Headers and CMake configuration (requires OpenSSL headers) |
| `libutils-dev-op` | CMake configuration of `libutils-op` |
| `libutils-dev-db` | CMake configuration of `libutils-db` |
| `libutils-dev-as` | CMake configuration of `libutils-as` |
| `libutils-op` | Optimized library |
| `libutils-db` | Debug library (debugging only) |
| `libutils-as` | AddressSanitizer library (debugging only) |

Windows, installed in the MinGW sysroot ([`readme/windows.md`](readme/windows.md)):

| File Name | Content |
| --------- | ------- |
| `mingw64-libutils` | `mingw64-libutils-dev` + `mingw64-libutils-op` |
| `mingw64-libutils-dev` | Headers and CMake configuration (requires MinGW OpenSSL) |
| `mingw64-libutils-dev-op` | CMake configuration of `mingw64-libutils-op` |
| `mingw64-libutils-dev-db` | CMake configuration of `mingw64-libutils-db` |
| `mingw64-libutils-op` | Optimized Windows library |
| `mingw64-libutils-db` | Debug Windows library (no asan with MinGW) |

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
> Restriction: `fedora-based (rpm)`, `debian-based (deb)`, `arch-based (pacman)`

No clone needed. The script:
1. detects the distribution (`/etc/os-release`)
2. adds the libutils package repository
3. imports its GPG key

```bash
curl -fsSL https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s
```

or with `wget`:

```bash
wget -qO- https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s
```

```bash
sudo dnf install libutils       # Fedora
sudo apt install libutils       # Debian/Ubuntu
sudo pacman -S libutils         # Arch
```

| Argument | Effect |
| -------- | ------ |
| `--no-sudo` | No `sudo` (run it as root) |
| `-h`, `--help` | Display the usage |

Steps per distribution and known errors: [`readme/install.md`](readme/install.md).

## Windows (cross compilation)
> Build `libutils.a` for Windows (x86_64) from Linux with MinGW-w64

```bash
# from the packages (after the setup.sh of the Quick Setup 2)
sudo dnf install mingw64-libutils       # Fedora (+ mingw64-libutils-db for Debug)
sudo apt install mingw64-libutils       # Debian/Ubuntu (OpenSSL for MinGW built by hand)
```

```bash
# from the sources
export BUILD_DIR=build-windows
cmake -S . -B $BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Optimized
cmake --build $BUILD_DIR --parallel $(nproc)
cmake --install $BUILD_DIR --prefix <windows_prefix>    # include/, lib/libutils.a, lib/cmake/utils/
cmake --build $BUILD_DIR --target package               # mingw64-libutils-* RPM/DEB
```

> [!IMPORTANT]
> Include `<windows.h>` before `<utils/utils.hpp>`, or define `WIN32_LEAN_AND_MEAN`: it declares an `_unused` member.

> [!WARNING]
> `Process::spawn()` (a `fork`) and `Process::dup` on a fd other than 0, 1, 2 raise a `FatalException`.

Same API on both systems. Implementations, `utils::platform::` calls and differences: [`readme/windows.md`](readme/windows.md).

## Usage

```cmake
find_package(utils 5.0.0 REQUIRED)                      # installed CMake configuration
target_link_libraries(${TARGET} PRIVATE utils::utils)   # includes, OpenSSL & library
```

The library follows the `CMAKE_BUILD_TYPE` of the project. A missing package stops the configuration with its name.

| Build type | Library | Package (Linux) | Package (Windows) |
| ---------- | ------- | --------------- | ----------------- |
| `Debug` | `libutils_debug.a` | `libutils-db` | `mingw64-libutils-db` |
| `Asan` | `libutils_asan.a` | `libutils-as` | none (use `Debug`) |
| anything else | `libutils.a` | `libutils-op` | `mingw64-libutils-op` |

```cpp
#define _Utils          // every section (or only the needed ones: _Network, _Cli, ...)
#include <utils.hpp>    // same as <utils/utils.hpp>
```

> [!WARNING]
> Everything is defined within the namespace `utils::`.

| Macro | Content |
| ----- | ------- |
| `_Utils` | `_Handling`, `_Tools`, `_Attribute` |
| `_Handling` | `_Exception`, `_Verbose`, `_Pool`, `_Cli`, `_Arguments`, `_Network` |
| `_Tools` | `_Math`, `_Concepts`, `_Encapsulation`, `_System`, `_CustomType`, `_Manip`, `_Algorithms`, `_Security`, `_Platform` |
| `_Exception` | `utils::exception::` error / warning / fatal exceptions |
| `_Network` | `utils::network::` sockets, `Client`, `Server` |
| `_CustomType` | `utils::type::` lookup table, vectors, matrices |
| `_Algorithms` | `utils::algorithms::` `c2dmp-hsm`, `s.o.s` |
| `_Security` | `utils::security::` AES, RSA, observers |
| `_Platform` | `utils::platform::` POSIX calls for Unix & Windows |
| `_Warning` / `_NoWarning` | Enable / disable the deprecation, linker, usage warnings |
| `UTILS_UNIX` / `UTILS_WINDOWS` | Detected platform (`UTILS_PLATFORM`: `"unix"` / `"windows"`) |

## C API

Every section except `_Concepts`, in C11 / C23: `utils/<section>/CApi.h`, included by `<utils.h>`.

```c
#define _Cli                    // same defines as utils.hpp (nothing defined = everything)
#include <utils.h>              // same as <utils/utils.h>

utils_cli_t* cli = utils_cli_create(true);              // a class = an opaque handle (NULL on error)
if (utils_cli_start(cli, NULL, 0, 1) != UTILS_OK)       // UTILS_OK / UTILS_KO (or NULL / 0)
    fprintf(stderr, "%s\n", utils_exception_format());  // thread local status of the last error
utils_cli_destroy(cli);
```

> [!IMPORTANT]
> Link with a C++ linker (`LINKER_LANGUAGE CXX`) or `-lstdc++`: libutils is C++.

Rules (errors, templates, strings, strict C): [`readme/c-api.md`](readme/c-api.md). Guide: [Use libutils from C](https://tsukini22.github.io/libutils/docs/c-api.html).

## Unit tests

```bash
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build $BUILD_DIR --parallel $(nproc)
ctest --test-dir $BUILD_DIR --output-on-failure --timeout 30   # GTest + C11 / C23 programs (or ./unit_tests)
```

> [!NOTE]
> No GPU needed: `HybridMemory` falls back to the RAM and its backend tests are skipped.

## Pre-commit hook

An [xstyle](https://github.com/TsukiNi22/xstyle) hook checks the staged lines: a `minor` issue or worse refuses the commit.

```bash
git config core.hooksPath .githooks   # once per clone
git commit --no-verify                # skip the hook once (or XSTYLE_HOOK=0 git commit ...)
xstyle --fix --staged                 # fix what can be, then git add -u
```

> [!NOTE]
> Without `xstyle` the hook only warns. Thresholds: `FAIL_ON` / `WARN_ON` in `.githooks/pre-commit`.

## Documentation

`docs/*.html` (EN / FR, search with `/` or Ctrl+K) is built from [`docs/site/`](docs/site) and the headers.

```bash
docs/site/build.sh            # docs/*.html, search index, sitemap
docs/site/build.sh --check    # + links, accessibility, style and search checks
```

> [!NOTE]
> Requires the `better-html-doc` skill (`SKILLS=<folder>`, default `~/.claude/skills`). `docs/graph.html` is generated apart.

## Workflows/Release
### Workflows
`Dispatch (CI/CD)` picks the jobs on each push to `main` or a `v*` tag; a commit with `[ignore]` skips the CI.
The 7 workflows (lint, tests with ASan, packages, GitHub release, docs) and the downstream notification: [`readme/workflows.md`](readme/workflows.md).

### Pre-Release (unstable)
- a tag `vX.Y.Z-<suffix>` (ex: `v5.0.1-pre`), or a commit with `[build]`

### Release (stable)
- a tag `vX.Y.Z` only: stable packages and the GitHub release (notes = its `CHANGELOG.md` section)

## License
[MIT](LICENSE), except `include/utils/algorithms/`: [CC BY-NC-SA 4.0](include/utils/algorithms/LICENSE.md) (commercial use: ask the author).
Dependencies: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
