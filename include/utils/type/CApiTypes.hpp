/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApiTypes.hpp

File Description:
##  C++ side of the fixed types of the C API: storage of a C type (str: std::string) & the conversions
\**************************************************************/

#ifndef TYPE_CAPITYPES_H
    #define TYPE_CAPITYPES_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _hot, _nodiscard
    #include "../exception/CApi.hpp"        // utils::exception::c_api::not_null
    #include "CApiTypes.h"                  // C_API_*_TYPES
    #include <string>                       // std::string

namespace utils::type::c_api { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

// Storage in c++ of a type of the C side & its conversions (the value types are kept as is)
template<typename C>
struct Storage {
    using Type = C;

    // ------------ Function ---------- //
    _hot _nodiscard static inline C cpp(const C value)   {return value;};
    _hot _nodiscard static inline C c(const Type& value) {return value;};
};

// str: copied in a std::string (NULL refused), given back as its c_str (valid while the std::string lives)
template<>
struct Storage<const char*> {
    using Type = std::string;

    // ------------ Function ---------- //
    _hot _nodiscard static inline std::string cpp(const char* value)      {return utils::exception::c_api::not_null(value, "string");};
    _hot _nodiscard static inline const char* c(const std::string& value) {return value.c_str();};
};

//----------------------------------------------------------------//
/* TYPEDEF */

template<typename C>
using cpp_t = typename utils::type::c_api::Storage<C>::Type; // storage in c++ of the C type C

} // namespace end
#endif /* TYPE_CAPITYPES_H */
