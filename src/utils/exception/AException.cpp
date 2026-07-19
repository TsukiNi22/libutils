/**************************************************************\
Edition:
##  @date 06/07/2026 by @author Tsukini

File Name:
##  @file AException.cpp

File Description:
##  Definition of the function used in the abstract for exception
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/AException.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/write/Color.hpp"
#include "utils/write/ANSI.hpp"
#include "utils/write/Style.hpp"
#include <source_location>
#include <sstream>
#include <cstddef>
#include <cstdint>
#include <string>

_cold void utils::exception::AException::subinit(void)
{
    // Setup the default info if nessecary
    if (this->_info == "[None]" && this->Info.at(this->_code))
        this->_info = std::string{this->Info.at(this->_code)};

    // Check the restriction for code & type combination
    std::uint8_t restriction = this->Restriction.at(this->_code);
    if (this->_code == utils::exception::Code::ExceptionCodeRestriction) return; // Check to counter any mistake and cause a infinit throw loop
    else if (restriction != 0 && (this->_type & restriction) != this->_type)
        throw utils::exception::ErrorException(utils::exception::Code::ExceptionCodeRestriction, this->_loc);
}

_nodiscard std::string utils::exception::AException::formated(void) const noexcept
{
    std::ostringstream oss;

    // Exception type
    oss << utils::write::strong();
    if (this->_type & utils::exception::Type::None)
        oss << utils::write::color_rgb(175, 100, 0) << "[None]";
    else if (this->_type & utils::exception::Type::Error)
        oss << utils::write::color_rgb(205, 0, 0) << "[Error]";
    else if (this->_type & utils::exception::Type::Warning)
        oss << utils::write::color_rgb(175, 0, 175) << "[Warning]";
    oss << " " << utils::write::reset();

    // Emplacement information
    oss << utils::write::strong() << this->_file << ":" << this->_line << utils::write::reset() << " -> " << this->Message.at(this->_code) << std::endl;

    // Content
    oss << utils::write::color_rgb(175, 100, 0) << "-------------------------------------------" << utils::write::reset() << std::endl;
    oss << utils::write::color(utils::write::Color::Cyan) << this->_func << utils::write::color_rgb(175, 100, 0) << " = " << utils::write::reset() << this->_info << std::endl;
    oss << utils::write::color_rgb(175, 100, 0) << "-------------------------------------------" << utils::write::reset();

    return oss.str();
}
