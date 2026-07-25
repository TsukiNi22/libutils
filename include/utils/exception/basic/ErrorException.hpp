/**************************************************************\
Edition:
##  @date 25/07/2026 by @author Tsukini

File Name:
##  @file ErrorException.hpp

File Description:
##  Exception class used for Error
\**************************************************************/

#ifndef ERROREXCEPTION_H
    #define ERROREXCEPTION_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../ExceptionDefine.hpp"           // utils::exception::Code, utils::exception::Type
    #include "../AException.hpp"                // utils::exception::AException
    #include "../../attribute/Attribute.hpp"    // _cold
    #include <source_location>                  // std::source_location

namespace utils::exception { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ErrorException: public utils::exception::AException {
    public:
        // ------------ Operator ---------- //
        ErrorException& operator=(const ErrorException& other) = delete;
        ErrorException& operator=(ErrorException&& other) = delete;

        // ---------- Constructor --------- //
        _cold explicit ErrorException(utils::exception::Code code = utils::exception::Code::Undefined, std::source_location loc = std::source_location::current()) : AException(loc, utils::exception::Type::Error, code) {};
        _cold ErrorException(utils::exception::Code code, std::string info, std::source_location loc = std::source_location::current()) : AException(loc, utils::exception::Type::Error, code, info) {};
        ErrorException(const ErrorException& other) = delete;
        ErrorException(ErrorException&& other) = delete;

        // ----------- Destructor --------- //
        ~ErrorException() = default;
};

} // namespace end
#endif /* ERROREXCEPTION_H */
