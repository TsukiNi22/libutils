/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/07/2026 by @author Tsukini

File Name:
##  @file ICodec.hpp

File Description:
##  Declaration of the interface used for different codec (base64, ...)
\**************************************************************/

#ifndef ICODEC_H
    #define ICODEC_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <string>   // std::string

namespace utils::iomanip::codec { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ICodec {
    public:
        // ------------ Function ---------- //
        // Could also be declared as 'static' but doesn't support polymorphism
        virtual std::string encode(std::string s) const = 0;
        virtual std::string decode(std::string s) const = 0;

        // ------------ Operator ---------- //
        ICodec& operator=(const ICodec& other) = delete;
        ICodec& operator=(ICodec&& other) = delete;

        // ---------- Constructor --------- //
        ICodec() = default;
        ICodec(const ICodec& other) = delete;
        ICodec(ICodec&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~ICodec() = default;
};

} // namespace end
#endif /* ICODEC_H */
