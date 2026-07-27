/**************************************************************\
Edition:
##  @date 27/07/2026 by @author Tsukini

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
#include <unistd.h>
#include <dlfcn.h>
#include <source_location>
#include <filesystem>
#include <sstream>
#include <cstddef>
#include <cstdint>
#include <string>
#include <array>

_cold void utils::exception::AException::subinit(void)
{
    // Setup the default info if nessecary
    if (this->_info == "[None]" && this->Info.at(this->_code))
        this->_info = std::string{this->Info.at(this->_code)};

    // Check the restriction for code & type combination
    std::uint8_t restriction = this->Restriction.at(this->_code);
    if (this->_code == utils::exception::InternalCode::ExceptionCodeRestriction) return; // Check to counter any mistake and cause a infinit throw loop
    else if (restriction != 0 && (this->_type & restriction) != this->_type)
        throw utils::exception::ErrorException(utils::exception::InternalCode::ExceptionCodeRestriction, this->_loc);
}

_cold _nodiscard static inline std::string shorten(const std::string &path)
{
    static constexpr std::array<std::string, 2> markers = {"src/", "include/"};

    // Find the last occurence
    for (const std::string& marker: markers) {
        std::size_t pos = path.rfind("/" + marker);
        if (pos != std::string::npos)
            return "..." + path.substr(pos);
    }

    // Fallback
    return path;
}

_cold _nodiscard static inline std::string canonical_or_raw(const std::string &path)
{
    try {
        return std::filesystem::canonical(path).string();
    } catch (const std::filesystem::filesystem_error&) {
        return path; // Fallback
    }
}

_cold _nodiscard static inline std::string module_name(const void *addr, const std::string& path)
{
    // Check if it's the utils library
    if (path.find("src/utils") != std::string::npos || path.find("include/utils") != std::string::npos)
        return "utils";

    // Get the module information
    Dl_info info;
    if (!addr || !dladdr(addr, &info) || !info.dli_fname)
        return "";
    std::string module_path = canonical_or_raw(info.dli_fname);

    // Get the binary name
    static const std::string self_exe = [] {
        char buf[4096];
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        return (len > 0) ? std::string(buf, len) : "";
    }();

    // Check if the actual module is the binary
    if (module_path == self_exe)
        return "";

    // Only keep the module name
    std::size_t slash = module_path.rfind('/');
    return (slash != std::string::npos) ? module_path.substr(slash + 1) : module_path;
}

_cold _nodiscard std::string utils::exception::AException::formated(void) const noexcept
{
    std::ostringstream oss;

    // Exception type
    std::string option = (this->isFatal() ? "(fatal)" : "");
    oss << utils::iomanip::strong();
    if (this->_type & utils::exception::Type::None)
        oss << utils::iomanip::color_rgb(175, 100, 0) << "[None" << option << "]";
    else if (this->_type & utils::exception::Type::Error)
        oss << utils::iomanip::color_rgb(205, 0, 0) << "[Error" << option << "]";
    else if (this->_type & utils::exception::Type::Warning)
        oss << utils::iomanip::color_rgb(175, 0, 175) << "[Warning" << option << "]";
    oss << " " << utils::iomanip::reset();

    // Emplacement information
    std::string absolutePath, moduleName;
    bool exists = false;
    try {
        absolutePath = std::filesystem::absolute(this->_file).lexically_normal().string();
        exists = std::filesystem::exists(absolutePath);
    } catch (const std::filesystem::filesystem_error &) {exists = false;}
    oss << utils::iomanip::strong();
    if (exists) oss << utils::iomanip::file_hyperlink(shorten(this->_file), absolutePath);
    else oss << shorten(this->_file);
    oss << ":" << this->_line << utils::iomanip::reset();
    if (!(moduleName = module_name(this->_caller_addr, absolutePath)).empty())
        oss << utils::iomanip::color_rgb(100, 100, 100) << " (" << moduleName << ")" << utils::iomanip::reset();
    oss << " -> " << this->Messages.at(this->_code) << std::endl;

    // Content
    oss << utils::iomanip::color_rgb(175, 100, 0) << "-------------------------------------------" << utils::iomanip::reset() << std::endl;
    oss << utils::iomanip::color(utils::iomanip::Color::Cyan) << this->_func << utils::iomanip::color_rgb(175, 100, 0) << " = " << utils::iomanip::reset() << this->_info << std::endl;
    oss << utils::iomanip::color_rgb(175, 100, 0) << "-------------------------------------------" << utils::iomanip::reset();

    return oss.str();
}
