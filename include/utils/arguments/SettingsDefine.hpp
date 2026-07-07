/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 08/07/2026 by @author Tsukini

File Name:
##  @file SettingsDefine.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef SETTINGSDEFINE_H
    #define SETTINGSDEFINE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #if __has_include(<cstdfloat>)
        #include <cstdfloat>
    #endif

namespace utils::arguments { // namespace start
//----------------------------------------------------------------//
/* TYPE */

#if __has_include(<cstdfloat>)
    using float16_t  = std::float16_t;
    using float32_t  = std::float32_t;
    using float64_t  = std::float64_t;
    using float128_t = std::float128_t;
#else
    using float16_t  = float;
    using float32_t  = float;
    using float64_t  = double;
    using float128_t = long double;
#endif

//----------------------------------------------------------------//
/* ENUM */

enum class CastType: std::size_t {
    None = 0,

    /* basic */
    Byte,
    Bool,

    /* integer */
    Int8,
    Int16,
    Int32,
    Int64,

    /* unsigned integer */
    UInt8,
    UInt16,
    UInt32,
    UInt64,

    /* floating */
    Float16,
    Float32,
    Float64,
    Float128,

    /* char */
    Char8,
    Char16,
    Char32,
    U8String,
    U16String,
    U32String,

    /* huge char */
    WChar,
    WString,

    /* special */
    Path,
};

} // namespace end
#endif /* SETTINGSDEFINE_H */
