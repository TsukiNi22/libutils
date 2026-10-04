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
##  Old name of the FixedString (kept for backward compatibility)
\**************************************************************/

#ifndef FIXED_STRING_H
    #define FIXED_STRING_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _migration
    #include "FixedString.hpp"                  // utils::smanip::FixedString
    #include <cstddef>                          // std::size_t

//----------------------------------------------------------------//
/* MIGRATION */
namespace utils::smanip {
    template<std::size_t N>
    using fixed_string _migration(4, 0, 0) = utils::smanip::FixedString<N>;
}

#endif /* FIXED_STRING_H */
