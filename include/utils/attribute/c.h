/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 13/11/2026 by @author Tsukini

File Name:
##  @file c.h

File Description:
##  Include the right file from the c version used or selected (same macros as c++.hpp)
##  Only included by Attribute.hpp in c (c++: c++.hpp)
\**************************************************************/

// xstyle: ignore-file CPP-INCLUDE-COMMENT (dispatch includes: the #if / comment above says what is included)

#ifndef ATTRIBUTE_C_H
    #define ATTRIBUTE_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    #if !defined(__GNUC__)
        #include "fallback.hpp"

    #elif defined(FORCED_C_STANDARD) // Forced version (year: 1999, 2011, 2017, 2023)

        #if FORCED_C_STANDARD >= 2023 // forced c23 and above
            #include "c23.h"
        #elif FORCED_C_STANDARD >= 2011 // forced c11 and above
            #include "c11.h"
        #elif FORCED_C_STANDARD >= 1999 // forced c99 and above
            #include "c99.h"
        #else
            #include "fallback.hpp"
        #endif

    #else // No forced version

        #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L // c23 and above
            #include "c23.h"
        #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L // c11 and above
            #include "c11.h"
        #else // c99 / gnu89
            #include "c99.h"
        #endif

    #endif

    // keywords & values common to every c version
    #include "common.h" // _thread_local, _restrict, _alignas, hardware_*_interference_size

    // BACKWARD_COMPATIBILITY_WARNING
    /* --- Different Messages ---
     * legacy -> Only kept for backward compatibility; no removal in sight ._.
     * migration -> Retained for backward compatibility; will be removed in a future version (estimated: ~vx.x.x)
     * GNU syntax used: works in every c version
    */
    #if defined(BACKWARD_COMPATIBILITY_WARNING) && !defined(NO_BACKWARD_COMPATIBILITY_WARNING) && defined(__GNUC__)
        #define _legacy                         __attribute__((deprecated("Only kept for backward compatibility; no removal in sight ._.")))
        #define _migration(major, minor, fix)   __attribute__((deprecated("Retained for backward compatibility; will be removed in a future version (estimated: ~v" #major "." #minor "." #fix ")")))
    #else
        #define _legacy                         // Not defined with this flag / compiler
        #define _migration(major, minor, fix)   // Not defined with this flag / compiler
    #endif

#endif /* ATTRIBUTE_C_H */
