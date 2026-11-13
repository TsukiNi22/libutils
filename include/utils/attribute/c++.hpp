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
##  @file c++.hpp

File Description:
##  Include the right file from the c++ version used or selected
\**************************************************************/

// xstyle: ignore-file CPP-INCLUDE-COMMENT (dispatch includes: the #if / comment above says what is included)

#ifndef ATTRIBUTE_CPP_H
    #define ATTRIBUTE_CPP_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    #if !defined(__cplusplus)
        #error "C++ compiler required"
    #elif !defined(__GNUC__)
        #include "fallback.hpp"

    #elif defined(FORCED_CXX_STANDARD) // Forced version

        #if FORCED_CXX_STANDARD >= 23 // forced c++23 and above
            #include "c++23.hpp"
        #elif FORCED_CXX_STANDARD >= 20 // forced c++20 and above
            #include "c++20.hpp"
        #elif FORCED_CXX_STANDARD >= 17 // forced c++17 and above
            #include "c++17.hpp"
        #elif FORCED_CXX_STANDARD >= 14 // forced c++14 and above
            #include "c++14.hpp"
        #elif FORCED_CXX_STANDARD >= 11 // forced c++11 and above
            #include "c++11.hpp"
        #else
            #include "fallback.hpp"
        #endif

    #else // No forced version

        #if __cplusplus >= 202302L // c++23 and above
            #include "c++23.hpp"
        #elif __cplusplus >= 202002L // c++20 and above
            #include "c++20.hpp"
        #elif __cplusplus >= 201703L // c++17 and above
            #include "c++17.hpp"
        #elif __cplusplus >= 201402L // c++14 and above
            #include "c++14.hpp"
        #elif __cplusplus >= 201103L // c++11 and above
            #include "c++11.hpp"
        #else
            #include "fallback.hpp"
        #endif

    #endif

    // keywords & values common to every c++ version
    #include "common.hpp" // _thread_local, _restrict, _alignas, std::hardware_*_interference_size

    // BACKWARD_COMPATIBILITY_WARNING
    /* --- Different Messages ---
     * legacy -> Only kept for backward compatibility; no removal in sight ._.
     * migration -> Retained for backward compatibility; will be removed in a future version (estimated: ~vx.x.x)
     * GNU syntax used: works in every c++ version ([[deprecated]] only for the other compilers, c++14)
    */
    #if defined(BACKWARD_COMPATIBILITY_WARNING) && !defined(NO_BACKWARD_COMPATIBILITY_WARNING) && defined(__GNUC__)
        #define _legacy                         __attribute__((deprecated("Only kept for backward compatibility; no removal in sight ._.")))
        #define _migration(major, minor, fix)   __attribute__((deprecated("Retained for backward compatibility; will be removed in a future version (estimated: ~v" #major "." #minor "." #fix ")")))
    #elif defined(BACKWARD_COMPATIBILITY_WARNING) && !defined(NO_BACKWARD_COMPATIBILITY_WARNING)
        #define _legacy                         [[deprecated("Only kept for backward compatibility; no removal in sight ._.")]]
        #define _migration(major, minor, fix)   [[deprecated("Retained for backward compatibility; will be removed in a future version (estimated: ~v" #major "." #minor "." #fix ")")]]
    #else
        #define _legacy                         // Not defined with this flag / compiler
        #define _migration(major, minor, fix)   // Not defined with this flag / compiler
    #endif

#endif /* ATTRIBUTE_CPP_H */
