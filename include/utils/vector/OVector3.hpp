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
    #define MAX_INDEX_OVECTOR3 3

namespace utils::vector { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class outdated("OVector3 dosen't have any concepts safty (Be careful!!!)") OVector3 {
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
        inline T length() const
        {return std::sqrt(x * x + y * y + z * z);}
        inline T lengthSquared() const
        {return x * x + y * y + z * z;}
        OVector3 normalize() const
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
        inline OVector3 operator-() const
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
