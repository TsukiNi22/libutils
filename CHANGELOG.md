# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

| Scope | Content |
| ----- | ------- |
| `In Progress` | Work that hasn't been completed yet (typically listed under [pre-release]) |
| `Added`       | New features, files, or sections introduced in this release |
| `Removed`     | Existing features, files, or sections that have been deleted |
| `Changed`     | Existing features or sections that were modified (behavior, structure, or content) |
| `Fixed`       | Bugs or issues that have been resolved |

> [!NOTE]
> The `(unofficial)` version are sub-version (such has fix typically) that don't have associated tag. The version often includes other sub-versions that weren't documented separately.
> A packaged builded at this version is not guaranteed!

---

## [pre-release] (empty)

## v3.0.0 - 2026-10-05
### Changed
- `utils::verbose::verbose` is a `std::atomic<utils::verbose::Verbose>` (was `volatile`): the level can be changed while other threads log
- `iomanip`: `flashing_slow` / `Style::FlashingSlow` is `5` and `flashing_fast` / `Style::FlashingFast` is `6` (were swapped), `strong_reset` / `ResetStyle::Strong` is `22` (`21` is a double underline on most terminals), `flashing_slow_reset` / `ResetStyle::FlashingSlow` is `25` (`26` is not a blink reset)
- `Process::spawn(path, args)` checks the `Dup` before the fork (throws `Dup` in the caller) and builds the arguments before the fork
- `AESKey::encrypt` / `decrypt` and `Base64Codec` throw `OutOfBounds` for data larger than `INT_MAX` bytes (OpenSSL limit)
- The packages: `libutils-dev` requires `openssl-devel` / `libssl-dev` (public headers include OpenSSL), the RPM license is `MIT AND CC-BY-NC-SA-4.0`, `THIRD_PARTY_NOTICES.md` and the algorithms license are installed with the `LICENSE`
- **[MAJOR]** The `EETPParser` framing of the special payloads changed: the `<type>` of `SYN`/`SO` is now in clear before the RSA encrypted data and every `<data>` is encoded with the codec (not compatible with the previous versions)
- **[MAJOR]** The sockets moved from `utils::network::socket` to `utils::network` (`utils::network::socket` is kept with a migration warning until ~v4.0.0)
- `LoadBalancer::kill(n)` doesn't have a default value anymore (ambiguous with `kill()`), new method `size`
- **[MAJOR]** The `EETPParser` AES payloads use a new random iv for each payload, sent with the tag (`<iv><tag>`), the `SO` payload only send the AES key and each `<data>` is preceded by `ETB` (not compatible with the previous versions)
- The observer instances are accessed by `instances::id_handler()` / `instances::notifiers()` (were the global variables `instances::IdHandler` / `instances::Notifiers`)
- `IdHandler::free(bool)` (free all) became `IdHandler::clear(bool)` (`free()` without argument is kept)
- New pure virtual methods `receive` (read once, never wait a full payload) & `discard` (forget the buffers of a fd, `-1` = every fd) in `ISocket`
- `SharedMemory::send(bytes, id, target, ...)` wrapper for an own id now take the target (the old one was unusable)
- `SharedMemory` `Target` with a limit of `0` = every connected reader for a global request
- `read_mouse_event` read the real classic mouse report (`ESC[M` + 3 raw bytes)
- The verbose output lock is a `std::recursive_mutex` and the verbose macros don't declare a named lock anymore
- `rotate_point_3D` use the same convention as `to_look` (y up, forward +z): rotating the forward vector give the look vector
- The `Cli` logic operators `&&` / `||` follow the bash semantics (only the next command is skipped)
- `LoadBalancer::kill` / `kill(n)` only kill the workers not working, `kill<true>` / `kill<true>(n)` also kill the working ones
- `Server` throws `UnknownFd` (was `UnknownId`) for an unknown client fd
- `Settings::cast_float16` out of range throws `OutOfBounds`, `Settings::cast_char16` / `cast_char32` with neither a code point nor a single character throws `InvalidArgument`
- `Cluster::spawn(args...)` (spawn one) requires `T` to be constructible from the arguments
- `Middlewares::addBefore` / `addAfter` take a `const` reference (temporaries allowed)
- `CustomException(type, info)` require the info (ambiguous with the other constructor)
- Public names aligned on the naming convention, the old names are kept with a migration warning until ~v4.0.0: `Settings::auto_cast` -> `autoCast`, `utils::cli::Flags` -> `utils::cli::flags`, `SharedObject::isloaded` -> `isLoaded`, `iomanip::setStyle` / `resetStyle` / `readCursorPosition` / `readMouseEvent` / `readAdvancedMouseEvent` -> `set_style` / `reset_style` / `read_cursor_position` / `read_mouse_event` / `read_advanced_mouse_event`, `encryption::keyToString` / `stringToKey` -> `key_to_string` / `string_to_key`, `smanip::fixed_string` -> `FixedString` (`FixedString.hpp`, `fixed_string.hpp` kept)
- `concepts::convertible_to` renamed `ConvertibleTo` (the old concept is kept until ~v4.0.0 without warning: a concept can't be deprecated)
- `Process::replace` and `FatalException::display` are marked `_noreturn`

### Fixed
- **[MAJOR]** `Process::spawn(path, args)`: an exception thrown in the child before `execvp` (failed `Dup`) went back to the caller's code, the child kept running a copy of the program
- `Server::join` / `Client::join`: `stop` / `kill` from another thread closed the epoll fd while it was waited on (the fd number could be reused under the wait), it's now closed by the last `join` leaving
- `SharedMemory`: the release of a slot had no release ordering (data race on the slot metadata between a reader and the next writer, visible on weak memory models)
- `SharedMemory::close` never woke up the `join` calls (blocked forever) and reset the state without the lock
- `<cctype>` functions called with a negative `char` (UB) in `Cli` (input), `Settings` and the `ArgParser` hooks (non-ASCII input)
- `AESKey` / `Base64Codec` silently truncated the size of data larger than `INT_MAX` bytes
- The exception header generator wrote tabs in the restriction comments
- Conversion warnings (`-Wsign-conversion`, `-Wshorten-64-to-32`) in `src/`
- **[MAJOR]** The `EETPParser` handshake was always failing: the `<type>` of `SYN`/`SO` was encrypted with the data while `parse` read it before decrypting
- The last `<data>` of a payload wasn't decoded by `EETPParser::parse`
- The raw AES key/iv sent by `EETPParser` (`SO`) could contain the `ETB` separator and break the splitting
- `EETPParser` accepted unencrypted default payloads and tried to encrypt without AES key (key exchange not done)
- `RSAKey` can now encrypt data of any size (encrypted block by block), the OAEP padding limited the data to ~214 bytes (`Data too big to be encrypted`)
- **[MAJOR]** The reader thread of `SharedMemory` was often never awaken: `futex_wake` was waking only one waiter (`UINT32_MAX` given as an `int` = -1), the `close` could also hang forever on the join
- The reader thread of `SharedMemory` missed the requests sent while it was starting
- `SharedMemory::send` without `failsafe` never searched an empty slot (always `OutOfMemory`)
- A global request of `SharedMemory` could be read twice by the same reader and kept it counted as a reader (hang of the slot reset)
- `SharedMemory::close` unmapped only the size of a slot instead of the whole mapping
- `Process::is` on a process already waited/killed was calling `kill(-1, 0)` (always true)
- `Process::spawn` reported a failed `fork` with the `Kill` code instead of `Fork`
- `ASocket::setPayloadSeparator(char)` stored `std::to_string(c)` (`'\n'` -> `"10"`)
- `ASocket::recvAll(fd)` checked the buffer of the socket own fd, on the server side only the first payload of each reception was returned
- The unsigned casts of `Settings` refused the `+` sign accepted by `auto_cast` (`"+42"`)
- The `Char16`/`Char32` casts of `Settings` only accepted a code point number, not the single character detected by `auto_cast` (`"é"`)
- A flag refused by one usage of the `ArgParser` threw an error even if another usage accepted it
- The `default` usage of the `ArgParser` didn't allow any flag (documented as allowing all the flags & options)
- `ArgParser::setFlag` only checked the `unlimited` flag without option when forced
- Out of bounds read on an ordered usage of the `ArgParser`
- The default `Int32` parsing hook refused the negative numbers
- **[MAJOR]** Every `Cli` parsed command with a limited arguments number was failing (`Not enough arguments`), built-in commands included
- A callback exception caught by the `Cli` (`CATCH`) gave the code `255` instead of `130`
- **[MAJOR]** `LoadBalancer` couldn't be instantiated (`std::optional<T&>`, unknown `setStatus`, ambiguous constructors), the class was reworked
- A lifespan of `0` (infinite) killed the unused workers instantly and a limit of `0` (infinite) refused every spawn
- The internal threads of `LoadBalancer` were using members after their destruction
- `Scheduler::cancel(id)` was canceling every task and then always throwing `UnknownId`
- The id of a canceled task could be freed twice by the `Scheduler`
- The tasks of the `Scheduler` were joined after the destruction of the members they use
- `IdHandler::use` refused every id not distributed yet (inverted check)
- `IdHandler` never removed a forced id from the used ones when freed (also on free all)
- **[MAJOR]** `BidirectionalLookupTable` couldn't be constructed (`Freezable` had no default constructor)
- `BidirectionalLookupTable` could only be edited once frozen (inverted check)
- A forced override or a removal of `BidirectionalLookupTable` kept a dangling reverse link
- `BidirectionalLookupTable::removeElement` was `noexcept` while it can throw
- `operator!=` of `Vector3` & `OVector3` compared `z` with `==` instead of `!=`
- The `reset` keyword documented for `smanip::format` was missing
- The `verbose` unit tests were depending on the verbose mode left by the previous tests
- **[MAJOR]** `AESKey` read out of the buffers with a key/iv/tag of a wrong size (a received tag shorter than 16 bytes), the sizes are now checked and the iv length is given to GCM
- **[MAJOR]** `EETPParser` reused the same AES key & iv for every payload in both directions (GCM nonce reuse), the constructor accepted a null codec / a type size of 0, an empty `<data>` was lost
- **[MAJOR]** An object deriving from an observer (`Base64Codec`, `RSAKey`, ...) with a static storage could be built before the observer instances (crash before `main`)
- **[MAJOR]** Sending to a closed peer raised `SIGPIPE` and killed the process (`TCPSocket`)
- **[MAJOR]** `Server::listen` / `Client::listen` blocked on a partial payload, `listen(fd)` spun forever while another client had unread data, `Server::flush()` used a removed client (use after free), `listen(fd)` re-created a removed client
- **[MAJOR]** The buffers of a closed connection were given to the next connection using the same fd number
- **[MAJOR]** The `Process` destructor threw in the child (`std::terminate`), the moved-from `Process` killed the moved child
- **[MAJOR]** `SharedMemory::send(id, ...)` kept its lock while waiting a free slot (deadlock with the reader thread), the interleaved layout misaligned the metadata (atomics), the reader thread could lose a wakeup
- **[MAJOR]** `ArgParser` read out of bounds/invalid iterators: option before a flag in an unordered or 'default' usage, ordered usage with reversed flags, empty argument after an unlimited flag
- **[MAJOR]** `Cli` deadlocks: a command editing the commands, a hook using the cli (hooks called with the lock), a middleware editing its middlewares; an exception escaping the `THREAD` mode (`std::terminate`), a failing prompt looping forever
- `CustomException()` / `CustomException(type)` didn't compile (ambiguous constructors)
- `WarningException` with an external code & an info was created as a `None` exception
- `_NoWarning` (utils.hpp) defined `BACKWARD_COMPATIBILITY_WARNING` instead of `NO_BACKWARD_COMPATIBILITY_WARNING`
- A verbose call inside a verbose `*Fn` macro deadlocked
- `readMouseEvent` blocked (waited a newline never sent by the terminal) and couldn't parse the real classic report
- `Base64Codec::decode` didn't remove the padding with trailing whitespace (`"QQ==\n"` -> `"A\0"`)
- `ASocket`: overflow `0` (documented unlimited) refused every payload, the raw mode (empty separator) returned `""` without reading then looped forever in `recvAll`, `getaddrinfo` result leaked on error
- `TCPSocket::connect` / `listen` left the socket open on error (the next start was `Up` on a not connected socket)
- `Client` / `Server`: `stop` / `kill` from another thread closed the fds under a `listen` / `join` (double close), `epfd` leaked on a failed start, an error on the server socket threw `UnknownId` instead of crashing the server
- `Pipe` / `SharedObject` move assignment leaked the previous fds / library, `Poll::close` kept the registered fds
- `SharedMemory`: `readable(LastOnly)` was handled like `NonZeroOnly`, `read` checked the data before taking the lock (race) and `read(id)` could dereference `end()`, an id was leaked when the send failed, `close` kept the 'last' state
- `ArgParser`: an out of order flag in an ordered usage was silently dropped, the 'mandatory before' rule was applied to unordered usages, a duplicated flag didn't consume its option(s), a flag without short name matched every short argument, the environment variables search stopped at the first missing one and a throwing hook terminated the program
- `ArgParser` hooks: `defaultFileParsingHook` / `defaultDirectoryParsingHook` threw a `filesystem_error` (name too long, ...), `defaultWritableParsingHook` crashed on an empty path and opened then removed a fifo / followed a dangling symlink
- `Settings::auto_cast` threw on float64 out of range values (`1e-400`, denormals, `1e99999`) and never ended on a long string (path regex backtracking), `cast_float16` had no range check
- `Cli`: `getHistory` used the wrong lock and the history was written without lock, `start` pushed the inputs before the running check (race, inputs kept on failure), the destructor didn't stop a running thread and restored the default signal handlers instead of the previous ones, the default getc hook replayed the last char forever on EOF, auto completion without command replaced the input by `[None]`, a middleware failure gave the code 255/130 instead of 3
- `Middlewares` called the middlewares with the lock (deadlock when a middleware edit the middlewares), `clear` without lock, the assignments could deadlock (lock order)
- `Cluster::spawn(n, args...)` built the elements after the first from moved-from arguments, `spawn(n)` of a type not constructible from an integer didn't compile
- `IdHandler`: a freed forced id above the counter was given twice, the overflow wasn't checked while skipping the forced ids (0 & duplicates given), `free(id)` was ambiguous for an integer literal, `preview` ignored the forced ids and aborted on overflow (`IdOverflow` is now an error, only `allocate` is fatal)
- `Scheduler::clear` read the finished tasks without the lock, a task canceling itself joined its own thread (`std::terminate`)
- `LoadBalancer::kill` destroyed the working workers (dangling reference given by `getWorker`)
- `Vector2` / `Vector3` `dot` & `cross` truncated to the vector type (common type now), `normalize` of a zero vector divided by zero (SIGFPE for integers), the reverse operators of `OVector2` / `OVector3` returned a vector of the scalar type, `Vector3` / `OVector3` bitwise operators between vectors didn't compile
- `rotate_point_3D` and `to_look` used different axis conventions for the same orientation
- Unit tests: classes with the same name & different definitions in 3 test files (ODR), misaligned reference on a packed `epoll_event`

### Added
- `include/utils/algorithms/LICENSE.md` (CC BY-NC-SA 4.0 of `c2dmp-hsm` & `s.o.s`) and `THIRD_PARTY_NOTICES.md`: the MIT license of libutils doesn't apply to the dependencies
- Unit tests: `Process` (dup errors, `replace`), `SharedMemory` (close/join, layout helpers), `Client` / `Server` (stop/kill during `join`, no fd leak), key tools / `AKey` / `IKey`, `ANotifier`, `Cli` hooks (on a pseudo-terminal), `ArgParser` help/remove, `Settings` forced casts & non-ASCII input, every `iomanip` sequence, `Middlewares<void, void>`, `BidirectionalLookupTable::removeElements`, `CFrame`, `verbose::locked`
- Attribute macro `_noreturn` (`[[noreturn]]`)
- Unit tests for all the sections (`exception`, `concepts`, `manip`, `math`, `arguments`, `cli`, `network`, `encapsulation`, `system`, `pool`, `type`, `security`, `algorithms`)
- Regression unit tests for every bug fixed by the audit
- New exception code `UnknownFd` (`network`)

### Removed
- `utils::iomanip::format` (migration of `utils::smanip::format` until v3.0.0)

## v2.14.0 - 2026-09-27
### Added
- Add a new custom type `Matrix` and it's light version `OMatrix` in the `type` section

## v2.13.22 - 2026-09-24 (unofficial)
### Fixed
- **[MAJOR]** Missing exception constructor for `ExternalCode`
- Multiple many fix on `ArgParser` handling/check that where invalid or craching
- Error during cleaning of metadata on `SharedMemory` slot, added a status for reset process
- Missing constructor/operator on `SharedObject` to allow std::move
- Invalid awake for last enable during join of `SharedMemory` and invalid ptr initialized during `init` call
- The internal thread of `SharedMemory` was trying to read uninitialized data
- In `SharedMemory`, fix the permission of the memory opened has a 'client' and add the closing missing check
- `SharedMemory` size init check (was always failling in normal mode) and forgot to reset counter on slot metadata after reading

### Added
- Add missing option to select a target to send on `SharedMemory`
- Add missing default hook to check directory for `ArgParser`
- Add auto ownership generation and join implementation (forgot) for `SharedMemory` and last filter mode for read
- Warning for `_legacy` or `_migration(major, minor, fix)` on attribute
- Warning/Macro for depreacated/soon removed support
- Method to replace the actual process using `Process` (other `exec*` except for `execvp` are not supported for now)

## [v2.13.6-pre] - 2026-09-14
### Changed
- Change the size setup for `SharedMemory` on 'client' side, it's auto now

### Added
- Cmake support of versionning requirement using `find_package`
- Methods `contains` for the `Settings` handler
- Features for the `ArgParser` to handle raw text options

## [v2.13.2] - 2026-09-06
### Changed
- Change the ways the usages match are returned (sorted from more match to least) in the `ArgParser`

## [v2.13.1-pre] - 2026-09-06
### Added 
- Features for the `ArgParser` to handle environement variables
- Add of encapsulation for `Poll` (epoll is used internaly)

## [v2.12.2-pre] - 2026-09-05
### Changed
- Change naming policy for normal function/tools without class <name>(<Name>)* -> <name>(_<name>)*
- Switch `vector` sub-section to `type` section

### Added
- Add the encapsulation class for `SharedMemory` (handle multiple writer & reader with multiple mode for layout optimisation)

## [v2.11.0-pre] - 2026-08-20
### Changed
- Remove some sub-section part like the sub-section `middleware`
- Add new method to the `IdHandler` to preview ids and allow other actions (free all, const arg, ...)
- Single type for legacy of class moved to the `type` section like the `Worker` or other class like `BidirectionalLookupTable`

### Added
- Class legacy for freeze handling `Freezable`

## [v2.10.1-pre] - 2026-08-20
### Changed
- Artifact retention period changed to 1 day
- `IdHandler` switched section from `security` to `system` and new method added to preview ids and allow other actions (free all, const arg, ...)

### Added
- A `Scheduler` for task, cancel/schedule/overview
- A class to handle load balancing for worker `LoadBalancer`
- `Process` encapsulation method to check if it's running
- Unsigned math type: U<type>

### Fixed
- Change the `EETPParser` encoding of base64, it's was missing for the internal data sended

## [v2.9.2] - 2026-08-20
### Fixed
- **[MAJOR]** invalid namespace `utils::exceptionutils::exception` -> `utils::exception`

## [v2.9.0-pre] - 2026-08-20
### Added
- Unit tests for multiple sections (not all sections are covered, only: `security`, `verbose`)
- Documentation using github wiki (some sections might be missing or not finished)
- Add of a cluster class to handle groups of entity (new section `pool`)
- New class to encapsulation the shared object usage (new section `encapsulation`)
- Process encapsulation (Pipe **OK**, Dup **OK**, Process **OK**)

### Changed
- Rework the warning, now all are desactivated and can be activated/desactivated such has global/group/solo
- `vector` section moved to `math/vector`
- `middleware` section moved to `pool/middleware`

### Fixed
- The `math` section is now avaible on the `utils.hpp` include

## [v2.3.3] - 2026-08-16
### Added
- A parser class to handle formating and parsing of the network communication
- Network/Manip (Codec **OK**, Key **OK**, Parser **OK**, Socket **OK**, Client **OK**, Server **OK**, Testing **OK**)

### Fixed
- Fix 2etp parser with wrong framing
- Remove 

## v2.3.2 - 2026-08-16 (unofficial)
### Added
- Add custom output for verbose macro (mutex lock standart output -> global ouput)

### Fixed
- Multiple fix/testing for the network handling that will be on the official release for `v2.3.x`

## [v2.2.0] - 2026-08-01
### Changed
- **[MAJOR]** Rework the whole `observer` system
- The section `warning` became `security`

## [v2.1.0] - 2026-07-29
### Added
- Support for `.deb` along the existing `.rpm`
- Setup script that support any distro `fedora-like` & `debian-like`
- Rework of the file dispatching between packages (headers, cmake, ...)

## [v2.0.0] - 2026-07-28
### Changed
- **[MAJOR]** Change the way the lib is use, from file copy to `dnf` package handled
- The `write` section became the `manip` section with 2 sub-section `iomanip` & `smanip`
- All attribute where fixed to be used with other include `<name>` -> `_<name>`
- All exception are writted on cerr (was on cout before)

### Added
- `s.o.s` Algorithm
- Add the codec class to handle encoding/decoding of string
- Add of a fatal exception that can't be catch
- Better exception handling (link to the file, module name detection, ...)

### Removed
- Auto sync with cpp_project_template

## [v1.0.0] - 2026-07-17
### Added
- Initial version of the utils library with multiple tools (see [README-1.0.0](https://github.com/TsukiNi22/libutils/blob/v1.0.0/README.md) for more details)

[pre-release]: https://github.com/TsukiNi22/libutils/compare/v3.0.0...HEAD
[v3.0.0]: https://github.com/TsukiNi22/libutils/compare/v2.13.6-pre...v3.0.0
[v2.13.6-pre]: https://github.com/TsukiNi22/libutils/compare/v2.13.2...v2.13.6-pre
[v2.13.2]: https://github.com/TsukiNi22/libutils/compare/v2.13.1-pre...v2.13.2
[v2.13.1-pre]: https://github.com/TsukiNi22/libutils/compare/v2.12.2-pre...v2.13.1-pre
[v2.12.2-pre]: https://github.com/TsukiNi22/libutils/compare/v2.11.0-pre...v2.12.2-pre
[v2.11.0-pre]: https://github.com/TsukiNi22/libutils/compare/v2.10.1-pre...v2.11.0-pre
[v2.10.1-pre]: https://github.com/TsukiNi22/libutils/compare/v2.9.2...v2.10.1-pre
[v2.9.2]: https://github.com/TsukiNi22/libutils/compare/v2.9.0-pre...v2.9.2
[v2.9.0-pre]: https://github.com/TsukiNi22/libutils/compare/v2.3.3-release...v2.9.0-pre
[v2.3.3]: https://github.com/TsukiNi22/libutils/compare/v2.2.0...v2.3.3-release
[v2.2.0]: https://github.com/TsukiNi22/libutils/compare/v2.1.0...v2.2.0
[v2.1.0]: https://github.com/TsukiNi22/libutils/compare/v2.0.0...v2.1.0
[v2.0.0]: https://github.com/TsukiNi22/libutils/compare/v1.0.0...v2.0.0
[v1.0.0]: https://github.com/TsukiNi22/libutils/releases/tag/v1.0.0

---
