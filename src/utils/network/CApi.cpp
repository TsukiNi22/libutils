/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.cpp

File Description:
##  C API of utils::network (sockets, Client, Server)
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/network/socket/TCPSocket.hpp"
#include "utils/network/socket/ASocket.hpp"
#include "utils/network/NetworkDefine.hpp"
#include "utils/network/NetworkType.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/manip/smanip/CApi.hpp"
#include "utils/platform/Socket.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/network/CApi.hpp"
#include "utils/network/CApi.h"
#include <unordered_map>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ status must stay the same */
static_assert(C_API_NETWORK_STATUS_UP         == static_cast<int>(utils::network::Status::Up));
static_assert(C_API_NETWORK_STATUS_DOWN       == static_cast<int>(utils::network::Status::Down));
static_assert(C_API_NETWORK_STATUS_TERMINATED == static_cast<int>(utils::network::Status::Terminated));
static_assert(C_API_NETWORK_STATUS_CRASHED    == static_cast<int>(utils::network::Status::Crashed));
static_assert(C_API_NETWORK_DEFAULT_PORT == DEFAULT_PORT);

//----------------------------------------------------------------//
/* CLASS */

// Socket of the C side: connect / listen given by the C side, raw accept / recv / send of the C side or of TCP
class CSocket final: public utils::network::ASocket {
    private:
        utils_socket_callbacks_t _callbacks = {};
        void* _data = nullptr;

        // ------------ Function ---------- //
        _cold void open_(int (*fn)(const utils_network_address_t*, void*), const utils::network::Address& address, const bool mode)
        {
            const utils_network_address_t given = {address.ip.first.c_str(), address.ip.second.c_str(), address.port};
            int fd = -1;

            if (this->_fd != -1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, "Can't open the socket, already open: " + std::to_string(this->_fd));
            }
            utils::exception::c_api::reset_status(); // only the error of this callback
            if ((fd = fn(&given, this->_data)) < 0) _unlikely {utils::exception::c_api::throw_callback_error();}
            this->_fd = fd;
            this->_mode = mode;
        };

    public:
        // ------------ Function ---------- //
        _cold void connect(const utils::network::Address& address) final {this->open_(this->_callbacks.connect, address, false);};
        _cold void listen(const utils::network::Address& address) final  {this->open_(this->_callbacks.listen, address, true);};

        /* getter */
        _hot _nodiscard bool hasAcceptOverload(void) const final {return true;};
        _hot _nodiscard bool hasRecvOverload(void) const final   {return true;};
        _hot _nodiscard bool hasSendOverload(void) const final   {return true;};

        /* raw (the high level overloads of ASocket stay visible) */
        using utils::network::ASocket::accept;
        using utils::network::ASocket::recv;
        using utils::network::ASocket::send;
        _hot _nodiscard int accept(int fd, sockaddr* addr, socklen_t* len) const final
        {return (this->_callbacks.accept ? this->_callbacks.accept(fd, addr, len, this->_data) : utils::platform::accept(fd, addr, len));};
        _hot _nodiscard ssize_t recv(int fd, char* buf, std::size_t len) const final
        {return (this->_callbacks.recv ? this->_callbacks.recv(fd, buf, len, this->_data) : utils::platform::recv(fd, buf, len, 0));};
        _hot _nodiscard ssize_t send(int fd, const char* buf, std::size_t len) const final
        {return (this->_callbacks.send ? this->_callbacks.send(fd, buf, len, this->_data) : utils::platform::send(fd, buf, len, MSG_NOSIGNAL));};

        // ---------- Constructor --------- //
        CSocket(const utils_socket_callbacks_t& callbacks, void* data): _callbacks{callbacks}, _data{data} { /* Nothing */ };
};

//----------------------------------------------------------------//
/* TOOLS */

