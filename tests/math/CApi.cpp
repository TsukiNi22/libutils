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
##  Unit tests of the C API of utils::math (trigonometry, geometry)
\**************************************************************/

#include "utils/math/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <numbers>

TEST(MathCApi, Trigonometry)
{
    EXPECT_DOUBLE_EQ(utils_math_deg_to_rad(180.0), std::numbers::pi);
    EXPECT_DOUBLE_EQ(utils_math_rad_to_deg(std::numbers::pi / 2.0), 90.0);
    EXPECT_DOUBLE_EQ(utils_math_rad_to_deg(utils_math_deg_to_rad(42.0)), 42.0);
}

TEST(MathCApi, RotatePoint2DIsTheCppOne)
{
    const utils::math::Coord2D expected = utils::math::geometry::rotate_point_2D({1.0, 2.0}, {3.0, 5.0}, 30.0);
    const utils_math_coord2d_t res = utils_math_rotate_point_2d({1.0, 2.0}, {3.0, 5.0}, 30.0, false);
    const utils_math_coord2d_t quarter = utils_math_rotate_point_2d({0.0, 0.0}, {1.0, 0.0}, std::numbers::pi / 2.0, true);

    EXPECT_DOUBLE_EQ(res.x, expected.x);
    EXPECT_DOUBLE_EQ(res.y, expected.y);
    EXPECT_NEAR(quarter.x, utils::math::geometry::rotate_point_2D({0.0, 0.0}, {1.0, 0.0}, std::numbers::pi / 2.0, true).x, 1e-12);
    EXPECT_NEAR(quarter.y, utils::math::geometry::rotate_point_2D({0.0, 0.0}, {1.0, 0.0}, std::numbers::pi / 2.0, true).y, 1e-12);
}

TEST(MathCApi, RotatePoint3DAndLookAreTheCppOnes)
{
    const utils::math::Coord expected = utils::math::geometry::rotate_point_3D({0.0, 1.0, 2.0}, {4.0, -1.0, 3.0}, {10.0, 20.0, 30.0});
    const utils_math_coord_t res = utils_math_rotate_point_3d({0.0, 1.0, 2.0}, {4.0, -1.0, 3.0}, {10.0, 20.0, 30.0}, false);
    const utils::math::Direction look = utils::math::geometry::to_look({15.0, 45.0, 0.0});
    const utils_math_direction_t cLook = utils_math_to_look({15.0, 45.0, 0.0});

    EXPECT_DOUBLE_EQ(res.x, expected.x);
    EXPECT_DOUBLE_EQ(res.y, expected.y);
    EXPECT_DOUBLE_EQ(res.z, expected.z);
    EXPECT_DOUBLE_EQ(cLook.x, look.x);
    EXPECT_DOUBLE_EQ(cLook.y, look.y);
    EXPECT_DOUBLE_EQ(cLook.z, look.z);
}
