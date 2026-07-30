/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 30/07/2026 by @author Tsukini

File Name:
##  @file Key.hpp

File Description:
##  Include for all the different key
\**************************************************************/

#ifndef KEY_H
    #define KEY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* interface */
    #include "IKey.hpp" // utils::security::encryption::IKey

    /* keys */
    #include "CommonRSAKey.hpp" // utils::security::encryption::CommonRSAKey
    #include "RSAKey.hpp"       // utils::security::encryption::RSAKey
    #include "AESKey.hpp"       // utils::security::encryption::AESKey

#endif /* KEY_H */
