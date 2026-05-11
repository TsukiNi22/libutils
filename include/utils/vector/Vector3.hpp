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
##  @file Vector3.hpp

File Description:
##  Vector hat contains 3 value respectivly x, y & z of undefined type
\**************************************************************/

#ifndef VECTOR3_H
    #define VECTOR3_H

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
    #define MAX_INDEX_VECTOR3 3

namespace utils::vector { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class Vector3: public utils::vector::IVector<T> {
    public:
        T x;
        T y;
        T z;

        // ----------- Function ----------- //
        T get(std::size_t index) const final {
            if (index >= MAX_INDEX_VECTOR3)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : (index == 1 ? y : z));
        }
        Vector3 clamp(const Vector3& min, const Vector3& max) const
        {return {std::clamp(x, min.x, max.x), std::clamp(y, min.y, max.y), std::clamp(z, min.z, max.z)};}

        // ------- Special-Function ------- //
        template<typename U>
        T dot(const Vector3<U>& v) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::AddableWith<T, U> {return x * v.x + y * v.y + z * v.z;}
        template<typename U>
        Vector3 cross(const Vector3<U>& v) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::SubtractableWith<T, U>
        {
            return {
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
            };
        }
        T length() const
        requires utils::concepts::Multipliable<T> {return std::sqrt(x * x + y * y + z * z);}
        T lengthSquared() const
        requires utils::concepts::Multipliable<T> && utils::concepts::Addable<T> {return x * x + y * y + z * z;}
        Vector3 normalize() const
        requires utils::concepts::Divisible<T>
        {
            T len = length();
            return {x / len, y / len, z / len};
        }

        // ------------ Operator ---------- //
        T& operator[](std::size_t index) {
            if (index >= MAX_INDEX_VECTOR3)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : (index == 1 ? y : z));
        }
        const T& operator[](std::size_t index) const {
            if (index >= MAX_INDEX_VECTOR3)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : (index == 1 ? y : z));
        }

        // -------- Basic-Operator -------- //
        template<typename U>
        auto operator+(const U& v) const
        requires utils::concepts::AddableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector3<R>(x + v, y + v, z + v);
        }

        template<typename U>
        auto operator+(const Vector3<U>& v) const
        requires utils::concepts::AddableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector3<R>(x + v.x, y + v.y, z + v.z);
        }

        template<typename U>
        auto operator-(const U& v) const
        requires utils::concepts::SubtractableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector3<R>(x - v, y - v, z - v);
        }

        template<typename U>
        auto operator-(const Vector3<U>& v) const
        requires utils::concepts::SubtractableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector3<R>(x - v.x, y - v.y, z - v.z);
        }

        template<typename U>
        auto operator*(const U& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector3<R>(x * v, y * v, z * v);
        }

        template<typename U>
        auto operator*(const Vector3<U>& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector3<R>(x * v.x, y * v.y, z * v.z);
        }

        template<typename U>
        auto operator/(const U& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector3<R>(x / v, y / v, z / v);
        }

        template<typename U>
        auto operator/(const Vector3<U>& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {
            using R = std::common_type_t<T, U>;
            return Vector3<R>(x / v.x, y / v.y, z / v.z);
        }

        // ----- Assignment-Operator ----- //
        template<typename U>
        Vector3& operator=(const Vector3<U>& v)
        requires std::assignable_from<T&, U>
        {
            x = v.x;
            y = v.y;
            z = v.z;
            return *this;
        }

        template<typename U>
        Vector3& operator=(Vector3<U>&& v)
        requires std::assignable_from<T&, U>
        {
            x = std::move(v.x);
            y = std::move(v.y);
            z = std::move(v.z);
            return *this;
        }

        template<typename U>
        Vector3& operator+=(const U& v)
        requires utils::concepts::AddAssignableWith<T, U> {x += v; y += v; z += v; return *this;}
        template<typename U>
        Vector3& operator+=(const Vector3<U>& v)
        requires utils::concepts::AddAssignableWith<T, U> {x += v.x; y += v.y; z += v.z; return *this;}
        template<typename U>
        Vector3& operator-=(const U& v)
        requires utils::concepts::SubtractAssignableWith<T, U> {x -= v; y -= v; z -= v; return *this;}
        template<typename U>
        Vector3& operator-=(const Vector3<U>& v)
        requires utils::concepts::SubtractAssignableWith<T, U> {x -= v.x; y -= v.y; z -= v.z; return *this;}
        template<typename U>
        Vector3& operator*=(const U& v)
        requires utils::concepts::MultiplyAssignableWith<T, U> {x *= v; y *= v; z *= v; return *this;}
        template<typename U>
        Vector3& operator*=(const Vector3<U>& v)
        requires utils::concepts::MultiplyAssignableWith<T, U> {x *= v.x; y *= v.y; z *= v.z; return *this;}
        template<typename U>
        Vector3& operator/=(const U& v)
        requires utils::concepts::DivideAssignableWith<T, U> {x /= v; y /= v; z /= v; return *this;}
        template<typename U>
        Vector3& operator/=(const Vector3<U>& v)
        requires utils::concepts::DivideAssignableWith<T, U> {x /= v.x; y /= v.y; z /= v.z; return *this;}

        // ---------- Comparison ---------- //
        template<typename U>
        bool operator==(const U& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (x == v && y == v && z == v);}
        template<typename U>
        bool operator==(const Vector3<U>& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (x == v.x && y == v.y && z == v.z);}
        template<typename U>
        bool operator!=(const U& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (x != v || y != v || z == v);}
        template<typename U>
        bool operator!=(const Vector3<U>& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (x != v.x || y != v.y || z == v.z);}
        template<typename U>
        bool operator<(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x < v && y < v && z < v);}
        template<typename U>
        bool operator<(const Vector3<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x < v.x && y < v.y && z < v.z);}
        template<typename U>
        bool operator<=(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x <= v && y <= v && z <= v);}
        template<typename U>
        bool operator<=(const Vector3<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x <= v.x && y <= v.y && z <= v.z);}
        template<typename U>
        bool operator>(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x > v && y > v && z > v);}
        template<typename U>
        bool operator>(const Vector3<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x > v.x && y > v.y && z > v.z);}
        template<typename U>
        bool operator>=(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x >= v && y >= v && z >= v);}
        template<typename U>
        bool operator>=(const Vector3<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (x >= v.x && y >= v.y && z >= v.z);}

        // ------------ Unary ------------- //
        Vector3 operator-() const
        requires utils::concepts::Negatable<T> {return {-x, -y, -z};}

        // ---------- Constructor --------- //
        Vector3() = default;
        template<typename U, typename R, typename J>
        Vector3(U x, R y, J z)
        requires std::constructible_from<T, U> && std::constructible_from<T, R> && std::constructible_from<T, J>: x(x), y(y), z(z) {}
        template<typename U>
        Vector3(const Vector3<U>& v)
        requires std::constructible_from<T, U>: x(v.x), y(v.y), z(v.z) {}
        template<typename U>
        Vector3(Vector3<U>&& v)
        requires std::constructible_from<T, U&&>: x(std::move(v.x)), y(std::move(v.y)), z(std::move(v.z)) {}

        // ----------- Destructor --------- //
        ~Vector3() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
