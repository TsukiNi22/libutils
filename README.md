# Utils
> [!NOTE]
> To have more information look directly in the files

## Dependencies

| Name + Link | Status | Last Update |
| ----------- | ------ | ----------- |
| [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) | ![CD - Algorithm](https://github.com/TsukiNi22/c2dmp-hsm/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/c2dmp-hsm) |
| [s.o.s](https://github.com/TsukiNi22/s.o.s) | ![CD - Algorithm](https://github.com/TsukiNi22/s.o.s/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/s.o.s) |

## Packages

| File Name | Content |
| -------------- | ---------- |
| `libutils-dev` | Contains all includes needed |
| `libutils-op` | Compiled with optimization options |
| `libutils-db` | Compiled with debug options (intended for debugging only) |
| `libutils-as` | Compiled with asan options (intended for debugging only) |

## Quick Setup (Fedora only)

Run the setup script directly, without cloning the repository manually. It's setup the different file to allow `dnf` to handle the different package from utils.

```bash
wget -qO- https://raw.githubusercontent.com/TsukiNi22/utils/main/setup.sh | bash -s
```

or with `curl`:

```bash
curl -fsSL https://raw.githubusercontent.com/TsukiNi22/utils/main/setup.sh | bash -s
```

## What the script does

1. Setup the `.repo` file to allow dnf to find the different packages
2. Get the official GPG key to check the signature

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
