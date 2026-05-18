/**************************************************************\
Edition:
##  @date 18/05/2026 by @author Tsukini

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

hot nodiscard utils::math::Coord2D utils::math::geometry::rotatePoint2D(const utils::math::Coord2D& origin, const utils::math::Coord2D& point, utils::math::Angle angle, const bool rad)
{
    utils::math::Type radian = rad ? angle : utils::math::trigo::degToRad(angle);

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

hot nodiscard utils::math::Coord utils::math::geometry::rotatePoint3D(const utils::math::Coord& origin, const utils::math::Coord& point, const utils::math::Direction& orientation, const bool rad)
{
    // Pre compute
    utils::math::Coord p = point - origin;
    utils::math::Type pitch = rad ? orientation.x : utils::math::trigo::degToRad(orientation.x);
    utils::math::Type yaw =   rad ? orientation.y : utils::math::trigo::degToRad(orientation.y);
    utils::math::Type roll =  rad ? orientation.z : utils::math::trigo::degToRad(orientation.z);

    // Rotation value
    utils::math::Type cosa = std::cos(yaw);
    utils::math::Type sina = std::sin(yaw);

    utils::math::Type cosb = std::cos(pitch);
    utils::math::Type sinb = std::sin(pitch);
    
    utils::math::Type cosc = std::cos(roll);
    utils::math::Type sinc = std::sin(roll);

    // Create the matrix
    utils::math::Type Axx = cosa * cosb;
    utils::math::Type Axy = cosa * sinb * sinc - sina * cosc;
    utils::math::Type Axz = cosa * sinb * cosc + sina * sinc;

    utils::math::Type Ayx = sina * cosb;
    utils::math::Type Ayy = sina * sinb * sinc + cosa * cosc;
    utils::math::Type Ayz = sina * sinb * cosc - cosa * sinc;

    utils::math::Type Azx = -sinb;
    utils::math::Type Azy = cosb * sinc;
    utils::math::Type Azz = cosb * cosc;

    // Apply the matrix
    utils::math::Type px = p.x;
    utils::math::Type py = p.y;
    utils::math::Type pz = p.z;
    p.x = Axx * px + Axy * py + Axz * pz;
    p.y = Ayx * px + Ayy * py + Ayz * pz;
    p.z = Azx * px + Azy * py + Azz * pz;

    // Re apply the origin
    return p + origin;
}
