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
    #include "../attribute/Attribute.hpp"   // _nodiscard, _restrict, _read_only, _write_only, _null_terminated_string_arg, _fd_arg, _fd_arg_read, _fd_arg_write
    #include "Platform.hpp"                 // UTILS_WINDOWS
    #include "Types.h"                      // STDIN_FILENO, struct pollfd, nfds_t (Windows)
    #include <sys/types.h>                  // ssize_t, pid_t (also on MinGW)
    #include <cstddef>                      // std::size_t
    #ifndef UTILS_WINDOWS
        #include <unistd.h>                 // STDIN_FILENO
        #include <poll.h>                   // struct pollfd, nfds_t
    #endif

namespace utils::platform { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* fd */
_fd_arg(1) int close(int fd); // any fd (Windows: socket, CRT fd or emulated epoll)
_nodiscard _fd_arg_read(1) _write_only(2, 3) ssize_t read(int fd, void* buf, std::size_t count); // Windows: recv on a socket
_fd_arg_write(1) _read_only(2, 3) ssize_t write(int fd, const void* buf, std::size_t count); // Windows: send on a socket
int pipe(int fds[2]); // Windows: binary mode
_nodiscard _fd_arg(1) int dup(int fd);
_fd_arg(1) int dup2(int fd, int fd2);
_nodiscard _fd_arg(1) int isatty(int fd); // Windows: console
_nodiscard int poll(struct pollfd* fds, nfds_t nfds, int timeout); // Windows: sockets (WSAPoll), console / pipe / file input

/* process */
_nodiscard pid_t getpid(void);
_nodiscard _null_terminated_string_arg(1) _write_only(2, 3) ssize_t readlink(const char* _restrict path, char* _restrict buf, std::size_t size); // Windows: "/proc/self/exe" = GetModuleFileName, else a symbolic link

/* error */
_nodiscard const char* strerror(int errnum); // Windows: also the socket errors (WSA) translated in errno

} // namespace end
#endif /* PLATFORM_UNISTD_H */
