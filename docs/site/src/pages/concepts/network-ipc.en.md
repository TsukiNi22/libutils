+++
title = "Communication"
summary = "How the TCP server and client exchange payloads, how the EETP protocol encrypts them, and how processes talk through pipes and the slotted shared memory."
section = "concepts"
order = 40
keywords = "network ipc tcp payload separator epoll listen join eetp syn so aes gcm rsa handshake process pipe shared memory slot ownership futex target"
+++

Two ways for programs to talk: the **network** (a TCP `Server` and `Client`, optionally encrypted by EETP) and the **processes** of one machine (pipes, a child process, a shared memory). The classes are on [Network](page:network), [SManip](page:smanip#eetp) and [Encapsulation](page:encapsulation).

## Payloads over TCP {#payloads}

A `Server` and a `Client` exchange **payloads** (strings ended by a separator, `'\n'` by default) through a socket object. `listen()` reads what is available without blocking, `join()` waits for the next epoll event; `stop()` / `kill()` can be called from another thread. On Windows the sockets use Winsock 2 and the epoll is emulated over `WSAPoll`.

:::html
<figure>
<svg viewBox="0 0 760 210" role="img" aria-label="Server and Client use an ISocket, the EETPParser frames the payloads">
<defs>
<marker id="arr2-fig4-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
<marker id="arr2a-fig4-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow-accent" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box" x="30" y="30" width="180" height="50" rx="8"/>
<text class="lbl" x="120" y="60" text-anchor="middle">Client</text>
<rect class="box" x="550" y="30" width="180" height="50" rx="8"/>
<text class="lbl" x="640" y="60" text-anchor="middle">Server</text>
<rect class="box-alt" x="30" y="130" width="180" height="50" rx="8"/>
<text class="lbl" x="120" y="160" text-anchor="middle">TCPSocket</text>
<rect class="box-alt" x="550" y="130" width="180" height="50" rx="8"/>
<text class="lbl" x="640" y="160" text-anchor="middle">TCPSocket + epoll</text>
<path class="edge" d="M120 80V126" marker-end="url(#arr2-fig4-en)"/>
<path class="edge" d="M640 80V126" marker-end="url(#arr2-fig4-en)"/>
<path class="edge-accent" d="M210 155H546" marker-start="url(#arr2a-fig4-en)" marker-end="url(#arr2a-fig4-en)"/>
<text class="small" x="378" y="146" text-anchor="middle">payload '\n' payload '\n' ...</text>
<text class="small" x="378" y="176" text-anchor="middle">TCP</text>
<path class="edge dash" d="M210 55H546" marker-end="url(#arr2-fig4-en)"/>
<text class="small" x="378" y="47" text-anchor="middle">EETPParser::format / parse</text>
</svg>
<figcaption>The network layers; the EETP parser is optional, on top of the payloads.</figcaption>
</figure>
:::

The socket splits the stream into payloads and keeps what is not complete yet: a payload longer than the overflow limit (`4048` characters without a separator) is an error, a peer that closes the connection is the `None` exception `SocketClosed` and the status `Down`.

## EETP {#eetp}

`EETPParser` keeps the keys of each peer: the client sends `SYN` (its RSA public key, encrypted with the common RSA key of `~/.ssh/common`), the server answers `SO` (a new AES-256 key, encrypted with the client key), then every message is AES-256-GCM encrypted with a new random iv.

| Type | Direction | Content |
|---|---|---|
| `SYN` | client → server | client RSA public key (common RSA) |
| `SO` | server → client | AES-256 key (client RSA) |
| other | ↔ | AES-256-GCM, frame `<iv><tag> ETB <content>`, Base64 encoded |
| `EM` | ↔ | disconnection, never encrypted |

> [!WARNING]
> The framing changed in `v3.0.0`: both sides must use `v3.0.0` or later.

## Processes {#processes}

`Process` wraps `fork` / `execvp` / `waitpid` with pipes and fd duplications applied in the child; `Poll` wraps epoll; `SharedObject` loads a `.so`. On Windows: `CreateProcess`, `WSAPoll`, `LoadLibrary` (`.dll`); there is no `fork`, so `spawn()` alone is a `FatalException` (`UndefinedCall`) and only the standard fds can be redirected.

## Shared memory {#shared-memory}

`SharedMemory` passes messages between processes through a POSIX shared memory split in **slots**. On Windows it is a file mapping with a named event.

:::html
<figure>
<svg viewBox="0 0 760 200" role="img" aria-label="Shared memory with a metadata header and slots read by reader threads">
<defs>
<marker id="arr3-fig5-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="group" x="170" y="20" width="420" height="160" rx="10"/>
<text class="title" x="186" y="44">/name (shm)</text>
<rect class="box-alt" x="186" y="56" width="388" height="30" rx="6"/>
<text class="small" x="380" y="76" text-anchor="middle">ShmMetadata: size, queue, connected, readable (futex), sequence</text>
<rect class="box" x="186" y="100" width="90" height="60" rx="6"/>
<text class="small" x="231" y="126" text-anchor="middle">slot 0</text>
<text class="small" x="231" y="146" text-anchor="middle">flag · target</text>
<rect class="box" x="285" y="100" width="90" height="60" rx="6"/>
<text class="small" x="330" y="134" text-anchor="middle">slot 1</text>
<rect class="box" x="384" y="100" width="90" height="60" rx="6"/>
<text class="small" x="429" y="134" text-anchor="middle">slot ...</text>
<rect class="box" x="483" y="100" width="90" height="60" rx="6"/>
<text class="small" x="528" y="134" text-anchor="middle">slot n-1</text>
<rect class="box" x="20" y="100" width="120" height="60" rx="8"/>
<text class="lbl" x="80" y="126" text-anchor="middle">process A</text>
<text class="small" x="80" y="146" text-anchor="middle">send(bytes, target)</text>
<rect class="box" x="620" y="100" width="120" height="60" rx="8"/>
<text class="lbl" x="680" y="126" text-anchor="middle">process B</text>
<text class="small" x="680" y="146" text-anchor="middle">reader thread</text>
<path class="edge" d="M140 130H182" marker-end="url(#arr3-fig5-en)"/>
<path class="edge" d="M574 130H616" marker-end="url(#arr3-fig5-en)"/>
</svg>
<figcaption>A writer claims a free slot (flag 0 → 1), writes, publishes it (flag 2) and wakes the readers; the reader thread of each instance copies the messages addressed to it into a local storage read with <code>read()</code>.</figcaption>
</figure>
:::

A message is addressed to one reader (`Target(limit, ownership)`) or to every reader (`Target(limit)`, `0` = all the connected ones). An `Id{id, ownership}` links the answers of a conversation, `join()` waits for a message and returns when the memory is closed. The **layout** of the slots depends on the writers: `Compact` for one writer, `Interleaved*` for many writers at the same time (one cache line per slot for `InterleavedAligned`).
