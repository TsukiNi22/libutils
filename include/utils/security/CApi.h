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
##  @file CApi.h

File Description:
##  C API of utils::security: AES & RSA keys (encryption) and the observers of the C side (memory leak report) (C & C++ compatible)
##  Every function that can fail returns UTILS_OK / UTILS_KO (or NULL / 0), details in utils_exception
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef SECURITY_CAPI_C_H
    #define SECURITY_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../manip/smanip/CApi.h"   // utils_string_t
    #include "../exception/CApi.h"      // utils_exception, UTILS_OK, UTILS_KO
    #include <stdbool.h>                // bool
    #include <stddef.h>                 // size_t
    #include <stdint.h>                 // uint16_t

    //----------------------------------------------------------------//
    /* DEFINE */

    /* path of the common RSA key (utils/security/encryption/CommonRSAKey.hpp): common (private) | common.pub (public) */
    #define C_API_SECURITY_DEFAULT_COMMON_RSA_PATH "~/.ssh/common"

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* opaque types */
typedef struct utils_aes_key utils_aes_key_t; // utils::security::encryption::AESKey
typedef struct utils_aes_data utils_aes_data_t; // utils::security::encryption::KeyAES (key, iv, tag)
typedef struct utils_rsa_key utils_rsa_key_t; // utils::security::encryption::CommonRSAKey (an RSAKey that can load the common key)
typedef struct utils_observer utils_observer_t; // instance of the C side linked to the notifiers (memory leak report)

//----------------------------------------------------------------//
/* PROTOTYPE */

/* AES key (encrypt: the key & iv of data are used, its tag is set, decrypt: key, iv & tag are used) */
utils_aes_key_t* utils_aes_key_create(void); // NULL on error
void utils_aes_key_destroy(utils_aes_key_t* key); // NULL: nothing
int utils_aes_key_random_bytes(const utils_aes_key_t* key, uint16_t size, utils_string_t* out); // out replaced
int utils_aes_key_encrypt(const utils_aes_key_t* key, const char* in, size_t size, utils_aes_data_t* data, utils_string_t* out); // out replaced
int utils_aes_key_decrypt(const utils_aes_key_t* key, const char* in, size_t size, utils_aes_data_t* data, utils_string_t* out); // out replaced
bool utils_aes_key_has_generate_overload(const utils_aes_key_t* key);
bool utils_aes_key_has_set_overload(const utils_aes_key_t* key);
bool utils_aes_key_has_get_overload(const utils_aes_key_t* key);

/* AES data (bytes: given back valid until data is changed, size: optional) */
utils_aes_data_t* utils_aes_data_create(void); // NULL on error
void utils_aes_data_destroy(utils_aes_data_t* data); // NULL: nothing
int utils_aes_data_set_key(utils_aes_data_t* data, const char* bytes, size_t size); // 32 bytes (AES-256)
int utils_aes_data_set_iv(utils_aes_data_t* data, const char* bytes, size_t size);
int utils_aes_data_set_tag(utils_aes_data_t* data, const char* bytes, size_t size);
const char* utils_aes_data_get_key(const utils_aes_data_t* data, size_t* size); // NULL on error
const char* utils_aes_data_get_iv(const utils_aes_data_t* data, size_t* size); // NULL on error
const char* utils_aes_data_get_tag(const utils_aes_data_t* data, size_t* size); // NULL on error

/* RSA key (keys in PEM, strings given back valid until the key is changed) */
utils_rsa_key_t* utils_rsa_key_create(void); // NULL on error
utils_rsa_key_t* utils_rsa_key_create_common(const char* path); // the common key loaded (path: NULL = C_API_SECURITY_DEFAULT_COMMON_RSA_PATH), NULL on error
void utils_rsa_key_destroy(utils_rsa_key_t* key); // NULL: nothing
int utils_rsa_key_load_common(utils_rsa_key_t* key, const char* path); // path: NULL = C_API_SECURITY_DEFAULT_COMMON_RSA_PATH
int utils_rsa_key_generate(utils_rsa_key_t* key);
int utils_rsa_key_set(utils_rsa_key_t* key, const char* private_key, const char* public_key); // NULL: ""
const char* utils_rsa_key_get_private(const utils_rsa_key_t* key); // NULL on error
const char* utils_rsa_key_get_public(const utils_rsa_key_t* key); // NULL on error
int utils_rsa_key_random_bytes(const utils_rsa_key_t* key, uint16_t size, utils_string_t* out); // out replaced
int utils_rsa_key_encrypt(const utils_rsa_key_t* key, const char* in, size_t size, utils_string_t* out); // with the public key, out replaced
int utils_rsa_key_decrypt(const utils_rsa_key_t* key, const char* in, size_t size, utils_string_t* out); // with the private key, out replaced
bool utils_rsa_key_has_generate_overload(const utils_rsa_key_t* key);
bool utils_rsa_key_has_set_overload(const utils_rsa_key_t* key);
bool utils_rsa_key_has_get_overload(const utils_rsa_key_t* key);

/* observers (an observer not destroyed is reported as a leak by the memory leak notifier, safe_mode: thread safe) */
utils_observer_t* utils_observer_create(const char* instance, bool safe_mode); // linked to the notifiers, NULL on error
void utils_observer_destroy(utils_observer_t* observer); // NULL: nothing (unlinked)
int utils_observer_trigger(void); // force every notifier to check its status now (memory leak: report the instances still linked)

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* SECURITY_CAPI_C_H */
