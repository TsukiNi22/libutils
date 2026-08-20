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
##  @file ICodec.hpp

File Description:
##  Declaration of the interface used for different codec (base64, ...)
\**************************************************************/

#ifndef ICODEC_H
    #define ICODEC_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../../security/observer/Observer.hpp"  // utils::security::observer::Observer
    #include <string>                                   // std::string

namespace utils::smanip::codec { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ICodec: private utils::security::observer::Observer<"ICodec"> {
    public:
        // ---------- Pre-Function -------- //
        // Could also be declared as 'static' but doesn't support polymorphism
        virtual std::string encode(std::string s) const = 0;
        virtual std::string decode(std::string s) const = 0;

        // ------------ Operator ---------- //
        ICodec& operator=(const ICodec& other) = default;
        ICodec& operator=(ICodec&& other) = default;

        // ---------- Constructor --------- //
        ICodec() = default;
        ICodec(const ICodec& other) = default;
        ICodec(ICodec&& other) = default;

        // ----------- Destructor --------- //
        virtual ~ICodec() = default;
};

} // namespace end
#endif /* ICODEC_H */
