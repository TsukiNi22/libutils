/**************************************************************\
Edition:
##  @date 06/07/2026 by @author Tsukini

File Name:
##  @file IException.hpp

File Description:
##  Interface for the cutomized exception handling
\**************************************************************/

#ifndef IEXCEPTION_H
    #define IEXCEPTION_H
    
    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type/class */
    #include "../warning/UnsafeObserver.hpp"    // utils::warning::UnsafeObserver
    #include "ExceptionDefine.hpp"              // utils::exception::Code, utils::exception::type
    #include <source_location>                  // std::source_location
    #include <exception>                        // std::exception
    #include <string>                           // std::string

namespace utils::exception { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class IException: public std::exception, private utils::warning::UnsafeObserver {
    public:
        // --------- Pre-Function --------- //
        virtual utils::exception::Type getType(void) const noexcept = 0;
        virtual utils::exception::Code getCode(void) const noexcept = 0;
        virtual bool isNone(void) const noexcept = 0;
        virtual bool isFatal(void) const noexcept = 0;
        virtual const char* what(void) const noexcept = 0;
        virtual const char* info(void) const noexcept = 0;
        virtual const std::source_location& loc(void) const noexcept = 0;
        virtual std::string formated(void) const noexcept = 0;

        // ------------ Operator ---------- //
        IException& operator=(const IException& other) = delete;
        IException& operator=(IException&& other) = delete;

        // ---------- Constructor --------- //
        IException() = default;
        IException(const IException& other) = delete;
        IException(IException&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~IException() = default;
};

} // namespace end
#endif /* IEXCEPTION_H */
