# Utils

See internal wiki for more details [...]() (not setup for now)

### Table of Contents
 - [Dependencies](#dependencies)
 - [Packages](#packages)
 - [Quick Setup 1 (All)](#quick-setup---1-all)
 - [Quick Setup 2 (Limited)](#quick-setup---2-limited)
 - [Workflows/Release](#workflowsrelease)

## Dependencies

| Name + Link | Status | Last Update |
| ----------- | ------ | ----------- |
| [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) | ![CD - Algorithm](https://github.com/TsukiNi22/c2dmp-hsm/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/c2dmp-hsm) |
| [s.o.s](https://github.com/TsukiNi22/s.o.s) | ![CD - Algorithm](https://github.com/TsukiNi22/s.o.s/actions/workflows/sync.yml/badge.svg) | ![](https://img.shields.io/github/last-commit/TsukiNi22/s.o.s) |

## Packages

> [!NOTE]
> Most packages have a pre-release/unstable version named `<package_name>-pre`.
> The `-pre` packages are marked as obsolete by any release/stable package (without `-pre`) of the same version or higher.

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
> If during the installation using tools like `dnf`, `apt`... you see an `invalid hash` error, reset the cache of the tool or relaunch the `setup.sh` script
> 
> If the error still persists, it might be due to an error in the repository

> [!WARNING]
> Restriction: `fedora-based (rpm)`, `debian-based (deb)`

> [!NOTE]
> The usage of `sudo` in the script can be remove using `--no-sudo` argument

Run the setup script directly, without cloning the repository manually. It's setup the different file to allow installing tools to handle the different package from libutils.

```bash
wget -qO- https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s
```

or with `curl`:

```bash
curl -fsSL https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s
```

## Workflows/Release

### Workflows

 - The workflow `CI/CD - Library` builds the packages and releases them
 - The workflow `CI - Library` builds the libraries and checks the compilation
> [!NOTE]
> Only when the `CI/CD - Library` workflow isn't triggered, the `CI - Library` workflow will run to check the compilation

### Pre-Release (unstable)

The pre-release of the packages can be triggered by 2 events:
 - Pushing a tag that matches the regex `vx.x.x-pre` (`x` stands for the version number: `major`, `minor`, `fix`)
 - Or pushing a commit containing the string `[build]`, preferably in the description

### Release (stable)

The release can only be triggered by pushing a tag that matches the regex `vx.x.x-release` (`x` stands for the version number: `major`, `minor`, `fix`)
