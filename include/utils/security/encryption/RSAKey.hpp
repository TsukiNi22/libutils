/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 03/08/2026 by @author Tsukini

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
    #include "../../attribute/Attribute.hpp"    // _cold, _nodiscard
    #include "AKey.hpp"                         // utils::security::encryption::AKey
    #include <string>                           // std::string

namespace utils::security::encryption { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

struct KeyPair {
    std::string priv;
    std::string pub;
};

//----------------------------------------------------------------//
/* CLASS */

#ifndef NO_RSAKEY_LINKER_WARNING
    #warning "[LINKER] The -lssl and -lcrypto are needed to use the RSAKey [-DNO_RSAKEY_LINKER_WARNING]"
#endif
class RSAKey: public utils::security::encryption::AKey<utils::security::encryption::KeyPair> {
    private:
        utils::security::encryption::KeyPair _keys;

    public:
        // ---------- Pre-Function -------- //
        void generate(void) final;
        std::string encrypt(const std::string& s) const final;
        std::string decrypt(const std::string& s) const final;

        // ------------ Function ---------- //
        _hot void set(const utils::security::encryption::KeyPair& keys) final {this->_keys = keys;};
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
