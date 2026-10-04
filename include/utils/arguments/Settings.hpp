/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 14/09/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"           // _cold, _hot, _nodiscard, _migration
    #include "../security/observer/Observer.hpp"    // utils::security::observer::Observer
    #include "SettingsDefine.hpp"                   // utils::arguments::CastType
    #include "Setting.hpp"                          // utils::arguments::Setting
    #include <unordered_map>                        // std::unordered_map
    #include <filesystem>                           // std::filesystem::path
    #include <functional>                           // std::equal_to, std::hash
    #include <string_view>                          // std::string_view
    #include <utility>                              // std::forward
    //#include <cstdfloat> -> handled by SettingsDefine
    #include <cstddef>                              // std::* (type)
    #include <cstdint>                              // std::* (type)
    #include <string>                               // std::string

namespace utils::arguments { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

// Transparent hash: allow lookup with std::string_view / const char* without building a std::string
struct SettingsHash {
    using is_transparent = void;
    _hot _nodiscard inline std::size_t operator()(std::string_view key) const noexcept {return std::hash<std::string_view>{}(key);};
};

//----------------------------------------------------------------//
/* CLASS */

class Settings: private utils::security::observer::Observer<"Settings"> {
    private:
        std::unordered_map<std::string, utils::arguments::Setting, utils::arguments::SettingsHash, std::equal_to<>> _settings;

        // ---------- Pre-Function -------- //
        utils::arguments::CastType getType_(const std::string& setting);

        /* basic */
        std::byte castByte_(const std::string& setting);
        bool castBool_(const std::string& setting);

        /* integer */
        std::int8_t  castInt8_(const std::string& setting);
        std::int16_t castInt16_(const std::string& setting);
        std::int32_t castInt32_(const std::string& setting);
        std::int64_t castInt64_(const std::string& setting);

        /* unsigned integer */
        std::uint8_t  castUInt8_(const std::string& setting);
        std::uint16_t castUInt16_(const std::string& setting);
        std::uint32_t castUInt32_(const std::string& setting);
        std::uint64_t castUInt64_(const std::string& setting);

        /* floating */
        utils::arguments::float16_t  castFloat16_(const std::string& setting);
        utils::arguments::float32_t  castFloat32_(const std::string& setting);
        utils::arguments::float64_t  castFloat64_(const std::string& setting);
        utils::arguments::float128_t castFloat128_(const std::string& setting);

        /* char */
        char8_t  castChar8_(const std::string& setting);
        char16_t castChar16_(const std::string& setting);
        char32_t castChar32_(const std::string& setting);
        std::u8string  castU8String_(const std::string& setting);
        std::u16string castU16String_(const std::string& setting);
        std::u32string castU32String_(const std::string& setting);

        /* huge char */
        wchar_t      castWChar_(const std::string& setting);
        std::wstring castWString_(const std::string& setting);

        /* special */
        std::filesystem::path castPath_(const std::string& setting);

        // ------------ Function ---------- //
        template<bool force, typename K, typename T>
        _cold void set_(K&& id, T&& setting) // K: const std::string& (copy) | std::string&& (move)
        {
            auto it = this->_settings.find(id); // single lookup
            if (it != this->_settings.end()) {
                if constexpr (force) it->second.assign(std::forward<T>(setting)); // edit in place: no node realloc
                else throw utils::exception::ErrorException(utils::exception::InternalCode::Override, std::string("A setting with this id is already defined: ") + id);
                return;
            }
            this->_settings.emplace(std::forward<K>(id), std::forward<T>(setting)); // key copied or moved
        };

    public:
        // ---------- Pre-Function -------- //
        const utils::arguments::Setting& at(std::string_view id) const;
        utils::arguments::Setting& at(std::string_view id);

