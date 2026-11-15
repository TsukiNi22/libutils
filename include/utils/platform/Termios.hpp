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
##  @file Termios.hpp

File Description:
##  Terminal calls of <termios.h> for Unix & Windows (same names, same contract: -1 & errno)
##  Unix: the system call, Windows: console mode (ICANON = line input, ECHO = echo, ISIG = ctrl+c)
\**************************************************************/

#ifndef PLATFORM_TERMIOS_H
    #define PLATFORM_TERMIOS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "Platform.hpp"             // UTILS_WINDOWS
    #include "Types.h"                  // struct termios, tcflag_t, ICANON, ECHO (Windows)
    #ifndef UTILS_WINDOWS
        #include <termios.h>            // struct termios, tcflag_t, ICANON, ECHO
    #endif

namespace utils::platform { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

int tcgetattr(int fd, struct termios* term); // Windows: ENOTTY when the fd is not a console
int tcsetattr(int fd, int action, const struct termios* term); // Windows: without ICANON the input gives the escape sequences (VT input)

} // namespace end
#endif /* PLATFORM_TERMIOS_H */
