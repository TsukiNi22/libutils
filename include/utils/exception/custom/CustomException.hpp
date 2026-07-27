/**************************************************************\
Edition:
##  @date 28/07/2026 by @author Tsukini

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
    #include "../ExceptionDefine.hpp"           // utils::exception::InternalCode, utils::exception::Type
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
        #ifdef GENERATED_EXCEPTION_HEADER_H
            _cold explicit CustomException(utils::exception::Type type, utils::exception::Code code, std::string info = "[None]", std::source_location loc = std::source_location::current()) : AException(loc, type, static_cast<utils::exception::InternalCode>(code), info) {};
        #endif
        _cold explicit CustomException(utils::exception::Type type = utils::exception::Type::None, utils::exception::InternalCode code = utils::exception::InternalCode::Undefined, std::string info = "[None]", std::source_location loc = std::source_location::current()) : AException(loc, type, code, info) {};
        _cold CustomException(utils::exception::Type type = utils::exception::Type::None, std::string info = "[None]", std::source_location loc = std::source_location::current()) : AException(loc, type, utils::exception::InternalCode::Undefined, info) {};
        CustomException(const CustomException& other) = delete;
        CustomException(CustomException&& other) = delete;

        // ----------- Destructor --------- //
        ~CustomException() = default;
};

} // namespace end
#endif /* CUSTOMEXCEPTION_H */