        // ------------ Function ---------- //
        template<bool force = false> // Can't override an exiting one by default
        _hot utils::arguments::CastType autoCast(const std::string& id, const std::string& setting)
        {
            utils::arguments::CastType type = this->getType_(setting); // only known at runtime
            switch (type) {
                /* string */
                case utils::arguments::CastType::None: this->set<force>(id, setting); break;

                /* basic */
                case utils::arguments::CastType::Byte: this->cast<utils::arguments::CastType::Byte, force>(id, setting); break;
                case utils::arguments::CastType::Bool: this->cast<utils::arguments::CastType::Bool, force>(id, setting); break;

                /* integer */
                case utils::arguments::CastType::Int8:  this->cast<utils::arguments::CastType::Int8,  force>(id, setting); break;
                case utils::arguments::CastType::Int16: this->cast<utils::arguments::CastType::Int16, force>(id, setting); break;
                case utils::arguments::CastType::Int32: this->cast<utils::arguments::CastType::Int32, force>(id, setting); break;
                case utils::arguments::CastType::Int64: this->cast<utils::arguments::CastType::Int64, force>(id, setting); break;

                /* unsigned integer */
                case utils::arguments::CastType::UInt8:  this->cast<utils::arguments::CastType::UInt8,  force>(id, setting); break;
                case utils::arguments::CastType::UInt16: this->cast<utils::arguments::CastType::UInt16, force>(id, setting); break;
                case utils::arguments::CastType::UInt32: this->cast<utils::arguments::CastType::UInt32, force>(id, setting); break;
                case utils::arguments::CastType::UInt64: this->cast<utils::arguments::CastType::UInt64, force>(id, setting); break;

                /* floating */
                case utils::arguments::CastType::Float16:  this->cast<utils::arguments::CastType::Float16,  force>(id, setting); break;
                case utils::arguments::CastType::Float32:  this->cast<utils::arguments::CastType::Float32,  force>(id, setting); break;
                case utils::arguments::CastType::Float64:  this->cast<utils::arguments::CastType::Float64,  force>(id, setting); break;
                case utils::arguments::CastType::Float128: this->cast<utils::arguments::CastType::Float128, force>(id, setting); break;

                /* char */
                case utils::arguments::CastType::Char8:     this->cast<utils::arguments::CastType::Char8,     force>(id, setting); break;
                case utils::arguments::CastType::Char16:    this->cast<utils::arguments::CastType::Char16,    force>(id, setting); break;
                case utils::arguments::CastType::Char32:    this->cast<utils::arguments::CastType::Char32,    force>(id, setting); break;
                case utils::arguments::CastType::U8String:  this->cast<utils::arguments::CastType::U8String,  force>(id, setting); break;
                case utils::arguments::CastType::U16String: this->cast<utils::arguments::CastType::U16String, force>(id, setting); break;
                case utils::arguments::CastType::U32String: this->cast<utils::arguments::CastType::U32String, force>(id, setting); break;

                /* huge char */
                case utils::arguments::CastType::WChar:   this->cast<utils::arguments::CastType::WChar,   force>(id, setting); break;
                case utils::arguments::CastType::WString: this->cast<utils::arguments::CastType::WString, force>(id, setting); break;

                /* special */
                case utils::arguments::CastType::Path: this->cast<utils::arguments::CastType::Path, force>(id, setting); break;

                default: throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownCast);
            }
            return type; // Return type found (None == String)
        };
        template<utils::arguments::CastType type, bool force = false> // Can't override an exiting one by default
        _hot void cast(const std::string& id, const std::string& setting)
        {
            switch (type) {
                /* basic */
                case utils::arguments::CastType::Byte: this->set<force>(id, this->castByte_(setting));      break;
                case utils::arguments::CastType::Bool: this->set<force>(id, this->castBool_(setting));      break;

                /* integer */
                case utils::arguments::CastType::Int8:  this->set<force>(id, this->castInt8_(setting));      break;
                case utils::arguments::CastType::Int16: this->set<force>(id, this->castInt16_(setting));     break;
                case utils::arguments::CastType::Int32: this->set<force>(id, this->castInt32_(setting));     break;
                case utils::arguments::CastType::Int64: this->set<force>(id, this->castInt64_(setting));     break;

                /* unsigned integer */
                case utils::arguments::CastType::UInt8:  this->set<force>(id, this->castUInt8_(setting));     break;
                case utils::arguments::CastType::UInt16: this->set<force>(id, this->castUInt16_(setting));    break;
                case utils::arguments::CastType::UInt32: this->set<force>(id, this->castUInt32_(setting));    break;
                case utils::arguments::CastType::UInt64: this->set<force>(id, this->castUInt64_(setting));    break;

                /* floating */
                case utils::arguments::CastType::Float16:  this->set<force>(id, this->castFloat16_(setting));   break;
                case utils::arguments::CastType::Float32:  this->set<force>(id, this->castFloat32_(setting));   break;
                case utils::arguments::CastType::Float64:  this->set<force>(id, this->castFloat64_(setting));   break;
                case utils::arguments::CastType::Float128: this->set<force>(id, this->castFloat128_(setting));  break;

                /* char */
                case utils::arguments::CastType::Char8:     this->set<force>(id, this->castChar8_(setting));     break;
                case utils::arguments::CastType::Char16:    this->set<force>(id, this->castChar16_(setting));    break;
                case utils::arguments::CastType::Char32:    this->set<force>(id, this->castChar32_(setting));    break;
                case utils::arguments::CastType::U8String:  this->set<force>(id, this->castU8String_(setting));  break;
                case utils::arguments::CastType::U16String: this->set<force>(id, this->castU16String_(setting)); break;
                case utils::arguments::CastType::U32String: this->set<force>(id, this->castU32String_(setting)); break;

                /* huge char */
                case utils::arguments::CastType::WChar:   this->set<force>(id, this->castWChar_(setting));     break;
                case utils::arguments::CastType::WString: this->set<force>(id, this->castWString_(setting));   break;

                /* special */
                case utils::arguments::CastType::Path: this->set<force>(id, this->castPath_(setting));      break;

                default: throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownCast);
            }
        };
        template<typename T>
        _cold inline void add(const std::string& id, T&& setting) {this->set_<false>(id, std::forward<T>(setting));};
        template<typename T>
        _cold inline void add(std::string&& id, T&& setting) {this->set_<false>(std::move(id), std::forward<T>(setting));};
        template<bool force = true, typename T> // Can override an exiting one by default
        _cold inline void set(const std::string& id, T&& setting) {this->set_<force>(id, std::forward<T>(setting));};
        template<bool force = true, typename T> // Can override an exiting one by default
        _cold inline void set(std::string&& id, T&& setting) {this->set_<force>(std::move(id), std::forward<T>(setting));};
        template<bool failsafe = false>
        _cold void remove(std::string_view id)
        {
            auto it = this->_settings.find(id);
            if (it == this->_settings.end()) {
                if constexpr (failsafe) return;
                else throw utils::exception::ErrorException(utils::exception::InternalCode::UnknownId, std::string(id));
            }
            this->_settings.erase(it);
        };
        _cold inline void clear(void) {this->_settings.clear();};
        template<typename T>
        _hot _nodiscard inline const T& get(std::string_view id) const {return this->at(id).template get<T>();};
        template<typename T>
        _hot _nodiscard inline T& get(std::string_view id)                                     {return this->at(id).template get<T>();};
        _hot _nodiscard inline const utils::arguments::Setting& get(std::string_view id) const {return this->at(id);};
        _hot _nodiscard inline bool contains(std::string_view id) const                        {return this->_settings.contains(id);};

        /* migration */
        template<bool force = false>
        _migration(4, 0, 0) inline utils::arguments::CastType auto_cast(const std::string& id, const std::string& setting) {return this->autoCast<force>(id, setting);};

        // ------------ Operator ---------- //
        Settings& operator=(const Settings& other) = delete;
        Settings& operator=(Settings&& other) = default;
        _hot _nodiscard inline utils::arguments::Setting& operator[](std::string_view id)             {return this->at(id);};
        _hot _nodiscard inline const utils::arguments::Setting& operator[](std::string_view id) const {return this->at(id);};

        // ---------- Constructor --------- //
        Settings() = default;
        Settings(const Settings& other) = delete;
        Settings(Settings&& other) = default;

        // ----------- Destructor --------- //
        ~Settings() = default;
};

} // namespace end
#endif /* SETTINGS_H */
