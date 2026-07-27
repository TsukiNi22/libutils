/**************************************************************\
Edition:
##  @date 28/07/2026 by @author Tsukini

File Name:
##  @file FatalException.hpp

File Description:
##  Exception class used for custom ones
\**************************************************************/

#ifndef FATALEXCEPTION_H
    #define FATALEXCEPTION_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../IException.hpp"                // utils::exception::IException
    #include "../AException.hpp"                // utils::exception::AException
    #include "../../attribute/Attribute.hpp"    // _cold
    #include <source_location>                  // std::source_location
    #include <iostream>                         // std::cerr
    #include <cstdlib>                          // std::abort
    
    //----------------------------------------------------------------//
    /* DEFINE */

    /* message*/
    #define EXCEPTION_ABORTED_HEADER  "[ABORTED] FatalException call"
    #define EXCEPTION_ABORTED_MESSAGE "Program terminated without proper RAII cleanup"

namespace utils::exception { // namespace start
//----------------------------------------------------------------//
/* CLASS */

#ifndef NO_USAGE_WARNING
    #warning "The FatalException can't be catch"
#endif
class FatalException: public utils::exception::AException {
    public:
        // ------------ Operator ---------- //
        FatalException& operator=(const FatalException& other) = delete;
        FatalException& operator=(FatalException&& other) = delete;

        // ---------- Constructor --------- //
        #ifdef GENERATED_EXCEPTION_HEADER_H
            _cold explicit FatalException(utils::exception::Code code, std::source_location loc = std::source_location::current()) noexcept : AException(loc, utils::exception::Type::Error | utils::exception::Type::Fatal, static_cast<utils::exception::InternalCode>(code))
            {
                std::cerr
                << EXCEPTION_ABORTED_HEADER  << std::endl
                << this->formated()          << std::endl
                << EXCEPTION_ABORTED_MESSAGE << std::endl;
                std::abort();
            };
            _cold FatalException(utils::exception::Type type, utils::exception::Code code, std::string info = "[None]", std::source_location loc = std::source_location::current()) noexcept : AException(loc, type | utils::exception::Type::Fatal, static_cast<utils::exception::InternalCode>(code), info)
            {
                std::cerr
                << EXCEPTION_ABORTED_HEADER  << std::endl
                << this->formated()          << std::endl
                << EXCEPTION_ABORTED_MESSAGE << std::endl;
                std::abort();
            };
        #endif
        _cold explicit FatalException(utils::exception::InternalCode code = utils::exception::InternalCode::Undefined, std::source_location loc = std::source_location::current()) noexcept : AException(loc, utils::exception::Type::Error | utils::exception::Type::Fatal, code)
        {
            std::cerr
            << EXCEPTION_ABORTED_HEADER  << std::endl
            << this->formated()          << std::endl
            << EXCEPTION_ABORTED_MESSAGE << std::endl;
            std::abort();
        };
        _cold FatalException(utils::exception::Type type = utils::exception::Type::None, utils::exception::InternalCode code = utils::exception::InternalCode::Undefined, std::string info = "[None]", std::source_location loc = std::source_location::current()) noexcept : AException(loc, type | utils::exception::Type::Fatal, code, info)
        {
            std::cerr
            << EXCEPTION_ABORTED_HEADER  << std::endl
            << this->formated()          << std::endl
            << EXCEPTION_ABORTED_MESSAGE << std::endl;
            std::abort();
        };
        _cold FatalException(const utils::exception::IException& e) noexcept : AException()
        {
            std::cerr
            << EXCEPTION_ABORTED_HEADER  << std::endl
            << e.formated()              << std::endl
            << EXCEPTION_ABORTED_MESSAGE << std::endl;
            std::abort();
        };
        FatalException(const FatalException& other) = delete;
        FatalException(FatalException&& other) = delete;

        // ----------- Destructor --------- //
        ~FatalException() = default;
};

} // namespace end
#endif /* FATALEXCEPTION_H */