// Address of the C side (NULL: the default one)
_cold _nodiscard static utils::network::Address address_of(const utils_network_address_t* address)
{
    if (!address) return {};
    return {{(address->ip ? address->ip : DEFAULT_IP), (address->hostname ? address->hostname : "")}, address->port};
}

// Socket of a handle (NULL: a new TCP socket)
_cold _nodiscard static std::shared_ptr<utils::network::ISocket> socket_of(const utils_socket_t* socket)
{
    if (!socket) return std::make_shared<utils::network::TCPSocket>();
    return socket->socket;
}

_hot _nodiscard static utils::network::ISocket& sock(const utils_socket_t* socket) {return *utils::exception::c_api::not_null_ref(socket, "socket").socket;};

// Payload of the C side (NULL only with a size of 0)
_hot _nodiscard static std::string payload_of(const char* data, const std::size_t size)
{
    if (size == 0) return "";
    return std::string(utils::exception::c_api::not_null(data, "payload"), size);
}

//----------------------------------------------------------------//
/* TOOLS (network) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_network_is_ip(const char* str)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::network::is_ip(utils::exception::c_api::not_null(str, "string"));});
}

_cold _nothrow const char* utils_network_resolve_hostname(const char* hostname)
{
    // xstyle: ignore-next CPP-THIS (false positive: _thread_local is the attribute macro, not a member)
    static _thread_local std::string ip;

    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {ip = utils::network::resolve_hostname(utils::exception::c_api::not_null(hostname, "hostname")); return ip.c_str();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow size_t utils_network_resolve_hostname_r(const char* hostname, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::network::resolve_hostname(utils::exception::c_api::not_null(hostname, "hostname")), buf, size);});
}

//----------------------------------------------------------------//
/* PAYLOADS */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_payloads_t* utils_payloads_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_payloads_t* {return new utils_payloads_t();});
}

_cold _nothrow void utils_payloads_destroy(utils_payloads_t* payloads)
{
    (void)utils::exception::c_api::guard([&](void) {delete payloads;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_payloads_count(const utils_payloads_t* payloads)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(payloads, "payloads").payloads.size();});
}

_hot _nothrow const char* utils_payloads_get(const utils_payloads_t* payloads, size_t i, size_t* size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {
        const std::string& payload = utils::exception::c_api::not_null_ref(payloads, "payloads").payloads.at(i);

        if (size) *size = payload.size();
        return payload.c_str();
    });
}

//----------------------------------------------------------------//
/* MESSAGES */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_network_messages_t* utils_network_messages_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_network_messages_t* {return new utils_network_messages_t();});
}

_cold _nothrow void utils_network_messages_destroy(utils_network_messages_t* messages)
{
    (void)utils::exception::c_api::guard([&](void) {delete messages;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_network_messages_count(const utils_network_messages_t* messages)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(messages, "messages").messages.size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_network_messages_fd(const utils_network_messages_t* messages, size_t i)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return utils::exception::c_api::not_null_ref(messages, "messages").messages.at(i).first;});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_network_messages_size(const utils_network_messages_t* messages, size_t i)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(messages, "messages").messages.at(i).second.size();});
}

_hot _nothrow const char* utils_network_messages_get(const utils_network_messages_t* messages, size_t i, size_t j, size_t* size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {
        const std::string& payload = utils::exception::c_api::not_null_ref(messages, "messages").messages.at(i).second.at(j);

        if (size) *size = payload.size();
        return payload.c_str();
    });
}

//----------------------------------------------------------------//
/* SOCKET */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_socket_t* utils_socket_create_tcp(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_socket_t* {return new utils_socket_t(std::make_shared<utils::network::TCPSocket>());});
}

_cold _nothrow utils_socket_t* utils_socket_create(const utils_socket_callbacks_t* callbacks, void* data)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_socket_t* {
        const utils_socket_callbacks_t& given = utils::exception::c_api::not_null_ref(callbacks, "callbacks");

        (void)utils::exception::c_api::not_null(given.connect, "connect callback");
        (void)utils::exception::c_api::not_null(given.listen, "listen callback");
        return new utils_socket_t(std::make_shared<CSocket>(given, data));
    });
}

