/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/10/2026 by @author Tsukini

File Name:
##  @file Geometry.cpp

File Description:
##  Unit tests of the math part (trigo convertion, point rotation, look vector)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <cmath>

#define EPS 1e-9

/* Convertion */
TEST(Trigo, DegToRad) {
    EXPECT_NEAR(utils::math::trigo::deg_to_rad(0.0), 0.0, EPS);
    EXPECT_NEAR(utils::math::trigo::deg_to_rad(180.0), M_PI, EPS);
    EXPECT_NEAR(utils::math::trigo::deg_to_rad(90.0), M_PI / 2.0, EPS);
    EXPECT_NEAR(utils::math::trigo::deg_to_rad(-360.0), -2.0 * M_PI, EPS);
}

TEST(Trigo, RadToDeg) {
    EXPECT_NEAR(utils::math::trigo::rad_to_deg(M_PI), 180.0, EPS);
    EXPECT_NEAR(utils::math::trigo::rad_to_deg(M_PI / 4.0), 45.0, EPS);
}

TEST(Trigo, RoundTrip) {
    for (double deg = -720.0; deg <= 720.0; deg += 37.5)
        EXPECT_NEAR(utils::math::trigo::rad_to_deg(utils::math::trigo::deg_to_rad(deg)), deg, 1e-9);
}

/* Point 2D */
TEST(Point, Rotate2DDeg) {
    utils::math::Coord2D p = utils::math::geometry::rotate_point_2D({0.0, 0.0}, {1.0, 0.0}, 90.0);
    EXPECT_NEAR(p.x, 0.0, EPS);
    EXPECT_NEAR(p.y, 1.0, EPS);
}

TEST(Point, Rotate2DRad) {
    utils::math::Coord2D p = utils::math::geometry::rotate_point_2D({0.0, 0.0}, {1.0, 0.0}, M_PI, true);
    EXPECT_NEAR(p.x, -1.0, EPS);
    EXPECT_NEAR(p.y, 0.0, EPS);
}

TEST(Point, Rotate2DAroundOrigin) {
    utils::math::Coord2D p = utils::math::geometry::rotate_point_2D({1.0, 1.0}, {2.0, 1.0}, -90.0);
    EXPECT_NEAR(p.x, 1.0, EPS);
    EXPECT_NEAR(p.y, 0.0, EPS);
}

TEST(Point, Rotate2DFullTurn) {
    utils::math::Coord2D p = utils::math::geometry::rotate_point_2D({3.0, -2.0}, {5.5, 7.25}, 360.0);
    EXPECT_NEAR(p.x, 5.5, 1e-9);
    EXPECT_NEAR(p.y, 7.25, 1e-9);
}

/* Point 3D */
TEST(Point, Rotate3DIdentity) {
    utils::math::Coord p = utils::math::geometry::rotate_point_3D({0.0, 0.0, 0.0}, {1.0, 2.0, 3.0}, {0.0, 0.0, 0.0});
    EXPECT_NEAR(p.x, 1.0, EPS);
    EXPECT_NEAR(p.y, 2.0, EPS);
    EXPECT_NEAR(p.z, 3.0, EPS);
}

TEST(Point, Rotate3DYaw) {
    // yaw (orientation.y) rotate around the y axis (same convention as to_look): forward z -> x
    utils::math::Coord p = utils::math::geometry::rotate_point_3D({0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 90.0, 0.0});
    EXPECT_NEAR(p.x, 1.0, EPS);
    EXPECT_NEAR(p.y, 0.0, EPS);
    EXPECT_NEAR(p.z, 0.0, EPS);
}

TEST(Point, Rotate3DPitch) {
    // positive pitch raise the forward vector
    utils::math::Coord p = utils::math::geometry::rotate_point_3D({0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {90.0, 0.0, 0.0});
    EXPECT_NEAR(p.y, 1.0, EPS);
    EXPECT_NEAR(p.z, 0.0, EPS);
}

TEST(Point, Rotate3DMatchesToLook) {
    // rotating the forward vector by an orientation give the look vector of this orientation
    for (double pitch = -80.0; pitch <= 80.0; pitch += 40.0)
        for (double yaw = 0.0; yaw < 360.0; yaw += 45.0)
            for (double roll = 0.0; roll < 360.0; roll += 90.0) {
                utils::math::Coord p = utils::math::geometry::rotate_point_3D({0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {pitch, yaw, roll});
                utils::math::Direction look = utils::math::geometry::to_look({pitch, yaw, roll});
                EXPECT_NEAR(p.x, look.x, 1e-9) << pitch << " " << yaw << " " << roll;
                EXPECT_NEAR(p.y, look.y, 1e-9) << pitch << " " << yaw << " " << roll;
                EXPECT_NEAR(p.z, look.z, 1e-9) << pitch << " " << yaw << " " << roll;
            }
}

TEST(Point, Rotate3DKeepDistance) {
    utils::math::Coord origin{1.0, -1.0, 2.0};
    utils::math::Coord point{4.0, 3.0, -5.0};
    utils::math::Coord p = utils::math::geometry::rotate_point_3D(origin, point, {33.0, 71.0, -12.0});
    EXPECT_NEAR((p - origin).length(), (point - origin).length(), 1e-9);
}

TEST(Point, Rotate3DAroundOrigin) {
    utils::math::Coord p = utils::math::geometry::rotate_point_3D({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {45.0, 45.0, 45.0});
    EXPECT_NEAR(p.x, 1.0, EPS);
    EXPECT_NEAR(p.y, 1.0, EPS);
    EXPECT_NEAR(p.z, 1.0, EPS);
}

/* Angle */
TEST(Angle, ToLookForward) {
    utils::math::Direction look = utils::math::geometry::to_look({0.0, 0.0, 0.0});
    EXPECT_NEAR(look.x, 0.0, EPS);
    EXPECT_NEAR(look.y, 0.0, EPS);
    EXPECT_NEAR(look.z, 1.0, EPS);
}

TEST(Angle, ToLookUp) {
    utils::math::Direction look = utils::math::geometry::to_look({90.0, 0.0, 0.0});
    EXPECT_NEAR(look.y, 1.0, EPS);
}

TEST(Angle, ToLookSide) {
    utils::math::Direction look = utils::math::geometry::to_look({0.0, 90.0, 0.0});
    EXPECT_NEAR(look.x, 1.0, EPS);
    EXPECT_NEAR(look.z, 0.0, EPS);
}

TEST(Angle, ToLookIsNormalized) {
    for (double pitch = -80.0; pitch <= 80.0; pitch += 20.0)
        for (double yaw = 0.0; yaw < 360.0; yaw += 45.0)
            EXPECT_NEAR(utils::math::geometry::to_look({pitch, yaw, 0.0}).length(), 1.0, 1e-9);
}

TEST(CFrame, DefaultIsOrigin) {
    utils::math::CFrame frame;
    EXPECT_EQ(frame.position.x, 0.0);
    EXPECT_EQ(frame.position.y, 0.0);
    EXPECT_EQ(frame.position.z, 0.0);
    EXPECT_EQ(frame.orientation.x, 0.0);
    EXPECT_EQ(frame.look.z, 0.0);
}
