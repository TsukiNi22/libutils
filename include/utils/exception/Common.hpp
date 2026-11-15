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
##  @file Common.hpp

File Description:
##  Defines common to the c++ exceptions and the C API (C & C++ compatible)
\**************************************************************/

#ifndef EXCEPTION_COMMON_H
    #define EXCEPTION_COMMON_H

    //----------------------------------------------------------------//
    /* DEFINE */

    /* Definition of the different return status (always defined, no collision) */
    #define UTILS_OK 0  // Valid
    #define UTILS_KO -1 // Error

    /* Short names (only if not already defined by something else, _UTILS_*_DEFINED: defined by libutils) */
    #ifndef OK // Valid
        #define _UTILS_OK_DEFINED
        #define OK UTILS_OK
    #elif OK != UTILS_OK
        #warning "libutils: OK is already defined with another value (use UTILS_OK)"
    #endif
    #ifndef KO // Error
        #define _UTILS_KO_DEFINED
        #define KO UTILS_KO
    #elif KO != UTILS_KO
        #warning "libutils: KO is already defined with another value (use UTILS_KO)"
    #endif

#endif /* EXCEPTION_COMMON_H */
