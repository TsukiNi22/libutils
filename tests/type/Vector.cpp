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
##  @file Vector.cpp

File Description:
##  Unit tests of the Vector2/Vector3 (concept checked) & OVector2/OVector3 (unchecked)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <cmath>

/* Typed tests to run the same checks on the checked & unchecked versions */
template<typename V>
class Vector2Test: public ::testing::Test {};
using Vector2Types = ::testing::Types<utils::type::Vector2<int>, utils::type::OVector2<int>>;
TYPED_TEST_SUITE(Vector2Test, Vector2Types);

template<typename V>
class Vector3Test: public ::testing::Test {};
using Vector3Types = ::testing::Types<utils::type::Vector3<int>, utils::type::OVector3<int>>;
TYPED_TEST_SUITE(Vector3Test, Vector3Types);

/* -------------------------------- Vector2 -------------------------------- */
TYPED_TEST(Vector2Test, Construction) {
    TypeParam v(1, 2);
    EXPECT_EQ(v.x, 1);
    EXPECT_EQ(v.y, 2);
    TypeParam c(v);
    EXPECT_EQ(c.x, 1);
    EXPECT_EQ(c.y, 2);
}

TYPED_TEST(Vector2Test, IndexAccess) {
    TypeParam v(3, 4);
    EXPECT_EQ(v[0], 3);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v.get(0), 3);
    EXPECT_EQ(v.get(1), 4);
    v[0] = 10;
    EXPECT_EQ(v.x, 10);
}

TYPED_TEST(Vector2Test, InvalidIndex) {
    TypeParam v(3, 4);
    try {
        (void)v[2];
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::VectorInvalidIndex);
    }
    EXPECT_THROW((void)v.get(5), utils::exception::IException);
}

TYPED_TEST(Vector2Test, Arithmetic) {
    TypeParam a(1, 2), b(3, 5);
    EXPECT_EQ(a + b, TypeParam(4, 7));
    EXPECT_EQ(b - a, TypeParam(2, 3));
    EXPECT_EQ(a * b, TypeParam(3, 10));
    EXPECT_EQ(b / a, TypeParam(3, 2));
    EXPECT_EQ(a + 1, TypeParam(2, 3));
    EXPECT_EQ(b - 1, TypeParam(2, 4));
    EXPECT_EQ(a * 3, TypeParam(3, 6));
    EXPECT_EQ(b / 2, TypeParam(1, 2));
    EXPECT_EQ(-a, TypeParam(-1, -2));
}

TYPED_TEST(Vector2Test, ReverseArithmetic) {
    TypeParam a(1, 2);
    EXPECT_EQ(1 + a, TypeParam(2, 3));
    EXPECT_EQ(10 - a, TypeParam(9, 8));
    EXPECT_EQ(2 * a, TypeParam(2, 4));
    EXPECT_EQ(4 / a, TypeParam(4, 2));
}

TYPED_TEST(Vector2Test, Assignment) {
    TypeParam a(1, 2);
    a += TypeParam(1, 1);
    EXPECT_EQ(a, TypeParam(2, 3));
    a -= 1;
    EXPECT_EQ(a, TypeParam(1, 2));
    a *= 4;
    EXPECT_EQ(a, TypeParam(4, 8));
    a /= TypeParam(2, 4);
    EXPECT_EQ(a, TypeParam(2, 2));
    a = TypeParam(7, 9);
    EXPECT_EQ(a, TypeParam(7, 9));
}

TYPED_TEST(Vector2Test, IncrementDecrement) {
    TypeParam a(1, 2);
    EXPECT_EQ(++a, TypeParam(2, 3));
    EXPECT_EQ(a++, TypeParam(2, 3));
    EXPECT_EQ(a, TypeParam(3, 4));
    EXPECT_EQ(--a, TypeParam(2, 3));
    EXPECT_EQ(a--, TypeParam(2, 3));
    EXPECT_EQ(a, TypeParam(1, 2));
}

TYPED_TEST(Vector2Test, Bitwise) {
    TypeParam a(0b1100, 0b1010), b(0b1010, 0b0110);
    EXPECT_EQ(a & b, TypeParam(0b1000, 0b0010));
    EXPECT_EQ(a | b, TypeParam(0b1110, 0b1110));
    EXPECT_EQ(a ^ b, TypeParam(0b0110, 0b1100));
}

