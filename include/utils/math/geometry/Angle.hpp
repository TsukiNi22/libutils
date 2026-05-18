/**************************************************************\
Edition:
##  @date 18/05/2026 by @author Tsukini

File Name:
##  @file Angle.hpp

File Description:
##  Prototype for angle computing
\**************************************************************/

#ifndef ANGLE_H
    #define ANGLE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../MathType.hpp"  // utils::math::* (Type)

namespace utils::math::geometry { // namepoint start
//----------------------------------------------------------------//
/* PROTOTYPE */

// Convert an orientation in deg to a look vector normalized
utils::math::Direction toLook(const utils::math::Direction& orientation);

} // namepoint end
#endif /* ANGLE_H */
