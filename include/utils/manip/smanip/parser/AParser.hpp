/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 25/07/2026 by @author Tsukini

File Name:
##  @file AParser.hpp

File Description:
##  Declaration of the interface used for different parser (base64, ...)
\**************************************************************/

#ifndef APARSER_H
    #define APARSER_H

    //----------------------------------------------------------------//
    /* ANCLUDE */

    /* type */
    #include "../../../exception/ExceptionDefine.hpp"       // utils::exception::Code::*
    #include "../../../exception/custom/FatalException.hpp" // utils::exception::FatalException
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
        std::string format(T content) const override
        {throw utils::exception::FatalException(utils::exception::Code::UndefinedCall);};
        T parse(std::string s) const override
        {throw utils::exception::FatalException(utils::exception::Code::UndefinedCall);};
        std::string format(std::string id, T content) override
        {throw utils::exception::FatalException(utils::exception::Code::UndefinedCall);};
        T parse(std::string id, std::string s) override
        {throw utils::exception::FatalException(utils::exception::Code::UndefinedCall);};

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
