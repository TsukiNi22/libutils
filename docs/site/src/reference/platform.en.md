+++
title = "Platform"
summary = "The system calls used by the whole library, with the names and contract of POSIX on Unix and on Windows, and the platform macros."
keywords = "platform posix windows unix close read write pipe dup poll socket epoll dlopen tcgetattr strerror UTILS_WINDOWS UTILS_UNIX UTILS_PLATFORM winsock utils_platform"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Platform` | `_Tools` | `utils::platform` | `utils/platform/` |

The system calls used by the whole library, with the **same names, arguments and contract as POSIX** (`-1` and `errno` on error): the system call on Unix, a Win32 / Winsock / CRT implementation on Windows. A code written with them builds on both platforms without any `#ifdef`. Why and how: [Platforms](page:platforms).

## Platform detection {#detection}

`utils/platform/Platform.hpp`, always included by `utils/utils.hpp` and `utils/utils.h`:

| Macro | Value |
|---|---|
| `UTILS_UNIX` | Defined on Linux, BSD, macOS |
| `UTILS_WINDOWS` | Defined on Windows (`_WIN32`, MinGW-w64) |
| `UTILS_PLATFORM` | `"unix"` / `"windows"` |

## Calls {#calls}

| Header | Calls |
|---|---|
| `utils/platform/Unistd.hpp` | `close`, `read`, `write`, `pipe`, `dup`, `dup2`, `isatty`, `poll`, `getpid`, `readlink`, `strerror` |
| `utils/platform/Socket.hpp` | `socket`, `connect`, `bind`, `listen`, `accept`, `recv`, `send`, `setsockopt`, `getsockname`, `getpeername`, `getaddrinfo`, `freeaddrinfo`, `gai_strerror` |
| `utils/platform/Epoll.hpp` | `epoll_create1`, `epoll_ctl`, `epoll_wait` (an epoll is closed by `close`) |
| `utils/platform/Dlfcn.hpp` | `dlopen`, `dlsym`, `dlclose`, `dlerror`, `dladdr` |
| `utils/platform/Termios.hpp` | `tcgetattr`, `tcsetattr` |

On Windows the POSIX types and constants of these calls are given with the Linux values by `utils/platform/Types.h` (C and C++): `struct epoll_event`, `EPOLLIN`, `EPOLL_CTL_ADD`, `struct termios`, `ICANON`, `ECHO`, `ISIG`, `TCSAFLUSH`, `Dl_info`, `RTLD_NOW`, `socklen_t`, `MSG_NOSIGNAL` (`0`), `STDIN_FILENO`...

> [!IMPORTANT]
> The public headers never include `<windows.h>` / `<winsock2.h>` (no Windows macros in your code): on Windows the socket structures (`sockaddr_in`, `addrinfo`, `pollfd`, `timeval`) come from `<winsock2.h>` / `<ws2tcpip.h>`, included by your code.

## Windows behavior {#windows-behavior}

| Call | Windows implementation |
|---|---|
| `close` | Any fd: CRT fd (`_close`), socket (`closesocket`) or emulated epoll |
| `read` / `write` | CRT (`_read` / `_write`), `recv` / `send` on a socket |
| `pipe` / `dup` / `dup2` | CRT `_pipe` (binary mode) / `_dup` / `_dup2` |
| `poll` | `WSAPoll` for sockets; console, pipe and file input checked by slices of 10 ms |
| `readlink` | `"/proc/self/exe"` = `GetModuleFileName`, else a symbolic link |
| `strerror` | Also the socket errors (message of the Winsock code), in English |
| socket calls | Winsock 2, started on the first use; `WSAGetLastError` translated in `errno` (`WSAECONNREFUSED` → `ECONNREFUSED`...); `SO_SNDTIMEO` / `SO_RCVTIMEO` take a `timeval`; `SO_REUSEADDR` is the default behavior (no-op) |
| `epoll_*` | Emulation over `WSAPoll`: sockets only (`ENOTSOCK`), level triggered (`EPOLLET` → `EINVAL`), `EPOLLONESHOT` emulated, at most 10 ms of latency |
| `dlopen` / `dlsym` / `dlclose` / `dladdr` | `LoadLibraryA` / `GetProcAddress` / `FreeLibrary` / `GetModuleHandleExA` + `GetModuleFileNameA` |
| `tcgetattr` / `tcsetattr` | Console mode: `ICANON` = line input, `ECHO` = echo, `ISIG` = ctrl+c; without `ICANON` the input gives the escape sequences (VT input) |

> [!WARNING]
> The complete `<windows.h>` declares a member named `_unused`, a libutils attribute macro: include `<windows.h>` **before** `<utils/utils.hpp>`, or define `WIN32_LEAN_AND_MEAN`.

## Example {#example}

```cpp
#define _Platform
#include <utils/utils.hpp>
#include <cerrno>
#include <iostream>

int fds[2];
if (utils::platform::pipe(fds) != 0) return 1;                 // -1 & errno on error, like POSIX
(void)utils::platform::write(fds[1], "ok", 2);
char buf[3] = {};
(void)utils::platform::read(fds[0], buf, 2);                    // "ok"
utils::platform::close(fds[0]);
utils::platform::close(fds[1]);
std::cout << utils::platform::strerror(ECONNREFUSED) << std::endl; // "Connection refused" (Unix & Windows)
```

## Errors {#errors}

The calls never throw: they return `-1` (`nullptr` for `dlopen` / `dlsym`, an `EAI_*` code for `getaddrinfo`) and set `errno` (`dlerror` for the dynamic loading), like the system calls. The classes built on them ([Encapsulation](page:encapsulation), [Network](page:network), [Cli](page:cli)) turn these errors into exceptions.

## In C {#in-c}

Every call is `utils_platform_<name>` with the same contract (`-1` and `errno`, no `utils_exception`). `dladdr` fills a `utils_platform_dl_info_t` instead of `Dl_info` (which needs `_GNU_SOURCE`).

> [!NOTE]
> In strict C (`-std=c11`, `-std=c23`) on Unix, the POSIX structures of these calls need the POSIX features: `#define _POSIX_C_SOURCE 200809L` before the first include (already the default with `-std=gnu11` / `gnu23`).

```c
#define _POSIX_C_SOURCE 200809L
#define _Platform
#include <utils.h>
#include <errno.h>
#include <stdio.h>

int main(void)
{
    int fds[2];
    char buf[3] = {0};

    if (utils_platform_pipe(fds) != 0)                          // -1 & errno on error, like POSIX
        return 1;
    utils_platform_write(fds[1], "ok", 2);
    utils_platform_read(fds[0], buf, 2);                        // "ok"
    utils_platform_close(fds[0]);
    utils_platform_close(fds[1]);
    printf("%s %s\n", buf, utils_platform_strerror(ECONNREFUSED)); // "Connection refused" (Unix & Windows)
    return 0;
}
```
