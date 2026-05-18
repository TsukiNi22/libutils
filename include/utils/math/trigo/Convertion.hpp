/**************************************************************\
Edition:
##  @date 18/05/2026 by @author Tsukini

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
    #include "../../attribute/Attribute.hpp"    // nodiscard
    #include "../MathType.hpp"                  // utils::math::* (Type)
    #include <cmath>                            // M_PI

namespace utils::math::trigo { // namepoint start
//----------------------------------------------------------------//
/* PROTOTYPE */

inline nodiscard utils::math::Type degToRad(utils::math::Angle deg)
{return deg * M_PI / 180.0;};

inline nodiscard utils::math::Angle radToDeg(utils::math::Type rad)
{return rad / M_PI * 180.0;};

} // namepoint end
#endif /* CONVERTION_H */
