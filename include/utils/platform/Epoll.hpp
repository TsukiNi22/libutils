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
##  @file Epoll.hpp

File Description:
##  epoll for Unix & Windows (same names, same behavior)
##  Unix: <sys/epoll.h>, Windows: emulation over WSAPoll (sockets only)
\**************************************************************/

#ifndef PLATFORM_EPOLL_H
    #define PLATFORM_EPOLL_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "Platform.hpp"                 // UTILS_WINDOWS
    #include "Types.h"                      // struct epoll_event, EPOLL* (Windows)
    #ifndef UTILS_WINDOWS
        #include <sys/epoll.h>              // struct epoll_event, EPOLL*
    #endif

namespace utils::platform { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

// Same contract as the Linux functions: -1 and errno on error, closed by utils::platform::close
// Windows: only sockets can be linked (ENOTSOCK), EPOLLET is refused (EINVAL), EPOLLONESHOT is emulated
int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);
int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);

} // namespace end
#endif /* PLATFORM_EPOLL_H */
