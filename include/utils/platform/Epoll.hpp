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
    #ifdef UTILS_WINDOWS
        #include <cstdint>                  // std::uint32_t, std::uint64_t
    #else
        #include <sys/epoll.h>              // struct epoll_event, EPOLL*
    #endif

    //----------------------------------------------------------------//
    /* DEFINE */

    #ifdef UTILS_WINDOWS
        // Same values as Linux
        #define EPOLLIN         0x001u
        #define EPOLLPRI        0x002u
        #define EPOLLOUT        0x004u
        #define EPOLLERR        0x008u
        #define EPOLLHUP        0x010u
        #define EPOLLRDNORM     0x040u
        #define EPOLLWRNORM     0x100u
        #define EPOLLRDHUP      0x2000u
        #define EPOLLONESHOT    (1u << 30)
        #define EPOLLET         (1u << 31)  // not available on Windows (level triggered only): EINVAL

        #define EPOLL_CLOEXEC   02000000    // ignored on Windows
        #define EPOLL_CTL_ADD   1
        #define EPOLL_CTL_DEL   2
        #define EPOLL_CTL_MOD   3
    #endif

//----------------------------------------------------------------//
/* STRUCT */

#ifdef UTILS_WINDOWS
    // Same layout as Linux (<sys/epoll.h>)
    union epoll_data { // xstyle: ignore CPP-CLASS-NAME (POSIX name)
        void* ptr;
        int fd;
        std::uint32_t u32;
        std::uint64_t u64;
    };

    struct epoll_event { // xstyle: ignore CPP-CLASS-NAME (POSIX name)
        std::uint32_t events;
        union epoll_data data;
    };
#endif

//----------------------------------------------------------------//
/* TYPEDEF */

#ifdef UTILS_WINDOWS
    using epoll_data_t = union epoll_data;
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
