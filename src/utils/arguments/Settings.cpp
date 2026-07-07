/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 07/07/2026 by @author Tsukini

File Name:
##  @file Settings.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/arguments/Settings.hpp"
#include "utils/arguments/SettingsDefine.hpp"
#include <filesystem>
//#include <cstdfloat> -> handled by SettingsDefine
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <regex>
#include <cmath>

static nodiscard std::u32string decode_utf8(const std::string& setting)
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

nodiscard const utils::arguments::Setting& utils::arguments::Settings::at(const std::string& id) const
{
    if (!this->_settings.contains(id))
        throw utils::exception::ErrorException(utils::exception::Code::UnknowId, id);
    return this->_settings.at(id);
}

nodiscard utils::arguments::CastType utils::arguments::Settings::getType(const std::string& setting)
{
    static const std::regex bool_pattern(R"(^(true|false|t|f)$)", std::regex::icase);
    static const std::regex int_pattern(R"(^[+-]?[0-9]+$)");
    static const std::regex float_pattern(R"(^[+-]?[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?$|^[+-]?[0-9]+[eE][+-]?[0-9]+$)");
    static const std::regex path_pattern(R"(^(\.{1,2}/|~/|/)|.*/.+)");

    /* basic */
    if (std::regex_match(setting, bool_pattern))
        return utils::arguments::CastType::Bool;

    /* path */
    if (std::regex_search(setting, path_pattern))
        return utils::arguments::CastType::Path;

    /* integer */
    if (std::regex_match(setting, int_pattern)) {
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
    if (std::regex_match(setting, float_pattern)) {
        return utils::arguments::CastType::Float64;
        try {
            char* end = nullptr;
            errno = 0;
            utils::arguments::float128_t value = std::strtold(setting.c_str(), &end);
            if (end != setting.c_str() + setting.size())
                throw std::invalid_argument("invalid float");
            if (errno == ERANGE)
                throw std::out_of_range("float overflow");
            utils::arguments::float128_t abs_value = std::fabsl(value);

            if (abs_value <= static_cast<long double>(std::numeric_limits<utils::arguments::float16_t>::max()))
                return utils::arguments::CastType::Float16;
            if (abs_value <= static_cast<long double>(std::numeric_limits<utils::arguments::float32_t>::max()))
                return utils::arguments::CastType::Float32;
            if (abs_value <= static_cast<long double>(std::numeric_limits<utils::arguments::float64_t>::max()))
                return utils::arguments::CastType::Float64;
            return utils::arguments::CastType::Float128;
        } catch (const std::exception&) {} // Fallback
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

nodiscard std::byte utils::arguments::Settings::cast_byte(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        if (!std::all_of(setting.begin(), setting.end(), ::isdigit))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value > 0xFF)
            throw std::out_of_range("byte overflow");
        return static_cast<std::byte>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard bool utils::arguments::Settings::cast_bool(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::string lower;
        std::transform(setting.begin(), setting.end(), std::back_inserter(lower), ::tolower);
        if (lower == "true" || lower == "t" || lower == "1")
            return true;
        if (lower == "false" || lower == "f" || lower == "0")
            return false;
        throw std::invalid_argument("not a boolean");
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::int8_t utils::arguments::Settings::cast_int8(const std::string& setting)
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
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::int16_t utils::arguments::Settings::cast_int16(const std::string& setting)
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
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::int32_t utils::arguments::Settings::cast_int32(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        if (!std::all_of(setting.begin(), setting.end(), ::isdigit))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        long value = std::stol(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value < std::numeric_limits<std::int32_t>::min() ||
            value > std::numeric_limits<std::int32_t>::max())
            throw std::out_of_range("int32 overflow");
        return static_cast<std::int32_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::int64_t utils::arguments::Settings::cast_int64(const std::string& setting)
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
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::uint8_t utils::arguments::Settings::cast_uint8(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        if (!std::all_of(setting.begin(), setting.end(), ::isdigit))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value > std::numeric_limits<std::uint8_t>::max())
            throw std::out_of_range("uint8 overflow");
        return static_cast<std::uint8_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::uint16_t utils::arguments::Settings::cast_uint16(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        if (!std::all_of(setting.begin(), setting.end(), ::isdigit))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value > std::numeric_limits<std::uint16_t>::max())
            throw std::out_of_range("uint16 overflow");
        return static_cast<std::uint16_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::uint32_t utils::arguments::Settings::cast_uint32(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        if (!std::all_of(setting.begin(), setting.end(), ::isdigit))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        if (value > std::numeric_limits<std::uint32_t>::max())
            throw std::out_of_range("uint32 overflow");
        return static_cast<std::uint32_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::uint64_t utils::arguments::Settings::cast_uint64(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        if (!std::all_of(setting.begin(), setting.end(), ::isdigit))
            throw std::invalid_argument("not numeric");
        std::size_t pos = 0;
        unsigned long long value = std::stoull(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("invalid number");
        return static_cast<std::uint64_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard utils::arguments::float16_t utils::arguments::Settings::cast_float16(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        float value = std::stof(setting, &pos);
        if (pos != setting.size())
            throw std::invalid_argument("not a float");
        return static_cast<utils::arguments::float16_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard utils::arguments::float32_t utils::arguments::Settings::cast_float32(const std::string& setting)
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
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard utils::arguments::float64_t utils::arguments::Settings::cast_float64(const std::string& setting)
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
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard utils::arguments::float128_t utils::arguments::Settings::cast_float128(const std::string& setting)
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
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard char8_t utils::arguments::Settings::cast_char8(const std::string& setting)
{
    try {
        if (setting.size() != 1)
            throw std::invalid_argument("expected a single character");
        return static_cast<char8_t>(setting[0]);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard char16_t utils::arguments::Settings::cast_char16(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos, 0); // base 0: auto-detection 0x/0
        if (pos != setting.size())
            throw std::invalid_argument("invalid code point");
        if (value > std::numeric_limits<char16_t>::max())
            throw std::out_of_range("char16 overflow");
        return static_cast<char16_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard char32_t utils::arguments::Settings::cast_char32(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        std::size_t pos = 0;
        unsigned long value = std::stoul(setting, &pos, 0);
        if (pos != setting.size())
            throw std::invalid_argument("invalid code point");
        if (value > 0x10FFFF)
            throw std::out_of_range("not a valid Unicode code point");
        return static_cast<char32_t>(value);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::u8string utils::arguments::Settings::cast_u8string(const std::string& setting)
{
    try {
        (void)decode_utf8(setting);
        return std::u8string(setting.begin(), setting.end());
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::u16string utils::arguments::Settings::cast_u16string(const std::string& setting)
{
    try {
        std::u32string codepoints = decode_utf8(setting);
        std::u16string result;
        for (char32_t cp : codepoints) {
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
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::u32string utils::arguments::Settings::cast_u32string(const std::string& setting)
{
    try {
        return decode_utf8(setting);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard wchar_t utils::arguments::Settings::cast_wchar(const std::string& setting)
{
    try {
        std::u32string codepoints = decode_utf8(setting);
        if (codepoints.size() != 1)
            throw std::invalid_argument("expected a single character");
        return static_cast<wchar_t>(codepoints[0]);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::wstring utils::arguments::Settings::cast_wstring(const std::string& setting)
{
    try {
        std::u32string codepoints = decode_utf8(setting);
        return std::wstring(codepoints.begin(), codepoints.end());
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}

nodiscard std::filesystem::path utils::arguments::Settings::cast_path(const std::string& setting)
{
    try {
        if (setting.empty())
            throw std::invalid_argument("empty");
        return std::filesystem::path(setting);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::Code::BadCast, std::string(e.what()) + ": " + setting);
    }
}
