/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 27/07/2026 by @author Tsukini

File Name:
##  @file Settings.cpp

File Description:
##  Settings methods definition
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/IException.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/arguments/Settings.hpp"
#include "utils/arguments/SettingsDefine.hpp"
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <iterator>
//#include <cstdfloat> -> handled by SettingsDefine
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <string_view>
#include <string>
#include <limits>
#include <regex>
#include <cmath>

_hot _nodiscard static std::u32string decode_utf8(const std::string& setting)
{
    std::u32string result;
    std::size_t i = 0;
    while (i < setting.size()) {
        unsigned char c = static_cast<unsigned char>(setting[i]);
        char32_t codepoint = 0;
        std::size_t extra = 0;
        if ((c & 0x80) == 0x00)      {codepoint = c;        extra = 0;}
        else if ((c & 0xE0) == 0xC0) {codepoint = c & 0x1F; extra = 1;}
        else if ((c & 0xF0) == 0xE0) {codepoint = c & 0x0F; extra = 2;}
        else if ((c & 0xF8) == 0xF0) {codepoint = c & 0x07; extra = 3;}
        else
            throw std::invalid_argument("invalid UTF-8 leading byte");
        if (i + extra >= setting.size())
            throw std::invalid_argument("truncated UTF-8 sequence");
        for (std::size_t j = 1; j <= extra; ++j) {
            unsigned char cc = static_cast<unsigned char>(setting[i + j]);
            if ((cc & 0xC0) != 0x80)
                throw std::invalid_argument("invalid UTF-8 continuation byte");
            codepoint = (codepoint << 6) | (cc & 0x3F);
        }
        result += codepoint;
        i += extra + 1;
    }
    return result;
}

_hot _nodiscard const utils::arguments::Setting& utils::arguments::Settings::at(std::string_view id) const
{
    auto it = this->_settings.find(id); // single lookup (heterogeneous, no std::string built)
    if (it == this->_settings.end())
        throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownId, std::string(id));
    return it->second;
}

_hot _nodiscard utils::arguments::Setting& utils::arguments::Settings::at(std::string_view id)
{
    auto it = this->_settings.find(id);
    if (it == this->_settings.end())
        throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownId, std::string(id));
    return it->second;
}

_hot _nodiscard utils::arguments::CastType utils::arguments::Settings::getType_(const std::string& setting)
{
    static const std::regex boolPattern(R"(^(true|false|t|f)$)", std::regex::icase);
    static const std::regex intPattern(R"(^[+-]?[0-9]+$)");
    static const std::regex floatPattern(R"(^[+-]?[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?$|^[+-]?[0-9]+[eE][+-]?[0-9]+$)");

    /* basic */
    if (std::regex_match(setting, boolPattern))
        return utils::arguments::CastType::Bool;

    /* path */
    // path: starts with './', '../', '~/', '/' or contains a '/' followed by something
    const std::size_t slash = setting.find('/');
    if (setting.starts_with("./") || setting.starts_with("../") || setting.starts_with("~/") || setting.starts_with('/')
        || (slash != std::string::npos && slash + 1 < setting.size()))
        return utils::arguments::CastType::Path;

    /* integer */
    if (std::regex_match(setting, intPattern)) {
        bool negative = (setting.front() == '-');
        try {
            if (negative) {
                std::int64_t value = std::stoll(setting);
                if (value >= std::numeric_limits<std::int32_t>::min() && value <= std::numeric_limits<std::int32_t>::max())
                    return utils::arguments::CastType::Int32;
                else
                    return utils::arguments::CastType::Int64;
            } else {
                std::uint64_t value = std::stoull(setting);
                if (value <= std::numeric_limits<std::uint32_t>::max())
                    return utils::arguments::CastType::UInt32;
                else
                    return utils::arguments::CastType::UInt64;
            }
        } catch (const std::exception&) {} // Fallback
    }

    /* floating */
    if (std::regex_match(setting, floatPattern)) {
        errno = 0;
        (void)std::strtod(setting.c_str(), nullptr);
        if (errno != ERANGE)
            return utils::arguments::CastType::Float64; // default: lossless for usual values
        errno = 0;
        (void)std::strtold(setting.c_str(), nullptr);
        if (errno != ERANGE)
            return utils::arguments::CastType::Float128; // does not fit in a float64 (too big, too small or denormal)
        return utils::arguments::CastType::None; // can't be represented: kept as a string
    }

    /* char */
    if (!setting.empty()) {
        try {
            std::u32string codepoints = decode_utf8(setting);
            if (codepoints.size() == 1) {
                return (codepoints[0] <= 0x7F)
                    ? utils::arguments::CastType::Char8
                    : utils::arguments::CastType::Char32;
            }
        } catch (const std::exception&) {} // Fallback
    }

    // Fallback (std::u8string == std::string)
    return utils::arguments::CastType::None;
}

