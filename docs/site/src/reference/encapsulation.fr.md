+++
title = "Encapsulation"
summary = "Wrappers RAII des outils système : Pipe, Dup, Process, Poll (epoll), SharedObject (.so / .dll) et SharedMemory (messages entre processus)."
keywords = "encapsulation pipe dup processus fork spawn exec wait kill poll epoll objet partagé dlopen mémoire partagée shm slot ownership target utils_process utils_shm"
+++

| Define | Groupe | Espace de noms | En-têtes |
|---|---|---|---|
| `_Encapsulation` | `_Tools` | `utils::encapsulation` | `utils/encapsulation/` |

Wrappers RAII autour des outils système (via [Platform](page:platform), même API sous Unix et Windows) : les fd sont fermés par les destructeurs, chaque erreur est une exception avec le message de `errno` en info. La mémoire partagée est expliquée dans [Communication](page:network-ipc#shared-memory).

## Pipe et Dup {#pipe-dup}

| `Pipe` | Description |
|---|---|
| `Pipe(read = -1, write = -1)` / `Pipe(fds)` | Enveloppe des fd existants (ou aucun) |
| `trigger()` | Crée le pipe (erreur `Pipe` s'il est déjà ouvert) |
| `getRead()`, `getWrite()` | fd (`-1` = fermé), les deux avec `getFds()` |
| `closeRead()`, `closeWrite()`, `close()` | Ferme un côté / les deux |
| `clear()` | Oublie les fd sans les fermer |

| `Dup` | Description |
|---|---|
| `Dup(origin = -1, clone = -1)` | `origin` sera dupliqué sur `clone` |
| `trigger()` | `dup(origin)` quand `clone == -1`, sinon `dup2(origin, clone)` (erreur `Dup` quand `origin` est fermé) |
| `getOrigin()`, `getClone()`, `setOrigin(fd)`, `setClone(fd)` | Accès |
| `closeOrigin()`, `closeClone()`, `close()`, `clear()` | Fermeture / oubli |

Les deux sont déplaçables, pas copiables.

## Process {#process}

| Méthode | Description |
|---|---|
| `pipe(read, write)` / `pipe(Pipe&)` / `pipe(std::vector<Pipe>&)` | Pipes créés (`trigger`) par le prochain `spawn` |
| `dup(origin, clone = -1)` / `dup(Dup&)` / `dup(std::vector<Dup>&)` | Dups appliqués dans l'enfant (ex : rediriger son stdout) |
| `spawn()` | `fork` : renvoie `0` dans l'enfant, le pid dans le parent (Windows : `FatalException` `UndefinedCall`) |
| `spawn(path, args)` | `fork` + `execvp(path, args)` dans l'enfant, renvoie le pid (code de sortie `127` quand l'exec ou un dup échoue) ; Windows : `CreateProcess`, cherché dans le `PATH` |
| `replace(path, args)` | `execvp` dans le processus courant (ne retourne jamais : exec ou `_exit(127)`) ; Windows : `_execvp` de la CRT |
| `wait()` | `waitpid`, renvoie un `Status{unknown, exited, code, sig}` (Windows : code de sortie, `sig` toujours à `0`) |
| `kill()` | `SIGKILL` puis attente (Windows : `TerminateProcess`, code de sortie `137`) |
| `is()` | L'enfant est-il vivant |
| `isChild()`, `isParent()`, `getPid()` | État |
| `getPipes()`, `getDups()`, `clear()` | Accès / fermeture des pipes et des dups |

> [!NOTE]
> Le destructeur du parent tue un enfant en cours d'exécution. Les dups sont vérifiés avant le `fork` : un dup invalide lance `Dup` dans l'appelant.

> [!IMPORTANT]
> Windows n'a pas de `fork` : `spawn()` lève une `FatalException` (`UndefinedCall`, le programme s'interrompt), et seuls les fd `0`, `1`, `2` de l'enfant peuvent être redirigés par `dup` (un autre `clone` → même `FatalException` au `spawn`). Seuls les handles standard redirigés sont donnés à l'enfant : les autres pipes ne fuient pas et l'EOF reste atteignable.

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

Wrapper d'`epoll` ([Platform](page:platform#windows-behavior) : émulé par-dessus `WSAPoll` sous Windows, sockets seulement, en mode level-triggered).

| Méthode | Description |
|---|---|
| `Poll(flags = EPOLL_CLOEXEC)` / `init(flags)` | Crée l'epoll |
| `link(fd, events, data = nullptr)` | Surveille un fd (`EPOLLIN`...) |
| `edit(fd, events, data)` | Change les événements surveillés |
| `unlink(fd)` | Arrête la surveillance (ignoré pour un fd fermé) |
| `wait(delay = -1, limits = 0)` | Attend des événements (`delay` en ms, `-1` = sans fin), renvoie les `epoll_event` |
| `size()`, `contains(fd)`, `getFd()`, `close()` | État |

## SharedObject {#shared-object}

Charge une bibliothèque partagée (`.so`, `.dll` sous Windows) et ses fonctions.

| Méthode | Description |
|---|---|
| `SharedObject(path)` | `dlopen` (erreur `Dlopen`), `LoadLibrary` sous Windows |
| `loadFunction<T>(name)` | `dlsym`, converti vers le type de pointeur de fonction `T` (erreur `Dlsym`) |
| `isLoaded()`, `path()`, `get()` | État, chemin, handle brut |

