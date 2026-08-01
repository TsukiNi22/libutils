/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/08/2026 by @author Tsukini

File Name:
##  @file IParser.hpp

File Description:
##  Declaration of the interface used for different parser (2etp, ...)
\**************************************************************/

#ifndef IPARSER_H
    #define IPARSER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../../security/observer/Observer.hpp"  // utils::security::observer::Observer
    #include <string>                                   // std::string

namespace utils::smanip::parser { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class IParser: private utils::security::observer::Observer<"IParser"> {
    public:
        // ---------- Pre-Function -------- //
        virtual std::string format(T content) const = 0;
        virtual T parse(std::string s) const = 0;
        // For potential need of identification / individual storage
        virtual std::string format(std::string id, T content) = 0;
        virtual T parse(std::string id, std::string s) = 0;
        virtual bool hasIdOverload(void) const = 0;
        virtual bool hasNoIdOverload(void) const = 0;

        // ------------ Operator ---------- //
        IParser& operator=(const IParser& other) = delete;
        IParser& operator=(IParser&& other) = delete;

        // ---------- Constructor --------- //
        IParser() = default;
        IParser(const IParser& other) = delete;
        IParser(IParser&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~IParser() = default;
};

} // namespace end
#endif /* IPARSER_H */
