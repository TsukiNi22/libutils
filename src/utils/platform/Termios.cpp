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
##  @file Termios.cpp

File Description:
##  Definition of the terminal calls of Termios.hpp (Unix: system, Windows: console mode)
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/platform/Platform.hpp"
#include "utils/platform/Termios.hpp"
#ifdef UTILS_WINDOWS
    #include "Windows.hpp"
#else
    #include <termios.h>
#endif
#include <cerrno>

_cold int utils::platform::tcgetattr(int fd, struct termios* term)
{
    #ifdef UTILS_WINDOWS
        if (fd < 0) _unlikely {errno = EBADF; return -1;}
        const HANDLE handle = reinterpret_cast<HANDLE>(::_get_osfhandle(fd));
        DWORD mode = 0;
        if (!term) _unlikely {errno = EINVAL; return -1;}
        if (handle == INVALID_HANDLE_VALUE || !::GetConsoleMode(handle, &mode)) _unlikely {errno = ENOTTY; return -1;}

        *term = {};
        term->c_console = mode;
        if (mode & ENABLE_LINE_INPUT) term->c_lflag |= ICANON;
        if (mode & ENABLE_ECHO_INPUT) term->c_lflag |= ECHO;
        if (mode & ENABLE_PROCESSED_INPUT) term->c_lflag |= ISIG;
        return 0;
    #else
        return ::tcgetattr(fd, term);
    #endif
}

_cold int utils::platform::tcsetattr(int fd, int action, const struct termios* term)
{
    #ifdef UTILS_WINDOWS
        if (fd < 0) _unlikely {errno = EBADF; return -1;}
        const HANDLE handle = reinterpret_cast<HANDLE>(::_get_osfhandle(fd));
        DWORD current = 0;
        if (!term) _unlikely {errno = EINVAL; return -1;}
        if (handle == INVALID_HANDLE_VALUE || !::GetConsoleMode(handle, &current)) _unlikely {errno = ENOTTY; return -1;}

        // From the mode read by tcgetattr, local flags applied
        DWORD mode = term->c_console;
        mode = (term->c_lflag & ICANON) ? (mode | ENABLE_LINE_INPUT) : (mode & ~static_cast<DWORD>(ENABLE_LINE_INPUT));
        mode = (term->c_lflag & ECHO) ? (mode | ENABLE_ECHO_INPUT) : (mode & ~static_cast<DWORD>(ENABLE_ECHO_INPUT));
        mode = (term->c_lflag & ISIG) ? (mode | ENABLE_PROCESSED_INPUT) : (mode & ~static_cast<DWORD>(ENABLE_PROCESSED_INPUT));
        if (!(term->c_lflag & ECHO) || !(term->c_lflag & ICANON)) mode &= ~static_cast<DWORD>(ENABLE_ECHO_INPUT); // echo needs the line input on Windows
        if (!(term->c_lflag & ICANON)) mode |= ENABLE_VIRTUAL_TERMINAL_INPUT; // raw mode: arrows & co as escape sequences (like a Unix terminal)

        if (action == TCSAFLUSH) (void)::FlushConsoleInputBuffer(handle);
        if (!::SetConsoleMode(handle, mode)) _unlikely {errno = EINVAL; return -1;}
        return 0;
    #else
        return ::tcsetattr(fd, action, term);
    #endif
}
