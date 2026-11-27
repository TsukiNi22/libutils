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
##  @file Socket.hpp

File Description:
##  BSD socket calls for Unix & Windows (same names, same contract: -1 & errno)
##  Unix: the system call, Windows: Winsock 2 (started on the first use, errors translated in errno)
\**************************************************************/

#ifndef PLATFORM_SOCKET_H
    #define PLATFORM_SOCKET_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _restrict, _read_only, _write_only, _null_terminated_string_arg, _fd_arg, _fd_arg_read, _fd_arg_write
    #include "Platform.hpp"                 // UTILS_WINDOWS
    #include "Types.h"                      // struct sockaddr, struct addrinfo, socklen_t, MSG_NOSIGNAL (Windows)
    #include <sys/types.h>                  // ssize_t
    #include <cstddef>                      // std::size_t
    #ifndef UTILS_WINDOWS
        #include <sys/socket.h>             // struct sockaddr, socklen_t, MSG_NOSIGNAL
        #include <netdb.h>                  // struct addrinfo
    #endif

namespace utils::platform { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

int socket(int domain, int type, int protocol);
_fd_arg(1) int connect(int fd, const struct sockaddr* addr, socklen_t len);
_fd_arg(1) int bind(int fd, const struct sockaddr* addr, socklen_t len);
_fd_arg(1) int listen(int fd, int backlog);
_fd_arg(1) int accept(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len);
_fd_arg_read(1) _write_only(2, 3) ssize_t recv(int fd, void* buf, std::size_t len, int flags);
_fd_arg_write(1) _read_only(2, 3) ssize_t send(int fd, const void* buf, std::size_t len, int flags);
_fd_arg(1) _read_only(4, 5) int setsockopt(int fd, int level, int name, const void* value, socklen_t len); // Windows: SO_SNDTIMEO / SO_RCVTIMEO take a timeval, SO_REUSEADDR is the default (no-op)
_fd_arg(1) int getsockname(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len);
_fd_arg(1) int getpeername(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len);
_null_terminated_string_arg(1) _null_terminated_string_arg(2) int getaddrinfo(const char* _restrict node, const char* _restrict service, const struct addrinfo* _restrict hints, struct addrinfo** _restrict res); // EAI_* error code (gai_strerror)
void freeaddrinfo(struct addrinfo* res);
#ifdef UTILS_WINDOWS
    #pragma push_macro("gai_strerror")
    #undef gai_strerror // <ws2tcpip.h> defines it as gai_strerrorA
#endif
const char* gai_strerror(int code);
#ifdef UTILS_WINDOWS
    _cold inline const char* gai_strerrorA(int code) {return utils::platform::gai_strerror(code);}; // call renamed by the <ws2tcpip.h> macro
    #pragma pop_macro("gai_strerror")
#endif

} // namespace end
#endif /* PLATFORM_SOCKET_H */
