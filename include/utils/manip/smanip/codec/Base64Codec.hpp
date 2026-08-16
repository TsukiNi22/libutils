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
##  @file Base64Codec.hpp

File Description:
##  Definition of the base 64 codec
\**************************************************************/

#ifndef BASE64CODEC_H
    #define BASE64CODEC_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "ICodec.hpp"   // utils::smanip::codec::ICodec
    #include <string>       // std::string

namespace utils::smanip::codec { // namespace start
//----------------------------------------------------------------//
/* CLASS */

#if defined(BASE64CODEC_LINKER_WARNING) && !defined(NO_BASE64CODEC_LINKER_WARNING)
    #warning "[LINKER] The -lssl and -lcrypto are needed to use the Base64Codec [-DNO_BASE64CODEC_LINKER_WARNING]"
#endif
class Base64Codec: public utils::smanip::codec::ICodec {
    public:
        // ---------- Pre-Function -------- //
        std::string encode(std::string s) const;
        std::string decode(std::string s) const;

        // ------------ Operator ---------- //
        Base64Codec& operator=(const Base64Codec& other) = delete;
        Base64Codec& operator=(Base64Codec&& other) = delete;

        // ---------- Constructor --------- //
        Base64Codec() = default;
        Base64Codec(const Base64Codec& other) = delete;
        Base64Codec(Base64Codec&& other) = delete;

        // ----------- Destructor --------- //
        ~Base64Codec() = default;
};

} // namespace end
#endif /* BASE64CODEC_H */
