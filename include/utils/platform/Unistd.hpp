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
##  @file Unistd.hpp

File Description:
##  POSIX calls of <unistd.h> & co for Unix & Windows (same names, same contract: -1 & errno)
##  Unix: the system call, Windows: CRT / Win32 implementation
\**************************************************************/

#ifndef PLATFORM_UNISTD_H
    #define PLATFORM_UNISTD_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "Platform.hpp"             // UTILS_WINDOWS
    #include <sys/types.h>              // ssize_t, pid_t (also on MinGW)
    #include <cstddef>                  // std::size_t
    #ifndef UTILS_WINDOWS
        #include <unistd.h>             // STDIN_FILENO
        #include <poll.h>               // struct pollfd, nfds_t
    #endif

    //----------------------------------------------------------------//
    /* DEFINE */

    #ifdef UTILS_WINDOWS
        #ifndef STDIN_FILENO
            #define STDIN_FILENO    0
            #define STDOUT_FILENO   1
            #define STDERR_FILENO   2
        #endif
    #endif

//----------------------------------------------------------------//
/* STRUCT */

#ifdef UTILS_WINDOWS
    struct pollfd; // <winsock2.h> (not included: no windows.h macros in the public headers)
#endif

//----------------------------------------------------------------//
/* TYPEDEF */

#ifdef UTILS_WINDOWS
    typedef unsigned long nfds_t;
#endif

namespace utils::platform { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* fd */
int close(int fd); // any fd (Windows: socket, CRT fd or emulated epoll)
ssize_t read(int fd, void* buf, std::size_t count); // Windows: recv on a socket
ssize_t write(int fd, const void* buf, std::size_t count); // Windows: send on a socket
int pipe(int fds[2]); // Windows: binary mode
int dup(int fd);
int dup2(int fd, int fd2);
int isatty(int fd); // Windows: console
int poll(struct pollfd* fds, nfds_t nfds, int timeout); // Windows: sockets (WSAPoll), console / pipe / file input

/* process */
pid_t getpid(void);
ssize_t readlink(const char* path, char* buf, std::size_t size); // Windows: "/proc/self/exe" = GetModuleFileName, else a symbolic link

/* error */
const char* strerror(int errnum); // Windows: also the socket errors (WSA) translated in errno

} // namespace end
#endif /* PLATFORM_UNISTD_H */
