+++
title = "Platforms"
summary = "One code for Unix and Windows: the library only calls the system through utils::platform, POSIX on Unix, Win32 / Winsock / CRT on Windows, cross compiled with MinGW-w64."
section = "concepts"
order = 50
keywords = "platforms unix windows posix mingw cross compilation utils::platform winsock wsapoll createprocess differences toolchain"
+++

Every class has the same API on Unix and Windows: the library only calls the system through `utils::platform::`, whose functions have the names, arguments and contract of POSIX (`-1` and `errno`). On Unix they are the system calls, on Windows a Win32 / Winsock / CRT implementation. The same calls are given to the programs by the section [Platform](page:platform), in C++ and in C.

:::html
<figure>
<svg viewBox="0 0 760 260" role="img" aria-label="The classes of libutils call utils::platform, implemented by the system calls on Unix and by Win32, Winsock and the CRT on Windows">
<defs>
<marker id="arr7-fig6-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box" x="140" y="20" width="480" height="50" rx="8"/>
<text class="lbl" x="380" y="50" text-anchor="middle">Server, Client, Poll, Process, Cli, SharedObject, Pipe...</text>
<rect class="box-alt" x="140" y="105" width="480" height="50" rx="8"/>
<text class="lbl" x="380" y="135" text-anchor="middle">utils::platform (close, socket, epoll_wait, dlopen...)</text>
<rect class="box" x="60" y="190" width="280" height="50" rx="8"/>
<text class="lbl" x="200" y="220" text-anchor="middle">Unix: system calls</text>
<rect class="box" x="420" y="190" width="280" height="50" rx="8"/>
<text class="lbl" x="560" y="220" text-anchor="middle">Windows: Win32 / Winsock / CRT</text>
<path class="edge" d="M380 70V101" marker-end="url(#arr7-fig6-en)"/>
<path class="edge" d="M260 155L204 186" marker-end="url(#arr7-fig6-en)"/>
<path class="edge" d="M500 155L556 186" marker-end="url(#arr7-fig6-en)"/>
</svg>
<figcaption>One code for both platforms, the difference is inside <code>utils::platform</code>.</figcaption>
</figure>
:::

## Calls {#calls}

| Header | Calls | Windows implementation |
|---|---|---|
| `Unistd.hpp` | `close`, `read`, `write`, `pipe`, `dup`, `dup2`, `isatty`, `poll`, `getpid`, `readlink`, `strerror` | CRT, or Winsock for a socket; `poll` also on the console / pipes / files; `strerror` knows the socket errors |
| `Socket.hpp` | `socket`, `connect`, `bind`, `listen`, `accept`, `recv`, `send`, `setsockopt`, `getsockname`, `getpeername`, `getaddrinfo` | Winsock 2 started on the first use, errors translated in `errno` |
| `Epoll.hpp` | `epoll_create1`, `epoll_ctl`, `epoll_wait` | emulation over `WSAPoll`: sockets only, level triggered, `EPOLLONESHOT` emulated |
| `Dlfcn.hpp` | `dlopen`, `dlsym`, `dlclose`, `dlerror`, `dladdr` | `LoadLibraryA`, `GetProcAddress`, `FreeLibrary`, `GetModuleHandleExA` |
| `Termios.hpp` | `tcgetattr`, `tcsetattr` | console mode (`ICANON` = line input, `ECHO`, `ISIG` = ctrl+c, VT input) |

The POSIX types and constants these calls need on Windows (`struct epoll_event`, `struct termios`, `Dl_info`, `socklen_t`, `RTLD_*`, `EPOLL*`...) are in `utils/platform/Types.h`, a header for C and C++.

## Differences {#differences}

No `fork` on Windows: `Process::spawn()` and a `dup` on a fd other than `0`, `1`, `2` raise a `FatalException` (`UndefinedCall`); `spawn(path, args)` uses `CreateProcess`.

Other differences: `Poll` / `Client` / `Server` only handle sockets (at most 10 ms of latency), a `Process` has no signal, <kbd>Ctrl</kbd>+<kbd>Z</kbd> is a char for the `Cli`, a `SharedMemory` disappears with its last handle. Include `<windows.h>` before `<utils/utils.hpp>` (it declares a member named `_unused`, a libutils macro), or define `WIN32_LEAN_AND_MEAN`.

## Cross compilation {#cross-compilation}

Windows binaries are built from Linux with MinGW-w64 and `cmake/toolchain/mingw-w64.cmake`, usable by any project. A project keeps the same CMake code for both platforms (`find_package(utils)` + `utils::utils`): only the configuration command adds the toolchain.

| Toolchain setting | Value |
|---|---|
| compilers | `x86_64-w64-mingw32-gcc` / `-g++` / `-windres` (`-DMINGW_PREFIX=<triplet>`) |
| `CMAKE_FIND_ROOT_PATH` | the MinGW sysroot: `/usr/x86_64-w64-mingw32/sys-root/mingw` (Fedora), `/usr/x86_64-w64-mingw32` (Debian); libraries, headers and packages searched only there |
| `CMAKE_CROSSCOMPILING_EMULATOR` | `wine` when installed (`WINEPATH` = the sysroot `bin/`): `ctest` runs `unit_tests.exe` |

Build types: `Debug` uses `mingw64-libutils-db`, anything else `mingw64-libutils-op`; no Asan with MinGW. The complete guide is [Windows](page:windows).
