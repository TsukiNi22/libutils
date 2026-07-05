/**************************************************************\
Edition:
##  @date 05/07/2026 by @author Tsukini

File Name:
##  @file Point.cpp

File Description:
##  Geometry angle handling
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/math/geometry/Angle.hpp"
#include "utils/math/trigo/Convertion.hpp"
#include "utils/math/MathType.hpp"
#include <cmath>

utils::math::Direction utils::math::geometry::toLook(const utils::math::Direction& orientation)
{
    utils::math::Type pitch = utils::math::trigo::degToRad(orientation.x);
    utils::math::Type yaw = utils::math::trigo::degToRad(orientation.y);
    utils::math::Direction look;
    look.x = std::cos(pitch) * std::sin(yaw);
    look.y = std::sin(pitch);
    look.z = std::cos(pitch) * std::cos(yaw);
    return look.normalize();
}
