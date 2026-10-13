/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 12/10/2026 by @author Tsukini

File Name:
##  @file RawSocket.hpp

File Description:
##  Raw TCP helpers of the tests (utils::platform calls: Unix & Windows)
\**************************************************************/

#ifndef RAWSOCKET_H
    #define RAWSOCKET_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "utils/platform/Socket.hpp"    // utils::platform::socket, utils::platform::connect
    #include "utils/platform/Unistd.hpp"    // utils::platform::close, utils::platform::write
    #ifdef _WIN32
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN     // windows.h (complete) declares a '_unused' member after the libutils attribute macros
        #endif
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #include <winsock2.h>               // sockaddr_in, struct linger, struct pollfd
        #include <ws2tcpip.h>               // inet_pton
    #else
        #include <netinet/in.h>             // sockaddr_in
        #include <arpa/inet.h>              // htons
        #include <poll.h>                   // struct pollfd
    #endif
    #include <cstddef>                      // std::size_t
    #include <cstdint>                      // std::uint16_t

namespace tests::tools { // namespace start
//----------------------------------------------------------------//
/* FUNCTION */

inline int raw_close(int fd) {return utils::platform::close(fd);};
inline long raw_write(int fd, const char* data, std::size_t size) {return static_cast<long>(utils::platform::write(fd, data, size));};

// Port the socket is bound to (0 on error)
inline std::uint16_t local_port(int fd)
{
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (utils::platform::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) < 0) return 0;
    return ntohs(addr.sin_port);
}

// Raw tcp client on 127.0.0.1 (to send partial payloads), -1 on error
inline int raw_connect(std::uint16_t port)
{
    const int fd = utils::platform::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (utils::platform::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {raw_close(fd); return -1;}
    return fd;
}

// Close with a reset of the connection (SO_LINGER 0)
inline void raw_reset(int fd)
{
    struct linger l{1, 0};
    utils::platform::setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
    raw_close(fd);
}

} // namespace end
#endif /* RAWSOCKET_H */
