/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 06/07/2026 by @author Tsukini

File Name:
##  @file OVector3.hpp

File Description:
##  Vector hat contains 3 value respectivly x, y & z of undefined type
##  Optimized version
\**************************************************************/

#ifndef OVECTOR3_H
    #define OVECTOR3_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"               // _deprecated
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
    #define MAX_INDEX_OVECTOR3 3

namespace utils::vector { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
//class _deprecated("OVector3 dosen't have any concepts safty (Be careful!!!)") OVector3 {
class OVector3 {
    public:
        T x;
        T y;
        T z;

        // ----------- Function ----------- //
        T get(std::size_t index) const {
            if (index >= MAX_INDEX_OVECTOR3)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : (index == 1 ? y : z));
        }
        inline OVector3 min(const OVector3& min) const
        {return {std::min(x, min.x), std::min(y, min.y), std::min(z, min.z)};}
        inline OVector3 max(const OVector3& max) const
        {return {std::max(x, max.x), std::max(y, max.y), std::max(z, max.z)};}
        inline OVector3 clamp(const OVector3& min, const OVector3& max) const
        {return {std::clamp(x, min.x, max.x), std::clamp(y, min.y, max.y), std::clamp(z, min.z, max.z)};}

        // ------- Special-Function ------- //
        template<typename U>
        inline T dot(const OVector3<U>& v) const
        {return x * v.x + y * v.y + z * v.z;}
        template<typename U>
        OVector3 cross(const OVector3<U>& v) const
        {
            return {
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
            };
        }
        inline T length(void) const
        {return std::sqrt(x * x + y * y + z * z);}
        inline T lengthSquared(void) const
        {return x * x + y * y + z * z;}
        inline OVector3 sign(void) const
        {return {(x > 0) - (x < 0), (y > 0) - (y < 0), (z > 0) - (z < 0)};}
        OVector3 normalize(void) const
        {
            T len = length();
            return {x / len, y / len, z / len};
        }

        // ------------ Operator ---------- //
        T& operator[](std::size_t index) {
            if (index >= MAX_INDEX_OVECTOR3)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : (index == 1 ? y : z));
        }
        const T& operator[](std::size_t index) const {
            if (index >= MAX_INDEX_OVECTOR3)
                throw utils::exception::ErrorException(utils::exception::Code::VectorInvalidIndex);
            return (index == 0 ? x : (index == 1 ? y : z));
        }

        // -------- Basic-Operator -------- //
        template<typename U>
        inline OVector3 operator+(const U& v) const
        {return {x + v, y + v, z + v};}
        template<typename U>
        inline OVector3 operator+(const OVector3<U>& v) const
        {return {x + v.x, y + v.y, z + v.z};}
        template<typename U>
        inline OVector3 operator-(const U& v) const
        {return {x - v, y - v, z - v};}
        template<typename U>
        inline OVector3 operator-(const OVector3<U>& v) const
        {return {x - v.x, y - v.y, z - v.z};}
        template<typename U>
        inline OVector3 operator*(const U& v) const
        {return {x * v, y * v, z * v};}
        template<typename U>
        inline OVector3 operator*(const OVector3<U>& v) const
        {return {x * v.x, y * v.y, z * v.z};}
        template<typename U>
        inline OVector3 operator/(const U& v) const
        {return {x / v, y / v, z / v};}
        template<typename U>
        inline OVector3 operator/(const OVector3<U>& v) const
        {return {x / v.x, y / v.y, z / v.z};}

        // -------- Special-Operator -------- //
        inline OVector3& operator++(void)
        {++x; ++y; ++z; return *this;}
        inline OVector3 operator++(int)
        {
            OVector3 tmp = *this;
            ++(*this);
            return tmp;
        }
        inline OVector3& operator--(void)
        {--x; --y; --z; return *this;}
        inline OVector3 operator--(int)
        {
            OVector3 tmp = *this;
            --(*this);
            return tmp;
        }

        // ----- Assignment-Operator ----- //
        template<typename U>
        OVector3& operator=(const OVector3<U>& v)
        {
            x = v.x;
            y = v.y;
            z = v.z;
            return *this;
        }

        template<typename U>
        OVector3& operator=(OVector3<U>&& v)
        {
            x = std::move(v.x);
            y = std::move(v.y);
            z = std::move(v.z);
            return *this;
        }

        template<typename U>
        inline OVector3& operator+=(const U& v)
        {x += v; y += v; z += v; return *this;}
        template<typename U>
        inline OVector3& operator+=(const OVector3<U>& v)
        {x += v.x; y += v.y; z += v.z; return *this;}
        template<typename U>
        inline OVector3& operator-=(const U& v)
        {x -= v; y -= v; z -= v; return *this;}
        template<typename U>
        inline OVector3& operator-=(const OVector3<U>& v)
        {x -= v.x; y -= v.y; z -= v.z; return *this;}
        template<typename U>
        inline OVector3& operator*=(const U& v)
        {x *= v; y *= v; z *= v; return *this;}
        template<typename U>
        inline OVector3& operator*=(const OVector3<U>& v)
        {x *= v.x; y *= v.y; z *= v.z; return *this;}
        template<typename U>
        inline OVector3& operator/=(const U& v)
        {x /= v; y /= v; z /= v; return *this;}
        template<typename U>
        inline OVector3& operator/=(const OVector3<U>& v)
        {x /= v.x; y /= v.y; z /= v.z; return *this;}

        // ---------- Comparison ---------- //
        template<typename U>
        inline bool operator==(const U& v) const
        {return (x == v && y == v && z == v);}
        template<typename U>
        inline bool operator==(const OVector3<U>& v) const
        {return (x == v.x && y == v.y && z == v.z);}
        template<typename U>
        inline bool operator!=(const U& v) const
        {return (x != v || y != v || z == v);}
        template<typename U>
        inline bool operator!=(const OVector3<U>& v) const
        {return (x != v.x || y != v.y || z == v.z);}
        template<typename U>
        inline bool operator<(const U& v) const
        {return (x < v && y < v && z < v);}
        template<typename U>
        inline bool operator<(const OVector3<U>& v) const
        {return (x < v.x && y < v.y && z < v.z);}
        template<typename U>
        inline bool operator<=(const U& v) const
        {return (x <= v && y <= v && z <= v);}
        template<typename U>
        inline bool operator<=(const OVector3<U>& v) const
        {return (x <= v.x && y <= v.y && z <= v.z);}
        template<typename U>
        inline bool operator>(const U& v) const
        {return (x > v && y > v && z > v);}
        template<typename U>
        inline bool operator>(const OVector3<U>& v) const
        {return (x > v.x && y > v.y && z > v.z);}
        template<typename U>
        inline bool operator>=(const U& v) const
        {return (x >= v && y >= v && z >= v);}
        template<typename U>
        inline bool operator>=(const OVector3<U>& v) const
        {return (x >= v.x && y >= v.y && z >= v.z);}

        // ------------ Unary ------------- //
        inline OVector3 operator-(void) const
        {return {-x, -y, -z};}

        // ---------- Constructor --------- //
        OVector3() = default;
        template<typename U, typename R, typename J>
        OVector3(U x, R y, J z): x(x), y(y), z(z) {}
        template<typename U>
        OVector3(const OVector3<U>& v): x(v.x), y(v.y), z(v.z) {}
        template<typename U>
        OVector3(OVector3<U>&& v): x(std::move(v.x)), y(std::move(v.y)), z(std::move(v.z)) {}

        // ----------- Destructor --------- //
        ~OVector3() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
