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
##  @file RSAKey.hpp

File Description:
##  Declaration of the key used for the RSA
\**************************************************************/

#ifndef RSAKEY_H
    #define RSAKEY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../../attribute/Attribute.hpp" // _cold, _nodiscard
    #include "AKey.hpp"                         // utils::smanip::key::AKey
    #include <string>                           // std::string

namespace utils::smanip::key { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

struct KeyPair {
    std::string priv;
    std::string pub;
};

//----------------------------------------------------------------//
/* CLASS */

class RSAKey: public utils::smanip::key::AKey<utils::smanip::key::KeyPair> {
    private:
        utils::smanip::key::KeyPair _keys;

    public:
        // ---------- Pre-Function -------- //
        void generate(void) final;
        std::string encrypt(const std::string& s) const final;
        std::string decrypt(const std::string& s) const final;

        // ------------ Function ---------- //
        _hot void set(const utils::smanip::key::KeyPair& keys) final {this->_keys = keys;};
        _cold _nodiscard bool hasGenerateOverload(void) const final {return true;};
        _cold _nodiscard bool hasSetOverload(void) const final {return true;};

        // ------------ Operator ---------- //
        RSAKey& operator=(const RSAKey& other) = delete;
        RSAKey& operator=(RSAKey&& other) = delete;

        // ---------- Constructor --------- //
        RSAKey() = default;
        RSAKey(const RSAKey& other) = delete;
        RSAKey(RSAKey&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~RSAKey() = default;
};

} // namespace end
#endif /* RSAKEY_H */
