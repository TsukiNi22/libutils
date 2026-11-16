+++
title = "Glossary"
summary = "The words of libutils and of this documentation, with the page that explains each one."
section = "resources"
order = 60
keywords = "glossary words definition section group define handle guard status payload eetp slot ownership failsafe safe_mode fixed type sysroot"
+++

The words used across the library and this documentation.

## Library {#library}

| Word | Meaning |
|---|---|
| Section | A part of the library: a folder of `include/utils/`, a namespace and a define (`_Network`). [Link and include](page:usage#sections) |
| Group | A define that enables several sections: `_Handling`, `_Tools`, `_Utils` |
| Internal / external code | An exception code of libutils (`InternalCode`) / of your project (`ExternalCode`). [Custom exception codes](page:custom-exceptions) |
| Type (of an exception) | How serious it is: `None`, `Warning`, `Error`, `Fatal`, combined as bits. [Exception](page:exception#type) |
| Restriction | The types a code accepts |
| `failsafe` | A parameter that turns an error into a printed warning |
| `safe_mode` | A parameter that takes a lock (thread safe call) |
| Observer | A base class that tracks the instances of a class; the memory leak notifier reports the ones still alive at exit. [Security](page:security#observer) |
| Migration / legacy | An old API kept for a time (`_migration(x, y, z)`, removed around that version) or for good (`_legacy`) |

## Network and IPC {#network-ipc}

| Word | Meaning |
|---|---|
| Payload | One message of a connection, ended by the separator (`'\n'` by default). [Network](page:network#types) |
| EETP | The protocol of `EETPParser`: an RSA handshake (`SYN`, `SO`) then AES-256-GCM messages. [Communication](page:network-ipc#eetp) |
| Common RSA key | The key pair shared by both sides of EETP, `~/.ssh/common` and `~/.ssh/common.pub` |
| Slot | A fixed size part of a shared memory holding one message. [Encapsulation](page:encapsulation#shared-memory) |
| Ownership | The id of a `SharedMemory` instance (its pid by default), used to address the messages |
| Target | Who reads a message of the shared memory: one ownership, or every reader |

## C API {#c-api}

| Word | Meaning |
|---|---|
| Handle | An opaque pointer to a C++ object (`utils_cli_t*`), created by `_create` and released by `_destroy`. [Use libutils from C](page:c-api#handles) |
| Guard | The `try` / `catch` around each C function that turns an exception into a return value and a status. [The C API design](page:c-api-design#guard) |
| Status | `utils_exception`, the thread local record of the last error |
| Fixed types | The C types a template is generated for (`i8` ... `u64`, `size`, `f32`, `f64`, `f128`, `bool`, `char`, `str`). [The C API design](page:c-api-design#fixed-types) |
| `_r` function | The version of a function that writes its string in your buffer instead of a thread local one |

## Build {#build}

| Word | Meaning |
|---|---|
| `-op` / `-db` / `-as` | The optimized / debug / AddressSanitizer variant of a package |
| `-pre` | A pre-release package |
| Sysroot | The folder of the Windows headers and libraries used by MinGW-w64. [Windows](page:windows#requirements) |
| Toolchain | The CMake file that sets the compilers and the sysroot of a cross compilation |
