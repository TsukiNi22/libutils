/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/08/2026 by @author Tsukini

File Name:
##  @file fixed_string.hpp

File Description:
##  Fixed string used in template definition
\**************************************************************/

#ifndef FIXED_STRING_H
    #define FIXED_STRING_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _nodiscard
    #include <string_view>                      // std::string_view
    #include <algorithm>                        // std::copy_n
    #include <cstddef>                          // std::size_t

namespace utils::smanip { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<std::size_t N>
struct fixed_string
{
    char value[N]{};
    consteval fixed_string(const char (&str)[N]) {std::copy_n(str, N, value);}
    _nodiscard constexpr std::string_view view(void) const noexcept {return std::string_view(value, N - 1);}
    _nodiscard constexpr std::size_t size(void) const noexcept {return N - 1;}
};

// Needed to deduce N without <...>
template<std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;

} // namespace end
#endif /* FIXED_STRING_H */
