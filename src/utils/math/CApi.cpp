/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.cpp

File Description:
##  C API of utils::math
\**************************************************************/

#include "utils/math/trigo/Convertion.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/math/geometry/Angle.hpp"
#include "utils/math/geometry/Point.hpp"
#include "utils/math/MathType.hpp"
#include "utils/math/CApi.h"
#include <type_traits>
#include <cstdint>

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ types must stay the same */
static_assert(std::is_same_v<utils_math_type_t, utils::math::Type>);
static_assert(std::is_same_v<utils_math_utype_t, utils::math::UType>);
static_assert(std::is_same_v<utils_math_angle_t, utils::math::Angle>);

//----------------------------------------------------------------//
/* TOOLS */

_hot _nodiscard static utils::math::Coord2D cpp(const utils_math_coord2d_t c) {return {c.x, c.y};};
_hot _nodiscard static utils::math::Coord cpp(const utils_math_coord_t c)     {return {c.x, c.y, c.z};}
_hot _nodiscard static utils_math_coord2d_t c(const utils::math::Coord2D& v)  {return {v.x, v.y};}
_hot _nodiscard static utils_math_coord_t c(const utils::math::Coord& v)      {return {v.x, v.y, v.z};}

//----------------------------------------------------------------//
/* TRIGONOMETRY */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot utils_math_type_t utils_math_deg_to_rad(utils_math_angle_t deg)
{
    return utils::math::trigo::deg_to_rad(deg);
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot utils_math_angle_t utils_math_rad_to_deg(utils_math_type_t rad)
{
    return utils::math::trigo::rad_to_deg(rad);
}

//----------------------------------------------------------------//
/* GEOMETRY */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot utils_math_direction_t utils_math_to_look(utils_math_direction_t orientation)
{
    return c(utils::math::geometry::to_look(cpp(orientation)));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot utils_math_coord2d_t utils_math_rotate_point_2d(utils_math_coord2d_t origin, utils_math_coord2d_t point, utils_math_angle_t angle, bool rad)
{
    return c(utils::math::geometry::rotate_point_2D(cpp(origin), cpp(point), angle, rad));
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot utils_math_coord_t utils_math_rotate_point_3d(utils_math_coord_t origin, utils_math_coord_t point, utils_math_direction_t orientation, bool rad)
{
    return c(utils::math::geometry::rotate_point_3D(cpp(origin), cpp(point), cpp(orientation), rad));
}
