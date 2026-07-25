/**************************************************************\
Edition:
##  @date 25/07/2026 by @author Tsukini

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
    #include "ExceptionDefine.hpp"          // utils::exception::Code, utils::exception::Type, utils::exception::Message
    #include "../attribute/Attribute.hpp"   // _nodiscard
    #include <source_location>              // std::source_location
    #include <unordered_map>                // std::unordered_map
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
        std::unordered_map<utils::exception::Code, const char*> Message;
        std::unordered_map<utils::exception::Code, const char*> Info;
        std::unordered_map<utils::exception::Code, const std::uint8_t> Restriction;

        // --------- Pre-Function --------- //
        void subinit(void);

    protected:
        /* Exception call info */
        std::source_location _loc;
        const char* _file = nullptr;
        const char* _func = nullptr;
        std::size_t _line = std::numeric_limits<std::size_t>::max();

        /* Exception config */
        std::string _info = "[None]";
        utils::exception::Type _type = utils::exception::Type::None;
        utils::exception::Code _code = utils::exception::Code::Undefined;

    public:
        // --------- Pre-Function --------- //
        std::string formated(void) const noexcept final;

        // ----------- Function ----------- //
        // auto -> const std::array<type auto, size auto>
        _nodiscard utils::exception::Type getType(void) const noexcept final {return this->_type;};
        _nodiscard utils::exception::Code getCode(void) const noexcept final {return this->_code;};
        _nodiscard bool isNone(void) const noexcept final {return (this->_type & utils::exception::Type::None);};
        _nodiscard bool isFatal(void) const noexcept final {return (this->_type & utils::exception::Type::Fatal);};
        _nodiscard const char* what(void) const noexcept final {return this->Message.at(this->_code);};
        _nodiscard const char* info(void) const noexcept final {return this->_info.c_str();};
        _nodiscard const std::source_location& loc(void) const noexcept final {return this->_loc;};

        // ------------ Operator ---------- //
        AException& operator=(const AException& other) = delete;
        AException& operator=(AException&& other) = delete;

        // ---------- Constructor --------- //
        _cold AException(std::source_location loc = std::source_location::current(), utils::exception::Type type = utils::exception::Type::None, utils::exception::Code code = utils::exception::Code::Undefined, std::string info = "[None]")
        : IException(),
            Message{}, Info{}, Restriction{},
            _loc{loc}, _file{loc.file_name()}, _func{loc.function_name()}, _line{loc.line()},
            _info{info}, _type{type}, _code{code}
        {
            this->Message.insert(std::begin(utils::exception::Message), std::end(utils::exception::Message));
            this->Info.insert(std::begin(utils::exception::Info), std::end(utils::exception::Info));
            this->Restriction.insert(std::begin(utils::exception::Restriction), std::end(utils::exception::Restriction));
            this->subinit();
        }
        AException(const AException& other) = delete;
        AException(AException&& other) = delete;

        // ----------- Destructor --------- //
        ~AException() = default;
};

} // namespace end
#endif /* AEXCEPTION_H */
