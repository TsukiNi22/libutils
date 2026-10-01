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
##  @file Matrix.cpp

File Description:
##  Unit tests of the Matrix (concept checked) & OMatrix (flat storage, unchecked)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <vector>
#include <iterator>

// Build a matrix of the given size filled row by row
template<typename M, typename T>
static M make(std::size_t rows, std::size_t cols, const std::vector<T>& values)
{
    M m(rows, cols);
    m.set(values.begin(), values.end());
    return m;
}

// Compare each value of a matrix to the expected ones (row by row)
template<typename M, typename T>
static void expectNear(const M& m, std::size_t rows, std::size_t cols, const std::vector<T>& values, double eps = 1e-9)
{
    ASSERT_EQ(m.row(), rows);
    ASSERT_EQ(m.col(), cols);
    for (std::size_t i = 0; i < rows; ++i)
        for (std::size_t j = 0; j < cols; ++j)
            EXPECT_NEAR(static_cast<double>(m.at(i, j)), static_cast<double>(values[i * cols + j]), eps) << "at (" << i << ", " << j << ")";
}

template<typename M>
class MatrixTest : public ::testing::Test {};
using MatrixTypes = ::testing::Types<utils::type::Matrix<double>, utils::type::OMatrix<double>>;
TYPED_TEST_SUITE(MatrixTest, MatrixTypes);

/* construction */
TYPED_TEST(MatrixTest, ConstructionZeroed) {
    TypeParam m(2, 3);
    EXPECT_EQ(m.size(), (std::pair<std::size_t, std::size_t>{2, 3}));
    EXPECT_EQ(m.row(), 2u);
    EXPECT_EQ(m.col(), 3u);
    expectNear(m, 2, 3, std::vector<double>(6, 0.0));
}

TYPED_TEST(MatrixTest, SquareConstruction) {
    TypeParam m(4);
    EXPECT_EQ(m.row(), 4u);
    EXPECT_EQ(m.col(), 4u);
}

TYPED_TEST(MatrixTest, NullSizeThrows) {
    try {
        TypeParam m(0, 3);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidArgument);
    }
    EXPECT_THROW(TypeParam(3, 0), utils::exception::IException);
}

TYPED_TEST(MatrixTest, CopyAndMove) {
    TypeParam a = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    TypeParam b(a);
    EXPECT_EQ(a, b);
    TypeParam c(std::move(b));
    EXPECT_EQ(c, a);
    EXPECT_EQ(b.size(), (std::pair<std::size_t, std::size_t>{0, 0}));
    TypeParam d(1);
    d = std::move(c);
    EXPECT_EQ(d, a);
    TypeParam e(1);
    e = a;
    EXPECT_EQ(e, a);
}

/* editor */
TYPED_TEST(MatrixTest, SetAndAt) {
    TypeParam m(2, 2);
    m.set(0, 1, 5.0);
    m.at(1, 0) = 7.0;
    m(1, 1) = 9.0;
    EXPECT_EQ(m.at(0, 1), 5.0);
    EXPECT_EQ(m(1, 0), 7.0);
    EXPECT_EQ(m.at(1, 1), 9.0);
}

TYPED_TEST(MatrixTest, SetIteratorsFromPosition) {
    TypeParam m(2, 3);
    std::vector<double> values = {1, 2, 3};
    m.set(values.begin(), values.end(), 0, 2); // (0,2) (1,0) (1,1)
    expectNear(m, 2, 3, std::vector<double>{0, 0, 1, 2, 3, 0});
}

TYPED_TEST(MatrixTest, SetInputIterator) {
    TypeParam m(2, 2);
    std::istringstream iss("1 2 3 4");
    m.set(std::istream_iterator<double>(iss), std::istream_iterator<double>());
    expectNear(m, 2, 2, std::vector<double>{1, 2, 3, 4});
}

TYPED_TEST(MatrixTest, SetTooManyValues) {
    TypeParam m(2, 2);
    std::vector<double> values = {1, 2, 3, 4, 5};
    try {
        m.set(values.begin(), values.end());
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::OutOfBounds);
    }
}

