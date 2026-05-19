/**************************************************************\
Edition:
##  @date 19/05/2026 by @author Tsukini

File Name:
##  @file AException.cpp

File Description:
##  Definition of the function used in the abstract for exception
\**************************************************************/

#define NO_OUTDATED_WARNING
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

cold void utils::exception::AException::subinit()
{
    std::size_t index = static_cast<std::size_t>(this->_code);

    // Setup the default info if nessecary
    if (this->_info == "[None]" && this->Info[index])
        this->_info = std::string{this->Info[index]};

    // Check the restriction for code & type combination
    std::uint8_t restriction = this->Restriction[index];
    if (this->_code == utils::exception::Code::ExceptionCodeRestriction) return; // Check to counter any mistake and cause a infinit throw loop
    else if (restriction != 0 && (this->_type & restriction) != this->_type)
        throw utils::exception::ErrorException(utils::exception::Code::ExceptionCodeRestriction, this->_loc);
}

nodiscard std::string utils::exception::AException::formated() const noexcept
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
    oss << utils::write::strong() << this->_file << ":" << this->_line << utils::write::reset() << " -> " << this->Message[static_cast<std::size_t>(this->_code)] << std::endl;

    // Content
    oss << utils::write::color_rgb(175, 100, 0) << "-------------------------------------------" << utils::write::reset() << std::endl;
    oss << utils::write::color(utils::write::Color::Cyan) << this->_func << utils::write::color_rgb(175, 100, 0) << " = " << utils::write::reset() << this->_info << std::endl;
    oss << utils::write::color_rgb(175, 100, 0) << "-------------------------------------------" << utils::write::reset();

    return oss.str();
}
