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
    #include "../attribute/Attribute.hpp"               // outdated
    #include "../concepts/OperationConcepts.hpp"        // Operation Concepts
    #include "../exception/ExceptionDefine.hpp"         // utils::exception::Code
    #include "../exception/basic/ErrorException.hpp"    // utils::exception::ErrorException
    #include <algorithm>                                // std::clamp
    #include <ostream>                                  // std::ostream
    #include <cstddef>                                  // std::size_t
    #include <cmath>                                    // std::sqrt

    //----------------------------------------------------------------//
    /* DEFINE */

    /* limits */
    #define MAX_INDEX_OVECTOR2 2

namespace utils::vector { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class outdated("OVector2 dosen't have any concepts safty (Be careful!!!)") OVector2 {
    public:
        T x;
        T y;

        // ----------- Function ----------- //
        T get(std::size_t index) const {
            if (index >= MAX_INDEX_OVECTOR2)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? this->x : this->y);
        }
        inline OVector2 clamp(const OVector2& min, const OVector2& max) const
        {return {std::clamp(x, min.x, max.x), std::clamp(y, min.y, max.y)};}

        // ------- Special-Function ------- //
        template<typename U>
        inline T dot(const OVector2<U>& v) const
        {return x * v.x + y * v.y;}
        template<typename U>
        inline T cross(const OVector2<U>& v) const
        {return x * v.y - y * v.x;}
        inline T length() const
        {return std::sqrt(x * x + y * y);}
        inline T lengthSquared() const
        {return x * x + y * y;}
        OVector2 normalize() const
        {
            T len = length();
            return {x / len, y / len};
        }

        // ------------ Operator ---------- //
        T& operator[](std::size_t index) {
            if (index >= MAX_INDEX_OVECTOR2)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : y);
        }
        const T& operator[](std::size_t index) const {
            if (index >= MAX_INDEX_OVECTOR2)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : y);
        }

        // -------- Basic-Operator -------- //
        template<typename U>
        inline OVector2 operator+(const U& v) const
        {return {x + v, y + v};}
        template<typename U>
        inline OVector2 operator+(const OVector2<U>& v) const
        {return {x + v.x, y + v.y};}
        template<typename U>
        inline OVector2 operator-(const U& v) const
        {return {x - v, y - v};}
        template<typename U>
        inline OVector2 operator-(const OVector2<U>& v) const
        {return {x - v.x, y - v.y};}
        template<typename U>
        inline OVector2 operator*(const U& v) const
        {return {x * v, y * v};}
        template<typename U>
        inline OVector2 operator*(const OVector2<U>& v) const
        {return {x * v.x, y * v.y};}
        template<typename U>
        inline OVector2 operator/(const U& v) const
        {return {x / v, y / v};}
        template<typename U>
        inline OVector2 operator/(const OVector2<U>& v) const
        {return {x / v.x, y / v.y};}

        // -------- Bitwise-Operator -------- //
        template<typename U>
        inline OVector2 operator&(const OVector2<U>& v) const
        {return {x & v.x, y & v.y};}
        template<typename U>
        inline OVector2 operator|(const OVector2<U>& v) const
        {return {x | v.x, y | v.y};}
        template<typename U>
        inline OVector2 operator^(const OVector2<U>& v) const
        {return {x ^ v.x, y ^ v.y};}

        // ----- Assignment-Operator ----- //
        template<typename U>
        OVector2& operator=(const OVector2<U>& v)
        {
            x = v.x;
            y = v.y;
            return *this;
        }

        template<typename U>
        OVector2& operator=(OVector2<U>&& v)
        {
            x = std::move(v.x);
            y = std::move(v.y);
            return *this;
        }

        template<typename U>
        OVector2& operator+=(const U& v)
        {x += v; y += v; return *this;}
        template<typename U>
        OVector2& operator+=(const OVector2<U>& v)
        {x += v.x; y += v.y; return *this;}
        template<typename U>
        OVector2& operator-=(const U& v)
        {x -= v; y -= v; return *this;}
        template<typename U>
        OVector2& operator-=(const OVector2<U>& v)
        {x -= v.x; y -= v.y; return *this;}
        template<typename U>
        OVector2& operator*=(const U& v)
        {x *= v; y *= v; return *this;}
        template<typename U>
        OVector2& operator*=(const OVector2<U>& v)
        {x *= v.x; y *= v.y; return *this;}
        template<typename U>
        OVector2& operator/=(const U& v)
        {x /= v; y /= v; return *this;}
        template<typename U>
        OVector2& operator/=(const OVector2<U>& v)
        {x /= v.x; y /= v.y; return *this;}

        // ---------- Comparison ---------- //
        template<typename U>
        inline bool operator==(const U& v) const
        {return (x == v && y == v);}
        template<typename U>
        inline bool operator==(const OVector2<U>& v) const
        {return (x == v.x && y == v.y);}
        template<typename U>
        inline bool operator!=(const U& v) const
        {return (x != v || y != v);}
        template<typename U>
        inline bool operator!=(const OVector2<U>& v) const
        {return (x != v.x || y != v.y);}
        template<typename U>
        inline bool operator<(const U& v) const
        {return (x < v && y < v);}
        template<typename U>
        inline bool operator<(const OVector2<U>& v) const
        {return (x < v.x && y < v.y);}
        template<typename U>
        inline bool operator<=(const U& v) const
        {return (x <= v && y <= v);}
        template<typename U>
        inline bool operator<=(const OVector2<U>& v) const
        {return (x <= v.x && y <= v.y);}
        template<typename U>
        inline bool operator>(const U& v) const
        {return (x > v && y > v);}
        template<typename U>
        inline bool operator>(const OVector2<U>& v) const
        {return (x > v.x && y > v.y);}
        template<typename U>
        inline bool operator>=(const U& v) const
        {return (x >= v && y >= v);}
        template<typename U>
        inline bool operator>=(const OVector2<U>& v) const
        {return (x >= v.x && y >= v.y);}

        // ------------ Unary ------------- //
        inline OVector2 operator-() const
        {return {-x, -y};}

        // ---------- Constructor --------- //
        OVector2() = default;
        template<typename U, typename R>
        OVector2(U x, R y): x(x), y(y) {}
        template<typename U>
        OVector2(const OVector2<U>& v): x(v.x), y(v.y) {}
        template<typename U>
        OVector2(OVector2<U>&& v): x(std::move(v.x)), y(std::move(v.y)) {}

        // ----------- Destructor --------- //
        ~OVector2() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