inline utils::vector::OVector3<T> operator+(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return {lhs + rhs.x, lhs + rhs.y, lhs + rhs.z};}

template<typename T, typename U>
inline utils::vector::OVector3<T> operator-(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return {lhs - rhs.x, lhs - rhs.y, lhs - rhs.z};}

template<typename T, typename U>
inline utils::vector::OVector3<T> operator*(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return {lhs * rhs.x, lhs * rhs.y, lhs * rhs.z};}

template<typename T, typename U>
inline utils::vector::OVector3<T> operator/(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return {lhs / rhs.x, lhs / rhs.y, lhs / rhs.z};}

// -------- Bitwise-Operator -------- //
template<typename T, typename U>
inline utils::vector::OVector3<T> operator&(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return {lhs & rhs.x, lhs & rhs.y, lhs & rhs.z};}

template<typename T, typename U>
inline utils::vector::OVector3<T> operator|(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return {lhs | rhs.x, lhs | rhs.y, lhs | rhs.z};}

template<typename T, typename U>
inline utils::vector::OVector3<T> operator^(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return {lhs ^ rhs.x, lhs ^ rhs.y, lhs ^ rhs.z};}

// -------- Comparison (reverse) -------- //
template<typename T, typename U>
inline bool operator==(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return (lhs == rhs.x && lhs == rhs.y && lhs == rhs.z);}

template<typename T, typename U>
inline bool operator!=(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return (lhs != rhs.x || lhs != rhs.y || lhs != rhs.z);}

template<typename T, typename U>
inline bool operator<(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return (lhs < rhs.x && lhs < rhs.y && lhs < rhs.z);}

template<typename T, typename U>
inline bool operator<=(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return (lhs <= rhs.x && lhs <= rhs.y && lhs <= rhs.z);}

template<typename T, typename U>
inline bool operator>(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return (lhs > rhs.x && lhs > rhs.y && lhs > rhs.z);}

template<typename T, typename U>
inline bool operator>=(const T& lhs, const utils::vector::OVector3<U>& rhs)
{return (lhs >= rhs.x && lhs >= rhs.y && lhs >= rhs.z);}

// -------- Output -------- //
template<typename T>
std::ostream& operator<<(std::ostream& os, const utils::vector::OVector3<T>& v)
{return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";}

} // namespace end
#endif /* OVECTOR3_H */