TYPED_TEST(Vector2Test, Comparison) {
    TypeParam a(1, 2), b(3, 4), c(1, 5);
    EXPECT_TRUE(a == TypeParam(1, 2));
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a != TypeParam(1, 2));
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(a < c); // component wise
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= TypeParam(3, 4));
    EXPECT_TRUE(TypeParam(2, 2) == 2);
    EXPECT_TRUE(TypeParam(2, 3) != 2);
    EXPECT_TRUE(a < 3);
    EXPECT_TRUE(2 == TypeParam(2, 2));
    EXPECT_TRUE(0 < a);
}

TYPED_TEST(Vector2Test, MinMaxClamp) {
    TypeParam a(1, 8), b(5, 3);
    EXPECT_EQ(a.min(b), TypeParam(1, 3));
    EXPECT_EQ(a.max(b), TypeParam(5, 8));
    EXPECT_EQ(TypeParam(-5, 50).clamp(TypeParam(0, 0), TypeParam(10, 10)), TypeParam(0, 10));
}

TYPED_TEST(Vector2Test, SpecialFunctions) {
    TypeParam a(3, 4), b(2, -1);
    EXPECT_EQ(a.dot(b), 2);
    EXPECT_EQ(a.cross(b), -11);
    EXPECT_EQ(a.lengthSquared(), 25);
    EXPECT_EQ(a.length(), 5);
    EXPECT_EQ(TypeParam(-7, 0).sign(), TypeParam(-1, 0));
    EXPECT_EQ(TypeParam(7, -2).sign(), TypeParam(1, -1));
}

TYPED_TEST(Vector2Test, Stream) {
    std::ostringstream oss;
    oss << TypeParam(1, -2);
    EXPECT_EQ(oss.str(), "(1, -2)");
}

TEST(Vector2, FloatingNormalize) {
    utils::type::Vector2<double> v(3.0, 4.0);
    utils::type::Vector2<double> n = v.normalize();
    EXPECT_DOUBLE_EQ(n.x, 0.6);
    EXPECT_DOUBLE_EQ(n.y, 0.8);
    EXPECT_DOUBLE_EQ(n.length(), 1.0);
}

TEST(Vector2, CommonTypePromotion) {
    utils::type::Vector2<int> a(1, 2);
    auto b = a + utils::type::Vector2<double>(0.5, 0.5);
    EXPECT_TRUE((std::is_same_v<decltype(b), utils::type::Vector2<double>>));
    EXPECT_DOUBLE_EQ(b.x, 1.5);
    auto c = a * 1.5;
    EXPECT_TRUE((std::is_same_v<decltype(c), utils::type::Vector2<double>>));
    EXPECT_DOUBLE_EQ(c.y, 3.0);
}

template<typename V>
concept HasBitwiseAnd = requires(V a) {a & a;};

TEST(Vector2, ConceptConstraints) {
    // Bitwise operation aren't allowed on floating vectors
    EXPECT_FALSE(HasBitwiseAnd<utils::type::Vector2<double>>);
    EXPECT_TRUE(HasBitwiseAnd<utils::type::Vector2<int>>);
}

TEST(Vector2, Polymorphism) {
    utils::type::Vector2<int> v(5, 6);
    const utils::type::IVector<int>& i = v;
    EXPECT_EQ(i.get(0), 5);
    EXPECT_EQ(i.get(1), 6);
}

TEST(OVector2, FloatingNormalize) {
    utils::type::OVector2<double> n = utils::type::OVector2<double>(0.0, -2.0).normalize();
    EXPECT_DOUBLE_EQ(n.x, 0.0);
    EXPECT_DOUBLE_EQ(n.y, -1.0);
}

/* -------------------------------- Vector3 -------------------------------- */
TYPED_TEST(Vector3Test, Construction) {
    TypeParam v(1, 2, 3);
    EXPECT_EQ(v.x, 1);
    EXPECT_EQ(v.y, 2);
    EXPECT_EQ(v.z, 3);
}

