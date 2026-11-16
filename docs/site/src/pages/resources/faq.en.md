+++
title = "FAQ"
summary = "Answers to the usual problems: install errors, a missing package at the configuration, the defines, Windows, OK / KO, and the C API."
section = "resources"
order = 50
keywords = "faq question problem error invalid hash missing package find_package windows.h _unused OK KO link stdc++ c api posix sections define"
+++

The problems met most often, with their answer.

## Install {#install}

**`dnf` / `apt` says `invalid hash`.** Reset the cache of the tool (`dnf clean all`, `apt clean`) or run `setup.sh` again; if it persists, the repository may be broken: open an issue.

**Which package for which build type?** `Debug` needs `libutils-db`, `Asan` needs `libutils-as`, anything else `libutils-op` (in `libutils`). The configuration stops with the name of the missing package. See [Link and include](page:usage#cmake).

**What is a `-pre` package?** The pre-release of the next version, built from `main`; any release of the same version or higher replaces it.

## Compile {#compile}

**A section I defined is missing.** `utils.hpp` has no global guard: the defines must be placed before **every** include of it. A bare include after a partial one only gets the sections already defined. See [Link and include](page:usage#rules).

**`OK` / `KO` clash with my own macros.** libutils always defines `UTILS_OK` / `UTILS_KO` (used inside the library) and only defines `OK` / `KO` when they don't exist yet (`_UTILS_OK_DEFINED` / `_UTILS_KO_DEFINED` tell when it did); a warning is given when they exist with another value.

**A deprecation warning about `_migration`.** The API is renamed and will be removed at the version given in the warning: move to the new name, or hide these warnings with `NO_BACKWARD_COMPATIBILITY_WARNING` ([Warnings](page:warnings)).

## Windows {#windows}

**Errors about `_unused` with `<windows.h>`.** The complete `<windows.h>` declares a member named `_unused`, a macro of libutils: include `<windows.h>` **before** `<utils/utils.hpp>`, or define `WIN32_LEAN_AND_MEAN`.

**`Process::spawn()` aborts.** Windows has no `fork`: use `spawn(path, args)` (`CreateProcess`). See [Windows](page:windows#code).

**No colors in the console.** Call `utils::iomanip::enable_ansi()` (C: `utils_iomanip_enable_ansi()`) once at the start.

## C API {#c-api}

**Undefined references to `std::` or `operator new` at the link.** libutils is C++: link with a C++ linker (`LINKER_LANGUAGE CXX` in CMake) or add `-lstdc++`. See [Use libutils from C](page:c-api#setup).

**Unknown `pid_t` / `struct sockaddr` in strict C.** Define `_POSIX_C_SOURCE 200809L` before the first include, or build with `-std=gnu11` / `gnu23`.

**How do I know why a call failed?** Read `utils_exception` in the same thread, or print `utils_exception_format()`. See [Errors](page:c-api#errors).

**My callback failed but the message is generic.** Return `utils_fail(code, "message")` instead of `UTILS_KO`: the message becomes the info of the error.

**Can I pass any handle to `utils_freezable_freeze`?** Yes, any handle of the C API: it fails with `InvalidArgument` when its class is not a `Freezable`.