_hot _nodiscard std::byte utils::arguments::Settings::castByte_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        const std::size_t sign = (setting.front() == '+'); // optional '+' (accepted by getType_)
        if (sign == setting.size() || !std::all_of(setting.begin() + static_cast<std::ptrdiff_t>(sign), setting.end(), [](unsigned char c) {return std::isdigit(c);}))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value > 0xFF)
            throw std::out_of_range("byte overflow");
        return static_cast<std::byte>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard bool utils::arguments::Settings::castBool_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::string lower;
        std::transform(setting.begin(), setting.end(), std::back_inserter(lower), [](unsigned char c) {return static_cast<char>(std::tolower(c));});
        if (lower == "true" || lower == "t" || lower == "1")
            return true;
        if (lower == "false" || lower == "f" || lower == "0")
            return false;
        throw std::invalid_argument("not a boolean");
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::int8_t utils::arguments::Settings::castInt8_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        long value = std::stol(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value < std::numeric_limits<std::int8_t>::min() ||
            value > std::numeric_limits<std::int8_t>::max())
            throw std::out_of_range("int8 overflow");
        return static_cast<std::int8_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::int16_t utils::arguments::Settings::castInt16_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        long value = std::stol(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value < std::numeric_limits<std::int16_t>::min() ||
            value > std::numeric_limits<std::int16_t>::max())
            throw std::out_of_range("int16 overflow");
        return static_cast<std::int16_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::int32_t utils::arguments::Settings::castInt32_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        long long value = std::stoll(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value < std::numeric_limits<std::int32_t>::min() ||
            value > std::numeric_limits<std::int32_t>::max())
            throw std::out_of_range("int32 overflow");
        return static_cast<std::int32_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::int64_t utils::arguments::Settings::castInt64_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        long long value = std::stoll(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        return static_cast<std::int64_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::uint8_t utils::arguments::Settings::castUInt8_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        const std::size_t sign = (setting.front() == '+'); // optional '+' (accepted by getType_)
        if (sign == setting.size() || !std::all_of(setting.begin() + static_cast<std::ptrdiff_t>(sign), setting.end(), [](unsigned char c) {return std::isdigit(c);}))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value > std::numeric_limits<std::uint8_t>::max())
            throw std::out_of_range("uint8 overflow");
        return static_cast<std::uint8_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::uint16_t utils::arguments::Settings::castUInt16_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        const std::size_t sign = (setting.front() == '+'); // optional '+' (accepted by getType_)
        if (sign == setting.size() || !std::all_of(setting.begin() + static_cast<std::ptrdiff_t>(sign), setting.end(), [](unsigned char c) {return std::isdigit(c);}))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value > std::numeric_limits<std::uint16_t>::max())
            throw std::out_of_range("uint16 overflow");
        return static_cast<std::uint16_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::uint32_t utils::arguments::Settings::castUInt32_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        const std::size_t sign = (setting.front() == '+'); // optional '+' (accepted by getType_)
        if (sign == setting.size() || !std::all_of(setting.begin() + static_cast<std::ptrdiff_t>(sign), setting.end(), [](unsigned char c) {return std::isdigit(c);}))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value > std::numeric_limits<std::uint32_t>::max())
            throw std::out_of_range("uint32 overflow");
        return static_cast<std::uint32_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::uint64_t utils::arguments::Settings::castUInt64_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        const std::size_t sign = (setting.front() == '+'); // optional '+' (accepted by getType_)
        if (sign == setting.size() || !std::all_of(setting.begin() + static_cast<std::ptrdiff_t>(sign), setting.end(), [](unsigned char c) {return std::isdigit(c);}))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long long value = std::stoull(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        return static_cast<std::uint64_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard utils::arguments::float16_t utils::arguments::Settings::castFloat16_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        float value = std::stof(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("not a float");
        if (std::isfinite(value) && std::isinf(static_cast<float>(static_cast<utils::arguments::float16_t>(value))))
            throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "float16 overflow: " + setting);
        return static_cast<utils::arguments::float16_t>(value);
    } catch (const utils::exception::IException&) {
        throw;
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard utils::arguments::float32_t utils::arguments::Settings::castFloat32_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        float value = std::stof(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("not a float");
        return static_cast<utils::arguments::float32_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard utils::arguments::float64_t utils::arguments::Settings::castFloat64_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        double value = std::stod(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("not a float");
        return static_cast<utils::arguments::float64_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard utils::arguments::float128_t utils::arguments::Settings::castFloat128_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        char* end = nullptr;
        long double value = std::strtold(setting.c_str(), &end);
        if (end != setting.c_str() + setting.size())
            throw std::invalid_argument("not a float");
        return static_cast<utils::arguments::float128_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard char8_t utils::arguments::Settings::castChar8_(const std::string& setting)
{
    try {
        if (setting.size() != 1)
            throw std::invalid_argument("expected a single character");
        return static_cast<char8_t>(setting[0]);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

// Code point given as a number (base auto-detected: 0x.., 0.., decimal) or as a single UTF-8 character
_hot _nodiscard static unsigned long code_point(const std::string& setting)
{
    std::size_t pos = 0;
    try {
        unsigned long value = std::stoul(setting, &pos, 0);
        if (pos == setting.size()) return value;
    } catch (const std::invalid_argument&) {} // not a number, try as a character
    std::u32string codepoints = decode_utf8(setting);
    if (codepoints.size() != 1)
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "Expected a code point or a single character: " + setting);
    return codepoints[0];
}

_hot _nodiscard char16_t utils::arguments::Settings::castChar16_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        unsigned long value = code_point(setting); // number (0x263A, 9786) or a single character
        if (value > std::numeric_limits<char16_t>::max())
            throw std::out_of_range("char16 overflow");
        return static_cast<char16_t>(value);
    } catch (const utils::exception::IException&) {
        throw;
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard char32_t utils::arguments::Settings::castChar32_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        unsigned long value = code_point(setting); // number (0x1F600, 128512) or a single character
        if (value > 0x10FFFF)
            throw std::out_of_range("not a valid Unicode code point");
        return static_cast<char32_t>(value);
    } catch (const utils::exception::IException&) {
        throw;
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::u8string utils::arguments::Settings::castU8String_(const std::string& setting)
{
    try {
        (void)decode_utf8(setting);
        return std::u8string(setting.begin(), setting.end());
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::u16string utils::arguments::Settings::castU16String_(const std::string& setting)
{
    try {
        std::u32string codepoints = decode_utf8(setting);
        std::u16string result;
        for (char32_t cp: codepoints) {
            if (cp <= 0xFFFF) {
                result += static_cast<char16_t>(cp);
            } else {
                cp -= 0x10000;
                result += static_cast<char16_t>(0xD800 + (cp >> 10));
                result += static_cast<char16_t>(0xDC00 + (cp & 0x3FF));
            }
        }
        return result;
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::u32string utils::arguments::Settings::castU32String_(const std::string& setting)
{
    try {
        return decode_utf8(setting);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard wchar_t utils::arguments::Settings::castWChar_(const std::string& setting)
{
    try {
        std::u32string codepoints = decode_utf8(setting);
        if (codepoints.size() != 1)
            throw std::invalid_argument("expected a single character");
        return static_cast<wchar_t>(codepoints[0]);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::wstring utils::arguments::Settings::castWString_(const std::string& setting)
{
    try {
        std::u32string codepoints = decode_utf8(setting);
        return std::wstring(codepoints.begin(), codepoints.end());
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}

_hot _nodiscard std::filesystem::path utils::arguments::Settings::castPath_(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        return std::filesystem::path(setting);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, std::string(e.what()) + ": " + setting);
    }
}
