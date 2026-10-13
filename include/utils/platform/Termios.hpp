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
    #ifndef UTILS_WINDOWS
        #include <termios.h>            // struct termios, tcflag_t, ICANON, ECHO
    #endif

    //----------------------------------------------------------------//
    /* DEFINE */

    #ifdef UTILS_WINDOWS
        // Same values as Linux, only the local flags are used on Windows
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
    typedef unsigned int tcflag_t;
    typedef unsigned char cc_t;
    typedef unsigned int speed_t;
#endif

//----------------------------------------------------------------//
/* STRUCT */

#ifdef UTILS_WINDOWS
    // Same fields as Linux (<termios.h>) + the console mode
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

namespace utils::platform { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

int tcgetattr(int fd, struct termios* term); // Windows: ENOTTY when the fd is not a console
int tcsetattr(int fd, int action, const struct termios* term); // Windows: without ICANON the input gives the escape sequences (VT input)

} // namespace end
#endif /* PLATFORM_TERMIOS_H */
