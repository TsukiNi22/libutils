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
    #define MAX_INDEX_OVECTOR3 3

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
//class _deprecated("OVector3 dosen't have any concepts safty (Be careful!!!)") OVector3 {
class OVector3 {
    public:
        T x;
        T y;
        T z;

        // ------------ Function ---------- //
        _hot _nodiscard T get(std::size_t index) const
        {
            if (index >= MAX_INDEX_OVECTOR3) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : (index == 1 ? this->y : this->z));
        };
        _hot _nodiscard inline OVector3 min(const OVector3& min) const
        {return {std::min(this->x, min.x), std::min(this->y, min.y), std::min(this->z, min.z)};};
        _hot _nodiscard inline OVector3 max(const OVector3& max) const
        {return {std::max(this->x, max.x), std::max(this->y, max.y), std::max(this->z, max.z)};};
        _hot _nodiscard inline OVector3 clamp(const OVector3& min, const OVector3& max) const
        {return {std::clamp(this->x, min.x, max.x), std::clamp(this->y, min.y, max.y), std::clamp(this->z, min.z, max.z)};};

        // ------- Special-Function ------- //
        template<typename U>
        _hot _nodiscard inline T dot(const OVector3<U>& v) const
        {return this->x * v.x + this->y * v.y + this->z * v.z;};
        template<typename U>
        _hot _nodiscard OVector3 cross(const OVector3<U>& v) const
        {
            return {
                this->y * v.z - this->z * v.y,
                this->z * v.x - this->x * v.z,
                this->x * v.y - this->y * v.x
            };
        };
        _hot _nodiscard inline T length(void) const
        {return static_cast<T>(std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z));}; // truncated for the integer types
        _hot _nodiscard inline T lengthSquared(void) const
        {return this->x * this->x + this->y * this->y + this->z * this->z;};
        _hot _nodiscard inline OVector3 sign(void) const
        {return {(this->x > 0) - (this->x < 0), (this->y > 0) - (this->y < 0), (this->z > 0) - (this->z < 0)};};
        _hot _nodiscard OVector3 normalize(void) const
        {
            T len = this->length();
            if (len == T{}) return *this; // zero vector: no direction
            return {this->x / len, this->y / len, this->z / len};
        };

        // ------------ Operator ---------- //
        _hot _nodiscard T& operator[](std::size_t index)
        {
            if (index >= MAX_INDEX_OVECTOR3) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : (index == 1 ? this->y : this->z));
        };
        _hot _nodiscard const T& operator[](std::size_t index) const
        {
            if (index >= MAX_INDEX_OVECTOR3) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::VectorInvalidIndex);
            }
            return (index == 0 ? this->x : (index == 1 ? this->y : this->z));
        };

        // -------- Basic-Operator -------- //
        template<typename U>
        _hot _nodiscard inline OVector3 operator+(const U& v) const
        {return {this->x + v, this->y + v, this->z + v};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator+(const OVector3<U>& v) const
        {return {this->x + v.x, this->y + v.y, this->z + v.z};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator-(const U& v) const
        {return {this->x - v, this->y - v, this->z - v};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator-(const OVector3<U>& v) const
        {return {this->x - v.x, this->y - v.y, this->z - v.z};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator*(const U& v) const
        {return {this->x * v, this->y * v, this->z * v};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator*(const OVector3<U>& v) const
        {return {this->x * v.x, this->y * v.y, this->z * v.z};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator/(const U& v) const
        {return {this->x / v, this->y / v, this->z / v};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator/(const OVector3<U>& v) const
        {return {this->x / v.x, this->y / v.y, this->z / v.z};};

        // -------- Special-Operator -------- //
        _hot inline OVector3& operator++(void)
        {++this->x; ++this->y; ++this->z; return *this;};
        _hot _nodiscard inline OVector3 operator++(int)
        {
            OVector3 tmp = *this;
            ++(*this);
            return tmp;
        };
        _hot inline OVector3& operator--(void)
        {--this->x; --this->y; --this->z; return *this;};
        _hot _nodiscard inline OVector3 operator--(int)
        {
            OVector3 tmp = *this;
            --(*this);
            return tmp;
        };

        // -------- Bitwise-Operator -------- //
        template<typename U>
        _hot _nodiscard inline OVector3 operator&(const OVector3<U>& v) const
        {return {this->x & v.x, this->y & v.y, this->z & v.z};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator|(const OVector3<U>& v) const
        {return {this->x | v.x, this->y | v.y, this->z | v.z};};
        template<typename U>
        _hot _nodiscard inline OVector3 operator^(const OVector3<U>& v) const
        {return {this->x ^ v.x, this->y ^ v.y, this->z ^ v.z};};

        // ----- Assignment-Operator ----- //
        template<typename U>
        _hot OVector3& operator=(const OVector3<U>& v)
        {
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
            return *this;
        };

        template<typename U>
        _hot OVector3& operator=(OVector3<U>&& v)
        {
            this->x = std::move(v.x);
            this->y = std::move(v.y);
            this->z = std::move(v.z);
            return *this;
        };

        template<typename U>
        _hot inline OVector3& operator+=(const U& v)
        {this->x += v; this->y += v; this->z += v; return *this;};
        template<typename U>
        _hot inline OVector3& operator+=(const OVector3<U>& v)
        {this->x += v.x; this->y += v.y; this->z += v.z; return *this;};
        template<typename U>
        _hot inline OVector3& operator-=(const U& v)
        {this->x -= v; this->y -= v; this->z -= v; return *this;};
        template<typename U>
        _hot inline OVector3& operator-=(const OVector3<U>& v)
        {this->x -= v.x; this->y -= v.y; this->z -= v.z; return *this;};
        template<typename U>
        _hot inline OVector3& operator*=(const U& v)
        {this->x *= v; this->y *= v; this->z *= v; return *this;};
        template<typename U>
        _hot inline OVector3& operator*=(const OVector3<U>& v)
        {this->x *= v.x; this->y *= v.y; this->z *= v.z; return *this;};
        template<typename U>
        _hot inline OVector3& operator/=(const U& v)
        {this->x /= v; this->y /= v; this->z /= v; return *this;};
        template<typename U>
        _hot inline OVector3& operator/=(const OVector3<U>& v)
        {this->x /= v.x; this->y /= v.y; this->z /= v.z; return *this;};

        // ---------- Comparison ---------- //
        template<typename U>
        _hot _nodiscard inline bool operator==(const U& v) const
        {return (this->x == v && this->y == v && this->z == v);};
        template<typename U>
        _hot _nodiscard inline bool operator==(const OVector3<U>& v) const
        {return (this->x == v.x && this->y == v.y && this->z == v.z);};
        template<typename U>
        _hot _nodiscard inline bool operator!=(const U& v) const
        {return (this->x != v || this->y != v || this->z != v);};
        template<typename U>
        _hot _nodiscard inline bool operator!=(const OVector3<U>& v) const
        {return (this->x != v.x || this->y != v.y || this->z != v.z);};
        template<typename U>
        _hot _nodiscard inline bool operator<(const U& v) const
        {return (this->x < v && this->y < v && this->z < v);};
        template<typename U>
        _hot _nodiscard inline bool operator<(const OVector3<U>& v) const
        {return (this->x < v.x && this->y < v.y && this->z < v.z);};
        template<typename U>
        _hot _nodiscard inline bool operator<=(const U& v) const
        {return (this->x <= v && this->y <= v && this->z <= v);};
        template<typename U>
        _hot _nodiscard inline bool operator<=(const OVector3<U>& v) const
        {return (this->x <= v.x && this->y <= v.y && this->z <= v.z);};
        template<typename U>
        _hot _nodiscard inline bool operator>(const U& v) const
        {return (this->x > v && this->y > v && this->z > v);};
        template<typename U>
        _hot _nodiscard inline bool operator>(const OVector3<U>& v) const
        {return (this->x > v.x && this->y > v.y && this->z > v.z);};
        template<typename U>
        _hot _nodiscard inline bool operator>=(const U& v) const
        {return (this->x >= v && this->y >= v && this->z >= v);};
        template<typename U>
        _hot _nodiscard inline bool operator>=(const OVector3<U>& v) const
        {return (this->x >= v.x && this->y >= v.y && this->z >= v.z);};

        // ------------ Unary ------------- //
        _hot _nodiscard inline OVector3 operator-(void) const
        {return {-this->x, -this->y, -this->z};};

        // ---------- Constructor --------- //
        OVector3() = default;
        template<typename U, typename R, typename J>
        OVector3(U x, R y, J z): x(x), y(y), z(z) {};
        template<typename U>
        OVector3(const OVector3<U>& v): x(v.x), y(v.y), z(v.z) {};
        template<typename U>
        OVector3(OVector3<U>&& v): x(std::move(v.x)), y(std::move(v.y)), z(std::move(v.z)) {};

        // ----------- Destructor --------- //
        ~OVector3() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector3<U> operator+(const T& lhs, const utils::type::OVector3<U>& rhs)
{return {lhs + rhs.x, lhs + rhs.y, lhs + rhs.z};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector3<U> operator-(const T& lhs, const utils::type::OVector3<U>& rhs)
{return {lhs - rhs.x, lhs - rhs.y, lhs - rhs.z};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector3<U> operator*(const T& lhs, const utils::type::OVector3<U>& rhs)
{return {lhs * rhs.x, lhs * rhs.y, lhs * rhs.z};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector3<U> operator/(const T& lhs, const utils::type::OVector3<U>& rhs)
{return {lhs / rhs.x, lhs / rhs.y, lhs / rhs.z};}

// -------- Bitwise-Operator -------- //
template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector3<U> operator&(const T& lhs, const utils::type::OVector3<U>& rhs)
{return {lhs & rhs.x, lhs & rhs.y, lhs & rhs.z};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector3<U> operator|(const T& lhs, const utils::type::OVector3<U>& rhs)
{return {lhs | rhs.x, lhs | rhs.y, lhs | rhs.z};}

template<typename T, typename U>
_hot _nodiscard inline utils::type::OVector3<U> operator^(const T& lhs, const utils::type::OVector3<U>& rhs)
{return {lhs ^ rhs.x, lhs ^ rhs.y, lhs ^ rhs.z};}

// -------- Comparison (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard inline bool operator==(const T& lhs, const utils::type::OVector3<U>& rhs)
{return (lhs == rhs.x && lhs == rhs.y && lhs == rhs.z);}

template<typename T, typename U>
_hot _nodiscard inline bool operator!=(const T& lhs, const utils::type::OVector3<U>& rhs)
{return (lhs != rhs.x || lhs != rhs.y || lhs != rhs.z);}

template<typename T, typename U>
_hot _nodiscard inline bool operator<(const T& lhs, const utils::type::OVector3<U>& rhs)
{return (lhs < rhs.x && lhs < rhs.y && lhs < rhs.z);}

template<typename T, typename U>
_hot _nodiscard inline bool operator<=(const T& lhs, const utils::type::OVector3<U>& rhs)
{return (lhs <= rhs.x && lhs <= rhs.y && lhs <= rhs.z);}

template<typename T, typename U>
_hot _nodiscard inline bool operator>(const T& lhs, const utils::type::OVector3<U>& rhs)
{return (lhs > rhs.x && lhs > rhs.y && lhs > rhs.z);}

template<typename T, typename U>
_hot _nodiscard inline bool operator>=(const T& lhs, const utils::type::OVector3<U>& rhs)
{return (lhs >= rhs.x && lhs >= rhs.y && lhs >= rhs.z);}

// -------- Output -------- //
template<typename T>
_cold std::ostream& operator<<(std::ostream& os, const utils::type::OVector3<T>& v)
{return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";}

} // namespace end
#endif /* OVECTOR3_H */
