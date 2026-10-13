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
##  @file Unistd.cpp

File Description:
##  Definition of the POSIX calls of Unistd.hpp (Unix: system, Windows: CRT / Win32)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/platform/Platform.hpp"
#include "utils/platform/Unistd.hpp"
#ifdef UTILS_WINDOWS
    #include "Windows.hpp"
#else
    #include <unistd.h>
    #include <poll.h>
#endif
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

#ifdef UTILS_WINDOWS
    // errno values of the socket errors (from socket_errno) -> Winsock code, for strerror
    static int wsa_code(int errnum)
    {
        static constexpr int codes[] = {
            WSAEWOULDBLOCK, WSAEINPROGRESS, WSAEALREADY, WSAENOTSOCK, WSAEDESTADDRREQ, WSAEMSGSIZE, WSAEPROTOTYPE,
            WSAENOPROTOOPT, WSAEPROTONOSUPPORT, WSAEOPNOTSUPP, WSAEAFNOSUPPORT, WSAEADDRINUSE, WSAEADDRNOTAVAIL,
            WSAENETDOWN, WSAENETUNREACH, WSAENETRESET, WSAECONNABORTED, WSAECONNRESET, WSAENOBUFS, WSAEISCONN,
            WSAENOTCONN, WSAETIMEDOUT, WSAECONNREFUSED, WSAEHOSTUNREACH
        };
        for (int code: codes) if (utils::platform::windows::socket_errno(code) == errnum) return code;
        return 0;
    }

    // Console input: drop the events that don't give a char (key release, mouse, focus...), true if a char is waiting
    static bool console_readable(HANDLE input)
    {
        while (true) {
            DWORD count = 0;
            INPUT_RECORD record{};
            if (!::PeekConsoleInputA(input, &record, 1, &count) || count == 0) return false;
            if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown && record.Event.KeyEvent.uChar.AsciiChar != 0) return true;
            (void)::ReadConsoleInputA(input, &record, 1, &count);
        }
    }

    // Readiness of a fd that is not a socket (console, pipe or file): POLLIN / POLLHUP / POLLNVAL
    static short input_revents(int fd, short events)
    {
        const HANDLE handle = reinterpret_cast<HANDLE>(::_get_osfhandle(fd));
        if (handle == INVALID_HANDLE_VALUE) return POLLNVAL;
        if (!(events & (POLLIN | POLLRDNORM))) return 0;

        DWORD mode = 0;
        switch (::GetFileType(handle)) {
            case FILE_TYPE_CHAR:
                if (::GetConsoleMode(handle, &mode)) return console_readable(handle) ? POLLRDNORM : 0;
                return POLLRDNORM; // NUL & co
            case FILE_TYPE_PIPE: {
                DWORD available = 0;
                if (!::PeekNamedPipe(handle, nullptr, 0, nullptr, &available, nullptr))
                    return (::GetLastError() == ERROR_BROKEN_PIPE) ? POLLHUP : POLLERR; // writer closed: EOF to read
                return available ? POLLRDNORM : 0;
            }
            default:
                return POLLRDNORM; // file: always readable
        }
    }
#endif

//----------------------------------------------------------------//
/* FUNCTION */

_cold int utils::platform::close(int fd)
{
    #ifdef UTILS_WINDOWS
        if (fd < 0) _unlikely {errno = EBADF; return -1;} // the CRT aborts on an invalid fd (invalid parameter handler)
        if (utils::platform::windows::epoll_release(fd)) return 0;
        if (utils::platform::windows::is_socket(fd))
            return (::closesocket(static_cast<SOCKET>(fd)) == 0) ? 0 : utils::platform::windows::socket_error();
        return ::_close(fd);
    #else
        return ::close(fd);
    #endif
}

_hot _nodiscard ssize_t utils::platform::read(int fd, void* buf, std::size_t count)
{
    #ifdef UTILS_WINDOWS
        if (fd < 0) _unlikely {errno = EBADF; return -1;} // the CRT aborts on an invalid fd (invalid parameter handler)
        if (utils::platform::windows::is_socket(fd)) {
            const int res = ::recv(static_cast<SOCKET>(fd), static_cast<char*>(buf), static_cast<int>(count), 0);
            return (res == SOCKET_ERROR) ? utils::platform::windows::socket_error() : res;
        }
        return ::_read(fd, buf, static_cast<unsigned int>(count));
    #else
        return ::read(fd, buf, count);
    #endif
}

_hot ssize_t utils::platform::write(int fd, const void* buf, std::size_t count)
{
    #ifdef UTILS_WINDOWS
        if (fd < 0) _unlikely {errno = EBADF; return -1;} // the CRT aborts on an invalid fd (invalid parameter handler)
        if (utils::platform::windows::is_socket(fd)) {
            const int res = ::send(static_cast<SOCKET>(fd), static_cast<const char*>(buf), static_cast<int>(count), 0);
            return (res == SOCKET_ERROR) ? utils::platform::windows::socket_error() : res;
        }
        return ::_write(fd, buf, static_cast<unsigned int>(count));
    #else
        return ::write(fd, buf, count);
    #endif
}