TYPED_TEST(MatrixTest, OutOfBounds) {
    TypeParam m(2, 3);
    EXPECT_THROW((void)m.at(2, 0), utils::exception::IException);
    EXPECT_THROW((void)m.at(0, 3), utils::exception::IException);
    EXPECT_THROW(m.set(5, 5, 1.0), utils::exception::IException);
    EXPECT_THROW((void)m.row(2), utils::exception::IException);
    EXPECT_THROW((void)m.col(3), utils::exception::IException);
    EXPECT_THROW(m.swapRow(0, 2), utils::exception::IException);
    EXPECT_THROW(m.swapCol(0, 3), utils::exception::IException);
}

TYPED_TEST(MatrixTest, Swaps) {
    TypeParam m = make<TypeParam>(2, 3, std::vector<double>{1, 2, 3, 4, 5, 6});
    m.swap({0, 0}, {1, 2});
    expectNear(m, 2, 3, std::vector<double>{6, 2, 3, 4, 5, 1});
    m.swapRow(0, 1);
    expectNear(m, 2, 3, std::vector<double>{4, 5, 1, 6, 2, 3});
    m.swapCol(0, 2);
    expectNear(m, 2, 3, std::vector<double>{1, 5, 4, 3, 2, 6});
}

TYPED_TEST(MatrixTest, Clear) {
    TypeParam m = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    m.clear();
    expectNear(m, 2, 2, std::vector<double>{0, 0, 0, 0});
}

TYPED_TEST(MatrixTest, RowCol) {
    TypeParam m = make<TypeParam>(2, 3, std::vector<double>{1, 2, 3, 4, 5, 6});
    EXPECT_EQ(m.row(1), (std::vector<double>{4, 5, 6}));
    EXPECT_EQ(m.col(2), (std::vector<double>{3, 6}));
}

/* computing */
TYPED_TEST(MatrixTest, TransposeSquare) {
    TypeParam m = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    m.transpose();
    expectNear(m, 2, 2, std::vector<double>{1, 3, 2, 4});
}

TYPED_TEST(MatrixTest, TransposeRectangle) {
    TypeParam m = make<TypeParam>(2, 3, std::vector<double>{1, 2, 3, 4, 5, 6});
    m.transpose();
    expectNear(m, 3, 2, std::vector<double>{1, 4, 2, 5, 3, 6});
}

TYPED_TEST(MatrixTest, Determinant) {
    EXPECT_NEAR((make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4})).det(), -2.0, 1e-9);
    EXPECT_NEAR((make<TypeParam>(3, 3, std::vector<double>{2, 0, 1, 1, 3, 2, 1, 1, 2})).det(), 6.0, 1e-9);
    EXPECT_NEAR((make<TypeParam>(2, 2, std::vector<double>{0, 1, 1, 0})).det(), -1.0, 1e-9);
    EXPECT_NEAR((make<TypeParam>(3, 3, std::vector<double>{1, 2, 3, 2, 4, 6, 1, 1, 1})).det(), 0.0, 1e-9);
    EXPECT_NEAR(TypeParam(1).det(), 0.0, 1e-9);
}

TYPED_TEST(MatrixTest, DeterminantNonSquare) {
    TypeParam m(2, 3);
    try {
        (void)m.det();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::UnauthorizedCall);
    }
}

TYPED_TEST(MatrixTest, Trace) {
    EXPECT_NEAR((make<TypeParam>(3, 3, std::vector<double>{1, 2, 3, 4, 5, 6, 7, 8, 9})).trace(), 15.0, 1e-9);
    EXPECT_THROW((void)TypeParam(2, 3).trace(), utils::exception::IException);
}

TYPED_TEST(MatrixTest, Invert) {
    TypeParam m = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    m.invert();
    expectNear(m, 2, 2, std::vector<double>{-2, 1, 1.5, -0.5});
}

