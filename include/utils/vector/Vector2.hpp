/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 29/04/2026 by @author Tsukini

File Name:
##  @file Vector2.hpp

File Description:
##  Vector hat contains 2 value respectivly x & y of undefined type
\**************************************************************/

#ifndef VECTOR2_H
    #define VECTOR2_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../concepts/OperationConcepts.hpp"        // Operation Concepts
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Code
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include "IVector.hpp"                              // utils::vector::IVector
    #include <algorithm>                                // std::clamp
    #include <ostream>                                  // std::ostream
    #include <cstddef>                                  // std::size_t
    #include <cmath>                                    // std::sqrt

    //----------------------------------------------------------------//
    /* DEFINE */

    /* limits */
    #define MAX_INDEX_VECTOR2 2

namespace utils::vector { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class Vector2: public utils::vector::IVector<T> {
    public:
        T x;
        T y;

        // ----------- Function ----------- //
        T get(std::size_t index) const final {
            if (index >= MAX_INDEX_VECTOR2)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? this->x : this->y);
        }
        Vector2 clamp(const Vector2& min, const Vector2& max) const
        {return {std::clamp(x, min.x, max.x), std::clamp(y, min.y, max.y)};}

        // ------- Special-Function ------- //
        template<typename U>
        T dot(const Vector2<U>& v) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::AddableWith<T, U> {return x * v.x + y * v.y;}
        template<typename U>
        T cross(const Vector2<U>& v) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::SubtractableWith<T, U> {return x * v.y - y * v.x;}
        T length() const
        requires utils::concepts::Multipliable<T> {return std::sqrt(x * x + y * y);}
        T lengthSquared() const
        requires utils::concepts::Multipliable<T> && utils::concepts::Addable<T> {return x * x + y * y;}
        Vector2 normalize() const
        requires utils::concepts::Divisible<T>
        {
            T len = length();
            return {x / len, y / len};
        }

        // ------------ Operator ---------- //
        T& operator[](std::size_t index) {
            if (index >= MAX_INDEX_VECTOR2)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : y);
        }
        const T& operator[](std::size_t index) const {
            if (index >= MAX_INDEX_VECTOR2)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : y);
        }

        // -------- Basic-Operator -------- //
        template<typename U>
        auto operator+(const U& v) const
        requires utils::concepts::AddableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x + v, y + v);
        }

        template<typename U>
        auto operator+(const Vector2<U>& v) const
        requires utils::concepts::AddableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x + v.x, y + v.y);
        }

        template<typename U>
        auto operator-(const U& v) const
        requires utils::concepts::SubtractableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x - v, y - v);
        }

        template<typename U>
        auto operator-(const Vector2<U>& v) const
        requires utils::concepts::SubtractableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x - v.x, y - v.y);
        }

        template<typename U>
        auto operator*(const U& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x * v, y * v);
        }

        template<typename U>
        auto operator*(const Vector2<U>& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x * v.x, y * v.y);
        }

        template<typename U>
        auto operator/(const U& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x / v, y / v);
        }

        template<typename U>
        auto operator/(const Vector2<U>& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x / v.x, y / v.y);
        }

        // -------- Bitwise-Operator -------- //
        template<typename U>
        auto operator&(const Vector2<U>& v) const
        requires utils::concepts::BitwiseAndableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x & v.x, y & v.y);
        }

        template<typename U>
        auto operator|(const Vector2<U>& v) const
        requires utils::concepts::BitwiseOrableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x | v.x, y | v.y);
        }

        template<typename U>
        auto operator^(const Vector2<U>& v) const
        requires utils::concepts::BitwiseXorableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector2<R>(x ^ v.x, y ^ v.y);
        }

        // ----- Assignment-Operator ----- //
        template<typename U>
        Vector2& operator=(const Vector2<U>& v)
        requires std::assignable_from<T&, U>
        {
            x = v.x;
            y = v.y;
            return *this;
        }

        template<typename U>
        Vector2& operator=(Vector2<U>&& v)
        requires std::assignable_from<T&, U>
        {
            x = std::move(v.x);
            y = std::move(v.y);
            return *this;
        }

        template<typename U>
        Vector2& operator+=(const U& v)
        requires utils::concepts::AddAssignableWith<T, U> {x += v; y += v; return *this;}
        template<typename U>
        Vector2& operator+=(const Vector2<U>& v)
        requires utils::concepts::AddAssignableWith<T, U> {x += v.x; y += v.y; return *this;}
        template<typename U>
        Vector2& operator-=(const U& v)
        requires utils::concepts::SubtractAssignableWith<T, U> {x -= v; y -= v; return *this;}
        template<typename U>
        Vector2& operator-=(const Vector2<U>& v)
        requires utils::concepts::SubtractAssignableWith<T, U> {x -= v.x; y -= v.y; return *this;}
        template<typename U>
        Vector2& operator*=(const U& v)
        requires utils::concepts::MultiplyAssignableWith<T, U> {x *= v; y *= v; return *this;}
        template<typename U>
        Vector2& operator*=(const Vector2<U>& v)
        requires utils::concepts::MultiplyAssignableWith<T, U> {x *= v.x; y *= v.y; return *this;}
        template<typename U>
        Vector2& operator/=(const U& v)
        requires utils::concepts::DivideAssignableWith<T, U> {x /= v; y /= v; return *this;}
        template<typename U>
        Vector2& operator/=(const Vector2<U>& v)
        requires utils::concepts::DivideAssignableWith<T, U> {x /= v.x; y /= v.y; return *this;}

        // ---------- Comparison ---------- //
        template<typename U>
        bool operator==(const U& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (x == v && y == v);}
        template<typename U>
        bool operator==(const Vector2<U>& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (x == v.x && y == v.y);}
        template<typename U>
        bool operator!=(const U& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (x != v || y != v);}
        template<typename U>
        bool operator!=(const Vector2<U>& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (x != v.x || y != v.y);}
        template<typename U>
        bool operator<(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x < v && y < v);}
        template<typename U>
        bool operator<(const Vector2<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x < v.x && y < v.y);}
        template<typename U>
        bool operator<=(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x <= v && y <= v);}
        template<typename U>
        bool operator<=(const Vector2<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x <= v.x && y <= v.y);}
        template<typename U>
        bool operator>(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x > v && y > v);}
        template<typename U>
        bool operator>(const Vector2<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x > v.x && y > v.y);}
        template<typename U>
        bool operator>=(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x >= v && y >= v);}
        template<typename U>
        bool operator>=(const Vector2<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x >= v.x && y >= v.y);}

        // ------------ Unary ------------- //
        Vector2 operator-() const
        requires utils::concepts::Negatable<T> {return {-x, -y};}

        // ---------- Constructor --------- //
        Vector2() = default;
        template<typename U, typename R>
        Vector2(U x, R y)
        requires std::constructible_from<T, U> && std::constructible_from<T, R>: x(x), y(y) {}
        template<typename U>
        Vector2(const Vector2<U>& v)
        requires std::constructible_from<T, U>: x(v.x), y(v.y) {}
        template<typename U>
        Vector2(Vector2<U>&& v)
        requires std::constructible_from<T, U&&>: x(std::move(v.x)), y(std::move(v.y)) {}

        // ----------- Destructor --------- //
        ~Vector2() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
