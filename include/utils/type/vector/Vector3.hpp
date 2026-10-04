/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/08/2026 by @author Tsukini

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
    #include "../../attribute/Attribute.hpp"            // _cold, _hot, _nodiscard, _unlikely
    #include "../../concepts/OperationConcepts.hpp"     // Operation Concepts
    #include "../../exception/ExceptionDefine.hpp"      // utils::exception::InternalCode
    #include "../../exception/basic/ErrorException.hpp" // utils::exception::ErrorException
    #include "IVector.hpp"                              // utils::type::IVector
    #include <type_traits>                              // std::common_type_t
    #include <algorithm>                                // std::min, std::max, std::clamp
    #include <concepts>                                 // std::assignable_from, std::constructible_from
    #include <utility>                                  // std::move
    #include <ostream>                                  // std::ostream
    #include <cstddef>                                  // std::size_t
    #include <cmath>                                    // std::sqrt

    //----------------------------------------------------------------//
    /* DEFINE */

    /* limits */
    #define MAX_INDEX_VECTOR3 3

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class Vector3: public utils::type::IVector<T> {
    public:
        T x;
        T y;
        T z;

        // ------------ Function ---------- //
        _hot _nodiscard T get(std::size_t index) const final
        {
            if (index >= MAX_INDEX_VECTOR3) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : (index == 1 ? this->y : this->z));
        };
        _hot _nodiscard Vector3 min(const Vector3& min) const
        requires utils::concepts::Comparable<T>
        {return {std::min(this->x, min.x), std::min(this->y, min.y), std::min(this->z, min.z)};};
        _hot _nodiscard Vector3 max(const Vector3& max) const
        requires utils::concepts::Comparable<T>
        {return {std::max(this->x, max.x), std::max(this->y, max.y), std::max(this->z, max.z)};};
        _hot _nodiscard Vector3 clamp(const Vector3& min, const Vector3& max) const
        requires utils::concepts::Comparable<T>
        {return {std::clamp(this->x, min.x, max.x), std::clamp(this->y, min.y, max.y), std::clamp(this->z, min.z, max.z)};};

        // ------- Special-Function ------- //
        template<typename U>
        _hot _nodiscard std::common_type_t<T, U> dot(const Vector3<U>& v) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::AddableWith<T, U> {return this->x * v.x + this->y * v.y + this->z * v.z;};
        template<typename U>
        Vector3<std::common_type_t<T, U>> cross(const Vector3<U>& v) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::SubtractableWith<T, U>
        {
            return Vector3<std::common_type_t<T, U>>(
                this->y * v.z - this->z * v.y,
                this->z * v.x - this->x * v.z,
                this->x * v.y - this->y * v.x
            );
        };
        _hot _nodiscard T length(void) const
        requires utils::concepts::Multipliable<T> {return static_cast<T>(std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z));}; // truncated for the integer types
        _hot _nodiscard T lengthSquared(void) const
        requires utils::concepts::Multipliable<T> && utils::concepts::Addable<T> {return this->x * this->x + this->y * this->y + this->z * this->z;};
        _hot _nodiscard Vector3 sign(void) const
        requires utils::concepts::ComparableWith<T, int>
        {return {(this->x > 0) - (this->x < 0), (this->y > 0) - (this->y < 0), (this->z > 0) - (this->z < 0)};};
        _hot _nodiscard Vector3 normalize(void) const
        requires utils::concepts::Divisible<T>
        {
            T len = this->length();
            if (len == T{}) return *this; // zero vector: no direction
            return {this->x / len, this->y / len, this->z / len};
        };

        // ------------ Operator ---------- //
        _hot _nodiscard T& operator[](std::size_t index)
        {
            if (index >= MAX_INDEX_VECTOR3) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : (index == 1 ? this->y : this->z));
        };
        _hot _nodiscard const T& operator[](std::size_t index) const
        {
            if (index >= MAX_INDEX_VECTOR3) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : (index == 1 ? this->y : this->z));
        };

        // -------- Basic-Operator -------- //
        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator+(const U& v) const
        requires utils::concepts::AddableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x + v, this->y + v, this->z + v);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator+(const Vector3<U>& v) const
        requires utils::concepts::AddableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x + v.x, this->y + v.y, this->z + v.z);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator-(const U& v) const
        requires utils::concepts::SubtractableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x - v, this->y - v, this->z - v);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator-(const Vector3<U>& v) const
        requires utils::concepts::SubtractableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x - v.x, this->y - v.y, this->z - v.z);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator*(const U& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x * v, this->y * v, this->z * v);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator*(const Vector3<U>& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x * v.x, this->y * v.y, this->z * v.z);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator/(const U& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x / v, this->y / v, this->z / v);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator/(const Vector3<U>& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x / v.x, this->y / v.y, this->z / v.z);};

        // -------- Special-Operator -------- //
        _hot Vector3& operator++(void)
        requires utils::concepts::Incrementable<T>
        {
            ++this->x; ++this->y; ++this->z;
            return *this;
        };

        _hot _nodiscard Vector3 operator++(int)
        requires utils::concepts::Incrementable<T>
        {
            Vector3 tmp = *this;
            ++(*this);
            return tmp;
        };

        _hot Vector3& operator--(void)
        requires utils::concepts::Decrementable<T>
        {
            --this->x; --this->y; --this->z;
            return *this;
        };

        _hot _nodiscard Vector3 operator--(int)
        requires utils::concepts::Decrementable<T>
        {
            Vector3 tmp = *this;
            --(*this);
            return tmp;
        };

        // -------- Bitwise-Operator -------- //
        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator&(const Vector3<U>& v) const
        requires utils::concepts::BitwiseAndableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x & v.x, this->y & v.y, this->z & v.z);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator|(const Vector3<U>& v) const
        requires utils::concepts::BitwiseOrableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x | v.x, this->y | v.y, this->z | v.z);};

        template<typename U>
        _hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator^(const Vector3<U>& v) const
        requires utils::concepts::BitwiseXorableWith<T, U>
        {return utils::type::Vector3<std::common_type_t<T, U>>(this->x ^ v.x, this->y ^ v.y, this->z ^ v.z);};

        // ----- Assignment-Operator ----- //
        template<typename U>
        _hot Vector3& operator=(const Vector3<U>& v)
        requires std::assignable_from<T&, U>
        {
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
            return *this;
        };

        template<typename U>
        _hot Vector3& operator=(Vector3<U>&& v)
        requires std::assignable_from<T&, U>
        {
            this->x = std::move(v.x);
            this->y = std::move(v.y);
            this->z = std::move(v.z);
            return *this;
        };

        template<typename U>
        _hot Vector3& operator+=(const U& v)
        requires utils::concepts::AddAssignableWith<T, U> {this->x += v; this->y += v; this->z += v; return *this;};
        template<typename U>
        _hot Vector3& operator+=(const Vector3<U>& v)
        requires utils::concepts::AddAssignableWith<T, U> {this->x += v.x; this->y += v.y; this->z += v.z; return *this;};
        template<typename U>
        _hot Vector3& operator-=(const U& v)
        requires utils::concepts::SubtractAssignableWith<T, U> {this->x -= v; this->y -= v; this->z -= v; return *this;};
        template<typename U>
        _hot Vector3& operator-=(const Vector3<U>& v)
        requires utils::concepts::SubtractAssignableWith<T, U> {this->x -= v.x; this->y -= v.y; this->z -= v.z; return *this;};
        template<typename U>
        _hot Vector3& operator*=(const U& v)
        requires utils::concepts::MultiplyAssignableWith<T, U> {this->x *= v; this->y *= v; this->z *= v; return *this;};
        template<typename U>
        _hot Vector3& operator*=(const Vector3<U>& v)
        requires utils::concepts::MultiplyAssignableWith<T, U> {this->x *= v.x; this->y *= v.y; this->z *= v.z; return *this;};
        template<typename U>
        _hot Vector3& operator/=(const U& v)
        requires utils::concepts::DivideAssignableWith<T, U> {this->x /= v; this->y /= v; this->z /= v; return *this;};
        template<typename U>
        _hot Vector3& operator/=(const Vector3<U>& v)
        requires utils::concepts::DivideAssignableWith<T, U> {this->x /= v.x; this->y /= v.y; this->z /= v.z; return *this;};

        // ---------- Comparison ---------- //
        template<typename U>
        _hot _nodiscard bool operator==(const U& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (this->x == v && this->y == v && this->z == v);};
        template<typename U>
        _hot _nodiscard bool operator==(const Vector3<U>& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (this->x == v.x && this->y == v.y && this->z == v.z);};
        template<typename U>
        _hot _nodiscard bool operator!=(const U& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (this->x != v || this->y != v || this->z != v);};
        template<typename U>
        _hot _nodiscard bool operator!=(const Vector3<U>& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (this->x != v.x || this->y != v.y || this->z != v.z);};
        template<typename U>
        _hot _nodiscard bool operator<(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x < v && this->y < v && this->z < v);};
        template<typename U>
        _hot _nodiscard bool operator<(const Vector3<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x < v.x && this->y < v.y && this->z < v.z);};
        template<typename U>
        _hot _nodiscard bool operator<=(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x <= v && this->y <= v && this->z <= v);};
        template<typename U>
        _hot _nodiscard bool operator<=(const Vector3<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x <= v.x && this->y <= v.y && this->z <= v.z);};
        template<typename U>
        _hot _nodiscard bool operator>(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x > v && this->y > v && this->z > v);};
        template<typename U>
        _hot _nodiscard bool operator>(const Vector3<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x > v.x && this->y > v.y && this->z > v.z);};
        template<typename U>
        _hot _nodiscard bool operator>=(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x >= v && this->y >= v && this->z >= v);};
        template<typename U>
        _hot _nodiscard bool operator>=(const Vector3<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x >= v.x && this->y >= v.y && this->z >= v.z);};

        // ------------ Unary ------------- //
        _hot _nodiscard Vector3 operator-(void) const
        requires utils::concepts::Negatable<T> {return {-this->x, -this->y, -this->z};};

        // ---------- Constructor --------- //
        Vector3() = default;
        template<typename U, typename R, typename J>
        Vector3(U x, R y, J z)
        requires std::constructible_from<T, U> && std::constructible_from<T, R> && std::constructible_from<T, J>: x(x), y(y), z(z) {};
        template<typename U>
        Vector3(const Vector3<U>& v)
        requires std::constructible_from<T, U>: x(v.x), y(v.y), z(v.z) {};
        template<typename U>
        Vector3(Vector3<U>&& v)
        requires std::constructible_from<T, U&&>: x(std::move(v.x)), y(std::move(v.y)), z(std::move(v.z)) {};

        // ----------- Destructor --------- //
        ~Vector3() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator+(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::AddableWith<T, U>
{return utils::type::Vector3<std::common_type_t<T, U>>(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator-(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::SubtractableWith<T, U>
{return utils::type::Vector3<std::common_type_t<T, U>>(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator*(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::MultipliableWith<T, U>
{return utils::type::Vector3<std::common_type_t<T, U>>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator/(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::DivisibleWith<T, U>
{return utils::type::Vector3<std::common_type_t<T, U>>(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);}

// -------- Bitwise-Operator -------- //
template<typename T, typename U>
_hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator&(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::BitwiseAndableWith<T, U>
{return utils::type::Vector3<std::common_type_t<T, U>>(lhs & rhs.x, lhs & rhs.y, lhs & rhs.z);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator|(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::BitwiseOrableWith<T, U>
{return utils::type::Vector3<std::common_type_t<T, U>>(lhs | rhs.x, lhs | rhs.y, lhs | rhs.z);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector3<std::common_type_t<T, U>> operator^(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::BitwiseXorableWith<T, U>
{return utils::type::Vector3<std::common_type_t<T, U>>(lhs ^ rhs.x, lhs ^ rhs.y, lhs ^ rhs.z);}

// -------- Comparison (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard bool operator==(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::EqualityComparableWith<T, U>
{return (lhs == rhs.x && lhs == rhs.y && lhs == rhs.z);}

template<typename T, typename U>
_hot _nodiscard bool operator!=(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::EqualityComparableWith<T, U>
{return (lhs != rhs.x || lhs != rhs.y || lhs != rhs.z);}

template<typename T, typename U>
_hot _nodiscard bool operator<(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs < rhs.x && lhs < rhs.y && lhs < rhs.z);}

template<typename T, typename U>
_hot _nodiscard bool operator<=(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs <= rhs.x && lhs <= rhs.y && lhs <= rhs.z);}

template<typename T, typename U>
_hot _nodiscard bool operator>(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs > rhs.x && lhs > rhs.y && lhs > rhs.z);}

template<typename T, typename U>
_hot _nodiscard bool operator>=(const T& lhs, const utils::type::Vector3<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs >= rhs.x && lhs >= rhs.y && lhs >= rhs.z);}

// -------- Output -------- //
template<typename T>
_cold std::ostream& operator<<(std::ostream& os, const utils::type::Vector3<T>& v)
{return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";}

} // namespace end
#endif /* VECTOR3_H */
