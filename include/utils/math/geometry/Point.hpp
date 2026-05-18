/**************************************************************\
Edition:
##  @date 18/05/2026 by @author Tsukini

File Name:
##  @file Point.hpp

File Description:
##  Prototype for point computing
\**************************************************************/

#ifndef POINT_H
    #define POINT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../MathType.hpp"  // utils::math::* (Type)

namespace utils::math::geometry { // namepoint start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* 2D */
/*
 * origin -> origin of rotation
 * point -> point to rotate
 * angle -> angle to rotate the point
 * rad -> is the angle in radian or deg
*/
utils::math::Coord2D rotatePoint2D(const utils::math::Coord2D& origin, const utils::math::Coord2D& point, utils::math::Angle angle, const bool rad = false);

/* 3D */
/*
 * origin -> origin of rotation
 * point -> point to rotate
 * orientation -> angles to rotate the point
 * rad -> is the orientation in radian or deg
*/
utils::math::Coord rotatePoint3D(const utils::math::Coord& origin, const utils::math::Coord& point, const utils::math::Direction& orientation, const bool rad = false);

} // namepoint end
#endif /* POINT_H */
