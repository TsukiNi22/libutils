/**************************************************************\
Edition:
##  @date 20/07/2026 by @author Tsukini

File Name:
##  @file AException.cpp

File Description:
##  Definition of the function used in the abstract for exception
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/AException.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/manip/iomanip/Color.hpp"
#include "utils/manip/iomanip/ANSI.hpp"
#include "utils/manip/iomanip/Style.hpp"
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
    oss << utils::iomanip::strong();
    if (this->_type & utils::exception::Type::None)
        oss << utils::iomanip::color_rgb(175, 100, 0) << "[None]";
    else if (this->_type & utils::exception::Type::Error)
        oss << utils::iomanip::color_rgb(205, 0, 0) << "[Error]";
    else if (this->_type & utils::exception::Type::Warning)
        oss << utils::iomanip::color_rgb(175, 0, 175) << "[Warning]";
    oss << " " << utils::iomanip::reset();

    // Emplacement information
    oss << utils::iomanip::strong() << this->_file << ":" << this->_line << utils::iomanip::reset() << " -> " << this->Message.at(this->_code) << std::endl;

    // Content
    oss << utils::iomanip::color_rgb(175, 100, 0) << "-------------------------------------------" << utils::iomanip::reset() << std::endl;
    oss << utils::iomanip::color(utils::iomanip::Color::Cyan) << this->_func << utils::iomanip::color_rgb(175, 100, 0) << " = " << utils::iomanip::reset() << this->_info << std::endl;
    oss << utils::iomanip::color_rgb(175, 100, 0) << "-------------------------------------------" << utils::iomanip::reset();

    return oss.str();
}
