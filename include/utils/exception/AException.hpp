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
    #include "ExceptionDefine.hpp"          // utils::exception::* (define / vars)
    #include "../attribute/Attribute.hpp"   // _cold, _nodiscard
    #include <source_location>              // std::source_location
    #include <unordered_map>                // std::unordered_map
    #include <iterator>                     // std::begin, std::end
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
        std::unordered_map<utils::exception::InternalCode, const char*> _messages;
        std::unordered_map<utils::exception::InternalCode, const char*> _infos; // default info of each code
        std::unordered_map<utils::exception::InternalCode, const std::uint8_t> _restrictions;

        // ---------- Pre-Function -------- //
        void subinit_(void);

    protected:
        /* Exception call info */
        std::source_location _loc;
        const void* _callerAddr = nullptr;
        const char* _file = nullptr;
        const char* _func = nullptr;
        std::size_t _line = std::numeric_limits<std::size_t>::max();

        /* Exception config */
        std::string _info = "[None]";
        utils::exception::Type _type = utils::exception::Type::None;
        utils::exception::InternalCode _code = utils::exception::InternalCode::Undefined;

    public:
        // ---------- Pre-Function -------- //
        std::string formated(void) const noexcept final;

        // ------------ Function ---------- //
        _cold _nodiscard utils::exception::Type getType(void) const noexcept final         {return this->_type;};
        _cold _nodiscard utils::exception::InternalCode getCode(void) const noexcept final {return this->_code;};
        _cold _nodiscard bool isNone(void) const noexcept final                            {return (this->_type & utils::exception::Type::None);};
        _cold _nodiscard bool isFatal(void) const noexcept final                           {return (this->_type & utils::exception::Type::Fatal);};
        _cold _nodiscard const char* what(void) const noexcept final                       {return this->_messages.at(this->_code);};
        _cold _nodiscard const char* info(void) const noexcept final                       {return this->_info.c_str();};
        _cold _nodiscard const std::source_location& loc(void) const noexcept final        {return this->_loc;};

        // ------------ Operator ---------- //
        AException& operator=(const AException& other) = delete;
        AException& operator=(AException&& other) = delete;

        // ---------- Constructor --------- //
        _cold AException(std::source_location loc = std::source_location::current(), utils::exception::Type type = utils::exception::Type::None, utils::exception::InternalCode code = utils::exception::InternalCode::Undefined, std::string info = "[None]")
        : IException(),
            _messages{}, _infos{}, _restrictions{},
            _loc{loc}, _callerAddr{__builtin_return_address(0)}, _file{loc.file_name()}, _func{loc.function_name()}, _line{loc.line()},
            _info{info}, _type{type}, _code{code}
        {
            this->_messages.insert(std::begin(utils::exception::InternalMessages), std::end(utils::exception::InternalMessages));
            this->_infos.insert(std::begin(utils::exception::InternalInfo), std::end(utils::exception::InternalInfo));
            this->_restrictions.insert(std::begin(utils::exception::InternalRestriction), std::end(utils::exception::InternalRestriction));
            #ifdef GENERATED_EXTERNAL_EXCEPTION_HEADER_H
                for (const auto &[externalCode, message]: utils::exception::ExternalMessages)
                    this->_messages.emplace(static_cast<utils::exception::InternalCode>(externalCode), message);
                for (const auto &[externalCode, externalInfo]: utils::exception::ExternalInfo)
                    this->_infos.emplace(static_cast<utils::exception::InternalCode>(externalCode), externalInfo);
                for (const auto &[externalCode, restriction]: utils::exception::ExternalRestriction)
                    this->_restrictions.emplace(static_cast<utils::exception::InternalCode>(externalCode), restriction);
            #endif
            this->subinit_();
        };
        AException(const AException& other) = delete;
        AException(AException&& other) = delete;

        // ----------- Destructor --------- //
        ~AException() = default;
};

} // namespace end
#endif /* AEXCEPTION_H */
