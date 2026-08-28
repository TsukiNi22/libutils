/**************************************************************\
Edition:
##  @date 29/08/2026 by @author Tsukini

File Name:
##  @file Point.hpp

File Description:
##  Prototype for point computing
\**************************************************************/

#ifndef CONVERTION_H
    #define CONVERTION_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"    // _nodiscard
    #include "../MathType.hpp"                  // utils::math::* (Type)
    #include <cmath>                            // M_PI

namespace utils::math::trigo { // namepoint start
//----------------------------------------------------------------//
/* PROTOTYPE */

_nodiscard inline utils::math::Type deg_to_rad(utils::math::Angle deg)
{return deg * M_PI / 180.0;};

_nodiscard inline utils::math::Angle rad_to_deg(utils::math::Type rad)
{return rad / M_PI * 180.0;};

} // namepoint end
#endif /* CONVERTION_H */
