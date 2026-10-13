/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 07/10/2026 by @author Tsukini

File Name:
##  @file Platform.hpp

File Description:
##  Detection of the target platform (Unix or Windows)
##  UTILS_UNIX / UTILS_WINDOWS, UTILS_PLATFORM
\**************************************************************/

#ifndef PLATFORM_H
    #define PLATFORM_H

    //----------------------------------------------------------------//
    /* DEFINE */

    #if defined(_WIN32) // Windows (MinGW-w64)
        #define UTILS_WINDOWS
        #define UTILS_PLATFORM "windows"
    #elif defined(__unix__) || defined(__unix) || defined(__APPLE__) // Linux, BSD, macOS
        #define UTILS_UNIX
        #define UTILS_PLATFORM "unix"
    #else
        #error "libutils: unsupported platform (only Unix & Windows are supported)"
    #endif

#endif /* PLATFORM_H */
