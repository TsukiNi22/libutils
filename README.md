# Utils

### Table of Contents
 - [Dependencies](#dependencies)
 - [Packages](#packages)
 - [Quick Setup 1 (All)](#quick-setup---1-all)
 - [Quick Setup 2 (Limited)](#quick-setup---2-limited)
 - [Utils Content](#utils---content-see-github-wiki-for-more-details)

## Dependencies

| Name + Link | Status | Last Update |
| ----------- | ------ | ----------- |
| [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) | ![CD - Algorithm](https://github.com/TsukiNi22/c2dmp-hsm/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/c2dmp-hsm) |
| [s.o.s](https://github.com/TsukiNi22/s.o.s) | ![CD - Algorithm](https://github.com/TsukiNi22/s.o.s/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/s.o.s) |

## Packages

| File Name | Content |
| --------- | ------- |
| `libutils` | Install the default packages (`libutils-dev`, `libutils-op`) |
| `libutils-dev` | Contains all includes needed and cmake global configuration |
| `libutils-dev-op` | Contains cmake configuration for `libutils-op` |
| `libutils-dev-db` | Contains cmake configuration for `libutils-db` |
| `libutils-dev-as` | Contains cmake configuration for `libutils-as` |
| `libutils-op` | Compiled with optimization options |
| `libutils-db` | Compiled with debug options (intended for debugging only) |
| `libutils-as` | Compiled with asan options (intended for debugging only) |

## Quick Setup - 1 (all)
> Setup into `/usr/local`

### Clone the repository
```bash
git clone https://github.com/TsukiNi22/libutils.git
cd libutils
```

### Install the lib
```bash
export BUILD=build
cmake -S . -B $BUILD_DIR
sudo cmake --build $BUILD_DIR --target install --parallel $(nproc)
```

## Quick Setup - 2 (Limited)
> Setup into `/usr`

> [!WARNING]
> Restriction: `fedora-based (rpm)`, `debian-based (deb)`

Run the setup script directly, without cloning the repository manually. It's setup the different file to allow installing tools to handle the different package from libutils.

```bash
wget -qO- https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s
```

or with `curl`:

```bash
curl -fsSL https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s
```

## Utils - Content (see github wiki for more details)
> [!NOTE]
> Define used to include only part of the Utils lib (by default everything is included)

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
