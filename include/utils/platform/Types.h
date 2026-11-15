/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file Types.h

File Description:
##  Windows versions of the POSIX defines, structures & types used by utils::platform (C & C++ compatible)
##  Same names, values & layout as Linux; nothing on Unix (the system headers give them)
\**************************************************************/

// xstyle: ignore-file CPP-TYPEDEF (C & C++ header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (POSIX names)

#ifndef PLATFORM_TYPES_H
    #define PLATFORM_TYPES_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "Platform.hpp"     // UTILS_WINDOWS
    #ifdef UTILS_WINDOWS
        #include <stdint.h>     // uint32_t, uint64_t
    #endif

    //----------------------------------------------------------------//
    /* DEFINE */

    #ifdef UTILS_WINDOWS
        /* unistd */
        #ifndef STDIN_FILENO
            #define STDIN_FILENO    0
            #define STDOUT_FILENO   1
            #define STDERR_FILENO   2
        #endif

        /* dlfcn: same values as Linux (ignored on Windows: a dll is always resolved at the load) */
        #define RTLD_LAZY   0x00001
        #define RTLD_NOW    0x00002
        #define RTLD_GLOBAL 0x00100
        #define RTLD_LOCAL  0

        /* epoll: same values as Linux */
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

        /* socket */
        #ifndef MSG_NOSIGNAL
            #define MSG_NOSIGNAL 0      // no SIGPIPE on Windows
        #endif

        /* termios: same values as Linux, only the local flags are used on Windows */
        #define NCCS        32
        #define ISIG        0000001
        #define ICANON      0000002
        #define ECHO        0000010
        #define TCSANOW     0
        #define TCSADRAIN   1
        #define TCSAFLUSH   2
    #endif

//----------------------------------------------------------------//
/* TYPEDEF */

#ifdef UTILS_WINDOWS
    /* unistd */
    typedef unsigned long nfds_t;

    /* socket */
    typedef int socklen_t; // <ws2tcpip.h>

    /* termios */
    typedef unsigned int tcflag_t;
    typedef unsigned char cc_t;
    typedef unsigned int speed_t;
#endif

//----------------------------------------------------------------//
/* STRUCT */

#ifdef UTILS_WINDOWS
    /* unistd */
    struct pollfd; // <winsock2.h> (not included: no windows.h macros in the public headers)

    /* dlfcn: same fields as Linux (<dlfcn.h>) */
    typedef struct Dl_info {
        const char* dli_fname; // path of the module (thread local storage, valid until the next dladdr of the thread)
        void* dli_fbase; // base address of the module
        const char* dli_sname; // always NULL on Windows
        void* dli_saddr; // always NULL on Windows
    } Dl_info;

    /* epoll: same layout as Linux (<sys/epoll.h>) */
    typedef union epoll_data {
        void* ptr;
        int fd;
        uint32_t u32;
        uint64_t u64;
    } epoll_data_t;

    struct epoll_event {
        uint32_t events;
        union epoll_data data;
    };

    /* socket: <winsock2.h> / <ws2tcpip.h> are not included (no windows.h macros in the public headers),
       include them in the code that needs the structures (sockaddr_in, addrinfo...) */
    struct sockaddr;
    struct addrinfo;

    /* termios: same fields as Linux (<termios.h>) + the console mode */
    struct termios {
        tcflag_t c_iflag;
        tcflag_t c_oflag;
        tcflag_t c_cflag;
        tcflag_t c_lflag; // ISIG, ICANON, ECHO -> console mode
        cc_t c_line;
        cc_t c_cc[NCCS];
        speed_t c_ispeed;
        speed_t c_ospeed;
        unsigned long c_console; // Windows: console mode read by tcgetattr (restored as is by tcsetattr)
    };
#endif

#endif /* PLATFORM_TYPES_H */
