/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 26/07/2026 by @author Tsukini

File Name:
##  @file AParser.hpp

File Description:
##  Declaration of the abstract used for different parser (2etp, ...)
\**************************************************************/

#ifndef APARSER_H
    #define APARSER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../../exception/ExceptionDefine.hpp"       // utils::exception::Code::*
    #include "../../../exception/custom/FatalException.hpp" // utils::exception::FatalException
    #include "../../../attribute/Attribute.hpp"             // _cold, _nodiscard, _unused
    #include "IParser.hpp"                                  // utils::smanip::parser::IParser
    #include <string>                                       // std::string

namespace utils::smanip::parser { // namespace start
//----------------------------------------------------------------//
/* CLASS */

// Definition of the AParser in case of no given definition for the one with or without 'id'
template<typename T>
class AParser: public utils::smanip::parser::IParser<T> {
    public:
        // ------------ Function ---------- //
        _cold std::string format(_unused T content) const override
        {throw utils::exception::FatalException(utils::exception::Code::UndefinedCall);};
        _cold T parse(_unused std::string s) const override
        {throw utils::exception::FatalException(utils::exception::Code::UndefinedCall);};
        _cold std::string format(_unused std::string id, _unused T content) override
        {throw utils::exception::FatalException(utils::exception::Code::UndefinedCall);};
        _cold T parse(_unused std::string id, _unused std::string s) override
        {throw utils::exception::FatalException(utils::exception::Code::UndefinedCall);};
        _cold _nodiscard inline bool hasIdOverload(void) const override {return false;};
        _cold _nodiscard inline bool hasNoIdOverload(void) const override {return false;};

        // ------------ Operator ---------- //
        AParser& operator=(const AParser& other) = delete;
        AParser& operator=(AParser&& other) = delete;

        // ---------- Constructor --------- //
        AParser() = default;
        AParser(const AParser& other) = delete;
        AParser(AParser&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~AParser() = default;
};

} // namespace end
#endif /* APARSER_H */
