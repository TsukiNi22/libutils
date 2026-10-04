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
##  @file c2dmp-hsm.hpp

File Description:
##  Header including all the different algorithms
\**************************************************************/

#ifndef C2DMPHSM_H
    #define C2DMPHSM_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* algorithm */
    #include "algorithm/foptimized.hpp" // utils::algorithms::c2dmp::c2dmp_foptimized
    #include "algorithm/optimized.hpp"  // utils::algorithms::c2dmp::c2dmp_optimized

    /* type */
    #include "utils/attribute/Attribute.hpp" // _hot, _nodiscard, _unused, _unlikely, _deprecated, _alignas
    #include <string_view>              // std::string_view
    #include <cstdint>                  // std::uint_fast8_t

namespace utils::algorithms::c2dmp { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* redirection */
template<std::uint_fast8_t prefixDepthSearch = 3, typename UIntT = std::uint_fast8_t, bool full = false> // full: count the misplaced chars of the whole 'a' (foptimized)
_hot _nodiscard inline float c2dmp(const std::string_view a, const std::string_view b)
{
    if constexpr (full) return utils::algorithms::c2dmp::c2dmp_foptimized<prefixDepthSearch, UIntT>(a, b);
    else return utils::algorithms::c2dmp::c2dmp_optimized<prefixDepthSearch, UIntT>(a, b);
}

} // namespace end
#endif /* C2DMPHSM_H */
