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
##  @file SharedObject.hpp

File Description:
##  Definition of the encapsulation for shared object (.so)
\**************************************************************/

#ifndef SHAREDOBJECT_H
    #define SHAREDOBJECT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../security/observer/Observer.hpp"        // utils::security::observer::Observer
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Type, utils::exception::InternalCode
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include <dlfcn.h>                                  // dlsym, dlerror
    #include <string_view>                              // std::string_view
    #include <string>                                   // std::string

namespace utils::encapsulation { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class SharedObject: private utils::security::observer::Observer<"SharedObject"> {
    private:
        void* _lib = nullptr;
        std::string _path;

    public:
        // ------------ Function ---------- //
        _cold _nodiscard inline bool isloaded(void) const {return this->_lib;};
        _cold _nodiscard inline std::string_view path(void) const {return this->_path;};
        _cold _nodiscard inline void* get(void) const {return this->_lib;};

        /* tools */
        template<typename T>
        _hot _nodiscard T loadFunction(const std::string& name)
        {
            T fn = reinterpret_cast<T>(::dlsym(this->_lib, name.c_str()));
            if (!fn) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::Dlsym, ::dlerror());
            }
            return fn;
        };

        // ------------ Operator ---------- //
        SharedObject& operator=(const SharedObject& other) = delete;
        SharedObject& operator=(SharedObject&& other) = delete;

        // ---------- Constructor --------- //
        SharedObject(const std::string& path);
        SharedObject(const SharedObject& other) = delete;
        SharedObject(SharedObject&& other) = delete;

        // ----------- Destructor --------- //
        ~SharedObject() noexcept;
};

} // namespace end
#endif /* SHAREDOBJECT_H */
