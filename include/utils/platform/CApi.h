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
##  @file CApi.h

File Description:
##  C API of utils::platform: the POSIX calls of libutils for Unix & Windows (C & C++ compatible)
##  Same contract as POSIX (-1 & errno, NULL & dlerror, EAI_* code), utils_exception is never used
##  Unix: the POSIX structures need the POSIX features (_POSIX_C_SOURCE >= 200112L, default of gnu11 / gnu23)
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef PLATFORM_CAPI_C_H
    #define PLATFORM_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _nodiscard, _nothrow, _restrict, _read_only, _write_only, _null_terminated_string_arg, _fd_arg, _fd_arg_read, _fd_arg_write
    #include "Platform.hpp"                 // UTILS_WINDOWS, UTILS_UNIX
    #include "Types.h"                      // Windows: STDIN_FILENO, RTLD_*, EPOLL*, struct epoll_event, socklen_t, struct termios, ...
    #include <sys/types.h>                  // ssize_t, pid_t (also on MinGW)
    #include <stddef.h>                     // size_t
    #ifndef UTILS_WINDOWS
        #include <sys/socket.h>         // struct sockaddr, socklen_t, MSG_NOSIGNAL
        #include <sys/epoll.h>          // struct epoll_event, EPOLL*
        #include <termios.h>            // struct termios, tcflag_t, ICANON, ECHO
        #include <unistd.h>             // STDIN_FILENO
        #include <netdb.h>              // struct addrinfo
        #include <dlfcn.h>              // RTLD_*
        #include <poll.h>               // struct pollfd, nfds_t
    #endif

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* STRUCT */

/* information of utils_platform_dladdr (fields of Dl_info, without the need of _GNU_SOURCE) */
typedef struct utils_platform_dl_info {
    const char* dli_fname; // path of the module
    void* dli_fbase; // base address of the module
    const char* dli_sname; // nearest symbol (always NULL on Windows)
    void* dli_saddr; // address of the nearest symbol (always NULL on Windows)
} utils_platform_dl_info_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* fd (unistd) */
_nodiscard _nothrow _fd_arg(1) int utils_platform_close(int fd); // any fd (Windows: socket, CRT fd or emulated epoll)
_nodiscard _nothrow _fd_arg_read(1) _write_only(2, 3) ssize_t utils_platform_read(int fd, void* buf, size_t count); // Windows: recv on a socket
_nodiscard _nothrow _fd_arg_write(1) _read_only(2, 3) ssize_t utils_platform_write(int fd, const void* buf, size_t count); // Windows: send on a socket
_nodiscard _nothrow int utils_platform_pipe(int fds[2]); // Windows: binary mode
_nodiscard _nothrow _fd_arg(1) int utils_platform_dup(int fd);
_nodiscard _nothrow _fd_arg(1) int utils_platform_dup2(int fd, int fd2);
_nodiscard _nothrow _fd_arg(1) int utils_platform_isatty(int fd); // Windows: console
_nodiscard _nothrow int utils_platform_poll(struct pollfd* fds, nfds_t nfds, int timeout); // Windows: sockets (WSAPoll), console / pipe / file input

/* process (unistd) */
_nodiscard _nothrow pid_t utils_platform_getpid(void);
_nodiscard _nothrow _null_terminated_string_arg(1) _write_only(2, 3) ssize_t utils_platform_readlink(const char* _restrict path, char* _restrict buf, size_t size); // Windows: "/proc/self/exe" = GetModuleFileName, else a symbolic link

/* error (unistd) */
_nodiscard _nothrow const char* utils_platform_strerror(int errnum); // Windows: also the socket errors (WSA) translated in errno

/* shared library (dlfcn) */
_nodiscard _nothrow _null_terminated_string_arg(1) void* utils_platform_dlopen(const char* path, int flags); // NULL on error (utils_platform_dlerror)
_nodiscard _nothrow _null_terminated_string_arg(2) void* utils_platform_dlsym(void* handle, const char* name); // NULL on error (utils_platform_dlerror)
_nodiscard _nothrow int utils_platform_dlclose(void* handle); // 0 on success
_nodiscard _nothrow char* utils_platform_dlerror(void); // last error of the thread, NULL if none (cleared by the call)
_nodiscard _nothrow int utils_platform_dladdr(const void* addr, utils_platform_dl_info_t* info); // 0 on error

/* epoll (Windows: only sockets can be linked (ENOTSOCK), EPOLLET is refused (EINVAL), EPOLLONESHOT is emulated) */
_nodiscard _nothrow int utils_platform_epoll_create1(int flags);
_nodiscard _nothrow _fd_arg(1) _fd_arg(3) int utils_platform_epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);
_nodiscard _nothrow _fd_arg(1) int utils_platform_epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);

/* socket */
_nodiscard _nothrow int utils_platform_socket(int domain, int type, int protocol);
_nodiscard _nothrow _fd_arg(1) _read_only(2, 3) int utils_platform_connect(int fd, const struct sockaddr* addr, socklen_t len);
_nodiscard _nothrow _fd_arg(1) _read_only(2, 3) int utils_platform_bind(int fd, const struct sockaddr* addr, socklen_t len);
_nodiscard _nothrow _fd_arg(1) int utils_platform_listen(int fd, int backlog);
_nodiscard _nothrow _fd_arg(1) int utils_platform_accept(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len);
_nodiscard _nothrow _fd_arg_read(1) _write_only(2, 3) ssize_t utils_platform_recv(int fd, void* buf, size_t len, int flags);
_nodiscard _nothrow _fd_arg_write(1) _read_only(2, 3) ssize_t utils_platform_send(int fd, const void* buf, size_t len, int flags);
_nodiscard _nothrow _fd_arg(1) _read_only(4, 5) int utils_platform_setsockopt(int fd, int level, int name, const void* value, socklen_t len); // Windows: SO_SNDTIMEO / SO_RCVTIMEO take a timeval, SO_REUSEADDR is the default (no-op)
_nodiscard _nothrow _fd_arg(1) int utils_platform_getsockname(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len);
_nodiscard _nothrow _fd_arg(1) int utils_platform_getpeername(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len);
_nodiscard _nothrow _null_terminated_string_arg(1) _null_terminated_string_arg(2) int utils_platform_getaddrinfo(const char* _restrict node, const char* _restrict service, const struct addrinfo* _restrict hints, struct addrinfo** _restrict res); // EAI_* error code (utils_platform_gai_strerror)
_nothrow void utils_platform_freeaddrinfo(struct addrinfo* res);
_nodiscard _nothrow const char* utils_platform_gai_strerror(int code);

/* terminal (termios) */
_nodiscard _nothrow _fd_arg(1) int utils_platform_tcgetattr(int fd, struct termios* term); // Windows: ENOTTY when the fd is not a console
_nodiscard _nothrow _fd_arg(1) int utils_platform_tcsetattr(int fd, int action, const struct termios* term); // Windows: without ICANON the input gives the escape sequences (VT input)

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* PLATFORM_CAPI_C_H */
