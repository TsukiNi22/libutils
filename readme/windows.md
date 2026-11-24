# Windows

Complete guide: [Windows](https://tsukini22.github.io/libutils/docs/windows.html).

## Packages and CMake
The `mingw64-libutils*` packages install in the MinGW sysroot: `/usr/x86_64-w64-mingw32/sys-root/mingw` (Fedora), `/usr/x86_64-w64-mingw32` (Debian/Ubuntu).
On Fedora they require `mingw64-openssl`; on Debian/Ubuntu OpenSSL for MinGW has no package and is built by hand.

The CMake code of a project stays the same: configure it with a MinGW toolchain whose `CMAKE_FIND_ROOT_PATH` is the sysroot.
Toolchains: `cmake/toolchain/mingw-w64.cmake` of libutils, or `/usr/share/mingw/toolchain-mingw64.cmake` / `mingw64-cmake` on Fedora.
`find_package(utils)` then finds the `mingw64-libutils*` packages, and `utils::utils` also links `ws2_32`.

> [!NOTE]
> No AddressSanitizer with MinGW: the `Asan` mode builds like `Debug`, and no `-as` package exists.

The tests (`-DBUILD_TESTS=ON`) download GoogleTest and run with `wine` through `ctest`.

## Implementations
Every class has the same API on Unix and Windows: only the implementation inside the library differs.

| Part | Windows implementation |
| ---- | ---------------------- |
| `utils::network::TCPSocket` (and `is_ip`, `resolve_*`) | Winsock 2 (`ws2_32`, linked by `utils::utils`), started on the first use |
| `utils::network::Client` / `Server` | `utils::platform::epoll_*` (emulation over `WSAPoll`) |
| `utils::encapsulation::Poll` | same `struct epoll_event` / `EPOLL*` API, emulated over `WSAPoll` |
| `utils::encapsulation::Process` | `CreateProcess` / `WaitForSingleObject` / `TerminateProcess`, `replace` with the CRT `_execvp` |
| `utils::encapsulation::SharedMemory` | `CreateFileMapping` / `MapViewOfFile` (`Local\utils-shm-<name>`), a named event instead of the futex |
| `utils::encapsulation::SharedObject` | `LoadLibrary` / `GetProcAddress` / `FreeLibrary` (`.dll`) |
| `utils::encapsulation::Pipe` / `Dup` | CRT `_pipe` (binary mode) / `_dup` / `_dup2` |
| `utils::cli::Cli` | `tcgetattr` / `tcsetattr` on the console (VT input), `poll` / `read` on its input |
| `utils::iomanip` reports (`read_cursor_position`, mouse) | CRT `_read` on the standard input |
| `utils::iomanip::enable_ansi()` | enables the virtual terminal mode of the console (call it once; `true` on Unix) |
| exceptions / observers module name | `dladdr` / `readlink("/proc/self/exe")` (`GetModuleHandleEx` / `GetModuleFileName`) |

## System calls (`_Platform`)
The library only calls the system through `utils::platform::`, and a program can use the same calls.
They keep the names and the contract of the POSIX functions (-1 & `errno`, Winsock errors translated in `errno`).

| Header | Calls |
| ------ | ----- |
| `utils/platform/Unistd.hpp` | `close`, `read`, `write`, `pipe`, `dup`, `dup2`, `isatty`, `poll`, `getpid`, `readlink`, `strerror` |
| `utils/platform/Socket.hpp` | `socket`, `connect`, `bind`, `listen`, `accept`, `recv`, `send`, `setsockopt`, `getsockname`, `getpeername`, `getaddrinfo`, `freeaddrinfo`, `gai_strerror` |
| `utils/platform/Epoll.hpp` | `epoll_create1`, `epoll_ctl`, `epoll_wait` (closed by `close`) |
| `utils/platform/Dlfcn.hpp` | `dlopen`, `dlsym`, `dlclose`, `dlerror`, `dladdr` |
| `utils/platform/Termios.hpp` | `tcgetattr`, `tcsetattr` |

> [!NOTE]
> The Winsock structures (`sockaddr_in`, `addrinfo`, `pollfd`...) come from `<winsock2.h>` / `<ws2tcpip.h>`, included by the program.

## Differences

> [!IMPORTANT]
> Include `<windows.h>` before `<utils/utils.hpp>`, or define `WIN32_LEAN_AND_MEAN`: its `winioctl.h` declares an `_unused` member.

No equivalent on Windows -> `FatalException` with `UndefinedCall` (like the raw calls of `ASocket`):
- `Process::spawn(void)`: a `fork`, use `spawn(path, args)`
- `Process::dup(origin, clone)` with a `clone` other than `0`, `1`, `2`: the child only gets the standard fds

Other behaviors:
- `Poll` / `Client` / `Server` only handle sockets (a pipe -> `ENOTSOCK` -> `EPoll` error)
- `EPOLLET` is refused: level triggered only
- a `Process` has no signal: `Status::sig` = `0`, killed = exit code `137`
- ctrl+z is a char for the `Cli` (no `SIGTSTP`)
- a `SharedMemory` is destroyed with its last handle (no `shm_unlink` persistence)
