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
    #include "Platform.hpp"             // UTILS_WINDOWS
    #include <sys/types.h>              // ssize_t
    #include <cstddef>                  // std::size_t
    #ifndef UTILS_WINDOWS
        #include <sys/socket.h>         // struct sockaddr, socklen_t, MSG_NOSIGNAL
        #include <netdb.h>              // struct addrinfo
    #endif

    //----------------------------------------------------------------//
    /* DEFINE */

    #ifdef UTILS_WINDOWS
        #ifndef MSG_NOSIGNAL
            #define MSG_NOSIGNAL 0      // no SIGPIPE on Windows
        #endif
    #endif

//----------------------------------------------------------------//
/* STRUCT */

#ifdef UTILS_WINDOWS
    // <winsock2.h> / <ws2tcpip.h> are not included (no windows.h macros in the public headers):
    // include them in the code that needs the structures (sockaddr_in, addrinfo...)
    struct sockaddr;
    struct addrinfo;
#endif

//----------------------------------------------------------------//
/* TYPEDEF */

#ifdef UTILS_WINDOWS
    typedef int socklen_t; // <ws2tcpip.h>
#endif

namespace utils::platform { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

int socket(int domain, int type, int protocol);
int connect(int fd, const struct sockaddr* addr, socklen_t len);
int bind(int fd, const struct sockaddr* addr, socklen_t len);
int listen(int fd, int backlog);
int accept(int fd, struct sockaddr* addr, socklen_t* len);
ssize_t recv(int fd, void* buf, std::size_t len, int flags);
ssize_t send(int fd, const void* buf, std::size_t len, int flags);
int setsockopt(int fd, int level, int name, const void* value, socklen_t len); // Windows: SO_SNDTIMEO / SO_RCVTIMEO take a timeval, SO_REUSEADDR is the default (no-op)
int getsockname(int fd, struct sockaddr* addr, socklen_t* len);
int getpeername(int fd, struct sockaddr* addr, socklen_t* len);
int getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res); // EAI_* error code (gai_strerror)
void freeaddrinfo(struct addrinfo* res);
#ifdef UTILS_WINDOWS
    #pragma push_macro("gai_strerror")
    #undef gai_strerror // <ws2tcpip.h> defines it as gai_strerrorA
#endif
const char* gai_strerror(int code);
#ifdef UTILS_WINDOWS
    inline const char* gai_strerrorA(int code) {return utils::platform::gai_strerror(code);} // call renamed by the <ws2tcpip.h> macro
    #pragma pop_macro("gai_strerror")
#endif

} // namespace end
#endif /* PLATFORM_SOCKET_H */
