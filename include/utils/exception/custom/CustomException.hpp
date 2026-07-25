/**************************************************************\
Edition:
##  @date 25/07/2026 by @author Tsukini

File Name:
##  @file CustomException.hpp

File Description:
##  Exception class used for custom ones
\**************************************************************/

#ifndef CUSTOMEXCEPTION_H
    #define CUSTOMEXCEPTION_H

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

class CustomException: public utils::exception::AException {
    public:
        // ------------ Operator ---------- //
        CustomException& operator=(const CustomException& other) = delete;
        CustomException& operator=(CustomException&& other) = delete;

        // ---------- Constructor --------- //
        _cold CustomException(utils::exception::Type type = utils::exception::Type::None, std::string info = "[None]", std::source_location loc = std::source_location::current()) : AException(loc, type, utils::exception::Code::Undefined, info) {};
        _cold explicit CustomException(utils::exception::Type type = utils::exception::Type::None, utils::exception::Code code = utils::exception::Code::Undefined, std::string info = "[None]", std::source_location loc = std::source_location::current()) : AException(loc, type, code, info) {};
        CustomException(const CustomException& other) = delete;
        CustomException(CustomException&& other) = delete;

        // ----------- Destructor --------- //
        ~CustomException() = default;
};

} // namespace end
#endif /* CUSTOMEXCEPTION_H */
