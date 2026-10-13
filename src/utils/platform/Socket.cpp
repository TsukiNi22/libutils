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
##  @file Socket.cpp

File Description:
##  Definition of the BSD socket calls of Socket.hpp (Unix: system, Windows: Winsock 2)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/platform/Platform.hpp"
#include "utils/platform/Socket.hpp"
#ifdef UTILS_WINDOWS
    #include "Windows.hpp"
#else
    #include <sys/socket.h>
    #include <netdb.h>
#endif

#ifdef UTILS_WINDOWS
    #define WSA_CALL(call) {const int res = (call); return (res == SOCKET_ERROR) ? utils::platform::windows::socket_error() : res;}
#endif

_cold _nodiscard int utils::platform::socket(int domain, int type, int protocol)
{
    #ifdef UTILS_WINDOWS
        utils::platform::windows::startup();
        const SOCKET fd = ::socket(domain, type, protocol);
        return (fd == INVALID_SOCKET) ? utils::platform::windows::socket_error() : static_cast<int>(fd); // a socket handle fits in an int
    #else
        return ::socket(domain, type, protocol);
    #endif
}

_cold int utils::platform::connect(int fd, const struct sockaddr* addr, socklen_t len)
{
    #ifdef UTILS_WINDOWS
        WSA_CALL(::connect(static_cast<SOCKET>(fd), addr, len))
    #else
        return ::connect(fd, addr, len);
    #endif
}

_cold int utils::platform::bind(int fd, const struct sockaddr* addr, socklen_t len)
{
    #ifdef UTILS_WINDOWS
        WSA_CALL(::bind(static_cast<SOCKET>(fd), addr, len))
    #else
        return ::bind(fd, addr, len);
    #endif
}

_cold int utils::platform::listen(int fd, int backlog)
{
    #ifdef UTILS_WINDOWS
        WSA_CALL(::listen(static_cast<SOCKET>(fd), backlog))
    #else
        return ::listen(fd, backlog);
    #endif
}

_hot _nodiscard int utils::platform::accept(int fd, struct sockaddr* addr, socklen_t* len)
{
    #ifdef UTILS_WINDOWS
        const SOCKET client = ::accept(static_cast<SOCKET>(fd), addr, len);
        return (client == INVALID_SOCKET) ? utils::platform::windows::socket_error() : static_cast<int>(client);
    #else
        return ::accept(fd, addr, len);
    #endif
}

_hot _nodiscard ssize_t utils::platform::recv(int fd, void* buf, std::size_t len, int flags)
{
    #ifdef UTILS_WINDOWS
        WSA_CALL(::recv(static_cast<SOCKET>(fd), static_cast<char*>(buf), static_cast<int>(len), flags))
    #else
        return ::recv(fd, buf, len, flags);
    #endif
}

_hot ssize_t utils::platform::send(int fd, const void* buf, std::size_t len, int flags)
{
    #ifdef UTILS_WINDOWS
        WSA_CALL(::send(static_cast<SOCKET>(fd), static_cast<const char*>(buf), static_cast<int>(len), flags))
    #else
        return ::send(fd, buf, len, flags);
    #endif
}

_cold int utils::platform::setsockopt(int fd, int level, int name, const void* value, socklen_t len)
{
    #ifdef UTILS_WINDOWS
        // Timeouts: timeval on Unix, DWORD milliseconds on Windows
        if (level == SOL_SOCKET && (name == SO_SNDTIMEO || name == SO_RCVTIMEO) && len == sizeof(struct timeval)) {
            const struct timeval* tv = static_cast<const struct timeval*>(value);
            const DWORD ms = static_cast<DWORD>(tv->tv_sec) * 1000 + static_cast<DWORD>(tv->tv_usec) / 1000;
            WSA_CALL(::setsockopt(static_cast<SOCKET>(fd), level, name, reinterpret_cast<const char*>(&ms), sizeof(ms)))
        }

        // Reuse of an address in TIME_WAIT: default on Windows (SO_REUSEADDR would let another process take a used port)
        if (level == SOL_SOCKET && name == SO_REUSEADDR) return 0;
        WSA_CALL(::setsockopt(static_cast<SOCKET>(fd), level, name, static_cast<const char*>(value), len))
    #else
        return ::setsockopt(fd, level, name, value, len);
    #endif
}

_cold int utils::platform::getsockname(int fd, struct sockaddr* addr, socklen_t* len)
{
    #ifdef UTILS_WINDOWS
        WSA_CALL(::getsockname(static_cast<SOCKET>(fd), addr, len))
    #else
        return ::getsockname(fd, addr, len);
    #endif
}

_cold int utils::platform::getpeername(int fd, struct sockaddr* addr, socklen_t* len)
{
    #ifdef UTILS_WINDOWS
        WSA_CALL(::getpeername(static_cast<SOCKET>(fd), addr, len))
    #else
        return ::getpeername(fd, addr, len);
    #endif
}

_cold _nodiscard int utils::platform::getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
{
    #ifdef UTILS_WINDOWS
        utils::platform::windows::startup();
    #endif
    return ::getaddrinfo(node, service, hints, res);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (the header does not include <netdb.h> / <ws2tcpip.h>)
_cold void utils::platform::freeaddrinfo(struct addrinfo* res)
{
    ::freeaddrinfo(res);
}

#ifdef UTILS_WINDOWS
    #undef gai_strerror // <ws2tcpip.h> macro (gai_strerrorA)
#endif
_cold _nodiscard const char* utils::platform::gai_strerror(int code)
{
    #ifdef UTILS_WINDOWS
        return ::gai_strerrorA(code);
    #else
        return ::gai_strerror(code);
    #endif
}
