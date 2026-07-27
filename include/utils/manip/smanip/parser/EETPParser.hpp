/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 27/07/2026 by @author Tsukini

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
    #include "../../../attribute/Attribute.hpp" // _cold, _nodiscard
    #include "../codec/ICodec.hpp"              // // utils::smanip::codec::ICodec
    #include "../codec/Base64Codec.hpp"         // utils::smanip::codec::Base64Codec
    #include "AParser.hpp"                      // utils::smanip::parser::AParser
    #include <cstddef>                          // std::size_t
    #include <cstdint>                          // std::uint16_t
    #include <memory>                           // std::unique_ptr, std::make_unique
    #include <vector>                           // std::vector
    #include <string>                           // std::string

namespace utils::smanip::parser { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

struct EETPContent {
    std::uint16_t type = 0;
    std::vector<std::string> data;
};

//----------------------------------------------------------------//
/* CLASS */

class EETPParser: public utils::smanip::parser::AParser<utils::smanip::parser::EETPContent> {
    private:
        std::unique_ptr<utils::smanip::codec::ICodec> _codec = std::make_unique<utils::smanip::codec::Base64Codec>();
        std::size_t _codeSize = 1;

    public:
        // ---------- Pre-Function -------- //
        std::string format(std::string id, utils::smanip::parser::EETPContent content) final;
        utils::smanip::parser::EETPContent parse(std::string id, std::string s) final;

        // ------------ Function ---------- //
        _cold void setCodec(std::unique_ptr<utils::smanip::codec::ICodec> codec) {this->_codec = std::move(codec);}; // default: Base64Codec
        _cold void setCodeSize(std::size_t codeSize) {this->_codeSize = codeSize;}; // default: 1 (0-127)
        _cold _nodiscard inline bool hasIdOverload(void) const final {return true;};

        // ------------ Operator ---------- //
        EETPParser& operator=(const EETPParser& other) = delete;
        EETPParser& operator=(EETPParser&& other) = delete;

        // ---------- Constructor --------- //
        EETPParser() = default;
        EETPParser(std::unique_ptr<utils::smanip::codec::ICodec> codec, std::size_t codeSize = 1): _codec{std::move(codec)}, _codeSize{codeSize} {};
        EETPParser(const EETPParser& other) = delete;
        EETPParser(EETPParser&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~EETPParser() = default;
};

} // namespace end
#endif /* EETPPARSER_H */
