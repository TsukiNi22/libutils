/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/08/2026 by @author Tsukini

File Name:
##  @file Setting.hpp

File Description:
##  Declaration of the Setting class used in Settings
\**************************************************************/

#ifndef SETTING_H
    #define SETTING_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"               // _cold, _hot, _nodiscard, _unlikely
    #include "../security/observer/Observer.hpp"        // utils::security::observer::Observer
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::* (Type)
    #include <type_traits>                              // std::remove_cvref_t
    #include <cxxabi.h>                                 // abi::__cxa_demangle
    #include <typeinfo>                                 // typeid
    #include <concepts>                                 // std::same_as
    #include <utility>                                  // std::forward, std::as_const
    #include <cstdlib>                                  // std::free
    #include <memory>                                   // std::unique_ptr
    #include <string>                                   // std::string
    #include <any>                                      // std::any, std::any_cast

namespace utils::arguments { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* tools */
_cold _nodiscard inline std::string demangle(const char* mangledName)
{
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> demangled(
        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
        std::free
    );
    return (status == 0 && demangled) ? demangled.get() : mangledName;
}

//----------------------------------------------------------------//
/* CLASS */

class Setting: private utils::security::observer::Observer<"Setting"> {
    private:
        std::any _setting;

    public:
        // ------------ Function ---------- //
        template<typename T>
        _hot _nodiscard const T& get(void) const
        {
            const T* value = std::any_cast<T>(&this->_setting); // single type check, no copy
            if (!value) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::BadCast, utils::arguments::demangle(this->_setting.type().name()) + " -> " + utils::arguments::demangle(typeid(T).name()));
            }
            return *value;
        };
        template<typename T>
        _hot _nodiscard inline T& get(void) {return const_cast<T&>(std::as_const(*this).template get<T>());}; // edit in place
        template<typename T>
        _hot _nodiscard inline bool is(void) const {return this->_setting.type() == typeid(T);};
        template<typename T>
        _hot inline void assign(T&& setting) {this->_setting = std::forward<T>(setting);}; // reuse the node (no Observer relink)

        // ------------ Operator ---------- //
        Setting& operator=(const Setting& other) = delete;
        Setting& operator=(Setting&& other) = default;
        template<typename T>
        _hot inline operator T(void) const {return this->template get<T>();};

        // ---------- Constructor --------- //
        template<typename T> requires (!std::same_as<std::remove_cvref_t<T>, utils::arguments::Setting>)
        Setting(T&& setting): _setting{std::forward<T>(setting)} {};
        Setting(const Setting& other) = delete;
        Setting(Setting&& other) = default;

        // ----------- Destructor --------- //
        ~Setting() = default;
};

} // namespace end
#endif /* SETTING_H */
