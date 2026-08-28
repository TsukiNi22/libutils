/**************************************************************\
Edition:
##  @date 29/08/2026 by @author Tsukini

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
    #include "../type/vector/OVector2.hpp"  // utils::type::OVector2
    #include "../type/vector/OVector3.hpp"  // utils::type::OVector3
    #include <cstdint>                      // std::int32_t, std::uint64_t

namespace utils::math { // namespace start
//----------------------------------------------------------------//
/* TYPE */
using Type = double; // Type used everywhere for coord, angle, direction, ...
using UType = std::uint64_t; // Type used everywhere for unsigned coord, angle, direction, ...

/* 2D */
using V2Type = utils::type::OVector2<utils::math::Type>;
using Coord2D = utils::type::OVector2<utils::math::Type>;
using V2UType = utils::type::OVector2<utils::math::UType>;
using UCoord2D = utils::type::OVector2<utils::math::UType>;

/* 3D */
using V3Type = utils::type::OVector3<utils::math::Type>;
using Coord = utils::type::OVector3<utils::math::Type>;
using V3UType = utils::type::OVector3<utils::math::UType>;
using UCoord = utils::type::OVector3<utils::math::UType>;

/* global */
using Angle = utils::math::Type; // Generaly in deg
using Direction = utils::type::OVector3<utils::math::Angle>; // Generaly normalized
using Chunk = utils::type::OVector3<std::int32_t>; // Used for spacial partitionning

struct CFrame {
    utils::math::Coord position = {0.0, 0.0, 0.0}; // Coord
    utils::math::Direction orientation = {0.0, 0.0, 0.0}; // Orientation in deg
    utils::math::Direction look = {0.0, 0.0, 0.0}; // Orientation normalized
};

} // namespace end
#endif /* MATHTYPE_H */
