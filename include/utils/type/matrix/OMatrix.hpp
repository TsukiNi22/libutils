/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 27/09/2026 by @author Tsukini

File Name:
##  @file OMatrix.hpp

File Description:
##  Matrix that contains x * y value of undefined type
##  x -> row & y -> column (row-major contiguous storage)
##  Optimized version
\**************************************************************/

#ifndef OMATRIX_H
    #define OMATRIX_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"            // _hot, _cold, _nodiscard, _likely, _unlikely, _deprecated
    #include "../../exception/ExceptionDefine.hpp"      // utils::exception::InternalCode
    #include "../../exception/basic/ErrorException.hpp" // utils::exception::ErrorException
    #include <type_traits>                              // std::is_integral_v, std::is_floating_point_v
    #include <algorithm>                                // std::fill, std::copy, std::swap_ranges
    #include <iterator>                                 // std::input_iterator, std::forward_iterator, std::iter_value_t, std::distance
    #include <concepts>                                 // std::same_as
    #include <utility>                                  // std::pair, std::swap, std::move, std::exchange
    #include <ostream>                                  // std::ostream
    #include <cstddef>                                  // std::size_t
    #include <vector>                                   // std::vector
    #include <cmath>                                    // std::abs

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
//class _deprecated("OMatrix doesn't have any concepts safety (Be careful!!!)") OMatrix {
class OMatrix {
    private:
        std::pair<std::size_t, std::size_t> _size; // (row, col)
        std::vector<T> _matrix;                    // _matrix[x * col + y]

