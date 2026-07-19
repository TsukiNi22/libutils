/**************************************************************\
Edition:
##  @date 06/07/2026 by @author Tsukini

File Name:
##  @file WarningException.hpp

File Description:
##  Exception class used for Warning
\**************************************************************/

#ifndef WARNINGEXCEPTION_H
    #define WARNINGEXCEPTION_H

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

class WarningException: public utils::exception::AException {
    public:
        // ------------ Operator ---------- //
        WarningException& operator=(const WarningException& other) = delete;
        WarningException& operator=(WarningException&& other) = delete;

        // ---------- Constructor --------- //
        _cold explicit WarningException(utils::exception::Code code = utils::exception::Code::Undefined, std::source_location loc = std::source_location::current()) : AException(loc, utils::exception::Type::Warning, code) {};
        _cold WarningException(utils::exception::Code code, std::string info, std::source_location loc = std::source_location::current()) : AException(loc, utils::exception::Type::Warning, code, info) {};
        WarningException(const WarningException& other) = delete;
        WarningException(WarningException&& other) = delete;

        // ----------- Destructor --------- //
        ~WarningException() = default;
};

} // namespace end
#endif /* WARNINGEXCEPTION_H */
