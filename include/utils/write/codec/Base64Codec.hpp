/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/07/2026 by @author Tsukini

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
    #include "ICodec.hpp"   // utils::write::codec::ICodec
    #include <string>       // std::string

namespace utils::write::codec { // namespace start
//----------------------------------------------------------------//
/* CLASS */

#ifndef NO_LINKER_WARNING
    #warning "The -lssl and -lcrypto are needed to use the Base64Codec"
#endif
class Base64Codec: public utils::write::codec::ICodec {
    public:
        // ------------ Function ---------- //
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
