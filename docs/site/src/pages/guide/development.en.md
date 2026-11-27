+++
title = "Build, tests and hooks"
summary = "Build the three libraries and the packages, run the unit tests (C++ and pure C), and enable the xstyle pre-commit hook."
section = "guide"
order = 70
keywords = "build cmake Debug Asan Optimized release package_release unit tests gtest ctest BUILD_TESTS c api test pre-commit hook xstyle documentation site"
+++

For working on libutils itself: the build types and targets, the tests and the pre-commit hook. The CI that runs them is on [CI / CD](page:ci-cd).

## Build {#build}

| Command | Result |
|---|---|
| `cmake -S . -B build -DCMAKE_BUILD_TYPE=Optimized\|Debug\|Asan` | `libutils.a` / `libutils_debug.a` / `libutils_asan.a` |
| `cmake --build build --target release` | The 3 libraries |
| `cmake --build build --target package_release` | Every RPM / DEB / Arch (`.pkg.tar.zst`, with `bsdtar`) package |
| `make debug` / `asan` / `optimized` / `tests` / `release` / `package` / `install` | The same commands from the `Makefile` (build in `build/`, `make tests` = Debug + `ctest`) |
| `-DCMAKE_TOOLCHAIN_FILE=...` | The Windows build with `cmake/toolchain/mingw-w64.cmake` ([Windows](page:windows)) |

The exception codes of `cmake/config/exceptions/**/*.json` are turned into `generated_internal_exception_header.hpp` at the build, and `utils/version.hpp` / `utils/version.h` are written in `build/generated/`.

## Unit tests {#tests}

```bash "Terminal"
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build $BUILD_DIR --parallel $(nproc)
ctest --test-dir $BUILD_DIR --output-on-failure --timeout 30   # or ./unit_tests (written at the root)
```

- GoogleTest, one file per module in `tests/` (the C API of a section in `tests/<section>/CApi.cpp`).
- `tests/c/CApi.c` is a pure C program built in C11 and C23 (`c_api_c11`, `c_api_c23` in `build/tests/`), run by `ctest` (`CApi.C11`, `CApi.C23`).
- The GPU libraries are loaded at runtime by `HybridMemory`: the backend tests are skipped without GPU.
- In `Asan` mode (like the CI): the leaks made on purpose by the leak detection tests are suppressed (`tests/lsan.supp`), the GPU tests and `HybridMemory.RamOverflow` are skipped (the drivers are not instrumented, a failed `operator new` aborts instead of throwing).

## Pre-commit hook {#hook}

A shared [xstyle](https://github.com/TsukiNi22/xstyle) hook (`.githooks/pre-commit`) checks the staged lines at each `git commit`: a `minor` (or worse) issue refuses the commit, anything else is only listed. The synced sources of `sos` and `c2dmp-hsm` are skipped.

```bash "Terminal"
git config core.hooksPath .githooks   # once per clone
git commit --no-verify                # skip the hook once (or XSTYLE_HOOK=0 git commit ...)
xstyle --fix --staged                 # fix what can be, then git add -u
```

> [!NOTE]
> Without `xstyle` installed the hook only warns and the commit is accepted. The thresholds are the variables at the top of `.githooks/pre-commit`.

## Documentation {#docs}

This site is built from `docs/site/` (Markdown pages, and generators reading the headers, the exception codes and the changelog) into `docs/`:

```bash "Terminal"
docs/site/build.sh            # docs/*.html, search-index.js, site.css, site.js, sitemap.xml
docs/site/build.sh --check    # + links, accessibility, style and search checks
```

The project graph (`docs/graph.html`) is generated separately. The [GitHub wiki](https://github.com/TsukiNi22/libutils/wiki) holds the same reference in Markdown, kept up to date with this site.
