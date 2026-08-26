/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/08/2026 by @author Tsukini

File Name:
##  @file Middlewares.hpp

File Description:
##  Global middlewares include
\**************************************************************/

#ifndef MIDDLEWARES_H
    #define MIDDLEWARES_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "MiddlewaresType.hpp"          // utils::pool::Middleware<...>
    #include "Middlewares_t-t.hpp"          // utils::pool::Middlewares<T, U>
    #include "Middlewares_t-void.hpp"       // utils::pool::Middlewares<T, void>
    #include "Middlewares_void-t.hpp"       // utils::pool::Middlewares<void, T>
    #include "Middlewares_void-void.hpp"    // utils::pool::Middlewares<void, void>

#endif /* MIDDLEWARES_H */
