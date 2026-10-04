/**************************************************************\
Edition:
##  @date 29/08/2026 by @author Tsukini

File Name:
##  @file Point.cpp

File Description:
##  Geometry point handling
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/math/geometry/Point.hpp"
#include "utils/math/trigo/Convertion.hpp"
#include "utils/math/MathType.hpp"
#include <cmath>

_hot _nodiscard utils::math::Coord2D utils::math::geometry::rotate_point_2D(const utils::math::Coord2D& origin, const utils::math::Coord2D& point, utils::math::Angle angle, const bool rad)
{
    utils::math::Type radian = rad ? angle : utils::math::trigo::deg_to_rad(angle);

    // Remove the origin
    utils::math::Type x = point.x - origin.x;
    utils::math::Type y = point.y - origin.y;

    // Rotation
    utils::math::Type cosR = std::cos(radian);
    utils::math::Type sinR = std::sin(radian);
    utils::math::Type xr = x * cosR - y * sinR;
    utils::math::Type yr = x * sinR + y * cosR;

    // Re apply the origin
    return {xr + origin.x, yr + origin.y};
}

_hot _nodiscard utils::math::Coord utils::math::geometry::rotate_point_3D(const utils::math::Coord& origin, const utils::math::Coord& point, const utils::math::Direction& orientation, const bool rad)
{
    // Same convention as to_look: roll (z), then pitch (x), then yaw (y)
    // Pre compute
    utils::math::Coord p = point - origin;
    utils::math::Type pitch = rad ? orientation.x : utils::math::trigo::deg_to_rad(orientation.x);
    utils::math::Type yaw =   rad ? orientation.y : utils::math::trigo::deg_to_rad(orientation.y);
    utils::math::Type roll =  rad ? orientation.z : utils::math::trigo::deg_to_rad(orientation.z);

    // Roll (around z)
    utils::math::Type x = p.x * std::cos(roll) - p.y * std::sin(roll);
    utils::math::Type y = p.x * std::sin(roll) + p.y * std::cos(roll);
    utils::math::Type z = p.z;

    // Pitch (around x): positive pitch raise the forward vector (+y)
    utils::math::Type y2 = y * std::cos(pitch) + z * std::sin(pitch);
    utils::math::Type z2 = -y * std::sin(pitch) + z * std::cos(pitch);
    y = y2;
    z = z2;

    // Yaw (around y): positive yaw turn the forward vector to +x
    utils::math::Type x2 = x * std::cos(yaw) + z * std::sin(yaw);
    z2 = -x * std::sin(yaw) + z * std::cos(yaw);
    x = x2;
    z = z2;

    // Re apply the origin
    return utils::math::Coord{x, y, z} + origin;
}