inline utils::vector::OVector2<T> operator+(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return {lhs + rhs.x, lhs + rhs.y};}

template<typename T, typename U>
inline utils::vector::OVector2<T> operator-(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return {lhs - rhs.x, lhs - rhs.y};}

template<typename T, typename U>
inline utils::vector::OVector2<T> operator*(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return {lhs * rhs.x, lhs * rhs.y};}

template<typename T, typename U>
inline utils::vector::OVector2<T> operator/(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return {lhs / rhs.x, lhs / rhs.y};}

// -------- Bitwise-Operator -------- //
template<typename T, typename U>
inline utils::vector::OVector2<T> operator&(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return {lhs & rhs.x, lhs & rhs.y};}

template<typename T, typename U>
inline utils::vector::OVector2<T> operator|(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return {lhs | rhs.x, lhs | rhs.y};}

template<typename T, typename U>
inline utils::vector::OVector2<T> operator^(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return {lhs ^ rhs.x, lhs ^ rhs.y};}

// -------- Comparison (reverse) -------- //
template<typename T, typename U>
inline bool operator==(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return (lhs == rhs.x && lhs == rhs.y);}

template<typename T, typename U>
inline bool operator!=(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return (lhs != rhs.x || lhs != rhs.y);}

template<typename T, typename U>
inline bool operator<(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return (lhs < rhs.x && lhs < rhs.y);}

template<typename T, typename U>
inline bool operator<=(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return (lhs <= rhs.x && lhs <= rhs.y);}

template<typename T, typename U>
inline bool operator>(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return (lhs > rhs.x && lhs > rhs.y);}

template<typename T, typename U>
inline bool operator>=(const T& lhs, const utils::vector::OVector2<U>& rhs)
{return (lhs >= rhs.x && lhs >= rhs.y);}

// -------- Comparison (reverse) -------- //
template<typename T>
std::ostream& operator<<(std::ostream& os, const utils::vector::OVector2<T>& v)
{return os << "(" << v.x << ", " << v.y << ")";}

} // namespace end
#endif /* OVECTOR2_H */