TYPED_TEST(Vector3Test, IndexAccess) {
    TypeParam v(3, 4, 5);
    EXPECT_EQ(v[0], 3);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v[2], 5);
    EXPECT_EQ(v.get(2), 5);
    v[2] = 9;
    EXPECT_EQ(v.z, 9);
    EXPECT_THROW((void)v[3], utils::exception::IException);
    EXPECT_THROW((void)v.get(3), utils::exception::IException);
}

TYPED_TEST(Vector3Test, Arithmetic) {
    TypeParam a(1, 2, 3), b(4, 6, 9);
    EXPECT_EQ(a + b, TypeParam(5, 8, 12));
    EXPECT_EQ(b - a, TypeParam(3, 4, 6));
    EXPECT_EQ(a * b, TypeParam(4, 12, 27));
    EXPECT_EQ(b / a, TypeParam(4, 3, 3));
    EXPECT_EQ(a + 1, TypeParam(2, 3, 4));
    EXPECT_EQ(a * 2, TypeParam(2, 4, 6));
    EXPECT_EQ(b / 3, TypeParam(1, 2, 3));
    EXPECT_EQ(-a, TypeParam(-1, -2, -3));
    EXPECT_EQ(1 + a, TypeParam(2, 3, 4));
    EXPECT_EQ(10 - a, TypeParam(9, 8, 7));
    EXPECT_EQ(2 * a, TypeParam(2, 4, 6));
    EXPECT_EQ(36 / b, TypeParam(9, 6, 4));
}

TYPED_TEST(Vector3Test, Assignment) {
    TypeParam a(1, 2, 3);
    a += TypeParam(1, 1, 1);
    EXPECT_EQ(a, TypeParam(2, 3, 4));
    a -= 2;
    EXPECT_EQ(a, TypeParam(0, 1, 2));
    a *= TypeParam(5, 5, 5);
    EXPECT_EQ(a, TypeParam(0, 5, 10));
    a /= 5;
    EXPECT_EQ(a, TypeParam(0, 1, 2));
}

TYPED_TEST(Vector3Test, IncrementDecrement) {
    TypeParam a(1, 2, 3);
    EXPECT_EQ(++a, TypeParam(2, 3, 4));
    EXPECT_EQ(a--, TypeParam(2, 3, 4));
    EXPECT_EQ(a, TypeParam(1, 2, 3));
}

TYPED_TEST(Vector3Test, Equality) {
    TypeParam a(1, 2, 3);
    EXPECT_TRUE(a == TypeParam(1, 2, 3));
    EXPECT_FALSE(a == TypeParam(1, 2, 4));
    EXPECT_TRUE(TypeParam(4, 4, 4) == 4);
}

TYPED_TEST(Vector3Test, Inequality) {
    TypeParam a(1, 2, 3);
    EXPECT_FALSE(a != TypeParam(1, 2, 3)); // same vector
    EXPECT_TRUE(a != TypeParam(1, 2, 4));  // only z differ
    EXPECT_TRUE(a != TypeParam(9, 2, 3));  // only x differ
    EXPECT_FALSE(TypeParam(4, 4, 4) != 4);
    EXPECT_TRUE(TypeParam(4, 4, 5) != 4);
    EXPECT_TRUE(5 != TypeParam(4, 4, 5));
}

TYPED_TEST(Vector3Test, Ordering) {
    TypeParam a(1, 2, 3), b(2, 3, 4);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= b);
    EXPECT_FALSE(TypeParam(1, 5, 1) < b);
    EXPECT_TRUE(a < 4);
    EXPECT_TRUE(0 < a);
}

TYPED_TEST(Vector3Test, MinMaxClamp) {
    TypeParam a(1, 8, 3), b(5, 3, 3);
    EXPECT_EQ(a.min(b), TypeParam(1, 3, 3));
    EXPECT_EQ(a.max(b), TypeParam(5, 8, 3));
    EXPECT_EQ(TypeParam(-5, 50, 5).clamp(TypeParam(0, 0, 0), TypeParam(10, 10, 10)), TypeParam(0, 10, 5));
}

TYPED_TEST(Vector3Test, SpecialFunctions) {
    TypeParam x(1, 0, 0), y(0, 1, 0), a(1, 2, 2);
    EXPECT_EQ(x.cross(y), TypeParam(0, 0, 1));
    EXPECT_EQ(y.cross(x), TypeParam(0, 0, -1));
    EXPECT_EQ(a.dot(TypeParam(2, 1, -1)), 2);
    EXPECT_EQ(a.lengthSquared(), 9);
    EXPECT_EQ(a.length(), 3);
    EXPECT_EQ(TypeParam(-3, 0, 8).sign(), TypeParam(-1, 0, 1));
}