```cpp
using StrlenFn = std::size_t (*)(const char*);
utils::encapsulation::SharedObject libc("libc.so.6");
StrlenFn fn = libc.loadFunction<StrlenFn>("strlen");
```

## SharedMemory {#shared-memory}

Passage de messages entre processus via une mémoire partagée POSIX divisée en **slots** (`queue` slots de `size` octets). Chaque instance a une **ownership** (par défaut : son pid) qui sert à adresser les messages ; un thread lecteur copie les messages destinés à cette instance dans un stockage local.

> [!NOTE]
> Windows : un file mapping (`Local\utils-shm-<name>`) et un événement nommé à la place du futex (les lecteurs sont réveillés en moins de 10 ms) ; la mémoire est détruite avec son dernier handle (pas de persistance comme avec `/dev/shm`).

| Méthode | Description |
|---|---|
| `SharedMemory(ownership = getpid())` | Instance avec son ownership (`pid_t`) |
| `init<true, policy>(name, size, queue)` | **Crée** la mémoire partagée (`policy` : disposition des slots, voir plus bas) |
| `init(name)` | **Ouvre** une mémoire existante (la taille et la queue y sont lues) |
| `send(bytes, target, lastSending, lastTransmission, failsafe)` | Envoie un message, renvoie son `Id` (`failsafe = true` attend un slot libre au lieu de `OutOfMemory`) |
| `send(bytes, id, target, ...)` | Répond sur un `Id` existant (conversation) |
| `readable(filter = All)` / `readable(id)` | Quelque chose à lire |
| `read(filter = All)` | `std::optional<std::unordered_map<Id, std::vector<std::vector<std::byte>>>>`, retiré du stockage |
| `read(id)` | Les messages d'un `Id` |
| `join(last = false)` / `join(id, last)` | Attend un message (ou un message `last`), retourne quand la mémoire est fermée |
| `close()` | Arrête le thread lecteur, supprime le mapping (aussi fait par le destructeur) |
| `ownership()` / `ownership(pid)` | Lit / définit l'ownership |

> [!WARNING]
> `SharedMemory(std::uint16_t ownership)` est une migration (`_migration(6, 0, 0)`) : il sera supprimé vers `v6.0.0`, utilisez le constructeur `pid_t`.

| Type | Description |
|---|---|
| `Target(limit, ownership)` | Vers un lecteur (`ownership`), lu `limit` fois |
| `Target(limit)` | Global : vers tous les lecteurs, `limit` lecteurs (`0` = tous les lecteurs connectés sauf l'émetteur) |
| `Id{id, ownership}` | Id de conversation (`0` = pas de conversation) |
| `ReadFilter` | `All`, `ZeroOnly` (id `0`), `NonZeroOnly`, `LastOnly` (messages envoyés avec `lastSending` / `lastTransmission`) |
| `LayoutPolicy` | `Compact` (1 écrivain), `CompactSemiAligned`, `Interleaved`, `InterleavedAligned` (plusieurs écrivains en même temps, une ligne de cache par slot) |

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

## Erreurs {#errors}

| Code | Quand |
|---|---|
| `Pipe`, `Dup`, `Fork`, `Waitpid`, `Kill`, `Process` | Échecs de `Pipe` / `Dup` / `Process` |
| `UndefinedCall` | Windows : `Process::spawn()` ou un `dup` sur un fd autre que `0`, `1`, `2` (`FatalException`) |
| `EPoll` | Échecs d'`epoll` (Windows : un fd qui n'est pas un socket, `EPOLLET`) |
| `Dlopen`, `Dlsym`, `Dlclose` | `SharedObject` |
| `ShmOpen`, `Ftruncate`, `Fstat`, `Mmap`, `OutOfMemory`, `OutOfBounds`, `InvalidArgument`, `InvalidId` | `SharedMemory` (aucun slot libre, message plus gros qu'un slot, taille nulle...) |

## En C {#in-c}

| C++ | C |
|---|---|
| `Pipe` / `Dup` | `utils_pipe_t*` / `utils_dup_t*` (`utils_pipe_create(read, write)`, `utils_pipe_trigger`...) |
| `Process` | `utils_process_t*` : `utils_process_spawn` (fork), `utils_process_spawn_exec(process, path, args, count)`, `utils_process_wait(process, &status)` (`utils_process_status_t`) |
| `Poll` | `utils_poll_t*` : `utils_poll_link`, `utils_poll_wait(poll, delay, events, max)` dans un tableau de `struct epoll_event` |
| `SharedObject` | `utils_shared_object_t*` : `utils_shared_object_load_function(so, name)` renvoie un `void*` à convertir |
| `SharedMemory` | `utils_shm_t*` : `utils_shm_init(shm, name, size, queue, create, layout, group_size, forced)`, la disposition étant choisie à l'exécution (`C_API_SHM_LAYOUT_*`) ; `utils_shm_send(shm, bytes, size, &target, ...)` avec un `utils_shm_target_t` ; `utils_shm_read` → `utils_shm_messages_t*` |

En C, la taille de groupe de `C_API_SHM_LAYOUT_COMPACT_SEMI_ALIGNED` est une puissance de 2 entre 1 et 64.

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
