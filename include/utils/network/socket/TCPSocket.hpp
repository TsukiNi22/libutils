/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 15/08/2026 by @author Tsukini

File Name:
##  @file TCPSocket.hpp

File Description:
##  Socket that handle tcp communication
\**************************************************************/

#ifndef TCPSOCKET_H
    #define TCPSOCKET_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _hot, _nodiscard
    #include "../NetworkType.hpp"               // utils::network::Address
    #include "ASocket.hpp"                      // utils::network::ASocket
    #include "../../platform/Socket.hpp"       // utils::platform::accept, utils::platform::recv, utils::platform::send, MSG_NOSIGNAL
    #include <unordered_map>                    // std::unordered_map
    #include <cstddef>                          // std::size_t
    #include <vector>                           // std::vector
    #include <string>                           // std::string, std::to_string

namespace utils::network { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class TCPSocket: public utils::network::ASocket {
    public:
        // ---------- Pre-Function -------- //
        // Can only be call one time, otherwise throw
        void connect(const utils::network::Address& address) final; // build a connection as a client
        void listen(const utils::network::Address& address) final; // build a connection as a server

        // ------------ Function ---------- //
        /* getter */
        _hot _nodiscard bool hasAcceptOverload(void) const final {return true;};
        _hot _nodiscard bool hasRecvOverload(void) const final   {return true;};
        _hot _nodiscard bool hasSendOverload(void) const final   {return true;};

        /* raw (the high level overloads of ASocket stay visible) */
        using utils::network::ASocket::accept;
        using utils::network::ASocket::recv;
        using utils::network::ASocket::send;
        _hot _nodiscard int accept(int fd, sockaddr* addr, socklen_t* len) const final
        {return utils::platform::accept(fd, addr, len);};
        _hot _nodiscard ssize_t recv(int fd, char* buf, std::size_t len) const final
        {return utils::platform::recv(fd, buf, len, 0);};
        _hot _nodiscard ssize_t send(int fd, const char* buf, std::size_t len) const final
        {return utils::platform::send(fd, buf, len, MSG_NOSIGNAL);}; // no SIGPIPE on a closed peer (EPIPE error)

        // ------------ Operator ---------- //
        TCPSocket& operator=(const TCPSocket& other) = delete;
        TCPSocket& operator=(TCPSocket&& other) = delete;

        // ---------- Constructor --------- //
        TCPSocket() = default;
        TCPSocket(const TCPSocket& other) = delete;
        TCPSocket(TCPSocket&& other) = delete;

        // ----------- Destructor --------- //
        ~TCPSocket() = default;
};

} // namespace end

#endif /* TCPSOCKET_H */
