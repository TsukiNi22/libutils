/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 31/07/2026 by @author Tsukini

File Name:
##  @file utils.hpp

File Description:
##  Dispatching include to the true one utils/utils.hpp
\**************************************************************/

#pragma once

//----------------------------------------------------------------//
/* DEFINE */

// Definition of the library version (set by the cmake)
#define __LIBUTILS_VERSION__ "[unknown]"

//----------------------------------------------------------------//
/* INCLUDE */

/* redirection (relatif) */
#if __has_include("utils/utils.hpp") // Try to find the main include otherwise throw an error
    #include "utils/utils.hpp"
#else
    #pragma message( \
        "\n\n" \
        "-----------------------------------------------------------------------\n" \
        "libutils: could not locate utils/utils.hpp\n" \
        "Try reinstalling the library. (actual version: " __LIBUTILS_VERSION__ ")\n" \
        "If the problem persists, open an issue at: https://github.com/TsukiNi22/libutils/issues\n" \
        "Please include how you installed libutils, plus the output of:\n" \
        "  ls /usr/include/utils /usr/local/include/utils\n" \
        "-----------------------------------------------------------------------\n\n" \
    )
    #error "utils/utils.hpp not found - see message above"
#endif
