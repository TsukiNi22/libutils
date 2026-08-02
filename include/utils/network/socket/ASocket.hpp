/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 02/08/2026 by @author Tsukini

File Name:
##  @file ASocket.hpp

File Description:
##  Abstract for socket handling
\**************************************************************/

#ifndef ASOCKET_H
    #define ASOCKET_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"                // _cold, _hot, _nodiscard
    #include "../../exception/ExceptionDefine.hpp"          // utils::exception::InternalCode
    #include "../../exception/custom/FatalException.hpp"    // utils::exception::FatalException
    #include "../NetworkDefine.hpp"                         // SOCKET_CHUNK_SIZE, OVERFLOW_LIMIT
    #include "../NetworkType.hpp"                           // utils::network::Address
    #include "ISocket.hpp"                                  // utils::network::socket::ISocket
    #include <sys/socket.h>                                 // socklen_t
    #include <unordered_map>                                // std::unordered_map
    #include <cstddef>                                      // std::size_t
    #include <vector>                                       // std::vector
    #include <string>                                       // std::string, std::to_string

namespace utils::network::socket { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* tools */
bool isIp(const std::string& s);
std::string resolveHostname(const std::string& hostname);
void resolveAddress(utils::network::Address& address);

//----------------------------------------------------------------//
/* CLASS */

// Any fd that equal to -1 is an undefined fd
class ASocket: public utils::network::socket::ISocket {
    protected:
        /* connection */
        bool _mode = false; // true: server | false: client
        int _fd = -1; // -1 = closed, can exec connect or listen
        std::unordered_map<int, std::string> _buffersRecv;
        std::unordered_map<int, std::string> _buffersSend;

        /* limits */
        std::string _separator = "\n";
        std::size_t _chunk = SOCKET_CHUNK_SIZE;
        std::size_t _overflow = OVERFLOW_LIMIT;

    public:
        // ------------ Function ---------- //
        _cold void reset(void) final {this->_fd = -1;}; // reset fd (DOES NOT CLOSE!!!)

        /* setter */
        _cold void setPayloadSeparator(char c = '\n') final {this->_separator = std::to_string(c);}; // default: '\n'
        _cold void setPayloadSeparator(std::string s = "\n") final {this->_separator = s;};
        _cold void setChunckSize(std::size_t size = SOCKET_CHUNK_SIZE) final {this->_chunk = size;}; // default: 4096
        _cold void setOverflow(std::size_t overflow = OVERFLOW_LIMIT) final {this->_overflow = overflow;}; // size without a valid payload before throw, default: 4096 (0 = unlimited)

        /* getter */
        _cold _nodiscard int getFd(void) const final {return this->_fd;}; // fd of the socket
        _hot _nodiscard bool hasAcceptOverload(void) const override {return false;};
        _hot _nodiscard bool hasRecvOverload(void) const override   {return false;};
        _hot _nodiscard bool hasSendOverload(void) const override   {return false;};

        /* raw */
        _hot _nodiscard int accept(_unused int fd, _unused sockaddr* addr, _unused socklen_t* len) const override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        _hot _nodiscard ssize_t recv(_unused int fd, _unused char* buf, _unused std::size_t len) const override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        _hot _nodiscard ssize_t send(_unused int fd, _unused const char* buf, _unused std::size_t len) const override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};

        // ---------- Pre-Function -------- //
        // Only in server mode, otherwise throw
        int accept(void) final; // accept a new connection (only server mode), fd is used as an id in server

        void close(void) final; // reallow the use of connect/listen

        // fd = -1 -> redirect on self (default value)
        bool empty(int fd = -1) const final; // is there no valid payload in the stored buffer ?
        std::string recv(int fd) final; // (default) read by chunck of 4096, store the payload overflow into a buffer
        std::vector<std::string> recvAll(int fd) final; // read by chunck of 4096, return all valid payloads, store the overflow into a buffer
        void send(const std::string& s, int fd) final; // (default) send it now
        void sendBuffered(const std::string& s, int fd) final; // store in a buffer

        // ------------ Operator ---------- //
        ASocket& operator=(const ASocket& other) = delete;
        ASocket& operator=(ASocket&& other) = delete;

        // ---------- Constructor --------- //
        ASocket() = default;
        ASocket(const ASocket& other) = delete;
        ASocket(ASocket&& other) = delete;

        // ----------- Destructor --------- //
        ~ASocket() = default;
};

} // namespace end
#endif /* ASOCKET_H */
