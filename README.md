# Utils

> [!TIP]
> Documentation [TsukiNi22/libutils/wiki](https://github.com/TsukiNi22/libutils/wiki) (v3.0.0).

C++20 utility library (`utils::`): exceptions, verbose, CLI, arguments, network, IPC, encryption, custom types, algorithms.
Built as a static library and distributed as RPM/DEB packages (optimized, debug and asan variants), or installed from the sources with CMake.

### Table of Contents
 - [Dependencies](#dependencies)
 - [Packages](#packages)
 - [Quick Setup 1 (All)](#quick-setup---1-all)
 - [Quick Setup 2 (Limited)](#quick-setup---2-limited)
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

## Usage

```cmake
find_package(utils 3.0.0 REQUIRED)                      # installed CMake configuration
target_link_libraries(${TARGET} PRIVATE utils::utils)   # includes, OpenSSL & library
```

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
| `_Tools` | `_Math`, `_Concepts`, `_Encapsulation`, `_System`, `_CustomType`, `_Manip`, `_Algorithms`, `_Security` |
| `_Exception` | `utils::exception::` error / warning / fatal exceptions with codes |
| `_Network` | `utils::network::` sockets, `Client`, `Server` |
| `_CustomType` | `utils::type::` `BidirectionalLookupTable`, vectors, matrices |
| `_Algorithms` | `utils::algorithms::` `c2dmp-hsm`, `s.o.s` |
| `_Security` | `utils::security::` encryption (AES, RSA), observers |
| `_Warning` / `_NoWarning` | Enable / disable every deprecation, linker and usage warning |

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
- `Build - Packages (CI/CD)`: builds the packages (RPM/DEB), signs them and publishes them on the `gh-pages` repository
- `Build - Libraries (CI)`: only builds the libraries and checks the compilation, without producing or publishing packages

> [!NOTE]
> `Build - Libraries (CI)` only runs when `Dispatch (CI/CD)` decides **not** to build the packages, to check that the code still compiles.

> [!TIP]
> A commit containing `[ignore]` (preferably in the description) skips the whole CI.

### Pre-Release (unstable)
The pre-release of the packages (`<package_name>-pre`) is triggered by:
- a tag with a suffix, matching `vx.x.x-<suffix>` (ex: `v3.0.1-pre`, `x` = `major`, `minor`, `fix`)
- or a commit containing `[build]`, preferably in the description

> [!NOTE]
> Only the 5 last versions of each `-pre` package are kept in the package repository, every release is kept.

### Release (stable)
The release of the packages (channel `stable`) is only triggered by a tag matching exactly `vx.x.x` (ex: `v3.0.0`).

### Downstream notification
`Dispatch (CI/CD)` notifies the `docker-image` repository (`repository_dispatch`) after the packages are published:
- on every package build (release or pre-release)
- or on a commit containing `[release]`, preferably in the description