TYPED_TEST(Vector3Test, Stream) {
    std::ostringstream oss;
    oss << TypeParam(1, -2, 3);
    EXPECT_EQ(oss.str(), "(1, -2, 3)");
}

TEST(Vector3, FloatingNormalize) {
    utils::type::Vector3<double> n = utils::type::Vector3<double>(0.0, 3.0, 4.0).normalize();
    EXPECT_DOUBLE_EQ(n.y, 0.6);
    EXPECT_DOUBLE_EQ(n.z, 0.8);
}

TEST(Vector3, Polymorphism) {
    utils::type::Vector3<int> v(5, 6, 7);
    const utils::type::IVector<int>& i = v;
    EXPECT_EQ(i.get(2), 7);
}

TEST(OVector3, FloatingNormalize) {
    utils::type::OVector3<double> n = utils::type::OVector3<double>(2.0, 0.0, 0.0).normalize();
    EXPECT_DOUBLE_EQ(n.x, 1.0);
}

/* -------------------------------- edge cases -------------------------------- */
TEST(VectorTypes, DotCrossCommonType) {
    EXPECT_DOUBLE_EQ(utils::type::Vector2<int>(1, 0).dot(utils::type::Vector2<double>(0.5, 0.0)), 0.5);
    EXPECT_DOUBLE_EQ(utils::type::Vector2<int>(1, 0).cross(utils::type::Vector2<double>(0.0, 0.5)), 0.5);
    EXPECT_DOUBLE_EQ(utils::type::Vector3<int>(1, 0, 0).dot(utils::type::Vector3<double>(0.5, 0.0, 0.0)), 0.5);
    auto c = utils::type::Vector3<int>(1, 0, 0).cross(utils::type::Vector3<double>(0.0, 0.5, 0.0));
    EXPECT_TRUE((std::is_same_v<decltype(c), utils::type::Vector3<double>>));
    EXPECT_DOUBLE_EQ(c.z, 0.5);
}

TEST(VectorTypes, NormalizeZero) {
    EXPECT_EQ(utils::type::Vector2<int>(0, 0).normalize(), utils::type::Vector2<int>(0, 0));
    EXPECT_EQ(utils::type::Vector3<int>(0, 0, 0).normalize(), utils::type::Vector3<int>(0, 0, 0));
    EXPECT_EQ(utils::type::OVector2<int>(0, 0).normalize(), utils::type::OVector2<int>(0, 0));
    EXPECT_EQ(utils::type::OVector3<int>(0, 0, 0).normalize(), utils::type::OVector3<int>(0, 0, 0));
    utils::type::Vector2<double> d = utils::type::Vector2<double>(0.0, 0.0).normalize();
    EXPECT_FALSE(std::isnan(d.x));
}

TEST(VectorTypes, OVectorReverseKeepElementType) {
    utils::type::OVector2<double> a = 2 * utils::type::OVector2<double>(1.25, 2.25);
    EXPECT_DOUBLE_EQ(a.x, 2.5);
    EXPECT_DOUBLE_EQ(a.y, 4.5);
    utils::type::OVector3<double> b = 2 * utils::type::OVector3<double>(0.5, 1.5, 2.5);
    EXPECT_DOUBLE_EQ(b.z, 5.0);
}

TEST(VectorTypes, Vector3Bitwise) {
    utils::type::Vector3<unsigned> a(0b1100, 0b1010, 0b1111), b(0b1010, 0b0110, 0b0001);
    EXPECT_EQ(a & b, utils::type::Vector3<unsigned>(0b1000, 0b0010, 0b0001));
    EXPECT_EQ(a | b, utils::type::Vector3<unsigned>(0b1110, 0b1110, 0b1111));
    EXPECT_EQ(a ^ b, utils::type::Vector3<unsigned>(0b0110, 0b1100, 0b1110));
    utils::type::OVector3<unsigned> c(1, 2, 3), d(3, 3, 3);
    EXPECT_EQ(c & d, utils::type::OVector3<unsigned>(1, 2, 3));
}