auto operator+(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::AddableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector2<R>(lhs + rhs.x, lhs + rhs.y);
}

template<typename T, typename U>
auto operator-(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::SubtractableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector2<R>(lhs - rhs.x, lhs - rhs.y);
}

template<typename T, typename U>
auto operator*(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::MultipliableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector2<R>(lhs * rhs.x, lhs * rhs.y);
}

template<typename T, typename U>
auto operator/(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::DivisibleWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector2<R>(lhs / rhs.x, lhs / rhs.y);
}

// -------- Bitwise-Operator -------- //
template<typename T, typename U>
auto operator&(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::BitwiseAndableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector2<R>(lhs & rhs.x, lhs & rhs.y);
}

template<typename T, typename U>
auto operator|(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::BitwiseOrableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector2<R>(lhs | rhs.x, lhs | rhs.y);
}

template<typename T, typename U>
auto operator^(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::BitwiseXorableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector2<R>(lhs ^ rhs.x, lhs ^ rhs.y);
}

// -------- Comparison (reverse) -------- //
template<typename T, typename U>
bool operator==(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::EqualityComparableWith<T, U>
{return (lhs == rhs.x && lhs == rhs.y);}

template<typename T, typename U>
bool operator!=(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::EqualityComparableWith<T, U>
{return (lhs != rhs.x || lhs != rhs.y);}

template<typename T, typename U>
bool operator<(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs < rhs.x && lhs < rhs.y);}

template<typename T, typename U>
bool operator<=(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs <= rhs.x && lhs <= rhs.y);}

template<typename T, typename U>
bool operator>(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs > rhs.x && lhs > rhs.y);}

template<typename T, typename U>
bool operator>=(const T& lhs, const utils::vector::Vector2<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs >= rhs.x && lhs >= rhs.y);}

// -------- Comparison (reverse) -------- //
template<typename T>
std::ostream& operator<<(std::ostream& os, const utils::vector::Vector2<T>& v)
{return os << "(" << v.x << ", " << v.y << ")";}

} // namespace end
#endif /* VECTOR2_H */
