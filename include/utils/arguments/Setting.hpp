/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/07/2026 by @author Tsukini

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
    #include "../attribute/Attribute.hpp"               // nodicard
    #include "../warning/Observer.hpp"                  // utils::warning::Observer
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::* (Type)
    #include <cxxabi.h>                                 // abi::__cxa_demangle
    #include <typeindex>                                // typeid
    #include <memory>                                   // std::free, std::unique_ptr
    #include <string>                                   // std::string
    #include <any>                                      // std::any, std::any_cast

//----------------------------------------------------------------//
/* TOOLS */

static inline nodiscard std::string demangle(const char* mangled_name)
{
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> demangled(
        abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status),
        std::free
    );
    return (status == 0 && demangled) ? demangled.get() : mangled_name;
}

namespace utils::arguments { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Setting: private utils::warning::Observer {
    private:
        std::any _setting;

    public:
        // ------------ Operator ---------- //
        Setting& operator=(const Setting& other) = delete;
        Setting& operator=(Setting&& other) = delete;
        template<typename T>
        operator T(void) const {
            if (this->_setting.type() != typeid(T))
                throw utils::exception::ErrorException(utils::exception::Code::BadCast, demangle(this->_setting.type().name()) + " -> " + demangle(typeid(T).name()));
            return std::any_cast<T>(this->_setting);
        }

        // ---------- Constructor --------- //
        template<typename T>
        Setting(T setting): _setting(std::move(setting)) {}
        Setting(const Setting& other) = delete;
        Setting(Setting&& other) = delete;

        // ----------- Destructor --------- //
        ~Setting() = default;
};

} // namespace end
#endif /* SETTING_H */
