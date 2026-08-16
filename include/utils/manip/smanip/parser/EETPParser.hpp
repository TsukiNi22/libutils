/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/08/2026 by @author Tsukini

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
    #include "../../../attribute/Attribute.hpp"                 // _cold, _nodiscard
    #include "../codec/ICodec.hpp"                              // utils::smanip::codec::ICodec
    #include "../codec/Base64Codec.hpp"                         // utils::smanip::codec::Base64Codec
    #include "../../../exception/ExceptionDefine.hpp"           // utils::exception::InternalCode::*
    #include "../../../exception/basic/ErrorException.hpp"      // utils::exception::ErrorException
    #include "../../../security/encryption/CommonRSAKey.hpp"    // utils::security::encryption::CommonRSAKey
    #include "../../../security/encryption/RSAKey.hpp"          // utils::security::encryption::RSAKey
    #include "../../../security/encryption/AESKey.hpp"          // utils::security::encryption::AESKey
    #include "AParser.hpp"                                      // utils::smanip::parser::AParser
    #include <cstddef>                                          // std::size_t
    #include <cstdint>                                          // std::uint16_t
    #include <memory>                                           // std::unique_ptr, std::make_unique
    #include <vector>                                           // std::vector
    #include <string>                                           // std::string

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
        utils::security::encryption::AESKey _AESKey;

        /* id data */
        std::unordered_map<std::string, utils::security::encryption::RSAKey> _RSAKeys; // Class
        std::unordered_map<std::string, utils::security::encryption::KeyAES> _KeysAES; // Storage

    public:
        // ---------- Pre-Function -------- //
        std::string format(std::string id, utils::smanip::parser::EETPContent content) final;
        utils::smanip::parser::EETPContent parse(std::string id, std::string s) final;

        // ------------ Function ---------- //
        _cold void setCodec(std::unique_ptr<utils::smanip::codec::ICodec> codec) {this->_codec = std::move(codec);}; // default: Base64Codec
        _cold void setTypeSize(std::size_t typeSize) { // default: 1 (0-127)
            if (typeSize < 1)
                throw utils::exception::ErrorException(utils::exception::InternalCode::Parser, "The <type> should always be at least one char");
            this->_typeSize = typeSize;
        };
        _cold _nodiscard inline bool hasIdOverload(void) const final {return true;};

        // ------------ Operator ---------- //
        EETPParser& operator=(const EETPParser& other) = delete;
        EETPParser& operator=(EETPParser&& other) = delete;

        // ---------- Constructor --------- //
        EETPParser() {this->_commonKey.loadCommon();};
        EETPParser(std::unique_ptr<utils::smanip::codec::ICodec> codec, std::size_t typeSize = 1): _codec{std::move(codec)}, _typeSize{typeSize} {this->_commonKey.loadCommon();};
        EETPParser(const EETPParser& other) = delete;
        EETPParser(EETPParser&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~EETPParser() = default;
};

} // namespace end
#endif /* EETPPARSER_H */
