/**************************************************************\
Edition:
##  @date 29/08/2026 by @author Tsukini

File Name:
##  @file Convertion.hpp

File Description:
##  Angle unit conversion (deg <-> rad)
\**************************************************************/

#ifndef CONVERTION_H
    #define CONVERTION_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _hot, _nodiscard
    #include "../MathType.hpp"                  // utils::math::* (Type)
    #include <numbers>                          // std::numbers::pi (M_PI is not standard: missing on MinGW in strict mode)

namespace utils::math::trigo { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

_hot _nodiscard inline utils::math::Type deg_to_rad(utils::math::Angle deg)
{return deg * std::numbers::pi / 180.0;};

_hot _nodiscard inline utils::math::Angle rad_to_deg(utils::math::Type rad)
{return rad / std::numbers::pi * 180.0;};

} // namespace end
#endif /* CONVERTION_H */
