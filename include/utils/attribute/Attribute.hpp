/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 14/11/2026 by @author Tsukini

File Name:
##  @file Attribute.hpp

File Description:
##  Redirection for the c or c++ attribute
\**************************************************************/

// xstyle: ignore-file CPP-INCLUDE-COMMENT (dispatch includes: the #if / comment above says what is included)

#ifndef ATTRIBUTE_H
    #define ATTRIBUTE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    #if defined(__cplusplus)
        #include "c++.hpp"
    #elif defined(__STDC__) || defined(__STDC_VERSION__)
        #include "c.h"
    #else // neither a C nor a C++ compiler
        #include "fallback.hpp"
    #endif

#endif /* ATTRIBUTE_H */
