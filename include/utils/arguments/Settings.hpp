/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/07/2026 by @author Tsukini

File Name:
##  @file Settings.hpp

File Description:
##  Declaration of the Settings class used for settings handling
\**************************************************************/

#ifndef SETTINGS_H
    #define SETTINGS_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // nodicard
    #include "../warning/Observer.hpp"      // utils::warning::Observer
    #include "SettingsDefine.hpp"           // utils::arguments::CastType
    #include "Setting.hpp"                  // utils::arguments::Setting
    #include <unordered_map>                // std::unordered_map
    #include <filesystem>                   // std::filesystem::path
    //#include <cstdfloat> -> handled by SettingsDefine
    #include <cstddef>                      // std::* (type)
    #include <cstdint>                      // std::* (type)
    #include <string>                       // std::string

namespace utils::arguments { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Settings: private utils::warning::Observer {
    private:
        std::unordered_map<std::string, utils::arguments::Setting> _settings;

        // ---------- Pre-Function -------- //
        utils::arguments::CastType getType(const std::string& setting);

        /* basic */
        std::byte cast_byte(const std::string& setting);
        bool cast_bool(const std::string& setting);

        /* integer */
        std::int8_t  cast_int8(const std::string& setting);
        std::int16_t cast_int16(const std::string& setting);
        std::int32_t cast_int32(const std::string& setting);
        std::int64_t cast_int64(const std::string& setting);

        /* unsigned integer */
        std::uint8_t  cast_uint8(const std::string& setting);
        std::uint16_t cast_uint16(const std::string& setting);
        std::uint32_t cast_uint32(const std::string& setting);
        std::uint64_t cast_uint64(const std::string& setting);

        /* floating */
        utils::arguments::float16_t  cast_float16(const std::string& setting);
        utils::arguments::float32_t  cast_float32(const std::string& setting);
        utils::arguments::float64_t  cast_float64(const std::string& setting);
        utils::arguments::float128_t cast_float128(const std::string& setting);

        /* char */
        char8_t  cast_char8(const std::string& setting);
        char16_t cast_char16(const std::string& setting);
        char32_t cast_char32(const std::string& setting);
        std::u8string  cast_u8string(const std::string& setting);
        std::u16string cast_u16string(const std::string& setting);
        std::u32string cast_u32string(const std::string& setting);

        /* huge char */
        wchar_t      cast_wchar(const std::string& setting);
        std::wstring cast_wstring(const std::string& setting);

        /* special */
        std::filesystem::path cast_path(const std::string& setting);

    public:
        // ---------- Pre-Function -------- //
        const utils::arguments::Setting& at(const std::string& id) const;
        template<bool force = false> // Can't override an exiting one by default
        utils::arguments::CastType auto_cast(const std::string& id, const std::string& setting)
        {
            utils::arguments::CastType type = this->getType(setting);
            if (type == utils::arguments::CastType::None) this->set<force>(id, setting);
            else this->cast<type, force>(id, setting);
            return type; // Return type found (None == String)
        }

        // ------------ Function ---------- //
        template<utils::arguments::CastType type, bool force = false> // Can't override an exiting one by default
        void cast(const std::string& id, const std::string& setting)
        {
            switch (type) {
                /* basic */
                case utils::arguments::CastType::Byte: this->set<force>(id, this->cast_byte(setting));      break;
                case utils::arguments::CastType::Bool: this->set<force>(id, this->cast_bool(setting));      break;

                /* integer */
                case utils::arguments::CastType::Int8:  this->set<force>(id, this->cast_int8(setting));      break;
                case utils::arguments::CastType::Int16: this->set<force>(id, this->cast_int16(setting));     break;
                case utils::arguments::CastType::Int32: this->set<force>(id, this->cast_int32(setting));     break;
                case utils::arguments::CastType::Int64: this->set<force>(id, this->cast_int64(setting));     break;

                /* unsigned integer */
                case utils::arguments::CastType::UInt8:  this->set<force>(id, this->cast_uint8(setting));     break;
                case utils::arguments::CastType::UInt16: this->set<force>(id, this->cast_uint16(setting));    break;
                case utils::arguments::CastType::UInt32: this->set<force>(id, this->cast_uint32(setting));    break;
                case utils::arguments::CastType::UInt64: this->set<force>(id, this->cast_uint64(setting));    break;

                /* floating */
                case utils::arguments::CastType::Float16:  this->set<force>(id, this->cast_float16(setting));   break;
                case utils::arguments::CastType::Float32:  this->set<force>(id, this->cast_float32(setting));   break;
                case utils::arguments::CastType::Float64:  this->set<force>(id, this->cast_float64(setting));   break;
                case utils::arguments::CastType::Float128: this->set<force>(id, this->cast_float128(setting));  break;

                /* char */
                case utils::arguments::CastType::Char8:     this->set<force>(id, this->cast_char8(setting));     break;
                case utils::arguments::CastType::Char16:    this->set<force>(id, this->cast_char16(setting));    break;
                case utils::arguments::CastType::Char32:    this->set<force>(id, this->cast_char32(setting));    break;
                case utils::arguments::CastType::U8String:  this->set<force>(id, this->cast_u8string(setting));  break;
                case utils::arguments::CastType::U16String: this->set<force>(id, this->cast_u16string(setting)); break;
                case utils::arguments::CastType::U32String: this->set<force>(id, this->cast_u32string(setting)); break;

                /* huge char */
                case utils::arguments::CastType::WChar:   this->set<force>(id, this->cast_wchar(setting));     break;
                case utils::arguments::CastType::WString: this->set<force>(id, this->cast_wstring(setting));   break;

                /* special */
                case utils::arguments::CastType::Path: this->set<force>(id, this->cast_path(setting));      break;

                default: throw utils::exception::ErrorException(utils::exception::Code::UnknowCast);
            }
        }
        template<typename T>
        void add(const std::string& id, const T& setting) {this->set<false>(id, setting);};
        template<bool force = true, typename T> // Can override an exiting one by default
        void set(const std::string& id, const T& setting)
        {
            if (this->_settings.contains(id)) {
                if constexpr (force) this->_settings.erase(id);
                else throw utils::exception::ErrorException(utils::exception::Code::Override, std::string("A setting with this id is already defined: ") + id);
            }
            this->_settings.emplace(id, setting);
        };
        template<bool failsafe = false>
        void remove(const std::string& id)
        {
            if (!this->_settings.contains(id)) {
                if constexpr (failsafe) return;
                else throw utils::exception::ErrorException(utils::exception::Code::UnknowId, id);
            }
            this->_settings.erase(id);
        };
        void clear(void) {this->_settings.clear();};
        _nodiscard const utils::arguments::Setting& get(const std::string& id) const {return this->at(id);};

        // ------------ Operator ---------- //
        Settings& operator=(const Settings& other) = delete;
        Settings& operator=(Settings&& other) = delete;
        _nodiscard const utils::arguments::Setting& operator[](const std::string& id) {return this->at(id);}
        _nodiscard const utils::arguments::Setting& operator[](const std::string& id) const {return this->at(id);}

        // ---------- Constructor --------- //
        Settings() = default;
        Settings(const Settings& other) = delete;
        Settings(Settings&& other) = delete;

        // ----------- Destructor --------- //
        ~Settings() = default;
};

} // namespace end
#endif /* SETTINGS_H */
