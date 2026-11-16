+++
title = "Platform"
summary = "Les appels système utilisés par toute la bibliothèque, avec les noms et le contrat de POSIX sous Unix et sous Windows, et les macros de plateforme."
keywords = "plateforme posix windows unix close read write pipe dup poll socket epoll dlopen tcgetattr strerror UTILS_WINDOWS UTILS_UNIX UTILS_PLATFORM winsock utils_platform"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_Platform` | `_Tools` | `utils::platform` | `utils/platform/` |

Les appels système utilisés par toute la bibliothèque, avec **les mêmes noms, arguments et contrat que POSIX** (`-1` et `errno` en cas d'erreur) : l'appel système sous Unix, une implémentation Win32 / Winsock / CRT sous Windows. Un code écrit avec eux compile sur les deux plateformes sans aucun `#ifdef`. Pourquoi et comment : [Plateformes](page:platforms).

## Détection de la plateforme {#detection}

`utils/platform/Platform.hpp`, toujours inclus par `utils/utils.hpp` et `utils/utils.h` :

| Macro | Valeur |
|---|---|
| `UTILS_UNIX` | Définie sous Linux, BSD, macOS |
| `UTILS_WINDOWS` | Définie sous Windows (`_WIN32`, MinGW-w64) |
| `UTILS_PLATFORM` | `"unix"` / `"windows"` |

## Appels {#calls}

| Header | Appels |
|---|---|
| `utils/platform/Unistd.hpp` | `close`, `read`, `write`, `pipe`, `dup`, `dup2`, `isatty`, `poll`, `getpid`, `readlink`, `strerror` |
| `utils/platform/Socket.hpp` | `socket`, `connect`, `bind`, `listen`, `accept`, `recv`, `send`, `setsockopt`, `getsockname`, `getpeername`, `getaddrinfo`, `freeaddrinfo`, `gai_strerror` |
| `utils/platform/Epoll.hpp` | `epoll_create1`, `epoll_ctl`, `epoll_wait` (un epoll se ferme avec `close`) |
| `utils/platform/Dlfcn.hpp` | `dlopen`, `dlsym`, `dlclose`, `dlerror`, `dladdr` |
| `utils/platform/Termios.hpp` | `tcgetattr`, `tcsetattr` |

Sous Windows, les types et constantes POSIX de ces appels sont fournis avec les valeurs de Linux par `utils/platform/Types.h` (C et C++) : `struct epoll_event`, `EPOLLIN`, `EPOLL_CTL_ADD`, `struct termios`, `ICANON`, `ECHO`, `ISIG`, `TCSAFLUSH`, `Dl_info`, `RTLD_NOW`, `socklen_t`, `MSG_NOSIGNAL` (`0`), `STDIN_FILENO`...

> [!IMPORTANT]
> Les headers publics n'incluent jamais `<windows.h>` / `<winsock2.h>` (pas de macros Windows dans votre code) : sous Windows, les structures des sockets (`sockaddr_in`, `addrinfo`, `pollfd`, `timeval`) viennent de `<winsock2.h>` / `<ws2tcpip.h>`, inclus par votre code.

## Comportement sous Windows {#windows-behavior}

| Appel | Implémentation Windows |
|---|---|
| `close` | N'importe quel fd : fd CRT (`_close`), socket (`closesocket`) ou epoll émulé |
| `read` / `write` | CRT (`_read` / `_write`), `recv` / `send` sur une socket |
| `pipe` / `dup` / `dup2` | `_pipe` CRT (mode binaire) / `_dup` / `_dup2` |
| `poll` | `WSAPoll` pour les sockets ; entrée console, pipe et fichier vérifiée par tranches de 10 ms |
| `readlink` | `"/proc/self/exe"` = `GetModuleFileName`, sinon un lien symbolique |
| `strerror` | Aussi les erreurs des sockets (message du code Winsock), en anglais |
| appels socket | Winsock 2, démarré à la première utilisation ; `WSAGetLastError` traduit en `errno` (`WSAECONNREFUSED` → `ECONNREFUSED`...) ; `SO_SNDTIMEO` / `SO_RCVTIMEO` prennent un `timeval` ; `SO_REUSEADDR` est le comportement par défaut (sans effet) |
| `epoll_*` | Émulation au-dessus de `WSAPoll` : sockets uniquement (`ENOTSOCK`), déclenchement sur niveau (`EPOLLET` → `EINVAL`), `EPOLLONESHOT` émulé, au plus 10 ms de latence |
| `dlopen` / `dlsym` / `dlclose` / `dladdr` | `LoadLibraryA` / `GetProcAddress` / `FreeLibrary` / `GetModuleHandleExA` + `GetModuleFileNameA` |
| `tcgetattr` / `tcsetattr` | Mode de la console : `ICANON` = saisie par ligne, `ECHO` = écho, `ISIG` = ctrl+c ; sans `ICANON` l'entrée donne les séquences d'échappement (entrée VT) |

> [!WARNING]
> Le `<windows.h>` complet déclare un membre nommé `_unused`, une macro d'attribut de libutils : incluez `<windows.h>` **avant** `<utils/utils.hpp>`, ou définissez `WIN32_LEAN_AND_MEAN`.

## Exemple {#example}

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

## Erreurs {#errors}

Les appels ne lèvent jamais d'exception : ils renvoient `-1` (`nullptr` pour `dlopen` / `dlsym`, un code `EAI_*` pour `getaddrinfo`) et définissent `errno` (`dlerror` pour le chargement dynamique), comme les appels système. Les classes construites au-dessus ([Encapsulation](page:encapsulation), [Network](page:network), [Cli](page:cli)) transforment ces erreurs en exceptions.

## En C {#in-c}

Chaque appel est `utils_platform_<name>` avec le même contrat (`-1` et `errno`, pas de `utils_exception`). `dladdr` remplit un `utils_platform_dl_info_t` au lieu de `Dl_info` (qui a besoin de `_GNU_SOURCE`).

> [!NOTE]
> En C strict (`-std=c11`, `-std=c23`) sous Unix, les structures POSIX de ces appels ont besoin des fonctionnalités POSIX : `#define _POSIX_C_SOURCE 200809L` avant le premier include (déjà le cas par défaut avec `-std=gnu11` / `gnu23`).

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
