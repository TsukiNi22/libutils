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
##  @file AKey.hpp

File Description:
##  Declaration of the abstract used for different key (RSA, AES, ...)
\**************************************************************/

#ifndef AKEY_H
    #define AKEY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../exception/ExceptionDefine.hpp"          // utils::exception::InternalCode::*
    #include "../../exception/basic/ErrorException.hpp"     // utils::exception::ErrorException
    #include "../../exception/custom/FatalException.hpp"    // utils::exception::FatalException
    #include "../../attribute/Attribute.hpp"                // _cold, _hot, _nodiscard, _unused, _unlikely, _migration
    #include "IKey.hpp"                                     // utils::security::encryption::IKey
    #include <openssl/rand.h>                               // RAND_bytes
    #include <cstdint>                                      // std::uint16_t, std::uint8_t
    #include <vector>                                       // std::vector
    #include <string>                                       // std::string

namespace utils::security::encryption { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* tools */
_hot _nodiscard inline std::string key_to_string(const std::vector<std::uint8_t>& data) {return std::string(data.begin(), data.end());};
_hot _nodiscard inline std::vector<std::uint8_t> string_to_key(const std::string& s)    {return std::vector<std::uint8_t>(s.begin(), s.end());};

//----------------------------------------------------------------//
/* CLASS */

// Definition of the AKey in case of no given definition for the one with or without 'key'
template<typename T>
class AKey: public utils::security::encryption::IKey<T> {
    public:
        // ------------ Function ---------- //
        /* tools */
        _hot _nodiscard std::string generateRandomBytes(std::uint16_t size) const final
        {
            std::vector<std::uint8_t> bytes(size);
            if (!RAND_bytes(bytes.data(), static_cast<int>(bytes.size()))) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Encryption, "Failed to generate random bytes");
            }
            return std::string(bytes.begin(), bytes.end());
        };

        /* default definition */
        _cold void generate(void) override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        _cold void set(_unused const T& data) override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        _cold const T& get(void) const override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        _cold std::string encrypt(_unused const std::string& s) const override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        _cold std::string decrypt(_unused const std::string& s) const override
        {throw utils::exception::FatalException(utils::exception::InternalCode::UndefinedCall);};
        // Auto fallback on simple encrypt/decrypt methods
        _cold std::string encrypt(const std::string& s, _unused T& data) const override {return this->encrypt(s);};
        _cold std::string decrypt(const std::string& s, _unused T& data) const override {return this->decrypt(s);};
        _cold _nodiscard bool hasGenerateOverload(void) const override                  {return false;};
        _cold _nodiscard bool hasSetOverload(void) const override                       {return false;};
        _cold _nodiscard bool hasGetOverload(void) const override                       {return false;};

        // ------------ Operator ---------- //
        AKey& operator=(const AKey& other) = default;
        AKey& operator=(AKey&& other) = default;

        // ---------- Constructor --------- //
        AKey() = default;
        AKey(const AKey& other) = default;
        AKey(AKey&& other) = default;

        // ----------- Destructor --------- //
        virtual ~AKey() = default;
};

} // namespace end

//----------------------------------------------------------------//
/* MIGRATION */
namespace utils::security::encryption {
    _migration(4, 0, 0) inline std::string keyToString(const std::vector<std::uint8_t>& data) {return utils::security::encryption::key_to_string(data);};
    _migration(4, 0, 0) inline std::vector<std::uint8_t> stringToKey(const std::string& s)    {return utils::security::encryption::string_to_key(s);};
}

#endif /* AKEY_H */
