/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 16/11/2026 by @author Tsukini

File Name:
##  @file CApi.hpp

File Description:
##  Macros of the C API of utils::iomanip (expanded in the .cpp)
\**************************************************************/

#ifndef IOMANIP_CAPI_H
    #define IOMANIP_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../exception/CApi.hpp" // utils::exception::c_api::guard_fallback
    #include "CApi.h"                   // utils_iomanip_*
    #include <string>                   // std::string

    //----------------------------------------------------------------//
    /* MACRO */

    /* static sequence (built once, always valid) */
    #define C_API_STATIC_SEQUENCE(call) utils::exception::c_api::guard_fallback(nullptr, [](void) -> const char* {static const std::string seq = (call); return seq.c_str();})

#endif /* IOMANIP_CAPI_H */