TYPED_TEST(MatrixTest, InvertNeedPivot) {
    TypeParam m = make<TypeParam>(3, 3, std::vector<double>{0, 1, 0, 1, 0, 0, 0, 0, 2});
    m.invert();
    expectNear(m, 3, 3, std::vector<double>{0, 1, 0, 1, 0, 0, 0, 0, 0.5});
}

TYPED_TEST(MatrixTest, InvertTimesOriginalIsIdentity) {
    TypeParam m = make<TypeParam>(3, 3, std::vector<double>{2, 0, 1, 1, 3, 2, 1, 1, 2});
    TypeParam inv(m);
    inv.invert();
    expectNear(m * inv, 3, 3, std::vector<double>{1, 0, 0, 0, 1, 0, 0, 0, 1});
}

TYPED_TEST(MatrixTest, InvertSingular) {
    TypeParam m = make<TypeParam>(2, 2, std::vector<double>{1, 2, 2, 4});
    try {
        m.invert();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::MatrixSingular);
    }
    expectNear(m, 2, 2, std::vector<double>{1, 2, 2, 4}); // untouched
}

TYPED_TEST(MatrixTest, InvertNonSquare) {
    TypeParam m(2, 3);
    EXPECT_THROW(m.invert(), utils::exception::IException);
}

/* operators */
TYPED_TEST(MatrixTest, AddSub) {
    TypeParam a = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    TypeParam b = make<TypeParam>(2, 2, std::vector<double>{5, 6, 7, 8});
    expectNear(a + b, 2, 2, std::vector<double>{6, 8, 10, 12});
    expectNear(b - a, 2, 2, std::vector<double>{4, 4, 4, 4});
    a += b;
    expectNear(a, 2, 2, std::vector<double>{6, 8, 10, 12});
    a -= b;
    expectNear(a, 2, 2, std::vector<double>{1, 2, 3, 4});
}

TYPED_TEST(MatrixTest, SizeMismatch) {
    TypeParam a(2, 2), b(2, 3);
    try {
        (void)(a + b);
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::InvalidArgument);
    }
    EXPECT_THROW((void)(a - b), utils::exception::IException);
    EXPECT_THROW(a += b, utils::exception::IException);
    EXPECT_THROW((void)(b * b), utils::exception::IException); // 2x3 * 2x3
}

TYPED_TEST(MatrixTest, Product) {
    TypeParam a = make<TypeParam>(2, 3, std::vector<double>{1, 2, 3, 4, 5, 6});
    TypeParam b = make<TypeParam>(3, 2, std::vector<double>{7, 8, 9, 10, 11, 12});
    expectNear(a * b, 2, 2, std::vector<double>{58, 64, 139, 154});
    expectNear(b * a, 3, 3, std::vector<double>{39, 54, 69, 49, 68, 87, 59, 82, 105});
    a *= b;
    expectNear(a, 2, 2, std::vector<double>{58, 64, 139, 154});
}

TYPED_TEST(MatrixTest, Division) {
    TypeParam a = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    expectNear(a / a, 2, 2, std::vector<double>{1, 0, 0, 1});
    TypeParam b(a);
    b /= a;
    expectNear(b, 2, 2, std::vector<double>{1, 0, 0, 1});
}

TYPED_TEST(MatrixTest, Scalar) {
    TypeParam a = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    expectNear(a * 2.0, 2, 2, std::vector<double>{2, 4, 6, 8});
    expectNear(2.0 * a, 2, 2, std::vector<double>{2, 4, 6, 8});
    expectNear(a / 2.0, 2, 2, std::vector<double>{0.5, 1, 1.5, 2});
    a *= 3.0;
    expectNear(a, 2, 2, std::vector<double>{3, 6, 9, 12});
    a /= 3.0;
    expectNear(a, 2, 2, std::vector<double>{1, 2, 3, 4});
}

