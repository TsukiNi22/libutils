+++
title = "Installation"
summary = "Install libutils from the signed RPM / DEB / Arch packages, from the sources with CMake, or by copying the built library into your project."
section = "guide"
order = 10
keywords = "install installation setup.sh packages rpm deb arch pacman dnf apt cmake sources dependencies openssl pre release"
+++

Three ways to install libutils: the packages of its own repository (recommended), the sources with CMake, or a copy of the built library in your project. Windows is on [its own page](page:windows).

## Dependencies {#dependencies}

| Name | Version | Fedora (`dnf`) | Debian / Ubuntu (`apt`) | Arch (`pacman`) |
|---|---|---|---|---|
| `clang++` | C++20 | `clang` | `clang` | `clang` |
| `cmake` | >= 3.20 | `cmake` | `cmake` | `cmake` |
| OpenSSL | >= 3.0 | `openssl-devel` | `libssl-dev` | `openssl` |
| GoogleTest (tests only) | - | `gtest-devel` | `libgtest-dev` | `gtest` |
| `bsdtar` (Arch packages only) | - | `bsdtar` | `libarchive-tools` | `libarchive` |
| GPU driver (`HybridMemory` only) | - | `libcuda` / `ocl-icd` / `vulkan-loader` | `libcuda1` / `ocl-icd-libopencl1` / `libvulkan1` | `nvidia-utils` / `ocl-icd` / `vulkan-icd-loader` |

The GPU libraries are loaded at runtime: none is needed to build, and a machine without GPU uses the RAM. The licenses of the dependencies are on [Licenses](page:licenses).

## Packages {#packages}

`setup.sh` registers the package repository of libutils (`https://tsukini22.github.io/libutils`) in your package manager, then the packages install like any other:

```bash "Terminal"
curl -fsSL https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s   # or: wget -qO- <same url> | bash -s
sudo dnf install libutils       # Fedora / RHEL
sudo apt install libutils       # Debian / Ubuntu
sudo pacman -S libutils         # Arch Linux
```

| Distribution | What `setup.sh` does |
|---|---|
| `rpm` | adds `/etc/yum.repos.d/libutils.repo` and imports the GPG key |
| `deb` | adds the GPG key in `/etc/apt/keyrings/libutils.gpg`, the source in `/etc/apt/sources.list.d/libutils.list`, then `apt-get update` |
| `arch` | adds the GPG key to the pacman keyring (`pacman-key --add` + `--lsign-key`), the `[libutils]` repository in `/etc/pacman.conf` (`SigLevel = Required`), then `pacman -Sy` |

| Argument | Effect |
|---|---|
| `--no-sudo` | Run without `sudo` (the script must already run as root) |
| `-h`, `--help` | Display the usage |

| Package | Content |
|---|---|
| `libutils` | The default packages: `libutils-dev` + `libutils-op` |
| `libutils-dev` | Every header and the global CMake configuration (requires `openssl-devel` / `libssl-dev`) |
| `libutils-dev-op` / `-dev-db` / `-dev-as` | CMake configuration of the optimized / debug / asan library |
| `libutils-op` | Library built with the optimization options (`libutils.a`) |
| `libutils-db` | Library built with the debug options (`libutils_debug.a`, for debugging only) |
| `libutils-as` | Library built with AddressSanitizer (`libutils_asan.a`, for debugging only) |

> [!NOTE]
> Most packages have a pre-release version named `<package>-pre`, replaced by any release (without `-pre`) of the same version or higher. The same packages exist as RPM (Fedora), DEB (Debian / Ubuntu) and Arch Linux (`.pkg.tar.zst`, libraries in `/usr/lib`): same names, content and dependencies.

> [!WARNING]
> An `invalid hash` error from `dnf` / `apt`: reset the cache of the tool or run `setup.sh` again. If it persists, the repository may be broken: open an issue.

## From the sources {#sources}

Installed in `/usr/local`:

```bash "Terminal"
git clone https://github.com/TsukiNi22/libutils.git && cd libutils
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR
sudo cmake --build $BUILD_DIR --target install --parallel $(nproc)
```

The build types and the targets (`release`, `package_release`) are on [Build, tests and hooks](page:development).

## Copy in your project {#copy}

Build the library, then copy the headers and the archive where your project wants them:

```bash "Terminal"
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Optimized
cmake --build $BUILD_DIR --parallel $(nproc)
cp -r include/* build/generated/* <your_project>/include/
cp build/libutils.a <your_project>/libs/
```

Then compile with `-I<your_project>/include/ -L<your_project>/libs/ -lutils -lssl -lcrypto`, or in CMake:

```cmake
find_package(OpenSSL REQUIRED)
target_include_directories(${TARGET} PRIVATE include)
target_link_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/libs/)
target_link_libraries(${TARGET} PRIVATE utils OpenSSL::SSL OpenSSL::Crypto)
```

Next: [link it and choose the sections](page:usage).
