+++
title = "Network"
summary = "A TCP Server and Client exchanging payloads through a socket object; the socket layer that splits the stream into payloads, and how to write your own transport."
keywords = "network tcp server client socket payload listen join send flush status ISocket ASocket TCPSocket is_ip resolve_hostname utils_server utils_client utils_socket"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Network` | `_Handling` | `utils::network` | `utils/network/` |
| `_Socket` (from `_Network`) | `_Network` | `utils::network` | `utils/network/socket/` |

A TCP `Server` and `Client` built on a socket object. The messages (**payloads**) are strings separated by `'\n'` by default; `listen` reads what is available without blocking, `join` waits for the next event. Same API on Unix and Windows (the epoll is emulated over `WSAPoll` on Windows, see [Platform](page:platform#windows-behavior)). The layers and the EETP protocol are explained in [Communication](page:network-ipc).

## Types {#types}

| Type | Definition |
|---|---|
| `Payload` | `std::string` (one message, without its separator) |
| `Payloads` | `std::vector<Payload>` |
| `Ip` | `std::pair<std::string, std::string>`: `<ipv4 or hostname, hostname>` |
| `Address` | `{Ip ip = {"localhost", ""}, std::uint16_t port = 8080}` (the ip is ignored by a server, port `0` = any free port) |
| `Status` | `Up`, `Down` (stopped, can be restarted), `Terminated` (killed), `Crashed` |

| Define | Value |
|---|---|
| `DEFAULT_IP` / `DEFAULT_PORT` | `"localhost"` / `8080` |
| `OVERFLOW_LIMIT` | `4048` characters without a separator before an error |
| `SOCKET_CHUNK_SIZE` | `4096` bytes read per call |
| `SOCKET_TIMEOUT` | `7` seconds |

## Server {#server}

`Server(std::shared_ptr<ISocket> socket, Address address = {})`: every client is identified by its fd.

| Method | Description |
|---|---|
| `start()` | Listen on the address (restart after a `stop`) |
| `stop()` / `kill()` | Close every connection; after `kill` the server can't be restarted |
| `listen()` | Accept the new clients, read what is available, return `fd → Payloads` (cleared at the next call) |
| `listen(fd)` | Same for one client (`UnknownFd` if not connected) |
| `join(fd = -1)` | Wait for the next event (any client, or one fd) |
| `send<buffered = false>(fd, payload)` | Send now, or store it until `flush` |
| `flush()` / `flush(fd)` | Send the stored payloads |
| `getFds()` | Connected clients |
| `getStatus()` | `Status` |

## Client {#client}

`Client(std::shared_ptr<ISocket> socket, Address address = {})`: the hostname of the address is resolved by the constructor.

| Method | Description |
|---|---|
| `start()` / `stop()` / `kill()` | Connect / disconnect / disconnect for good |
| `listen()` | Read what is available, return the `Payloads` (cleared at the next call) |
| `join()` | Wait for the next event |
| `send<buffered = false>(payload)` / `flush()` | Send now or later |
| `getStatus()` | `Status` (`Down` when the server closed the connection) |

## Threads {#threads}

`stop()` and `kill()` can be called from another thread while a `listen` / `join` runs: the `join` returns, the epoll is closed when the last `join` leaves. Sending to a closed peer never raises `SIGPIPE`.

## Example {#example}

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

## Errors {#errors}

| Code | When |
|---|---|
| `AlreadyRunning`, `Killed` (warnings) | `start` while up / after a `kill` |
| `UnknownFd` | `listen(fd)` / `send(fd)` / `join(fd)` on an unknown client |
| `SocketInit`, `Socket`, `Poll`, `Accept` | Socket / epoll failure (the server is `Crashed`); `Socket` also for a payload over the overflow limit |
| `SocketClosed` (`None`) | The peer closed the connection (the status becomes `Down`) |

## Socket {#socket}

The socket layer used by the `Server` and the `Client`: it opens the connection, splits the stream into payloads and buffers what is not complete yet. One socket object handles every fd of a server (each method takes the fd, `-1` = the socket itself). Same API on Unix and Windows (Winsock 2 through [Platform](page:platform)).

| Class | Description |
|---|---|
| `ISocket` | Interface used by the `Server` / `Client` |
| `ASocket` | Common implementation: buffers, payload splitting, overflow check |
| `TCPSocket` | TCP implementation (`connect`, `listen`, `accept`, `recv`, `send` of [Platform](page:platform), `MSG_NOSIGNAL`) |

### Connection {#socket-connection}

| Method | Description |
|---|---|
| `connect(address)` | Connect as a client |
| `listen(address)` | Listen as a server |
| `accept()` | Accept a new client, return its fd (server only) |
| `close()` | Close the socket, `connect` / `listen` can be used again |
| `reset()` | Forget the fd and the buffers **without closing** the fd |
| `getFd()` | fd of the socket (`-1` = closed) |

### Payloads {#socket-payloads}

| Method | Description |
|---|---|
| `send(s, fd = -1)` | Send `s` followed by the separator now |
| `sendBuffered(s, fd = -1)` / `flush(fd = -1)` | Store the payload, send the stored ones |
| `receive(fd = -1)` | Read **once** (after a poll event) into the internal buffer, never waits for a full payload, returns the number of bytes read |
| `recv(fd = -1)` | Read until one full payload is available and return it (the rest stays in the buffer) |
| `recvAll(fd = -1)` | Return every full payload of the buffer |
| `empty(fd = -1)` | No full payload in the buffer |
| `discard(fd = -1)` | Forget the buffers of a fd (closed client), `-1` = every fd |

### Settings {#socket-settings}

| Method | Default | Description |
|---|---|---|
| `setPayloadSeparator(char)` / `setPayloadSeparator(std::string)` | `'\n'` | End of a payload (an empty separator = raw mode, every read is a payload) |
| `setChunckSize(size)` | `4096` | Bytes read per call |
| `setOverflow(size)` | `4048` | Characters without a separator before a `Socket` error (`0` = unlimited) |

### Tools {#socket-tools}

| Function | Description |
|---|---|
| `is_ip(s)` | Is `s` an IPv4 address |
| `resolve_hostname(hostname)` | IPv4 of a hostname |
| `resolve_address(address)` | Fill the ip of the `Address` from its hostname (done by the `Client` constructor) |

### Custom socket {#custom-socket}

`TCPSocket` is `final` on its methods: a new transport derives `ASocket`, implements `connect` / `listen` and the 3 system calls, and returns `true` from the matching `has*Overload()` so that `ASocket` uses them (the buffering and payload splitting stay the ones of `ASocket`).

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

## In C {#in-c}

| C++ | C |
|---|---|
| `std::make_shared<TCPSocket>()` | `utils_socket_create_tcp()`; a transport of the C side: `utils_socket_create(&callbacks, data)` (`utils_socket_callbacks_t`) |
| `Server(socket, address)` / `Client(socket, address)` | `utils_server_create(socket, &address)` / `utils_client_create(socket, &address)` (socket `NULL` = a new TCP socket, address `NULL` = the default one) |
| `Address` | `utils_network_address_t` |
| `server.listen()` | `utils_server_listen(server, messages)` into a `utils_network_messages_t*` (fd → payloads) |
| `client.listen()` | `utils_client_listen(client, payloads)` into a `utils_payloads_t*` |
| `send(payload)` / `send<true>` / `flush` | `utils_*_send(..., data, size)` / `utils_*_send_buffered` / `utils_*_flush` (a payload can hold `'\0'`) |
| `getStatus()` | `utils_*_get_status` (`C_API_NETWORK_STATUS_*`) |
| `is_ip` / `resolve_hostname` | `utils_network_is_ip` / `utils_network_resolve_hostname` (and `_r`) |

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
