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
##  @file CApi.cpp

File Description:
##  C API of utils::platform (same contract as POSIX, an exception of the Windows emulation gives errno)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/platform/Termios.hpp"
#include "utils/platform/Unistd.hpp"
#include "utils/platform/Socket.hpp"
#include "utils/platform/Dlfcn.hpp"
#include "utils/platform/Epoll.hpp"
#include "utils/platform/CApi.h"
#ifdef UTILS_WINDOWS
    #include "Windows.hpp"
#endif
#include <exception>
#include <cerrno>
#include <new>

//----------------------------------------------------------------//
/* TOOLS */

// Never let an exception cross the C boundary: the POSIX error of the call (errno ENOMEM / EIO)
template<typename R, typename F>
_hot _nodiscard static R posix(const R error, F&& f) noexcept
{
    try {
        return f();
    } catch (const std::bad_alloc&) {
        errno = ENOMEM;
    } catch (...) {
        errno = EIO;
    }
    return error;
}

//----------------------------------------------------------------//
/* FD (UNISTD) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_close(int fd)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::close(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow ssize_t utils_platform_read(int fd, void* buf, size_t count)
{
    return posix<ssize_t>(-1, [&](void) -> ssize_t {return utils::platform::read(fd, buf, count);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow ssize_t utils_platform_write(int fd, const void* buf, size_t count)
{
    return posix<ssize_t>(-1, [&](void) -> ssize_t {return utils::platform::write(fd, buf, count);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_pipe(int fds[2])
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::pipe(fds);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_dup(int fd)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::dup(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_dup2(int fd, int fd2)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::dup2(fd, fd2);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_isatty(int fd)
{
    return posix<int>(0, [&](void) -> int {return utils::platform::isatty(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_poll(struct pollfd* fds, nfds_t nfds, int timeout)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::poll(fds, nfds, timeout);});
}

//----------------------------------------------------------------//
/* PROCESS (UNISTD) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow pid_t utils_platform_getpid(void)
{
    return posix<pid_t>(-1, [&](void) -> pid_t {return utils::platform::getpid();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow ssize_t utils_platform_readlink(const char* _restrict path, char* _restrict buf, size_t size)
{
    return posix<ssize_t>(-1, [&](void) -> ssize_t {return utils::platform::readlink(path, buf, size);});
}

//----------------------------------------------------------------//
/* ERROR (UNISTD) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_platform_strerror(int errnum)
{
    return posix<const char*>(nullptr, [&](void) -> const char* {return utils::platform::strerror(errnum);});
}

//----------------------------------------------------------------//
/* SHARED LIBRARY (DLFCN) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow void* utils_platform_dlopen(const char* path, int flags)
{
    return posix<void*>(nullptr, [&](void) -> void* {return utils::platform::dlopen(path, flags);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow void* utils_platform_dlsym(void* handle, const char* name)
{
    return posix<void*>(nullptr, [&](void) -> void* {return utils::platform::dlsym(handle, name);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_dlclose(void* handle)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::dlclose(handle);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow char* utils_platform_dlerror(void)
{
    return posix<char*>(nullptr, [&](void) -> char* {return utils::platform::dlerror();});
}

_hot _nothrow int utils_platform_dladdr(const void* addr, utils_platform_dl_info_t* info)
{
    Dl_info dl = {};

    if (!info) _unlikely {return 0;}
    const int ret = posix<int>(0, [&](void) -> int {return utils::platform::dladdr(addr, &dl);});
    if (ret == 0) return 0;
    info->dli_fname = dl.dli_fname;
    info->dli_fbase = dl.dli_fbase;
    info->dli_sname = dl.dli_sname;
    info->dli_saddr = dl.dli_saddr;
    return ret;
}

//----------------------------------------------------------------//
/* EPOLL */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_epoll_create1(int flags)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::epoll_create1(flags);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::epoll_ctl(epfd, op, fd, event);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::epoll_wait(epfd, events, maxevents, timeout);});
}

//----------------------------------------------------------------//
/* SOCKET */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_socket(int domain, int type, int protocol)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::socket(domain, type, protocol);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_connect(int fd, const struct sockaddr* addr, socklen_t len)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::connect(fd, addr, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_bind(int fd, const struct sockaddr* addr, socklen_t len)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::bind(fd, addr, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_listen(int fd, int backlog)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::listen(fd, backlog);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_accept(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::accept(fd, addr, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow ssize_t utils_platform_recv(int fd, void* buf, size_t len, int flags)
{
    return posix<ssize_t>(-1, [&](void) -> ssize_t {return utils::platform::recv(fd, buf, len, flags);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow ssize_t utils_platform_send(int fd, const void* buf, size_t len, int flags)
{
    return posix<ssize_t>(-1, [&](void) -> ssize_t {return utils::platform::send(fd, buf, len, flags);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_setsockopt(int fd, int level, int name, const void* value, socklen_t len)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::setsockopt(fd, level, name, value, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_getsockname(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::getsockname(fd, addr, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_getpeername(int fd, struct sockaddr* _restrict addr, socklen_t* _restrict len)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::getpeername(fd, addr, len);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_getaddrinfo(const char* _restrict node, const char* _restrict service, const struct addrinfo* _restrict hints, struct addrinfo** _restrict res)
{
    return posix<int>(EAI_MEMORY, [&](void) -> int {return utils::platform::getaddrinfo(node, service, hints, res);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow void utils_platform_freeaddrinfo(struct addrinfo* res)
{
    (void)posix(0, [&](void) -> int {utils::platform::freeaddrinfo(res); return 0;});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_platform_gai_strerror(int code)
{
    return posix<const char*>(nullptr, [&](void) -> const char* {return utils::platform::gai_strerror(code);});
}

//----------------------------------------------------------------//
/* TERMINAL (TERMIOS) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_tcgetattr(int fd, struct termios* term)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::tcgetattr(fd, term);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow int utils_platform_tcsetattr(int fd, int action, const struct termios* term)
{
    return posix<int>(-1, [&](void) -> int {return utils::platform::tcsetattr(fd, action, term);});
}