TYPED_TEST(MatrixTest, Negate) {
    TypeParam a = make<TypeParam>(1, 3, std::vector<double>{1, -2, 0});
    expectNear(-a, 1, 3, std::vector<double>{-1, 2, 0});
}

TYPED_TEST(MatrixTest, Equality) {
    TypeParam a = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    TypeParam b = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    TypeParam c = make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 5});
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_TRUE(a != TypeParam(2, 3));
    EXPECT_FALSE(a == TypeParam(4, 1));
}

TYPED_TEST(MatrixTest, Stream) {
    std::ostringstream oss;
    oss << make<TypeParam>(2, 2, std::vector<double>{1, 2, 3, 4});
    EXPECT_EQ(oss.str(), "[(1, 2)\n (3, 4)]");
    std::ostringstream single;
    single << make<TypeParam>(1, 1, std::vector<double>{7});
    EXPECT_EQ(single.str(), "[(7)]");
}

/* Matrix only (concepts & integer) */
TEST(Matrix, IntegerDeterminantBareiss) {
    EXPECT_EQ((make<utils::type::Matrix<int>>(3, 3, std::vector<int>{2, 0, 1, 1, 3, 2, 1, 1, 2})).det(), 6);
    EXPECT_EQ((make<utils::type::Matrix<int>>(2, 2, std::vector<int>{0, 1, 1, 0})).det(), -1);
    EXPECT_EQ((make<utils::type::Matrix<int>>(3, 3, std::vector<int>{6, 1, 1, 4, -2, 5, 2, 8, 7})).det(), -306);
    EXPECT_EQ((make<utils::type::Matrix<int>>(2, 2, std::vector<int>{1, 2, 2, 4})).det(), 0);
}

template<typename M>
concept CanInvert = requires(M m) {m.invert();};

TEST(Matrix, IntegerInvertDisabled) {
    EXPECT_FALSE(CanInvert<utils::type::Matrix<int>>);
    EXPECT_TRUE(CanInvert<utils::type::Matrix<double>>);
}

TEST(Matrix, CommonTypePromotion) {
    utils::type::Matrix<int> a = make<utils::type::Matrix<int>>(1, 2, std::vector<int>{1, 2});
    utils::type::Matrix<double> b = make<utils::type::Matrix<double>>(1, 2, std::vector<double>{0.5, 0.5});
    auto c = a + b;
    EXPECT_TRUE((std::is_same_v<decltype(c), utils::type::Matrix<double>>));
    expectNear(c, 1, 2, std::vector<double>{1.5, 2.5});
    auto d = a * 0.5;
    EXPECT_TRUE((std::is_same_v<decltype(d), utils::type::Matrix<double>>));
}

TEST(Matrix, ConvertingConstructor) {
    utils::type::Matrix<int> a = make<utils::type::Matrix<int>>(2, 1, std::vector<int>{3, 4});
    utils::type::Matrix<double> b(a);
    expectNear(b, 2, 1, std::vector<double>{3, 4});
}

TEST(OMatrix, IntegerDeterminantBareiss) {
    EXPECT_EQ((make<utils::type::OMatrix<int>>(3, 3, std::vector<int>{6, 1, 1, 4, -2, 5, 2, 8, 7})).det(), -306);
}

TEST(OMatrix, ConvertingConstructor) {
    utils::type::OMatrix<int> a = make<utils::type::OMatrix<int>>(2, 1, std::vector<int>{3, 4});
    utils::type::OMatrix<double> b(a);
    expectNear(b, 2, 1, std::vector<double>{3, 4});
}

TEST(OMatrix, SameResultAsMatrix) {
    std::vector<double> values = {4, 7, 2, 3, 6, 1, 2, 5, 3};
    auto m = make<utils::type::Matrix<double>>(3, 3, values);
    auto o = make<utils::type::OMatrix<double>>(3, 3, values);
    EXPECT_NEAR(m.det(), o.det(), 1e-9);
    m.invert();
    o.invert();
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            EXPECT_NEAR(m.at(i, j), o.at(i, j), 1e-9);
}
