/**************************************************************\
Edition:
##  @date 28/07/2026 by @author Tsukini

File Name:
##  @file AException.hpp

File Description:
##  Absract for the cutomized exception handling
\**************************************************************/

#ifndef AEXCEPTION_H
    #define AEXCEPTION_H
    
    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type/class */
    #include "IException.hpp"               // utils::exception::IException
    #include "ExceptionDefine.hpp"          // utils::exception::InternalCode, utils::exception::Type, utils::exception::Messages
    #include "../attribute/Attribute.hpp"   // _nodiscard
    #include <dlfcn.h>                      // __builtin_return_address
    #include <source_location>              // std::source_location
    #include <unordered_map>                // std::unordered_map
    #include <iostream>                     // st:cerr
    #include <cstdint>                      // std::uint8_t
    #include <cstddef>                      // std::size_t
    #include <limits>                       // std::numeric_limits
    #include <vector>                       // std::vector
    #include <string>                       // std::string

namespace utils::exception { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class AException: public utils::exception::IException {
    private:
        /* Exception Data */
        std::unordered_map<utils::exception::InternalCode, const char*> Messages;
        std::unordered_map<utils::exception::InternalCode, const char*> Info;
        std::unordered_map<utils::exception::InternalCode, const std::uint8_t> Restriction;

        // --------- Pre-Function --------- //
        void subinit(void);

    protected:
        /* Exception call info */
        std::source_location _loc;
        const void* _caller_addr = nullptr;
        const char* _file = nullptr;
        const char* _func = nullptr;
        std::size_t _line = std::numeric_limits<std::size_t>::max();

        /* Exception config */
        std::string _info = "[None]";
        utils::exception::Type _type = utils::exception::Type::None;
        utils::exception::InternalCode _code = utils::exception::InternalCode::Undefined;

    public:
        // --------- Pre-Function --------- //
        std::string formated(void) const noexcept final;

        // ----------- Function ----------- //
        // auto -> const std::array<type auto, size auto>
        _nodiscard utils::exception::Type getType(void) const noexcept final {return this->_type;};
        _nodiscard utils::exception::InternalCode getCode(void) const noexcept final {return this->_code;};
        _nodiscard bool isNone(void) const noexcept final {return (this->_type & utils::exception::Type::None);};
        _nodiscard bool isFatal(void) const noexcept final {return (this->_type & utils::exception::Type::Fatal);};
        _nodiscard const char* what(void) const noexcept final {return this->Messages.at(this->_code);};
        _nodiscard const char* info(void) const noexcept final {return this->_info.c_str();};
        _nodiscard const std::source_location& loc(void) const noexcept final {return this->_loc;};

        // ------------ Operator ---------- //
        AException& operator=(const AException& other) = delete;
        AException& operator=(AException&& other) = delete;

        // ---------- Constructor --------- //
        _cold AException(std::source_location loc = std::source_location::current(), utils::exception::Type type = utils::exception::Type::None, utils::exception::InternalCode code = utils::exception::InternalCode::Undefined, std::string info = "[None]")
        : IException(),
            Messages{}, Info{}, Restriction{},
            _loc{loc}, _caller_addr(__builtin_return_address(0)), _file{loc.file_name()}, _func{loc.function_name()}, _line{loc.line()},
            _info{info}, _type{type}, _code{code}
        {
            this->Messages.insert(std::begin(utils::exception::InternalMessages), std::end(utils::exception::InternalMessages));
            this->Info.insert(std::begin(utils::exception::InternalInfo), std::end(utils::exception::InternalInfo));
            this->Restriction.insert(std::begin(utils::exception::InternalRestriction), std::end(utils::exception::InternalRestriction));
            #ifdef GENERATED_EXCEPTION_HEADER_H
                for (const auto& [code, message]: utils::exception::Messages)
                    this->Messages.emplace(static_cast<utils::exception::InternalCode>(code), message);
                for (const auto& [code, info]: utils::exception::Info)
                    this->Info.emplace(static_cast<utils::exception::InternalCode>(code), info);
                for (const auto& [code, restriction]: utils::exception::Restriction)
                    this->Restriction.emplace(static_cast<utils::exception::InternalCode>(code), restriction);
            #endif
            this->subinit();
        }
        AException(const AException& other) = delete;
        AException(AException&& other) = delete;

        // ----------- Destructor --------- //
        ~AException() = default;
};

} // namespace end
#endif /* AEXCEPTION_H */