        // ------------ Function ---------- //
        _hot _nodiscard inline std::size_t index_(const std::size_t x, const std::size_t y) const
        {return x * this->_size.second + y;};
        _hot inline void requireIndex_(const std::size_t x, const std::size_t y) const
        {
            if (x >= this->_size.first || y >= this->_size.second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid index on the matrix");
            }
        };
        _hot inline void requireSquare_(void) const
        {
            if (this->_size.first != this->_size.second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::UnauthorizedCall, "The matrix must be square");
            }
        };
        template<typename U>
        _hot inline void requireSameSize_(const utils::type::OMatrix<U>& m) const
        {
            if (this->_size != m._size) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "Both matrix must have the same size");
            }
        };
        template<typename U>
        _hot inline void requireProduct_(const utils::type::OMatrix<U>& m) const
        {
            if (this->_size.second != m._size.first) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The column count must match the row count of the other matrix");
            }
        };
        _hot _nodiscard static std::size_t pivot_(const std::vector<T>& m, const std::size_t n, const std::size_t k)
        {
            std::size_t best = k;

            for (std::size_t i = k + 1; i < n; ++i) {
                if constexpr (std::is_floating_point_v<T>) {
                    if (std::abs(m[i * n + k]) > std::abs(m[best * n + k])) // biggest value (numerical stability)
                        best = i;
                } else if (m[best * n + k] == T{}) // first non-null value
                    best = i;
            }
            return best;
        };
        _hot static inline void swapLine_(std::vector<T>& m, const std::size_t n, const std::size_t a, const std::size_t b)
        {
            if (a != b)
                std::swap_ranges(m.begin() + a * n, m.begin() + (a + 1) * n, m.begin() + b * n);
        };

        template<typename U>
        friend class OMatrix;

    public:
        // ------------ Function ---------- //
        /* editor */
        template<typename It>
        _hot void set(It begin, It end, const std::size_t x = 0, const std::size_t y = 0) // fill row by row from (x, y)
        requires std::input_iterator<It> && std::same_as<std::iter_value_t<It>, T>
        {
            this->requireIndex_(x, y);
            std::size_t i = this->index_(x, y);

            if constexpr (std::forward_iterator<It>) {
                if (static_cast<std::size_t>(std::distance(begin, end)) > this->_matrix.size() - i) _unlikely {
                    throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Not enough space left in the matrix for all the values");
                }
                std::copy(begin, end, this->_matrix.begin() + i);
            } else {
                for (; begin != end; ++begin, ++i) {
                    if (i >= this->_matrix.size()) _unlikely {
                        throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Not enough space left in the matrix for all the values");
                    }
                    this->_matrix[i] = *begin;
                }
            }
        };
        _hot inline void set(const std::size_t x, const std::size_t y, const T& value)
        {
            this->requireIndex_(x, y);
            this->_matrix[this->index_(x, y)] = value;
        };
        _hot inline void set(const std::size_t x, const std::size_t y, T&& value)
        {
            this->requireIndex_(x, y);
            this->_matrix[this->index_(x, y)] = std::move(value);
        };
        inline void swap(const std::pair<std::size_t, std::size_t>& a, const std::pair<std::size_t, std::size_t>& b)
        {
            this->requireIndex_(a.first, a.second);
            this->requireIndex_(b.first, b.second);
            std::swap(this->_matrix[this->index_(a.first, a.second)], this->_matrix[this->index_(b.first, b.second)]);
        };
        _hot void swapCol(const std::size_t a, const std::size_t b)
        {
            if (a >= this->_size.second || b >= this->_size.second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid column index on the matrix");
            }
            for (std::size_t i = 0; i < this->_size.first; ++i)
                std::swap(this->_matrix[this->index_(i, a)], this->_matrix[this->index_(i, b)]);
        };
        inline void swapRow(const std::size_t a, const std::size_t b)
        {
            if (a >= this->_size.first || b >= this->_size.first) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid row index on the matrix");
            }
            this->swapLine_(this->_matrix, this->_size.second, a, b);
        };
        inline void clear(void) // reinit all value to default
        {std::fill(this->_matrix.begin(), this->_matrix.end(), T{});}

        /* computing */
        _hot void transpose(void)
        {
            const std::size_t rows = this->_size.first;
            const std::size_t cols = this->_size.second;

            if (rows == cols) {
                for (std::size_t i = 0; i < rows; ++i)
                    for (std::size_t j = i + 1; j < cols; ++j)
                        std::swap(this->_matrix[i * cols + j], this->_matrix[j * cols + i]);
                return;
            }
            std::vector<T> transposed(this->_matrix.size());

            for (std::size_t i = 0; i < rows; ++i)
                for (std::size_t j = 0; j < cols; ++j)
                    transposed[j * rows + i] = std::move(this->_matrix[i * cols + j]);
            this->_matrix = std::move(transposed);
            std::swap(this->_size.first, this->_size.second);
        };
        _hot void invert(void) // integer type will be truncated (Be careful!!!)
        {
            this->requireSquare_();
            const std::size_t n = this->_size.first;
            std::vector<T> m = this->_matrix;
            std::vector<T> inverse(n * n, T{});

            for (std::size_t i = 0; i < n; ++i)
                inverse[i * n + i] = T(1);
            // Gauss-Jordan elimination (done on a copy, the matrix stay untouched if singular)
            for (std::size_t k = 0; k < n; ++k) {
                const std::size_t p = this->pivot_(m, n, k);
                if (m[p * n + k] == T{}) _unlikely {
                    throw utils::exception::ErrorException(utils::exception::InternalCode::MatrixSingular);
                }
                this->swapLine_(m, n, p, k);
                this->swapLine_(inverse, n, p, k);
                const T div = m[k * n + k];
                for (std::size_t j = k; j < n; ++j)
                    m[k * n + j] /= div;
                for (std::size_t j = 0; j < n; ++j)
                    inverse[k * n + j] /= div;
                for (std::size_t i = 0; i < n; ++i) {
                    const T factor = m[i * n + k];
                    if (i == k || factor == T{})
                        continue;
                    for (std::size_t j = k; j < n; ++j)
                        m[i * n + j] -= factor * m[k * n + j];
                    for (std::size_t j = 0; j < n; ++j)
                        inverse[i * n + j] -= factor * inverse[k * n + j];
                }
            }
            this->_matrix = std::move(inverse);
        };
        _hot _nodiscard T det(void) const
        {
            this->requireSquare_();
            const std::size_t n = this->_size.first;
            std::vector<T> m = this->_matrix;
            bool negative = false;

            if constexpr (std::is_integral_v<T>) {
                // Bareiss algorithm (fraction-free, every division is exact for integer)
                T previous = T(1);

                for (std::size_t k = 0; k + 1 < n; ++k) {
                    const std::size_t p = this->pivot_(m, n, k);
                    if (m[p * n + k] == T{}) _unlikely {
                        return T{};
                    }
                    if (p != k) {
                        this->swapLine_(m, n, p, k);
                        negative = !negative;
                    }
                    for (std::size_t i = k + 1; i < n; ++i)
                        for (std::size_t j = k + 1; j < n; ++j)
                            m[i * n + j] = (m[i * n + j] * m[k * n + k] - m[i * n + k] * m[k * n + j]) / previous;
                    previous = m[k * n + k];
                }
                return (negative ? -m[n * n - 1] : m[n * n - 1]);
            } else {
                // Gaussian elimination (partial pivoting for floating point)
                T result = T(1);

                for (std::size_t k = 0; k < n; ++k) {
                    const std::size_t p = this->pivot_(m, n, k);
                    if (m[p * n + k] == T{}) _unlikely {
                        return T{};
                    }
                    if (p != k) {
                        this->swapLine_(m, n, p, k);
                        negative = !negative;
                    }
                    const T value = m[k * n + k];
                    result *= value;
                    for (std::size_t i = k + 1; i < n; ++i) {
                        const T factor = m[i * n + k] / value;
                        for (std::size_t j = k + 1; j < n; ++j)
                            m[i * n + j] -= factor * m[k * n + j];
                    }
                }
                return (negative ? -result : result);
            }
        };
        _nodiscard inline T trace(void) const
        {
            this->requireSquare_();
            T result = T{};

            for (std::size_t i = 0; i < this->_size.first; ++i)
                result += this->_matrix[i * this->_size.second + i];
            return result;
        };

        /* getter */
        _hot _nodiscard inline T& at(const std::size_t x, const std::size_t y)
        {
            this->requireIndex_(x, y);
            return this->_matrix[this->index_(x, y)];
        };
        _hot _nodiscard inline const T& at(const std::size_t x, const std::size_t y) const
        {
            this->requireIndex_(x, y);
            return this->_matrix[this->index_(x, y)];
        };
        _cold _nodiscard std::vector<T> col(const std::size_t n) const
        {
            if (n >= this->_size.second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid column index on the matrix");
            }
            std::vector<T> column(this->_size.first);

            for (std::size_t i = 0; i < this->_size.first; ++i)
                column[i] = this->_matrix[this->index_(i, n)];
            return column;
        };
        _cold _nodiscard std::vector<T> row(const std::size_t n) const
        {
            if (n >= this->_size.first) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid row index on the matrix");
            }
            const auto begin = this->_matrix.begin() + n * this->_size.second;

            return std::vector<T>(begin, begin + this->_size.second);
        };
        _hot _nodiscard inline std::pair<std::size_t, std::size_t> size(void) const {return this->_size;};
        _hot _nodiscard inline std::size_t col(void) const                          {return this->_size.second;};
        _hot _nodiscard inline std::size_t row(void) const                          {return this->_size.first;};

        // ------------ Operator ---------- //
        _hot _nodiscard inline T& operator()(const std::size_t x, const std::size_t y)             {return this->_matrix[this->index_(x, y)];}; // no bound check
        _hot _nodiscard inline const T& operator()(const std::size_t x, const std::size_t y) const {return this->_matrix[this->index_(x, y)];}; // no bound check

        // -------- Basic-Operator -------- //
        /* matrix */
        template<typename U>
        _nodiscard inline OMatrix operator+(const utils::type::OMatrix<U>& m) const
        {OMatrix result(*this); result += m; return result;}
        template<typename U>
        _nodiscard inline OMatrix operator-(const utils::type::OMatrix<U>& m) const
        {OMatrix result(*this); result -= m; return result;}
        template<typename U>
        _hot _nodiscard OMatrix operator*(const utils::type::OMatrix<U>& m) const
        {
            this->requireProduct_(m);
            const std::size_t rows = this->_size.first;
            const std::size_t inner = this->_size.second;
            const std::size_t cols = m._size.second;
            OMatrix result(rows, cols);

            for (std::size_t i = 0; i < rows; ++i) {
                for (std::size_t k = 0; k < inner; ++k) {
                    const T value = this->_matrix[i * inner + k];
                    for (std::size_t j = 0; j < cols; ++j)
                        result._matrix[i * cols + j] += value * m._matrix[k * cols + j];
                }
            }
            return result;
        };
        template<typename U>
        _nodiscard inline OMatrix operator/(const utils::type::OMatrix<U>& m) const // this * m^-1
        {
            OMatrix inverse(m);

            inverse.invert();
            return (*this) * inverse;
        };

        /* value */
        template<typename U>
        _nodiscard inline OMatrix operator*(const U& v) const
        {OMatrix result(*this); result *= v; return result;}
        template<typename U>
        _nodiscard inline OMatrix operator/(const U& v) const
        {OMatrix result(*this); result /= v; return result;}

        // ----- Assignment-Operator ----- //
        OMatrix& operator=(const OMatrix& other) = default;
        OMatrix& operator=(OMatrix&& other) noexcept
        {
            if (this != &other) _likely {
                this->_size = std::exchange(other._size, {0, 0});
                this->_matrix = std::move(other._matrix);
            }
            return *this;
        };

        /* matrix */
        template<typename U>
        _hot OMatrix& operator+=(const utils::type::OMatrix<U>& m)
        {
            this->requireSameSize_(m);
            for (std::size_t i = 0; i < this->_matrix.size(); ++i)
                this->_matrix[i] += m._matrix[i];
            return *this;
        };
        template<typename U>
        _hot OMatrix& operator-=(const utils::type::OMatrix<U>& m)
        {
            this->requireSameSize_(m);
            for (std::size_t i = 0; i < this->_matrix.size(); ++i)
                this->_matrix[i] -= m._matrix[i];
            return *this;
        };
        template<typename U>
        _hot OMatrix& operator*=(const utils::type::OMatrix<U>& m)
        {*this = (*this) * m; return *this;}
        template<typename U>
        _hot OMatrix& operator/=(const utils::type::OMatrix<U>& m)
        {*this = (*this) / m; return *this;}

        /* value */
        template<typename U>
        _hot OMatrix& operator*=(const U& v)
        {for (T& value: this->_matrix) value *= v; return *this;}
        template<typename U>
        _hot OMatrix& operator/=(const U& v)
        {for (T& value: this->_matrix) value /= v; return *this;}

        // ---------- Comparison ---------- //
        template<typename U>
        _hot _nodiscard bool operator==(const utils::type::OMatrix<U>& m) const
        {
            if (this->_size != m._size)
                return false;
            for (std::size_t i = 0; i < this->_matrix.size(); ++i)
                if (this->_matrix[i] != m._matrix[i])
                    return false;
            return true;
        };
        template<typename U>
        _nodiscard inline bool operator!=(const utils::type::OMatrix<U>& m) const
        {return !(*this == m);};

        // ------------ Unary ------------- //
        _nodiscard inline OMatrix operator-(void) const
        {
            OMatrix result(*this);

            for (T& value: result._matrix)
                value = -value;
            return result;
        };

        // ---------- Constructor --------- //
        explicit OMatrix(const std::size_t n): OMatrix(n, n) {};
        OMatrix(const std::size_t x, const std::size_t y): _size{x, y}, _matrix(x * y)
        {
            if (x == 0 || y == 0) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "A matrix can't have a null size");
            }
        };
        template<typename U>
        OMatrix(const utils::type::OMatrix<U>& m): _size(m._size), _matrix(m._matrix.begin(), m._matrix.end()) {};
        OMatrix(const OMatrix& other) = default;
        OMatrix(OMatrix&& other) noexcept: _size(std::exchange(other._size, {0, 0})), _matrix(std::move(other._matrix)) {};

        // ----------- Destructor --------- //
        ~OMatrix() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
_nodiscard inline utils::type::OMatrix<U> operator*(const T& lhs, const utils::type::OMatrix<U>& rhs)
{
    utils::type::OMatrix<U> result(rhs);

    for (std::size_t i = 0; i < result.row(); ++i)
        for (std::size_t j = 0; j < result.col(); ++j)
            result(i, j) = lhs * result(i, j);
    return result;
}

// ------------ Stream ------------ //
template<typename T>
_cold std::ostream& operator<<(std::ostream& os, const utils::type::OMatrix<T>& m)
{
    for (std::size_t i = 0; i < m.row(); ++i) {
        os << (i == 0 ? "[(" : " (");
        for (std::size_t j = 0; j < m.col(); ++j)
            os << m(i, j) << (j + 1 < m.col() ? ", " : ")");
        os << (i + 1 < m.row() ? "\n" : "]");
    }
    return os;
}

} // namespace end
#endif /* OMATRIX_H */
