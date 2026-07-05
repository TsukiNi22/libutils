/**************************************************************\
Edition:
##  @date 05/07/2026 by @author Tsukini

File Name:
##  @file SpaceType.hpp

File Description:
##  Type definition used in math computing
\**************************************************************/

#ifndef MATHTYPE_H
    #define MATHTYPE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../vector/OVector2.hpp"   // utils::vector::OVector2
    #include "../vector/OVector3.hpp"   // utils::vector::OVector3
    #include <cstdint>                  // std::int32_t

namespace utils::math { // namespace start
//----------------------------------------------------------------//
/* TYPE */
using Type = double; // Type used everywhere for coord, angle, direction, ...

/* 2D */
using V2Type = utils::vector::OVector2<utils::math::Type>;
using Coord2D = utils::vector::OVector2<utils::math::Type>;

/* 3D */
using V3Type = utils::vector::OVector3<utils::math::Type>;
using Coord = utils::vector::OVector3<utils::math::Type>;

/* global */
using Angle = utils::math::Type; // Generaly in deg
using Direction = utils::vector::OVector3<utils::math::Angle>; // Generaly normalized
using Chunk = utils::vector::OVector3<std::int32_t>; // Used for spacial partitionning

struct CFrame {
    utils::math::Coord position = {0.0, 0.0, 0.0}; // Coord
    utils::math::Direction orientation = {0.0, 0.0, 0.0}; // Orientation in deg
    utils::math::Direction look = {0.0, 0.0, 0.0}; // Orientation normalized
};

} // namespace end
#endif /* MATHTYPE_H */
