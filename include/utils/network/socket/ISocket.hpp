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
##  @file ISocket.hpp

File Description:
##  Interface for socket handling
\**************************************************************/

#ifndef ISOCKET_H
    #define ISOCKET_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../NetworkType.hpp"   // utils::network::Address
    #include <sys/socket.h>         // socklen_t
    #include <sys/types.h>          // ssize_t
    #include <cstddef>              // std::size_t
    #include <vector>               // std::vector
    #include <string>               // std::string

namespace utils::network::socket { // namespace start
//----------------------------------------------------------------//
/* CLASS */

// Any fd that equal to -1 is an undefined fd
class ISocket {
    public:
        // ---------- Pre-Function -------- //
        /* setter */
        virtual void setPayloadSeparator(char c) = 0; // default: '\n'
        virtual void setPayloadSeparator(std::string s) = 0;
        virtual void setChunckSize(std::size_t size) = 0; // default: 4096
        virtual void setOverflow(std::size_t overflow) = 0; // size without a valid payload before throw, default: 4096 (0 = unlimited)

        /* getter */
        virtual int getFd(void) const = 0; // fd of the socket
        virtual bool hasAcceptOverload(void) const = 0;
        virtual bool hasRecvOverload(void) const = 0;
        virtual bool hasSendOverload(void) const = 0;

        // Can only be call one time, otherwise throw
        virtual void connect(const utils::network::Address& address) = 0; // build a connection as a client
        virtual void listen(const utils::network::Address& address) = 0; // build a connection as a server

        virtual void reset(void) = 0; // reset fd (DOES NOT CLOSE!!!)
        virtual void close(void) = 0; // reallow the use of connect/listen

        // Only in server mode, otherwise throw
        virtual int accept(void); // accept a new connection (only server mode), fd is used as an id in server

        /* raw */
        virtual int accept(int fd, sockaddr* addr, socklen_t* len) const = 0;
        virtual ssize_t recv(int fd, char* buf, std::size_t len) const = 0;
        virtual ssize_t send(int fd, const char* buf, std::size_t len) const = 0;

        // fd = -1 -> redirect on self (default value)
        virtual bool empty(int fd) const = 0; // is there no valid payload in the stored buffer ?
        virtual std::string recv(int fd) = 0; // (default) read by chunck of 4096, store the payload overflow into a buffer
        virtual std::vector<std::string> recvAll(int fd) = 0; // read by chunck of 4096, return all valid payloads, store the overflow into a buffer
        virtual void send(const std::string& s, int fd) = 0; // (default) send it now
        virtual void sendBuffered(const std::string& s, int fd) = 0; // store in a buffer

        // ------------ Operator ---------- //
        ISocket& operator=(const ISocket& other) = delete;
        ISocket& operator=(ISocket&& other) = delete;

        // ---------- Constructor --------- //
        ISocket() = default;
        ISocket(const ISocket& other) = delete;
        ISocket(ISocket&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~ISocket() = default;
};

} // namespace end
#endif /* ISOCKET_H */
