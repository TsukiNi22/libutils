+++
title = "Plateformes"
summary = "Un seul code pour Unix et Windows : la bibliothèque n'appelle le système qu'à travers utils::platform, POSIX sous Unix, Win32 / Winsock / CRT sous Windows, compilé en croisé avec MinGW-w64."
keywords = "plateformes unix windows posix mingw compilation croisée utils::platform winsock wsapoll createprocess différences toolchain"
+++

Chaque classe a la même API sous Unix et Windows : la bibliothèque n'appelle le système qu'à travers `utils::platform::`, dont les fonctions ont les noms, les arguments et le contrat de POSIX (`-1` et `errno`). Sous Unix, ce sont les appels système ; sous Windows, une implémentation Win32 / Winsock / CRT. Les mêmes appels sont fournis aux programmes par la section [Platform](page:platform), en C++ et en C.

:::html
<figure>
<svg viewBox="0 0 760 260" role="img" aria-label="The classes of libutils call utils::platform, implemented by the system calls on Unix and by Win32, Winsock and the CRT on Windows">
<defs>
<marker id="arr7-fig6-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
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
<path class="edge" d="M380 70V101" marker-end="url(#arr7-fig6-fr)"/>
<path class="edge" d="M260 155L204 186" marker-end="url(#arr7-fig6-fr)"/>
<path class="edge" d="M500 155L556 186" marker-end="url(#arr7-fig6-fr)"/>
</svg>
<figcaption>Un seul code pour les deux plateformes, la différence est dans <code>utils::platform</code>.</figcaption>
</figure>
:::

## Appels {#calls}

| Header | Appels | Implémentation Windows |
|---|---|---|
| `Unistd.hpp` | `close`, `read`, `write`, `pipe`, `dup`, `dup2`, `isatty`, `poll`, `getpid`, `readlink`, `strerror` | CRT, ou Winsock pour un socket ; `poll` aussi sur la console / les pipes / les fichiers ; `strerror` connaît les erreurs de socket |
| `Socket.hpp` | `socket`, `connect`, `bind`, `listen`, `accept`, `recv`, `send`, `setsockopt`, `getsockname`, `getpeername`, `getaddrinfo` | Winsock 2 démarré à la première utilisation, erreurs traduites dans `errno` |
| `Epoll.hpp` | `epoll_create1`, `epoll_ctl`, `epoll_wait` | émulation sur `WSAPoll` : sockets uniquement, level triggered, `EPOLLONESHOT` émulé |
| `Dlfcn.hpp` | `dlopen`, `dlsym`, `dlclose`, `dlerror`, `dladdr` | `LoadLibraryA`, `GetProcAddress`, `FreeLibrary`, `GetModuleHandleExA` |
| `Termios.hpp` | `tcgetattr`, `tcsetattr` | mode console (`ICANON` = saisie par ligne, `ECHO`, `ISIG` = ctrl+c, entrée VT) |

Les types et constantes POSIX dont ces appels ont besoin sous Windows (`struct epoll_event`, `struct termios`, `Dl_info`, `socklen_t`, `RTLD_*`, `EPOLL*`...) sont dans `utils/platform/Types.h`, un header pour C et C++.

## Différences {#differences}

Pas de `fork` sous Windows : `Process::spawn()` et un `dup` sur un fd autre que `0`, `1`, `2` lèvent une `FatalException` (`UndefinedCall`) ; `spawn(path, args)` utilise `CreateProcess`.

Autres différences : `Poll` / `Client` / `Server` ne gèrent que des sockets (au plus 10 ms de latence), un `Process` n'a pas de signal, <kbd>Ctrl</kbd>+<kbd>Z</kbd> est un caractère pour la `Cli`, une `SharedMemory` disparaît avec son dernier handle. Incluez `<windows.h>` avant `<utils/utils.hpp>` (il déclare un membre nommé `_unused`, une macro de libutils), ou définissez `WIN32_LEAN_AND_MEAN`.

## Compilation croisée {#cross-compilation}

Les binaires Windows sont compilés depuis Linux avec MinGW-w64 et `cmake/toolchain/mingw-w64.cmake`, utilisable par n'importe quel projet. Un projet garde le même code CMake pour les deux plateformes (`find_package(utils)` + `utils::utils`) : seule la commande de configuration ajoute la toolchain.

| Réglage de la toolchain | Valeur |
|---|---|
| compilateurs | `x86_64-w64-mingw32-gcc` / `-g++` / `-windres` (`-DMINGW_PREFIX=<triplet>`) |
| `CMAKE_FIND_ROOT_PATH` | le sysroot MinGW : `/usr/x86_64-w64-mingw32/sys-root/mingw` (Fedora), `/usr/x86_64-w64-mingw32` (Debian) ; bibliothèques, headers et paquets cherchés uniquement là |
| `CMAKE_CROSSCOMPILING_EMULATOR` | `wine` quand il est installé (`WINEPATH` = le `bin/` du sysroot) : `ctest` lance `unit_tests.exe` |

Types de build : `Debug` utilise `mingw64-libutils-db`, tout le reste `mingw64-libutils-op` ; pas d'Asan avec MinGW. Le guide complet est [Windows](page:windows).
