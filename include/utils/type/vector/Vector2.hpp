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
##  @file Vector2.hpp

File Description:
##  Vector hat contains 2 value respectivly x & y of undefined type
\**************************************************************/

#ifndef VECTOR2_H
    #define VECTOR2_H

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
    #define MAX_INDEX_VECTOR2 2

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class Vector2: public utils::type::IVector<T> {
    public:
        T x;
        T y;

        // ------------ Function ---------- //
        _hot _nodiscard T get(std::size_t index) const final
        {
            if (index >= MAX_INDEX_VECTOR2) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : this->y);
        };
        _hot _nodiscard Vector2 min(const Vector2& min) const
        requires utils::concepts::Comparable<T>
        {return {std::min(this->x, min.x), std::min(this->y, min.y)};};
        _hot _nodiscard Vector2 max(const Vector2& max) const
        requires utils::concepts::Comparable<T>
        {return {std::max(this->x, max.x), std::max(this->y, max.y)};};
        _hot _nodiscard Vector2 clamp(const Vector2& min, const Vector2& max) const
        requires utils::concepts::Comparable<T>
        {return {std::clamp(this->x, min.x, max.x), std::clamp(this->y, min.y, max.y)};};

        // ------- Special-Function ------- //
        template<typename U>
        _hot _nodiscard std::common_type_t<T, U> dot(const Vector2<U>& v) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::AddableWith<T, U> {return this->x * v.x + this->y * v.y;};
        template<typename U>
        _hot _nodiscard std::common_type_t<T, U> cross(const Vector2<U>& v) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::SubtractableWith<T, U> {return this->x * v.y - this->y * v.x;};
        _hot _nodiscard T length(void) const
        requires utils::concepts::Multipliable<T> {return static_cast<T>(std::sqrt(this->x * this->x + this->y * this->y));}; // truncated for the integer types
        _hot _nodiscard T lengthSquared(void) const
        requires utils::concepts::Multipliable<T> && utils::concepts::Addable<T> {return this->x * this->x + this->y * this->y;};
        _hot _nodiscard Vector2 sign(void) const
        requires utils::concepts::ComparableWith<T, int>
        {return {(this->x > 0) - (this->x < 0), (this->y > 0) - (this->y < 0)};};
        _hot _nodiscard Vector2 normalize(void) const
        requires utils::concepts::Divisible<T>
        {
            T len = this->length();
            if (len == T{}) return *this; // zero vector: no direction
            return {this->x / len, this->y / len};
        };

        // ------------ Operator ---------- //
        _hot _nodiscard T& operator[](std::size_t index)
        {
            if (index >= MAX_INDEX_VECTOR2) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : this->y);
        };
        _hot _nodiscard const T& operator[](std::size_t index) const
        {
            if (index >= MAX_INDEX_VECTOR2) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : this->y);
        };

        // -------- Basic-Operator -------- //
        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator+(const U& v) const
        requires utils::concepts::AddableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x + v, this->y + v);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator+(const Vector2<U>& v) const
        requires utils::concepts::AddableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x + v.x, this->y + v.y);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator-(const U& v) const
        requires utils::concepts::SubtractableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x - v, this->y - v);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator-(const Vector2<U>& v) const
        requires utils::concepts::SubtractableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x - v.x, this->y - v.y);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator*(const U& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x * v, this->y * v);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator*(const Vector2<U>& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x * v.x, this->y * v.y);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator/(const U& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x / v, this->y / v);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator/(const Vector2<U>& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x / v.x, this->y / v.y);};

        // -------- Special-Operator -------- //
        _hot Vector2& operator++(void)
        requires utils::concepts::Incrementable<T>
        {
            ++this->x; ++this->y;
            return *this;
        };

        _hot _nodiscard Vector2 operator++(int)
        requires utils::concepts::Incrementable<T>
        {
            Vector2 tmp = *this;
            ++(*this);
            return tmp;
        };

        _hot Vector2& operator--(void)
        requires utils::concepts::Decrementable<T>
        {
            --this->x; --this->y;
            return *this;
        };

        _hot _nodiscard Vector2 operator--(int)
        requires utils::concepts::Decrementable<T>
        {
            Vector2 tmp = *this;
            --(*this);
            return tmp;
        };

        // -------- Bitwise-Operator -------- //
        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator&(const Vector2<U>& v) const
        requires utils::concepts::BitwiseAndableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x & v.x, this->y & v.y);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator|(const Vector2<U>& v) const
        requires utils::concepts::BitwiseOrableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x | v.x, this->y | v.y);};

        template<typename U>
        _hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator^(const Vector2<U>& v) const
        requires utils::concepts::BitwiseXorableWith<T, U>
        {return utils::type::Vector2<std::common_type_t<T, U>>(this->x ^ v.x, this->y ^ v.y);};

        // ----- Assignment-Operator ----- //
        template<typename U>
        _hot Vector2& operator=(const Vector2<U>& v)
        requires std::assignable_from<T&, U>
        {
            this->x = v.x;
            this->y = v.y;
            return *this;
        };

        template<typename U>
        _hot Vector2& operator=(Vector2<U>&& v)
        requires std::assignable_from<T&, U>
        {
            this->x = std::move(v.x);
            this->y = std::move(v.y);
            return *this;
        };

        template<typename U>
        _hot Vector2& operator+=(const U& v)
        requires utils::concepts::AddAssignableWith<T, U> {this->x += v; this->y += v; return *this;};
        template<typename U>
        _hot Vector2& operator+=(const Vector2<U>& v)
        requires utils::concepts::AddAssignableWith<T, U> {this->x += v.x; this->y += v.y; return *this;};
        template<typename U>
        _hot Vector2& operator-=(const U& v)
        requires utils::concepts::SubtractAssignableWith<T, U> {this->x -= v; this->y -= v; return *this;};
        template<typename U>
        _hot Vector2& operator-=(const Vector2<U>& v)
        requires utils::concepts::SubtractAssignableWith<T, U> {this->x -= v.x; this->y -= v.y; return *this;};
        template<typename U>
        _hot Vector2& operator*=(const U& v)
        requires utils::concepts::MultiplyAssignableWith<T, U> {this->x *= v; this->y *= v; return *this;};
        template<typename U>
        _hot Vector2& operator*=(const Vector2<U>& v)
        requires utils::concepts::MultiplyAssignableWith<T, U> {this->x *= v.x; this->y *= v.y; return *this;};
        template<typename U>
        _hot Vector2& operator/=(const U& v)
        requires utils::concepts::DivideAssignableWith<T, U> {this->x /= v; this->y /= v; return *this;};
        template<typename U>
        _hot Vector2& operator/=(const Vector2<U>& v)
        requires utils::concepts::DivideAssignableWith<T, U> {this->x /= v.x; this->y /= v.y; return *this;};

        // ---------- Comparison ---------- //
        template<typename U>
        _hot _nodiscard bool operator==(const U& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (this->x == v && this->y == v);};
        template<typename U>
        _hot _nodiscard bool operator==(const Vector2<U>& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (this->x == v.x && this->y == v.y);};
        template<typename U>
        _hot _nodiscard bool operator!=(const U& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (this->x != v || this->y != v);};
        template<typename U>
        _hot _nodiscard bool operator!=(const Vector2<U>& v) const
        requires utils::concepts::EqualityComparableWith<T, U> {return (this->x != v.x || this->y != v.y);};
        template<typename U>
        _hot _nodiscard bool operator<(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x < v && this->y < v);};
        template<typename U>
        _hot _nodiscard bool operator<(const Vector2<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x < v.x && this->y < v.y);};
        template<typename U>
        _hot _nodiscard bool operator<=(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x <= v && this->y <= v);};
        template<typename U>
        _hot _nodiscard bool operator<=(const Vector2<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x <= v.x && this->y <= v.y);};
        template<typename U>
        _hot _nodiscard bool operator>(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x > v && this->y > v);};
        template<typename U>
        _hot _nodiscard bool operator>(const Vector2<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x > v.x && this->y > v.y);};
        template<typename U>
        _hot _nodiscard bool operator>=(const U& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x >= v && this->y >= v);};
        template<typename U>
        _hot _nodiscard bool operator>=(const Vector2<U>& v) const
        requires utils::concepts::ComparableWith<T, U> {return (this->x >= v.x && this->y >= v.y);};

        // ------------ Unary ------------- //
        _hot _nodiscard Vector2 operator-(void) const
        requires utils::concepts::Negatable<T> {return {-this->x, -this->y};};

        // ---------- Constructor --------- //
        Vector2() = default;
        template<typename U, typename R>
        Vector2(U x, R y)
        requires std::constructible_from<T, U> && std::constructible_from<T, R>: x(x), y(y) {};
        template<typename U>
        Vector2(const Vector2<U>& v)
        requires std::constructible_from<T, U>: x(v.x), y(v.y) {};
        template<typename U>
        Vector2(Vector2<U>&& v)
        requires std::constructible_from<T, U&&>: x(std::move(v.x)), y(std::move(v.y)) {};

        // ----------- Destructor --------- //
        ~Vector2() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator+(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::AddableWith<T, U>
{return utils::type::Vector2<std::common_type_t<T, U>>(lhs + rhs.x, lhs + rhs.y);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator-(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::SubtractableWith<T, U>
{return utils::type::Vector2<std::common_type_t<T, U>>(lhs - rhs.x, lhs - rhs.y);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator*(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::MultipliableWith<T, U>
{return utils::type::Vector2<std::common_type_t<T, U>>(lhs * rhs.x, lhs * rhs.y);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator/(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::DivisibleWith<T, U>
{return utils::type::Vector2<std::common_type_t<T, U>>(lhs / rhs.x, lhs / rhs.y);}

// -------- Bitwise-Operator -------- //
template<typename T, typename U>
_hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator&(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::BitwiseAndableWith<T, U>
{return utils::type::Vector2<std::common_type_t<T, U>>(lhs & rhs.x, lhs & rhs.y);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator|(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::BitwiseOrableWith<T, U>
{return utils::type::Vector2<std::common_type_t<T, U>>(lhs | rhs.x, lhs | rhs.y);}

template<typename T, typename U>
_hot _nodiscard utils::type::Vector2<std::common_type_t<T, U>> operator^(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::BitwiseXorableWith<T, U>
{return utils::type::Vector2<std::common_type_t<T, U>>(lhs ^ rhs.x, lhs ^ rhs.y);}

// -------- Comparison (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard bool operator==(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::EqualityComparableWith<T, U>
{return (lhs == rhs.x && lhs == rhs.y);}

template<typename T, typename U>
_hot _nodiscard bool operator!=(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::EqualityComparableWith<T, U>
{return (lhs != rhs.x || lhs != rhs.y);}

template<typename T, typename U>
_hot _nodiscard bool operator<(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs < rhs.x && lhs < rhs.y);}

template<typename T, typename U>
_hot _nodiscard bool operator<=(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs <= rhs.x && lhs <= rhs.y);}

template<typename T, typename U>
_hot _nodiscard bool operator>(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs > rhs.x && lhs > rhs.y);}

template<typename T, typename U>
_hot _nodiscard bool operator>=(const T& lhs, const utils::type::Vector2<U>& rhs)
requires utils::concepts::ComparableWith<T, U>
{return (lhs >= rhs.x && lhs >= rhs.y);}

// ------------ Stream ------------ //
template<typename T>
_cold std::ostream& operator<<(std::ostream& os, const utils::type::Vector2<T>& v)
{return os << "(" << v.x << ", " << v.y << ")";}

} // namespace end
#endif /* VECTOR2_H */
