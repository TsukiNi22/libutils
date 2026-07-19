/**************************************************************\
Edition:
##  @date 06/07/2026 by @author Tsukini

File Name:
##  @file NoneException.hpp

File Description:
##  Exception class used for None (like in simple exit)
\**************************************************************/

#ifndef NONEEXCEPTION_H
    #define NONEEXCEPTION_H

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

class NoneException: public utils::exception::AException {
    public:
        // ------------ Operator ---------- //
        NoneException& operator=(const NoneException& other) = delete;
        NoneException& operator=(NoneException&& other) = delete;

        // ---------- Constructor --------- //
        _cold explicit NoneException(utils::exception::Code code = utils::exception::Code::Undefined, std::source_location loc = std::source_location::current()) : AException(loc, utils::exception::Type::None, code) {};
        _cold NoneException(utils::exception::Code code, std::string info, std::source_location loc = std::source_location::current()) : AException(loc, utils::exception::Type::None, code, info) {};
        NoneException(const NoneException& other) = delete;
        NoneException(NoneException&& other) = delete;

        // ----------- Destructor --------- //
        ~NoneException() = default;
};

} // namespace end
#endif /* NONEEXCEPTION_H */
