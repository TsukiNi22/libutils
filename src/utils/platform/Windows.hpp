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
##  @file Windows.hpp

File Description:
##  Internal (not installed, Windows only: included under #ifdef UTILS_WINDOWS):
##  Win32 headers and the helpers shared by the platform implementations
\**************************************************************/

#ifndef PLATFORM_WINDOWS_H
    #define PLATFORM_WINDOWS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN     // windows.h (complete) declares a '_unused' member after the libutils attribute macros
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <winsock2.h>               // SOCKET, WSAPoll, WSAGetLastError
    #include <ws2tcpip.h>               // socklen_t, getaddrinfo, inet_pton
    #include <windows.h>                // HANDLE, DWORD, FormatMessage
    #include <process.h>                // _getpid
    #include <fcntl.h>                  // _O_BINARY
    #include <string>                   // std::string
    #include <io.h>                     // _read, _write, _close, _pipe, _dup

namespace utils::platform::windows { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

std::string strerror(DWORD code); // message of a Win32 / Winsock error code (FormatMessage)
void startup(void); // start Winsock once (WSACleanup at the exit), throw an ErrorException on error
int socket_errno(int code); // Winsock error -> errno value (EIO when unknown)
int socket_error(void); // set errno from WSAGetLastError, return -1
bool is_socket(int fd); // the fd is a Winsock socket
bool epoll_release(int epfd); // release an emulated epoll (Epoll.cpp), false when it's not one

} // namespace end
#endif /* PLATFORM_WINDOWS_H */