_cold int utils::platform::pipe(int fds[2])
{
    #ifdef UTILS_WINDOWS
        return ::_pipe(fds, 4096, _O_BINARY); // 4096: size of the pipe buffer
    #else
        return ::pipe(fds);
    #endif
}

_cold _nodiscard int utils::platform::dup(int fd)
{
    #ifdef UTILS_WINDOWS
        if (fd < 0) _unlikely {errno = EBADF; return -1;} // the CRT aborts on an invalid fd (invalid parameter handler)
        return ::_dup(fd);
    #else
        return ::dup(fd);
    #endif
}

_cold int utils::platform::dup2(int fd, int fd2)
{
    #ifdef UTILS_WINDOWS
        if (fd < 0 || fd2 < 0) _unlikely {errno = EBADF; return -1;} // the CRT aborts on an invalid fd (invalid parameter handler)
        return (::_dup2(fd, fd2) == 0) ? fd2 : -1; // the CRT returns 0
    #else
        return ::dup2(fd, fd2);
    #endif
}

_cold _nodiscard int utils::platform::isatty(int fd)
{
    #ifdef UTILS_WINDOWS
        if (fd < 0) _unlikely {errno = EBADF; return 0;}
        return ::_isatty(fd) ? 1 : 0;
    #else
        return ::isatty(fd);
    #endif
}

_hot _nodiscard int utils::platform::poll(struct pollfd* fds, nfds_t nfds, int timeout)
{
    #ifdef UTILS_WINDOWS
        // Only sockets: WSAPoll as is
        const bool sockets = std::all_of(fds, fds + nfds, [](const struct pollfd& pfd) {return utils::platform::windows::is_socket(static_cast<int>(pfd.fd));});
        if (sockets && nfds > 0) {
            const int res = ::WSAPoll(fds, static_cast<ULONG>(nfds), timeout);
            return (res == SOCKET_ERROR) ? utils::platform::windows::socket_error() : res;
        }

        // Mixed (console, pipe, file): check each fd by slices of 10ms
        using clock = std::chrono::steady_clock;
        const clock::time_point deadline = clock::now() + std::chrono::milliseconds(std::max(timeout, 0));
        while (true) {
            int ready = 0;
            for (nfds_t i = 0; i < nfds; ++i) {
                const int fd = static_cast<int>(fds[i].fd);
                fds[i].revents = 0;
                if (fd < 0) continue;
                if (utils::platform::windows::is_socket(fd)) {
                    WSAPOLLFD one{fds[i].fd, fds[i].events, 0};
                    if (::WSAPoll(&one, 1, 0) > 0) fds[i].revents = one.revents;
                } else {
                    fds[i].revents = input_revents(fd, fds[i].events);
                }
                if (fds[i].revents) ++ready;
            }
            if (ready > 0) return ready;
            if (timeout == 0) return 0;
            if (timeout > 0 && clock::now() >= deadline) return 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout < 0 ? 10 : std::clamp<long long>(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - clock::now()).count(), 0, 10)));
        }
    #else
        return ::poll(fds, nfds, timeout);
    #endif
}

_cold _nodiscard pid_t utils::platform::getpid(void)
{
    #ifdef UTILS_WINDOWS
        return static_cast<pid_t>(::_getpid());
    #else
        return ::getpid();
    #endif
}

_cold _nodiscard ssize_t utils::platform::readlink(const char* path, char* buf, std::size_t size)
{
    #ifdef UTILS_WINDOWS
        std::string target;
        if (std::strcmp(path, "/proc/self/exe") == 0) {
            char exe[MAX_PATH];
            const DWORD len = ::GetModuleFileNameA(nullptr, exe, MAX_PATH);
            if (len == 0 || len >= MAX_PATH) _unlikely {errno = ENAMETOOLONG; return -1;}
            target.assign(exe, len);
        } else {
            std::error_code ec;
            if (!std::filesystem::exists(std::filesystem::symlink_status(path, ec))) _unlikely {errno = ENOENT; return -1;}
            target = std::filesystem::read_symlink(path, ec).string();
            if (ec) _unlikely {errno = EINVAL; return -1;} // not a symbolic link
        }
        const std::size_t len = std::min(size, target.size()); // no '\0' (POSIX)
        std::memcpy(buf, target.data(), len);
        return static_cast<ssize_t>(len);
    #else
        return ::readlink(path, buf, size);
    #endif
}

_cold _nodiscard const char* utils::platform::strerror(int errnum)
{
    #ifdef UTILS_WINDOWS
        // Socket errors: the CRT doesn't know them ("Unknown error"), message of the Winsock code
        const int code = wsa_code(errnum);
        if (code == 0) return ::strerror(errnum);
        thread_local std::string message;
        message = utils::platform::windows::strerror(static_cast<DWORD>(code));
        return message.c_str();
    #else
        return ::strerror(errnum);
    #endif
}
