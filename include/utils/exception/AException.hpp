/**************************************************************\
Edition:
##  @date 19/05/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"   // nodiscard
    #include <source_location>              // std::source_location
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
        std::vector<const char*> Message;
        std::vector<const char*> Info;
        std::vector<std::uint8_t> Restriction;

        // --------- Pre-Function --------- //
        void subinit();

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
        std::string formated() const noexcept final;

        // ----------- Function ----------- //
        // auto -> const std::array<type auto, size auto>
        nodiscard utils::exception::Type getType() const noexcept final {return this->_type;};
        nodiscard utils::exception::Code getCode() const noexcept final {return this->_code;};
        nodiscard bool isNone() const noexcept final {return (this->_type & utils::exception::Type::None);};
        nodiscard bool isFatal() const noexcept final {return (this->_type & utils::exception::Type::Fatal);};
        nodiscard const char* what() const noexcept final {return this->Message[static_cast<std::size_t>(this->_code)];};
        nodiscard const char* info() const noexcept final {return this->_info.c_str();};
        nodiscard const std::source_location& loc() const noexcept final {return this->_loc;};
    
        // ------------ Operator ---------- //
        AException& operator=(const AException& other) = delete;
        AException& operator=(AException&& other) = delete;

        // ---------- Constructor --------- //
        cold AException(std::source_location loc = std::source_location::current(), utils::exception::Type type = utils::exception::Type::None, utils::exception::Code code = utils::exception::Code::Undefined, std::string info = "[None]")
        : IException(),
            Message{}, Info{}, Restriction{},
            _loc{loc}, _file{loc.file_name()}, _func{loc.function_name()}, _line{loc.line()},
            _info{info}, _type{type}, _code{code}
        {
            std::size_t size = static_cast<std::size_t>(utils::exception::Code::CODE_SENTINEL);
            this->Message.assign(utils::exception::Message, utils::exception::Message + size);
            this->Info.assign(utils::exception::Info, utils::exception::Info + size);
            this->Restriction.assign(utils::exception::Restriction, utils::exception::Restriction + size);
            this->subinit();
        }
        AException(const AException& other) = delete;
        AException(AException&& other) = delete;

        // ----------- Destructor --------- //
        ~AException() = default;
};

} // namespace end
#endif /* AEXCEPTION_H */
