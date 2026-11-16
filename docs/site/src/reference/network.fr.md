+++
title = "Network"
summary = "Un Server et un Client TCP qui échangent des payloads via un objet socket ; la couche socket qui découpe le flux en payloads, et comment écrire son propre transport."
keywords = "réseau tcp serveur client socket payload listen join send flush statut ISocket ASocket TCPSocket is_ip resolve_hostname utils_server utils_client utils_socket"
+++

| Define | Groupe | Espace de noms | En-têtes |
|---|---|---|---|
| `_Network` | `_Handling` | `utils::network` | `utils/network/` |
| `_Socket` (inclus par `_Network`) | `_Network` | `utils::network` | `utils/network/socket/` |

Un `Server` et un `Client` TCP construits sur un objet socket. Les messages (**payloads**) sont des chaînes séparées par `'\n'` par défaut ; `listen` lit ce qui est disponible sans bloquer, `join` attend le prochain événement. Même API sous Unix et Windows (l'epoll est émulé par-dessus `WSAPoll` sous Windows, voir [Platform](page:platform#windows-behavior)). Les couches et le protocole EETP sont expliqués dans [Communication](page:network-ipc).

## Types {#types}

| Type | Définition |
|---|---|
| `Payload` | `std::string` (un message, sans son séparateur) |
| `Payloads` | `std::vector<Payload>` |
| `Ip` | `std::pair<std::string, std::string>` : `<ipv4 or hostname, hostname>` |
| `Address` | `{Ip ip = {"localhost", ""}, std::uint16_t port = 8080}` (l'ip est ignorée par un serveur, port `0` = n'importe quel port libre) |
| `Status` | `Up`, `Down` (arrêté, peut être redémarré), `Terminated` (tué), `Crashed` |

| Define | Valeur |
|---|---|
| `DEFAULT_IP` / `DEFAULT_PORT` | `"localhost"` / `8080` |
| `OVERFLOW_LIMIT` | `4048` caractères sans séparateur avant une erreur |
| `SOCKET_CHUNK_SIZE` | `4096` octets lus par appel |
| `SOCKET_TIMEOUT` | `7` secondes |

## Server {#server}

`Server(std::shared_ptr<ISocket> socket, Address address = {})` : chaque client est identifié par son fd.

| Méthode | Description |
|---|---|
| `start()` | Écoute sur l'adresse (redémarre après un `stop`) |
| `stop()` / `kill()` | Ferme toutes les connexions ; après `kill`, le serveur ne peut plus être redémarré |
| `listen()` | Accepte les nouveaux clients, lit ce qui est disponible, renvoie `fd → Payloads` (vidé au prochain appel) |
| `listen(fd)` | Pareil pour un seul client (`UnknownFd` s'il n'est pas connecté) |
| `join(fd = -1)` | Attend le prochain événement (n'importe quel client, ou un fd) |
| `send<buffered = false>(fd, payload)` | Envoie tout de suite, ou le stocke jusqu'au `flush` |
| `flush()` / `flush(fd)` | Envoie les payloads stockés |
| `getFds()` | Clients connectés |
| `getStatus()` | `Status` |

## Client {#client}

`Client(std::shared_ptr<ISocket> socket, Address address = {})` : le hostname de l'adresse est résolu par le constructeur.

| Méthode | Description |
|---|---|
| `start()` / `stop()` / `kill()` | Connexion / déconnexion / déconnexion définitive |
| `listen()` | Lit ce qui est disponible, renvoie les `Payloads` (vidés au prochain appel) |
| `join()` | Attend le prochain événement |
| `send<buffered = false>(payload)` / `flush()` | Envoie maintenant ou plus tard |
| `getStatus()` | `Status` (`Down` quand le serveur a fermé la connexion) |

## Threads {#threads}

`stop()` et `kill()` peuvent être appelés depuis un autre thread pendant qu'un `listen` / `join` tourne : le `join` retourne, l'epoll est fermé quand le dernier `join` se termine. Un envoi vers un pair fermé ne lève jamais `SIGPIPE`.

## Exemple {#example}

```cpp
#define _Network
#include <utils/utils.hpp>

// Server: echo every payload
utils::network::Server server(std::make_shared<utils::network::TCPSocket>(), {{"", ""}, 4242});
server.start();
while (server.getStatus() == utils::network::Status::Up) {
    server.join();
    for (const auto& [fd, payloads]: server.listen())
        for (const utils::network::Payload& payload: payloads)
            server.send(fd, payload);
}

// Client
utils::network::Client client(std::make_shared<utils::network::TCPSocket>(), {{"127.0.0.1", ""}, 4242});
client.start();
client.send("hello");
client.join();
for (const utils::network::Payload& payload: client.listen())
    std::cout << payload << std::endl;
```

## Erreurs {#errors}

| Code | Quand |
|---|---|
| `AlreadyRunning`, `Killed` (avertissements) | `start` alors qu'il tourne déjà / après un `kill` |
| `UnknownFd` | `listen(fd)` / `send(fd)` / `join(fd)` sur un client inconnu |
| `SocketInit`, `Socket`, `Poll`, `Accept` | Échec du socket / de l'epoll (le serveur passe à `Crashed`) ; `Socket` aussi pour un payload au-delà de la limite d'overflow |
| `SocketClosed` (`None`) | Le pair a fermé la connexion (le statut devient `Down`) |

## Socket {#socket}

La couche socket utilisée par le `Server` et le `Client` : elle ouvre la connexion, découpe le flux en payloads et garde en buffer ce qui n'est pas encore complet. Un seul objet socket gère tous les fd d'un serveur (chaque méthode prend le fd, `-1` = le socket lui-même). Même API sous Unix et Windows (Winsock 2 via [Platform](page:platform)).

| Classe | Description |
|---|---|
| `ISocket` | Interface utilisée par le `Server` / `Client` |
| `ASocket` | Implémentation commune : buffers, découpage en payloads, contrôle de l'overflow |
| `TCPSocket` | Implémentation TCP (`connect`, `listen`, `accept`, `recv`, `send` de [Platform](page:platform), `MSG_NOSIGNAL`) |

### Connexion {#socket-connection}

| Méthode | Description |
|---|---|
| `connect(address)` | Se connecte en tant que client |
| `listen(address)` | Écoute en tant que serveur |
| `accept()` | Accepte un nouveau client, renvoie son fd (serveur seulement) |
| `close()` | Ferme le socket, `connect` / `listen` peuvent être réutilisés |
| `reset()` | Oublie le fd et les buffers **sans fermer** le fd |
| `getFd()` | fd du socket (`-1` = fermé) |

### Payloads {#socket-payloads}

| Méthode | Description |
|---|---|
| `send(s, fd = -1)` | Envoie `s` suivi du séparateur, tout de suite |
| `sendBuffered(s, fd = -1)` / `flush(fd = -1)` | Stocke le payload, envoie ceux qui sont stockés |
| `receive(fd = -1)` | Lit **une fois** (après un événement de poll) dans le buffer interne, n'attend jamais un payload complet, renvoie le nombre d'octets lus |
| `recv(fd = -1)` | Lit jusqu'à ce qu'un payload complet soit disponible et le renvoie (le reste reste dans le buffer) |
| `recvAll(fd = -1)` | Renvoie tous les payloads complets du buffer |
| `empty(fd = -1)` | Aucun payload complet dans le buffer |
| `discard(fd = -1)` | Oublie les buffers d'un fd (client fermé), `-1` = tous les fd |

### Réglages {#socket-settings}

| Méthode | Défaut | Description |
|---|---|---|
| `setPayloadSeparator(char)` / `setPayloadSeparator(std::string)` | `'\n'` | Fin d'un payload (un séparateur vide = mode brut, chaque lecture est un payload) |
| `setChunckSize(size)` | `4096` | Octets lus par appel |
| `setOverflow(size)` | `4048` | Caractères sans séparateur avant une erreur `Socket` (`0` = illimité) |

### Outils {#socket-tools}

| Fonction | Description |
|---|---|
| `is_ip(s)` | `s` est-il une adresse IPv4 |
| `resolve_hostname(hostname)` | IPv4 d'un hostname |
| `resolve_address(address)` | Remplit l'ip de l'`Address` à partir de son hostname (fait par le constructeur du `Client`) |

### Socket personnalisé {#custom-socket}

`TCPSocket` est `final` sur ses méthodes : un nouveau transport dérive d'`ASocket`, implémente `connect` / `listen` et les 3 appels système, et renvoie `true` depuis le `has*Overload()` correspondant pour qu'`ASocket` les utilise (le buffering et le découpage en payloads restent ceux d'`ASocket`).

```cpp
#define _Socket
#include <utils/utils.hpp>

class MySocket: public utils::network::ASocket {
    public:
        void connect(const utils::network::Address& address) override; // set the fd of the socket
        void listen(const utils::network::Address& address) override;

        bool hasAcceptOverload(void) const override {return true;};
        bool hasRecvOverload(void) const override   {return true;};
        bool hasSendOverload(void) const override   {return true;};
        int accept(int fd, sockaddr* addr, socklen_t* len) const override;
        ssize_t recv(int fd, char* buf, std::size_t len) const override;
        ssize_t send(int fd, const char* buf, std::size_t len) const override;
};

utils::network::Server server(std::make_shared<MySocket>(), {{"", ""}, 4242});
```

## En C {#in-c}

| C++ | C |
|---|---|
| `std::make_shared<TCPSocket>()` | `utils_socket_create_tcp()` ; un transport côté C : `utils_socket_create(&callbacks, data)` (`utils_socket_callbacks_t`) |
| `Server(socket, address)` / `Client(socket, address)` | `utils_server_create(socket, &address)` / `utils_client_create(socket, &address)` (socket `NULL` = un nouveau socket TCP, adresse `NULL` = celle par défaut) |
| `Address` | `utils_network_address_t` |
| `server.listen()` | `utils_server_listen(server, messages)` dans un `utils_network_messages_t*` (fd → payloads) |
| `client.listen()` | `utils_client_listen(client, payloads)` dans un `utils_payloads_t*` |
| `send(payload)` / `send<true>` / `flush` | `utils_*_send(..., data, size)` / `utils_*_send_buffered` / `utils_*_flush` (un payload peut contenir `'\0'`) |
| `getStatus()` | `utils_*_get_status` (`C_API_NETWORK_STATUS_*`) |
| `is_ip` / `resolve_hostname` | `utils_network_is_ip` / `utils_network_resolve_hostname` (et `_r`) |

```c
#define _POSIX_C_SOURCE 200809L // strict C on Unix: the POSIX structures (pid_t, struct sockaddr...)
#define _Network
#include <utils.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    utils_client_t* client = utils_client_create(NULL, NULL);     // new TCP socket, localhost:8080
    utils_payloads_t* payloads = utils_payloads_create();
    int ret = 1;

    if (client && payloads && utils_client_start(client) == UTILS_OK
        && utils_client_send(client, "hello", strlen("hello")) == UTILS_OK
        && utils_client_join(client) == UTILS_OK
        && utils_client_listen(client, payloads) == UTILS_OK) {
        for (size_t i = 0; i < utils_payloads_count(payloads); ++i)
            printf("%s\n", utils_payloads_get(payloads, i, NULL));
        ret = 0;
    } else {
        fprintf(stderr, "%s\n", utils_exception_format());
    }
    utils_payloads_destroy(payloads);
    utils_client_destroy(client);
    return ret;
}
```