_cold _nothrow void utils_socket_destroy(utils_socket_t* socket)
{
    (void)utils::exception::c_api::guard([&](void) {delete socket;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_set_payload_separator(utils_socket_t* socket, const char* separator)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).setPayloadSeparator(std::string(utils::exception::c_api::not_null(separator, "separator")));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_set_chunk_size(utils_socket_t* socket, size_t size)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).setChunckSize(size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_set_overflow(utils_socket_t* socket, size_t overflow)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).setOverflow(overflow);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_get_fd(const utils_socket_t* socket)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return sock(socket).getFd();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_socket_has_accept_overload(const utils_socket_t* socket)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return sock(socket).hasAcceptOverload();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_socket_has_recv_overload(const utils_socket_t* socket)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return sock(socket).hasRecvOverload();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_socket_has_send_overload(const utils_socket_t* socket)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return sock(socket).hasSendOverload();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_connect(utils_socket_t* socket, const utils_network_address_t* address)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).connect(address_of(address));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_listen(utils_socket_t* socket, const utils_network_address_t* address)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).listen(address_of(address));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_reset(utils_socket_t* socket)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).reset();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_close(utils_socket_t* socket)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).close();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_accept(utils_socket_t* socket)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return sock(socket).accept();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_socket_accept_raw(const utils_socket_t* socket, int fd, struct sockaddr* addr, socklen_t* len)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return sock(socket).accept(fd, addr, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow ssize_t utils_socket_recv_raw(const utils_socket_t* socket, int fd, char* buf, size_t len)
{
    return utils::exception::c_api::guard_fallback(static_cast<ssize_t>(-1), [&](void) -> ssize_t {return sock(socket).recv(fd, buf, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow ssize_t utils_socket_send_raw(const utils_socket_t* socket, int fd, const char* buf, size_t len)
{
    return utils::exception::c_api::guard_fallback(static_cast<ssize_t>(-1), [&](void) -> ssize_t {return sock(socket).send(fd, buf, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow bool utils_socket_empty(const utils_socket_t* socket, int fd)
{
    return utils::exception::c_api::guard_fallback(true, [&](void) -> bool {return sock(socket).empty(fd);});
}

_hot _nothrow int utils_socket_recv(utils_socket_t* socket, int fd, utils_string_t* payload)
{
    return utils::exception::c_api::guard([&](void) {
        utils_string_t& out = utils::exception::c_api::not_null_ref(payload, "payload");

        out.str = sock(socket).recv(fd);
    });
}

_hot _nothrow int utils_socket_recv_all(utils_socket_t* socket, int fd, utils_payloads_t* payloads)
{
    return utils::exception::c_api::guard([&](void) {
        utils_payloads_t& out = utils::exception::c_api::not_null_ref(payloads, "payloads");

        out.payloads = sock(socket).recvAll(fd);
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_socket_flush(utils_socket_t* socket, int fd)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).flush(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_socket_receive(utils_socket_t* socket, int fd)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return sock(socket).receive(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_socket_discard(utils_socket_t* socket, int fd)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).discard(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_socket_send(utils_socket_t* socket, const char* data, size_t size, int fd)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).send(payload_of(data, size), fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_socket_send_buffered(utils_socket_t* socket, const char* data, size_t size, int fd)
{
    return utils::exception::c_api::guard([&](void) {sock(socket).sendBuffered(payload_of(data, size), fd);});
}

//----------------------------------------------------------------//
/* CLIENT */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_client_t* utils_client_create(const utils_socket_t* socket, const utils_network_address_t* address)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_client_t* {return new utils_client_t(socket_of(socket), address_of(address));});
}

_cold _nothrow void utils_client_destroy(utils_client_t* client)
{
    (void)utils::exception::c_api::guard([&](void) {delete client;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_client_start(utils_client_t* client)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(client, "client").start();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_client_stop(utils_client_t* client)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(client, "client").stop();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_client_kill(utils_client_t* client)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(client, "client").kill();});
}

_hot _nothrow int utils_client_listen(utils_client_t* client, utils_payloads_t* payloads)
{
    return utils::exception::c_api::guard([&](void) {
        utils_payloads_t& out = utils::exception::c_api::not_null_ref(payloads, "payloads");

        out.payloads = utils::exception::c_api::not_null_ref(client, "client").listen();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_client_join(utils_client_t* client)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(client, "client").join();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_client_flush(utils_client_t* client)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(client, "client").flush();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_client_send(utils_client_t* client, const char* payload, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(client, "client").send<false>(payload_of(payload, size));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_client_send_buffered(utils_client_t* client, const char* payload, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(client, "client").send<true>(payload_of(payload, size));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_network_status_t utils_client_get_status(const utils_client_t* client)
{
    return utils::exception::c_api::guard_fallback(C_API_NETWORK_STATUS_CRASHED, [&](void) -> utils_network_status_t {return static_cast<utils_network_status_t>(utils::exception::c_api::not_null_ref(client, "client").getStatus());});
}

//----------------------------------------------------------------//
/* SERVER */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_server_t* utils_server_create(const utils_socket_t* socket, const utils_network_address_t* address)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_server_t* {return new utils_server_t(socket_of(socket), address_of(address));});
}

_cold _nothrow void utils_server_destroy(utils_server_t* server)
{
    (void)utils::exception::c_api::guard([&](void) {delete server;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_server_start(utils_server_t* server)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(server, "server").start();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_server_stop(utils_server_t* server)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(server, "server").stop();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow int utils_server_kill(utils_server_t* server)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(server, "server").kill();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_server_join(utils_server_t* server, int fd)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(server, "server").join(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_server_flush(utils_server_t* server)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(server, "server").flush();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_server_flush_fd(utils_server_t* server, int fd)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(server, "server").flush(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_server_send(utils_server_t* server, int fd, const char* payload, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(server, "server").send<false>(fd, payload_of(payload, size));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_server_send_buffered(utils_server_t* server, int fd, const char* payload, size_t size)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(server, "server").send<true>(fd, payload_of(payload, size));});
}

_hot _nothrow int utils_server_listen(utils_server_t* server, utils_network_messages_t* messages)
{
    return utils::exception::c_api::guard([&](void) {
        utils_network_messages_t& out = utils::exception::c_api::not_null_ref(messages, "messages");
        const std::unordered_map<int, utils::network::Payloads>& read = utils::exception::c_api::not_null_ref(server, "server").listen();

        out.messages.assign(read.begin(), read.end());
    });
}

_hot _nothrow int utils_server_listen_fd(utils_server_t* server, int fd, utils_payloads_t* payloads)
{
    return utils::exception::c_api::guard([&](void) {
        utils_payloads_t& out = utils::exception::c_api::not_null_ref(payloads, "payloads");

        out.payloads = utils::exception::c_api::not_null_ref(server, "server").listen(fd);
    });
}

_cold _nothrow size_t utils_server_get_fds(const utils_server_t* server, int* fds, size_t max)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const std::vector<int> list = utils::exception::c_api::not_null_ref(server, "server").getFds();

        if (max > 0) std::copy_n(list.begin(), std::min(max, list.size()), utils::exception::c_api::not_null(fds, "fds"));
        return list.size();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow utils_network_status_t utils_server_get_status(const utils_server_t* server)
{
    return utils::exception::c_api::guard_fallback(C_API_NETWORK_STATUS_CRASHED, [&](void) -> utils_network_status_t {return static_cast<utils_network_status_t>(utils::exception::c_api::not_null_ref(server, "server").getStatus());});
}
