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
##  @file AESKey.hpp

File Description:
##  Declaration of the key used for the AES
\**************************************************************/

#ifndef AESKEY_H
    #define AESKEY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../../attribute/Attribute.hpp" // _cold, _nodiscard
    #include "AKey.hpp"                         // utils::smanip::key::AKey
    #include <string>                           // std::string

namespace utils::smanip::key { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

struct KeyAES {
    std::string AES;
    std::string iv;
    std::string tag;
};

//----------------------------------------------------------------//
/* CLASS */

class AESKey: public utils::smanip::key::AKey<utils::smanip::key::KeyAES> {
    public:
        // ---------- Pre-Function -------- //
        std::string encrypt(const std::string& s, utils::smanip::key::KeyAES& key) const final;
        std::string decrypt(const std::string& s, utils::smanip::key::KeyAES& key) const final;

        // ------------ Operator ---------- //
        AESKey& operator=(const AESKey& other) = delete;
        AESKey& operator=(AESKey&& other) = delete;

        // ---------- Constructor --------- //
        AESKey() = default;
        AESKey(const AESKey& other) = delete;
        AESKey(AESKey&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~AESKey() = default;
};

} // namespace end
#endif /* AESKEY_H */
