+++
title = "Encapsulation"
summary = "RAII wrappers of the system tools: Pipe, Dup, Process, Poll (epoll), SharedObject (.so / .dll) and SharedMemory (messages between processes)."
keywords = "encapsulation pipe dup process fork spawn exec wait kill poll epoll shared object dlopen shared memory shm slot ownership target utils_process utils_shm"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Encapsulation` | `_Tools` | `utils::encapsulation` | `utils/encapsulation/` |

RAII wrappers around the system tools (through [Platform](page:platform), same API on Unix and Windows): the fds are closed by the destructors, every error is an exception with the `errno` message as info. The shared memory is explained in [Communication](page:network-ipc#shared-memory).

## Pipe and Dup {#pipe-dup}

| `Pipe` | Description |
|---|---|
| `Pipe(read = -1, write = -1)` / `Pipe(fds)` | Wrap existing fds (or none) |
| `trigger()` | Create the pipe (`Pipe` error if already open) |
| `getRead()`, `getWrite()` | fds (`-1` = closed), both with `getFds()` |
| `closeRead()`, `closeWrite()`, `close()` | Close one side / both |
| `clear()` | Forget the fds without closing them |

| `Dup` | Description |
|---|---|
| `Dup(origin = -1, clone = -1)` | `origin` will be duplicated on `clone` |
| `trigger()` | `dup(origin)` when `clone == -1`, else `dup2(origin, clone)` (`Dup` error when `origin` is closed) |
| `getOrigin()`, `getClone()`, `setOrigin(fd)`, `setClone(fd)` | Access |
| `closeOrigin()`, `closeClone()`, `close()`, `clear()` | Close / forget |

Both are movable, not copyable.

## Process {#process}

| Method | Description |
|---|---|
| `pipe(read, write)` / `pipe(Pipe&)` / `pipe(std::vector<Pipe>&)` | Pipes created (`trigger`) by the next `spawn` |
| `dup(origin, clone = -1)` / `dup(Dup&)` / `dup(std::vector<Dup>&)` | Dups applied in the child (ex: redirect its stdout) |
| `spawn()` | `fork`: returns `0` in the child, the pid in the parent (Windows: `FatalException` `UndefinedCall`) |
| `spawn(path, args)` | `fork` + `execvp(path, args)` in the child, returns the pid (exit code `127` when the exec or a dup fails); Windows: `CreateProcess`, searched in the `PATH` |
| `replace(path, args)` | `execvp` in the current process (never returns: exec or `_exit(127)`); Windows: CRT `_execvp` |
| `wait()` | `waitpid`, returns a `Status{unknown, exited, code, sig}` (Windows: exit code, `sig` always `0`) |
| `kill()` | `SIGKILL` then wait (Windows: `TerminateProcess`, exit code `137`) |
| `is()` | Is the child alive |
| `isChild()`, `isParent()`, `getPid()` | State |
| `getPipes()`, `getDups()`, `clear()` | Access / close the pipes and dups |

> [!NOTE]
> The destructor of the parent kills a running child. The dups are checked before the `fork`: an invalid one throws `Dup` in the caller.

> [!IMPORTANT]
> Windows has no `fork`: `spawn()` raises a `FatalException` (`UndefinedCall`, the program aborts), and only the fds `0`, `1`, `2` of the child can be redirected by `dup` (another `clone` → same `FatalException` at `spawn`). Only the redirected standard handles are given to the child, so the other pipes don't leak and EOF stays reachable.

```cpp
#define _Encapsulation
#include <utils/utils.hpp>

