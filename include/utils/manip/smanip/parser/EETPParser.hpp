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
##  @file EETPParser.hpp

File Description:
##  Declaration of the parser used for the 2etp protocol
\**************************************************************/

#ifndef EETPPARSER_H
    #define EETPPARSER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../../attribute/Attribute.hpp"                 // _cold, _nodiscard, _unlikely
    #include "../codec/ICodec.hpp"                              // utils::smanip::codec::ICodec
    #include "../codec/Base64Codec.hpp"                         // utils::smanip::codec::Base64Codec
    #include "../../../exception/ExceptionDefine.hpp"           // utils::exception::InternalCode::*
    #include "../../../exception/basic/ErrorException.hpp"      // utils::exception::ErrorException
    #include "../../../security/encryption/CommonRSAKey.hpp"    // utils::security::encryption::CommonRSAKey
    #include "../../../security/encryption/RSAKey.hpp"          // utils::security::encryption::RSAKey
    #include "../../../security/encryption/AESKey.hpp"          // utils::security::encryption::AESKey
    #include "AParser.hpp"                                      // utils::smanip::parser::AParser
    #include <unordered_map>                                    // std::unordered_map
    #include <cstddef>                                          // std::size_t
    #include <cstdint>                                          // std::uint16_t
    #include <utility>                                          // std::move
    #include <memory>                                           // std::unique_ptr, std::make_unique
    #include <vector>                                           // std::vector
    #include <string>                                           // std::string

    //----------------------------------------------------------------//
    /* DEFINE */

    /* AES */
    #define EETP_AES_KEY_SIZE 32 // bytes (AES-256)
    #define EETP_AES_IV_SIZE 12  // bytes (GCM nonce)
    #define EETP_AES_TAG_SIZE 16 // bytes (GCM tag)

namespace utils::smanip::parser { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

struct EETPContent {
    std::string type;
    std::vector<std::string> data;
};

//----------------------------------------------------------------//
/* CLASS */

#if defined(EETPPARSER_USAGE_WARNING) && !defined(NO_EETPPARSER_USAGE_WARNING)
    #warning "[USAGE] Custom ICodec implementations must guarantee that ETB (0x17) and EOT (0x04) never appear in their encoded output, as these bytes are reserved for protocol framing [-DNO_EETPPARSER_USAGE_WARNING]"
#endif
class EETPParser: public utils::smanip::parser::AParser<utils::smanip::parser::EETPContent> {
    private:
        /* global data */
        std::unique_ptr<utils::smanip::codec::ICodec> _codec = std::make_unique<utils::smanip::codec::Base64Codec>();
        std::size_t _typeSize = 1;
        utils::security::encryption::CommonRSAKey _commonKey;
        utils::security::encryption::AESKey _aesKey;

        /* id data */
        std::unordered_map<std::string, utils::security::encryption::RSAKey> _rsaKeys; // Class
        std::unordered_map<std::string, utils::security::encryption::KeyAES> _aesKeys; // Storage

    public:
        // ---------- Pre-Function -------- //
        std::string format(std::string id, utils::smanip::parser::EETPContent content) final;
        utils::smanip::parser::EETPContent parse(std::string id, std::string s) final;

        // ------------ Function ---------- //
        _cold void setCodec(std::unique_ptr<utils::smanip::codec::ICodec> codec) // default: Base64Codec
        {
            if (!codec) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidCodec, "The codec can't be null");
            }
            this->_codec = std::move(codec);
        };
        _cold void setTypeSize(std::size_t typeSize) // default: 1 (0-127)
        {
            if (typeSize < 1) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "The <type> should always be at least one char");
            }
            this->_typeSize = typeSize;
        };
        _cold _nodiscard inline bool hasIdOverload(void) const final {return true;};

        // ------------ Operator ---------- //
        EETPParser& operator=(const EETPParser& other) = delete;
        EETPParser& operator=(EETPParser&& other) = default;

        // ---------- Constructor --------- //
        EETPParser()                                                                              {this->_commonKey.loadCommon();};
        EETPParser(std::unique_ptr<utils::smanip::codec::ICodec> codec, std::size_t typeSize = 1) {this->setCodec(std::move(codec)); this->setTypeSize(typeSize); this->_commonKey.loadCommon();};
        EETPParser(const EETPParser& other) = delete;
        EETPParser(EETPParser&& other) = default;

        // ----------- Destructor --------- //
        virtual ~EETPParser() = default;
};

} // namespace end
#endif /* EETPPARSER_H */
