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
##  @file CApi.hpp

File Description:
##  Handles of the C API of utils::security (usable in c++ to get the object)
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef SECURITY_CAPI_H
    #define SECURITY_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "encryption/CommonRSAKey.hpp"  // utils::security::encryption::CommonRSAKey
    #include "../exception/CApi.hpp"        // utils_handle
    #include "encryption/AESKey.hpp"        // utils::security::encryption::AESKey, utils::security::encryption::KeyAES
    #include "CApi.h"                       // utils_aes_key_t, utils_aes_data_t, utils_rsa_key_t, utils_observer_t
    #include <cstdint>                      // std::uint64_t
    #include <string>                       // std::string

//----------------------------------------------------------------//
/* STRUCT */

/* handles of the C API (the object itself) */
struct utils_aes_key: public utils_handle, public utils::security::encryption::AESKey {};

struct utils_aes_data: public utils_handle {
    utils::security::encryption::KeyAES data;
};

struct utils_rsa_key: public utils_handle, public utils::security::encryption::CommonRSAKey {};

/* instance linked to the notifiers (like utils::security::observer::AObserver, with a name given at runtime) */
struct utils_observer: public utils_handle {
    std::uint64_t id = 0;
    std::string instance;
    bool safe = true;
};

#endif /* SECURITY_CAPI_H */