utils::encapsulation::Pipe pipe;
pipe.trigger();
utils::encapsulation::Process process;
process.dup(pipe.getWrite(), STDOUT_FILENO); // child: stdout -> pipe
(void)process.spawn("echo", {"hello"});
pipe.closeWrite();
// read pipe.getRead() -> "hello\n"
utils::encapsulation::Status status = process.wait(); // status.exited, status.code == 0
```

## Poll {#poll}

Wrapper of `epoll` ([Platform](page:platform#windows-behavior): emulated over `WSAPoll` on Windows, sockets only, level triggered).

| Method | Description |
|---|---|
| `Poll(flags = EPOLL_CLOEXEC)` / `init(flags)` | Create the epoll |
| `link(fd, events, data = nullptr)` | Watch a fd (`EPOLLIN`...) |
| `edit(fd, events, data)` | Change the watched events |
| `unlink(fd)` | Stop watching (ignored for a closed fd) |
| `wait(delay = -1, limits = 0)` | Wait for events (`delay` in ms, `-1` = forever), returns the `epoll_event`s |
| `size()`, `contains(fd)`, `getFd()`, `close()` | State |

## SharedObject {#shared-object}

Load a shared library (`.so`, `.dll` on Windows) and its functions.

| Method | Description |
|---|---|
| `SharedObject(path)` | `dlopen` (`Dlopen` error), `LoadLibrary` on Windows |
| `loadFunction<T>(name)` | `dlsym`, cast to the function pointer type `T` (`Dlsym` error) |
| `isLoaded()`, `path()`, `get()` | State, path, raw handle |

```cpp
using StrlenFn = std::size_t (*)(const char*);
utils::encapsulation::SharedObject libc("libc.so.6");
StrlenFn fn = libc.loadFunction<StrlenFn>("strlen");
```

## SharedMemory {#shared-memory}

Message passing between processes through a POSIX shared memory divided in **slots** (`queue` slots of `size` bytes). Each instance has an **ownership** (default: its pid) used to address the messages; a reader thread copies the messages for this instance into a local storage.

> [!NOTE]
> Windows: a file mapping (`Local\utils-shm-<name>`) and a named event instead of the futex (the readers are woken within 10 ms); the memory is destroyed with its last handle (no persistence like `/dev/shm`).

| Method | Description |
|---|---|
| `SharedMemory(ownership = getpid())` | Instance with its ownership (`pid_t`) |
| `init<true, policy>(name, size, queue)` | **Create** the shared memory (`policy`: slot layout, see below) |
| `init(name)` | **Open** an existing one (the size and queue are read from it) |
| `send(bytes, target, lastSending, lastTransmission, failsafe)` | Send a message, returns its `Id` (`failsafe = true` waits for a free slot instead of `OutOfMemory`) |
| `send(bytes, id, target, ...)` | Answer on an existing `Id` (conversation) |
| `readable(filter = All)` / `readable(id)` | Something to read |
| `read(filter = All)` | `std::optional<std::unordered_map<Id, std::vector<std::vector<std::byte>>>>`, removed from the storage |
| `read(id)` | The messages of one `Id` |
| `join(last = false)` / `join(id, last)` | Wait for a message (or for a `last` one), returns when the memory is closed |
| `close()` | Stop the reader thread, unmap (also done by the destructor) |
| `ownership()` / `ownership(pid)` | Get / set the ownership |

> [!WARNING]
> `SharedMemory(std::uint16_t ownership)` is a migration (`_migration(6, 0, 0)`): it will be removed around `v6.0.0`, use the `pid_t` constructor.

| Type | Description |
|---|---|
| `Target(limit, ownership)` | To one reader (`ownership`), read `limit` times |
| `Target(limit)` | Global: to every reader, `limit` readers (`0` = every connected reader except the sender) |
| `Id{id, ownership}` | Conversation id (`0` = no conversation) |
| `ReadFilter` | `All`, `ZeroOnly` (id `0`), `NonZeroOnly`, `LastOnly` (messages sent with `lastSending` / `lastTransmission`) |
| `LayoutPolicy` | `Compact` (1 writer), `CompactSemiAligned`, `Interleaved`, `InterleavedAligned` (many writers at the same time, one cache line per slot) |

```cpp
// process A
utils::encapsulation::SharedMemory server(pid_t{1001});
server.init<true>("/my-shm", 64, 4);                        // 4 slots of 64 bytes
utils::encapsulation::shm::Id id = server.send(bytes, utils::encapsulation::shm::Target(1, pid_t{1002}));

// process B
utils::encapsulation::SharedMemory client(pid_t{1002});
client.init("/my-shm");
client.join();
auto messages = client.read();                               // id -> payloads
```

## Errors {#errors}

| Code | When |
|---|---|
| `Pipe`, `Dup`, `Fork`, `Waitpid`, `Kill`, `Process` | `Pipe` / `Dup` / `Process` failures |
| `UndefinedCall` | Windows: `Process::spawn()` or a `dup` on a fd other than `0`, `1`, `2` (`FatalException`) |
| `EPoll` | `epoll` failures (Windows: a non socket fd, `EPOLLET`) |
| `Dlopen`, `Dlsym`, `Dlclose` | `SharedObject` |
| `ShmOpen`, `Ftruncate`, `Fstat`, `Mmap`, `OutOfMemory`, `OutOfBounds`, `InvalidArgument`, `InvalidId` | `SharedMemory` (no free slot, message bigger than a slot, empty size...) |

## In C {#in-c}

| C++ | C |
|---|---|
| `Pipe` / `Dup` | `utils_pipe_t*` / `utils_dup_t*` (`utils_pipe_create(read, write)`, `utils_pipe_trigger`...) |
| `Process` | `utils_process_t*`: `utils_process_spawn` (fork), `utils_process_spawn_exec(process, path, args, count)`, `utils_process_wait(process, &status)` (`utils_process_status_t`) |
| `Poll` | `utils_poll_t*`: `utils_poll_link`, `utils_poll_wait(poll, delay, events, max)` into an array of `struct epoll_event` |
| `SharedObject` | `utils_shared_object_t*`: `utils_shared_object_load_function(so, name)` returns a `void*` to cast |
| `SharedMemory` | `utils_shm_t*`: `utils_shm_init(shm, name, size, queue, create, layout, group_size, forced)`, the layout chosen at runtime (`C_API_SHM_LAYOUT_*`); `utils_shm_send(shm, bytes, size, &target, ...)` with a `utils_shm_target_t`; `utils_shm_read` → `utils_shm_messages_t*` |

In C the group size of `C_API_SHM_LAYOUT_COMPACT_SEMI_ALIGNED` is a power of 2 from 1 to 64.

```c
#define _POSIX_C_SOURCE 200809L // strict C on Unix: the POSIX structures (pid_t, struct sockaddr...)
#define _Encapsulation
#include <utils.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    utils_pipe_t* pipe = utils_pipe_create(-1, -1);
    utils_process_t* process = utils_process_create();
    utils_process_status_t status = {0};
    const char* args[] = {"hello"};
    char buf[16] = {0};

    if (!pipe || !process || utils_pipe_trigger(pipe) != UTILS_OK)
        return 1;
    utils_process_dup(process, utils_pipe_get_write(pipe), STDOUT_FILENO); // child: stdout -> pipe
    if (utils_process_spawn_exec(process, "echo", args, 1) == -1)
        return 1;
    utils_pipe_close_write(pipe);
    if (read(utils_pipe_get_read(pipe), buf, sizeof(buf) - 1) > 0)
        printf("child said: %s", buf);                                     // "hello\n"
    utils_process_wait(process, &status);                                  // status.exited, status.code == 0
    utils_process_destroy(process);
    utils_pipe_destroy(pipe);
    return status.code;
}
```
