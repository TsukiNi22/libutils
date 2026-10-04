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
##  @file OVector2.hpp

File Description:
##  Vector hat contains 2 value respectivly x & y of undefined type
##  Optimized version
\**************************************************************/

#ifndef OVECTOR2_H
    #define OVECTOR2_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"            // _cold, _hot, _nodiscard, _unlikely, _deprecated
    #include "../../concepts/OperationConcepts.hpp"     // Operation Concepts
    #include "../../exception/ExceptionDefine.hpp"      // utils::exception::InternalCode
    #include "../../exception/basic/ErrorException.hpp" // utils::exception::ErrorException
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
    #define MAX_INDEX_OVECTOR2 2

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
//class _deprecated("OVector2 dosen't have any concepts safty (Be careful!!!)") OVector2 {
class OVector2 {
    public:
        T x;
        T y;

        // ------------ Function ---------- //
        _hot _nodiscard T get(std::size_t index) const
        {
            if (index >= MAX_INDEX_OVECTOR2) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : this->y);
        };
        _hot _nodiscard inline OVector2 min(const OVector2& min) const
        {return {std::min(this->x, min.x), std::min(this->y, min.y)};};
        _hot _nodiscard inline OVector2 max(const OVector2& max) const
        {return {std::max(this->x, max.x), std::max(this->y, max.y)};};
        _hot _nodiscard inline OVector2 clamp(const OVector2& min, const OVector2& max) const
        {return {std::clamp(this->x, min.x, max.x), std::clamp(this->y, min.y, max.y)};};

        // ------- Special-Function ------- //
        template<typename U>
        _hot _nodiscard inline T dot(const OVector2<U>& v) const
        {return this->x * v.x + this->y * v.y;};
        template<typename U>
        _hot _nodiscard inline T cross(const OVector2<U>& v) const
        {return this->x * v.y - this->y * v.x;};
        _hot _nodiscard inline T length(void) const
        {return static_cast<T>(std::sqrt(this->x * this->x + this->y * this->y));}; // truncated for the integer types
        _hot _nodiscard inline T lengthSquared(void) const
        {return this->x * this->x + this->y * this->y;};
        _hot _nodiscard inline OVector2 sign(void) const
        {return {(this->x > 0) - (this->x < 0), (this->y > 0) - (this->y < 0)};};
        _hot _nodiscard OVector2 normalize(void) const
        {
            T len = this->length();
            if (len == T{}) return *this; // zero vector: no direction
            return {this->x / len, this->y / len};
        };

        // ------------ Operator ---------- //
        _hot _nodiscard T& operator[](std::size_t index)
        {
            if (index >= MAX_INDEX_OVECTOR2) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : this->y);
        };
        _hot _nodiscard const T& operator[](std::size_t index) const
        {
            if (index >= MAX_INDEX_OVECTOR2) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : this->y);
        };

        // -------- Basic-Operator -------- //
        template<typename U>
        _hot _nodiscard inline OVector2 operator+(const U& v) const
        {return {this->x + v, this->y + v};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator+(const OVector2<U>& v) const
        {return {this->x + v.x, this->y + v.y};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator-(const U& v) const
        {return {this->x - v, this->y - v};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator-(const OVector2<U>& v) const
        {return {this->x - v.x, this->y - v.y};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator*(const U& v) const
        {return {this->x * v, this->y * v};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator*(const OVector2<U>& v) const
        {return {this->x * v.x, this->y * v.y};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator/(const U& v) const
        {return {this->x / v, this->y / v};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator/(const OVector2<U>& v) const
        {return {this->x / v.x, this->y / v.y};};

        // -------- Special-Operator -------- //
        _hot inline OVector2& operator++(void)
        {++this->x; ++this->y; return *this;};
        _hot _nodiscard inline OVector2 operator++(int)
        {
            OVector2 tmp = *this;
            ++(*this);
            return tmp;
        };
        _hot inline OVector2& operator--(void)
        {--this->x; --this->y; return *this;};
        _hot _nodiscard inline OVector2 operator--(int)
        {
            OVector2 tmp = *this;
            --(*this);
            return tmp;
        };

        // -------- Bitwise-Operator -------- //
        template<typename U>
        _hot _nodiscard inline OVector2 operator&(const OVector2<U>& v) const
        {return {this->x & v.x, this->y & v.y};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator|(const OVector2<U>& v) const
        {return {this->x | v.x, this->y | v.y};};
        template<typename U>
        _hot _nodiscard inline OVector2 operator^(const OVector2<U>& v) const
        {return {this->x ^ v.x, this->y ^ v.y};};

        // ----- Assignment-Operator ----- //
        template<typename U>
        _hot OVector2& operator=(const OVector2<U>& v)
        {
            this->x = v.x;
            this->y = v.y;
            return *this;
        };

        template<typename U>
        _hot OVector2& operator=(OVector2<U>&& v)
        {
            this->x = std::move(v.x);
            this->y = std::move(v.y);
            return *this;
        };

        template<typename U>
        _hot OVector2& operator+=(const U& v)
        {this->x += v; this->y += v; return *this;};
        template<typename U>
        _hot OVector2& operator+=(const OVector2<U>& v)
        {this->x += v.x; this->y += v.y; return *this;};
        template<typename U>
        _hot OVector2& operator-=(const U& v)
        {this->x -= v; this->y -= v; return *this;};
        template<typename U>
        _hot OVector2& operator-=(const OVector2<U>& v)
        {this->x -= v.x; this->y -= v.y; return *this;};
        template<typename U>
        _hot OVector2& operator*=(const U& v)
        {this->x *= v; this->y *= v; return *this;};
        template<typename U>
        _hot OVector2& operator*=(const OVector2<U>& v)
        {this->x *= v.x; this->y *= v.y; return *this;};
        template<typename U>
        _hot OVector2& operator/=(const U& v)
        {this->x /= v; this->y /= v; return *this;};
        template<typename U>
        _hot OVector2& operator/=(const OVector2<U>& v)
        {this->x /= v.x; this->y /= v.y; return *this;};

        // ---------- Comparison ---------- //
        template<typename U>
        _hot _nodiscard inline bool operator==(const U& v) const
        {return (this->x == v && this->y == v);};
        template<typename U>
        _hot _nodiscard inline bool operator==(const OVector2<U>& v) const
        {return (this->x == v.x && this->y == v.y);};
        template<typename U>
        _hot _nodiscard inline bool operator!=(const U& v) const
        {return (this->x != v || this->y != v);};
        template<typename U>
        _hot _nodiscard inline bool operator!=(const OVector2<U>& v) const
        {return (this->x != v.x || this->y != v.y);};
        template<typename U>
        _hot _nodiscard inline bool operator<(const U& v) const
        {return (this->x < v && this->y < v);};
        template<typename U>
        _hot _nodiscard inline bool operator<(const OVector2<U>& v) const
        {return (this->x < v.x && this->y < v.y);};
        template<typename U>
        _hot _nodiscard inline bool operator<=(const U& v) const
        {return (this->x <= v && this->y <= v);};
        template<typename U>
        _hot _nodiscard inline bool operator<=(const OVector2<U>& v) const
        {return (this->x <= v.x && this->y <= v.y);};
        template<typename U>
        _hot _nodiscard inline bool operator>(const U& v) const
        {return (this->x > v && this->y > v);};
        template<typename U>
        _hot _nodiscard inline bool operator>(const OVector2<U>& v) const
        {return (this->x > v.x && this->y > v.y);};
        template<typename U>
        _hot _nodiscard inline bool operator>=(const U& v) const
        {return (this->x >= v && this->y >= v);};
        template<typename U>
        _hot _nodiscard inline bool operator>=(const OVector2<U>& v) const
        {return (this->x >= v.x && this->y >= v.y);};

        // ------------ Unary ------------- //
        _hot _nodiscard inline OVector2 operator-(void) const
        {return {-this->x, -this->y};};

        // ---------- Constructor --------- //
        OVector2() = default;
        template<typename U, typename R>
        OVector2(U x, R y): x(x), y(y) {};
        template<typename U>
        OVector2(const OVector2<U>& v): x(v.x), y(v.y) {};
        template<typename U>
        OVector2(OVector2<U>&& v): x(std::move(v.x)), y(std::move(v.y)) {};

        // ----------- Destructor --------- //
        ~OVector2() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector2<U> operator+(const T& lhs, const utils::type::OVector2<U>& rhs)
{return {lhs + rhs.x, lhs + rhs.y};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector2<U> operator-(const T& lhs, const utils::type::OVector2<U>& rhs)
{return {lhs - rhs.x, lhs - rhs.y};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector2<U> operator*(const T& lhs, const utils::type::OVector2<U>& rhs)
{return {lhs * rhs.x, lhs * rhs.y};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector2<U> operator/(const T& lhs, const utils::type::OVector2<U>& rhs)
{return {lhs / rhs.x, lhs / rhs.y};}

// -------- Bitwise-Operator -------- //
template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector2<U> operator&(const T& lhs, const utils::type::OVector2<U>& rhs)
{return {lhs & rhs.x, lhs & rhs.y};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector2<U> operator|(const T& lhs, const utils::type::OVector2<U>& rhs)
{return {lhs | rhs.x, lhs | rhs.y};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector2<U> operator^(const T& lhs, const utils::type::OVector2<U>& rhs)
{return {lhs ^ rhs.x, lhs ^ rhs.y};}

// -------- Comparison (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard inline bool operator==(const T& lhs, const utils::type::OVector2<U>& rhs)
{return (lhs == rhs.x && lhs == rhs.y);}

template<typename T, typename U>
_hot _nodiscard inline bool operator!=(const T& lhs, const utils::type::OVector2<U>& rhs)
{return (lhs != rhs.x || lhs != rhs.y);}

template<typename T, typename U>
_hot _nodiscard inline bool operator<(const T& lhs, const utils::type::OVector2<U>& rhs)
{return (lhs < rhs.x && lhs < rhs.y);}

template<typename T, typename U>
_hot _nodiscard inline bool operator<=(const T& lhs, const utils::type::OVector2<U>& rhs)
{return (lhs <= rhs.x && lhs <= rhs.y);}

template<typename T, typename U>
_hot _nodiscard inline bool operator>(const T& lhs, const utils::type::OVector2<U>& rhs)
{return (lhs > rhs.x && lhs > rhs.y);}

template<typename T, typename U>
_hot _nodiscard inline bool operator>=(const T& lhs, const utils::type::OVector2<U>& rhs)
{return (lhs >= rhs.x && lhs >= rhs.y);}

// ------------ Stream ------------ //
template<typename T>
_cold std::ostream& operator<<(std::ostream& os, const utils::type::OVector2<T>& v)
{return os << "(" << v.x << ", " << v.y << ")";}

} // namespace end
#endif /* OVECTOR2_H */
