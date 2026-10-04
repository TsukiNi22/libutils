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
##  @file FixedString.hpp

File Description:
##  Fixed string used in template definition
\**************************************************************/

#ifndef FIXEDSTRING_H
    #define FIXEDSTRING_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _hot, _nodiscard
    #include <string_view>                      // std::string_view
    #include <algorithm>                        // std::copy_n
    #include <cstddef>                          // std::size_t

namespace utils::smanip { // namespace start
//----------------------------------------------------------------//
/* STRUCT */

template<std::size_t N>
struct FixedString {
    char value[N]{};

    // ------------ Function ---------- //
    _hot _nodiscard constexpr std::string_view view(void) const noexcept {return std::string_view(this->value, N - 1);};
    _hot _nodiscard constexpr std::size_t size(void) const noexcept      {return N - 1;};

    // ---------- Constructor --------- //
    consteval FixedString(const char (&str)[N]) {std::copy_n(str, N, this->value);};
};

// Needed to deduce N without <...>
template<std::size_t N>
FixedString(const char (&)[N]) -> FixedString<N>;

} // namespace end
#endif /* FIXEDSTRING_H */
