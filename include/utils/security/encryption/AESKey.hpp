/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/08/2026 by @author Tsukini

File Name:
##  @file AESKey.hpp

File Description:
##  Declaration of the key used for the AES
\**************************************************************/

#ifndef AESKEY_H
    #define AESKEY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _cold, _nodiscard
    #include "AKey.hpp"                         // utils::security::encryption::AKey
    #include <string>                           // std::string

    //----------------------------------------------------------------//
    /* DEFINE */

    /* AES-256-GCM */
    #define AES_KEY_SIZE 32     // 256 bits
    #define AES_MIN_IV_SIZE 12  // 96 bits (GCM nonce)
    #define AES_TAG_SIZE 16     // 128 bits

namespace utils::security::encryption { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

struct KeyAES {
    std::string AES;
    std::string iv;
    std::string tag;
};

//----------------------------------------------------------------//
/* CLASS */

#if defined(AESKEY_LINKER_WARNING) && !defined(NO_AESKEY_LINKER_WARNING)
    #warning "[LINKER] The -lssl and -lcrypto are needed to use the AESKey [-DNO_AESKEY_LINKER_WARNING]"
#endif
class AESKey: public utils::security::encryption::AKey<utils::security::encryption::KeyAES> {
    public:
        // ---------- Pre-Function -------- //
        std::string encrypt(const std::string& s, utils::security::encryption::KeyAES& key) const final;
        std::string decrypt(const std::string& s, utils::security::encryption::KeyAES& key) const final;

        // ------------ Operator ---------- //
        AESKey& operator=(const AESKey& other) = delete;
        AESKey& operator=(AESKey&& other) = default;

        // ---------- Constructor --------- //
        AESKey() = default;
        AESKey(const AESKey& other) = delete;
        AESKey(AESKey&& other) = default;

        // ----------- Destructor --------- //
        virtual ~AESKey() = default;
};

} // namespace end
#endif /* AESKEY_H */