auto operator+(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::AddableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector3<R>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
}

template<typename T, typename U>
auto operator-(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::SubtractableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector3<R>(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
}

template<typename T, typename U>
auto operator*(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::MultipliableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector3<R>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

template<typename T, typename U>
auto operator/(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::DivisibleWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector3<R>(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
}

// -------- Bitwise-Operator -------- //
template<typename T, typename U>
auto operator&(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::BitwiseAndableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector3<R>(lhs & rhs.x, lhs & rhs.y, lhs & rhs.z);
}

template<typename T, typename U>
auto operator|(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::BitwiseOrableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector3<R>(lhs | rhs.x, lhs | rhs.y, lhs | rhs.z);
}

template<typename T, typename U>
auto operator^(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::BitwiseXorableWith<T, U>
{
    using R = std::common_type_t<T, U>;
    return utils::vector::Vector3<R>(lhs ^ rhs.x, lhs ^ rhs.y, lhs ^ rhs.z);
}

// -------- Comparison (reverse) -------- //
template<typename T, typename U>
bool operator==(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::EqualityComparableWith<T, U>
{return (lhs == rhs.x && lhs == rhs.y && lhs == rhs.z);}

template<typename T, typename U>
bool operator!=(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::EqualityComparableWith<T, U>
{return (lhs != rhs.x || lhs != rhs.y || lhs != rhs.z);}

template<typename T, typename U>
bool operator<(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs < rhs.x && lhs < rhs.y && lhs < rhs.z);}

template<typename T, typename U>
bool operator<=(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs <= rhs.x && lhs <= rhs.y && lhs <= rhs.z);}

template<typename T, typename U>
bool operator>(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs > rhs.x && lhs > rhs.y && lhs > rhs.z);}

template<typename T, typename U>
bool operator>=(const T& lhs, const utils::vector::Vector3<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs >= rhs.x && lhs >= rhs.y && lhs >= rhs.z);}

// -------- Output -------- //
template<typename T>
std::ostream& operator<<(std::ostream& os, const utils::vector::Vector3<T>& v)
{return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";}

} // namespace end
#endif /* VECTOR3_H */
