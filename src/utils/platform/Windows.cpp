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
##  @file Windows.cpp

File Description:
##  Definition of the internal Win32 helpers (only compiled on Windows)
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/attribute/Attribute.hpp"
#include "Windows.hpp"
#include <cerrno>
#include <string>

// Winsock life time: started on the first use, cleaned at the exit
class WinsockSession {
    public:
        WinsockSession()
        {
            WSADATA data{};
            const int status = ::WSAStartup(MAKEWORD(2, 2), &data);
            if (status != 0) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::SocketInit, "WSAStartup failed: " + utils::platform::windows::strerror(static_cast<DWORD>(status)));
            }
        };
        ~WinsockSession() {::WSACleanup();};
};

_cold _nodiscard std::string utils::platform::windows::strerror(DWORD code)
{
    constexpr DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    char* buffer = nullptr;

    // English like the strerror of Unix (C locale), the language of the system otherwise
    DWORD size = ::FormatMessageA(flags, nullptr, code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
    if (size == 0) size = ::FormatMessageA(flags, nullptr, code, 0, reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
    std::string message = (size && buffer) ? std::string(buffer, size) : "Windows error " + std::to_string(code);
    if (buffer) ::LocalFree(buffer);
    while (!message.empty() && (message.back() == '\n' || message.back() == '\r' || message.back() == ' ')) message.pop_back();
    return message;
}

_cold void utils::platform::windows::startup(void)
{
    static const WinsockSession session; // thread safe initialization (retried on the next call if it throws)
    (void)session;
}

_hot _nodiscard int utils::platform::windows::socket_errno(int code)
{
    switch (code) {
        case WSAEINTR:          return EINTR;
        case WSAEBADF:          return EBADF;
        case WSAEACCES:         return EACCES;
        case WSAEFAULT:         return EFAULT;
        case WSAEINVAL:         return EINVAL;
        case WSAEMFILE:         return EMFILE;
        case WSAEWOULDBLOCK:    return EWOULDBLOCK;
        case WSAEINPROGRESS:    return EINPROGRESS;
        case WSAEALREADY:       return EALREADY;
        case WSAENOTSOCK:       return ENOTSOCK;
        case WSAEDESTADDRREQ:   return EDESTADDRREQ;
        case WSAEMSGSIZE:       return EMSGSIZE;
        case WSAEPROTOTYPE:     return EPROTOTYPE;
        case WSAENOPROTOOPT:    return ENOPROTOOPT;
        case WSAEPROTONOSUPPORT:return EPROTONOSUPPORT;
        case WSAEOPNOTSUPP:     return EOPNOTSUPP;
        case WSAEAFNOSUPPORT:   return EAFNOSUPPORT;
        case WSAEADDRINUSE:     return EADDRINUSE;
        case WSAEADDRNOTAVAIL:  return EADDRNOTAVAIL;
        case WSAENETDOWN:       return ENETDOWN;
        case WSAENETUNREACH:    return ENETUNREACH;
        case WSAENETRESET:      return ENETRESET;
        case WSAECONNABORTED:   return ECONNABORTED;
        case WSAECONNRESET:     return ECONNRESET;
        case WSAENOBUFS:        return ENOBUFS;
        case WSAEISCONN:        return EISCONN;
        case WSAENOTCONN:       return ENOTCONN;
        case WSAESHUTDOWN:      return EPIPE;
        case WSAETIMEDOUT:      return ETIMEDOUT;
        case WSAECONNREFUSED:   return ECONNREFUSED;
        case WSAEHOSTUNREACH:   return EHOSTUNREACH;
        default:                return EIO;
    }
}

_hot int utils::platform::windows::socket_error(void)
{
    errno = utils::platform::windows::socket_errno(::WSAGetLastError());
    return -1;
}

_hot _nodiscard bool utils::platform::windows::is_socket(int fd)
{
    if (fd < 0) _unlikely return false;
    int type = 0;
    int len = sizeof(type);
    return ::getsockopt(static_cast<SOCKET>(fd), SOL_SOCKET, SO_TYPE, reinterpret_cast<char*>(&type), &len) == 0;
}
