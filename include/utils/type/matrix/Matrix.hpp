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
##  @file Matrix.hpp

File Description:
##  Matrix that contains x * y value of undefined type
##  x -> row & y -> column (_matrix[x][y])
\**************************************************************/

#ifndef MATRIX_H
    #define MATRIX_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"            // _hot, _cold, _nodiscard, _likely, _unlikely
    #include "../../security/observer/Observer.hpp"     // utils::security::observer::Observer
    #include "../../concepts/OperationConcepts.hpp"     // Operation Concepts
    #include "../../exception/ExceptionDefine.hpp"      // utils::exception::InternalCode
    #include "../../exception/basic/ErrorException.hpp" // utils::exception::ErrorException
    #include <type_traits>                              // std::common_type_t, std::is_integral_v, std::is_floating_point_v
    #include <algorithm>                                // std::fill
    #include <iterator>                                 // std::input_iterator, std::forward_iterator, std::iter_value_t, std::distance
    #include <concepts>                                 // std::same_as, std::constructible_from
    #include <utility>                                  // std::pair, std::swap, std::move, std::exchange
    #include <ostream>                                  // std::ostream
    #include <cstddef>                                  // std::size_t
    #include <vector>                                   // std::vector
    #include <cmath>                                    // std::abs

namespace utils::type { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class Matrix: private utils::security::observer::Observer<"Matrix"> {
    private:
        std::pair<std::size_t, std::size_t> _size; // <row, col>
        std::vector<std::vector<T>> _matrix;       // _matrix[x][y]

        // ------------ Function ---------- //
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
        _hot inline void requireSameSize_(const utils::type::Matrix<U>& m) const
        {
            if (this->_size != m._size) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "Both matrix must have the same size");
            }
        };
        template<typename U>
        _hot inline void requireProduct_(const utils::type::Matrix<U>& m) const
        {
            if (this->_size.second != m._size.first) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The column count must match the row count of the other matrix");
            }
        };
        _hot _nodiscard static std::size_t pivot_(const std::vector<std::vector<T>>& m, const std::size_t k)
        {
            std::size_t best = k;

            for (std::size_t i = k + 1; i < m.size(); ++i) {
                if constexpr (std::is_floating_point_v<T>) {
                    if (std::abs(m[i][k]) > std::abs(m[best][k])) // biggest value (numerical stability)
                        best = i;
                } else if (m[best][k] == T{}) // first non-null value
                    best = i;
            }
            return best;
        };

        template<typename U>
        friend class Matrix;

    public:
        // ------------ Function ---------- //
        /* editor */
        template<typename It>
        _hot void set(It begin, It end, const std::size_t x = 0, const std::size_t y = 0) // fill row by row from (x, y)
        requires std::input_iterator<It> && std::same_as<std::iter_value_t<It>, T>
        {
            this->requireIndex_(x, y);
            std::size_t i = x;
            std::size_t j = y;

            if constexpr (std::forward_iterator<It>) {
                const std::size_t space = this->_size.first * this->_size.second - (x * this->_size.second + y);
                if (static_cast<std::size_t>(std::distance(begin, end)) > space) _unlikely {
                    throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Not enough space left in the matrix for all the values");
                }
            }
            for (; begin != end; ++begin) {
                if (i >= this->_size.first) _unlikely {
                    throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Not enough space left in the matrix for all the values");
                }
                this->_matrix[i][j] = *begin;
                if (++j >= this->_size.second) {
                    j = 0;
                    ++i;
                }
            }
        };
        _hot inline void set(const std::size_t x, const std::size_t y, const T& value)
        {
            this->requireIndex_(x, y);
            this->_matrix[x][y] = value;
        };
        _hot inline void set(const std::size_t x, const std::size_t y, T&& value)
        {
            this->requireIndex_(x, y);
            this->_matrix[x][y] = std::move(value);
        };
        _hot void swap(const std::pair<std::size_t, std::size_t>& a, const std::pair<std::size_t, std::size_t>& b)
        {
            this->requireIndex_(a.first, a.second);
            this->requireIndex_(b.first, b.second);
            std::swap(this->_matrix[a.first][a.second], this->_matrix[b.first][b.second]);
        };
        _hot void swapCol(const std::size_t a, const std::size_t b)
        {
            if (a >= this->_size.second || b >= this->_size.second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid column index on the matrix");
            }
            for (std::vector<T>& line: this->_matrix)
                std::swap(line[a], line[b]);
        };
        _hot void swapRow(const std::size_t a, const std::size_t b)
        {
            if (a >= this->_size.first || b >= this->_size.first) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid row index on the matrix");
            }
            std::swap(this->_matrix[a], this->_matrix[b]);
        };
        _cold void clear(void) // reinit all value to default
        {
            for (std::vector<T>& line: this->_matrix)
                std::fill(line.begin(), line.end(), T{});
        };

        /* computing */
        _hot void transpose(void)
        {
            if (this->_size.first == this->_size.second) {
                for (std::size_t i = 0; i < this->_size.first; ++i)
                    for (std::size_t j = i + 1; j < this->_size.second; ++j)
                        std::swap(this->_matrix[i][j], this->_matrix[j][i]);
                return;
            }
            std::vector<std::vector<T>> transposed(this->_size.second, std::vector<T>(this->_size.first));

            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t j = 0; j < this->_size.second; ++j)
                    transposed[j][i] = std::move(this->_matrix[i][j]);
            this->_matrix = std::move(transposed);
            std::swap(this->_size.first, this->_size.second);
        };
        _hot void invert(void) // disabled for integer (the result would be truncated)
        requires utils::concepts::Arithmetic<T> && utils::concepts::EqualityComparable<T> && (!std::is_integral_v<T>)
        {
            this->requireSquare_();
            const std::size_t n = this->_size.first;
            std::vector<std::vector<T>> m = this->_matrix;
            std::vector<std::vector<T>> inverse(n, std::vector<T>(n, T{}));

            for (std::size_t i = 0; i < n; ++i)
                inverse[i][i] = T(1);
            // Gauss-Jordan elimination (done on a copy, the matrix stay untouched if singular)
            for (std::size_t k = 0; k < n; ++k) {
                const std::size_t p = this->pivot_(m, k);
                if (m[p][k] == T{}) _unlikely {
                    throw utils::exception::ErrorException(utils::exception::InternalCode::MatrixSingular);
                }
                if (p != k) {
                    std::swap(m[p], m[k]);
                    std::swap(inverse[p], inverse[k]);
                }
                const T div = m[k][k];
                for (std::size_t j = 0; j < n; ++j) {
                    m[k][j] = m[k][j] / div;
                    inverse[k][j] = inverse[k][j] / div;
                }
                for (std::size_t i = 0; i < n; ++i) {
                    const T factor = m[i][k];
                    if (i == k || factor == T{})
                        continue;
                    for (std::size_t j = 0; j < n; ++j) {
                        m[i][j] = m[i][j] - factor * m[k][j];
                        inverse[i][j] = inverse[i][j] - factor * inverse[k][j];
                    }
                }
            }
            this->_matrix = std::move(inverse);
        };
        _hot _nodiscard T det(void) const
        requires utils::concepts::Arithmetic<T> && utils::concepts::EqualityComparable<T> && utils::concepts::Negatable<T>
        {
            this->requireSquare_();
            const std::size_t n = this->_size.first;
            std::vector<std::vector<T>> m = this->_matrix;
            bool negative = false;

            if constexpr (std::is_integral_v<T>) {
                // Bareiss algorithm (fraction-free, every division is exact for integer)
                T previous = T(1);

                for (std::size_t k = 0; k + 1 < n; ++k) {
                    const std::size_t p = this->pivot_(m, k);
                    if (m[p][k] == T{}) _unlikely {
                        return T{};
                    }
                    if (p != k) {
                        std::swap(m[p], m[k]);
                        negative = !negative;
                    }
                    for (std::size_t i = k + 1; i < n; ++i)
                        for (std::size_t j = k + 1; j < n; ++j)
                            m[i][j] = (m[i][j] * m[k][k] - m[i][k] * m[k][j]) / previous;
                    previous = m[k][k];
                }
                return (negative ? -m[n - 1][n - 1] : m[n - 1][n - 1]);
            } else {
                // Gaussian elimination (partial pivoting for floating point)
                T result = T(1);

                for (std::size_t k = 0; k < n; ++k) {
                    const std::size_t p = this->pivot_(m, k);
                    if (m[p][k] == T{}) _unlikely {
                        return T{};
                    }
                    if (p != k) {
                        std::swap(m[p], m[k]);
                        negative = !negative;
                    }
                    result = result * m[k][k];
                    for (std::size_t i = k + 1; i < n; ++i) {
                        const T factor = m[i][k] / m[k][k];
                        for (std::size_t j = k + 1; j < n; ++j)
                            m[i][j] = m[i][j] - factor * m[k][j];
                    }
                }
                return (negative ? -result : result);
            }
        };
        _hot _nodiscard T trace(void) const
        requires utils::concepts::Addable<T>
        {
            this->requireSquare_();
            T result = T{};

            for (std::size_t i = 0; i < this->_size.first; ++i)
                result = result + this->_matrix[i][i];
            return result;
        };

        /* getter */
        _hot _nodiscard inline T& at(const std::size_t x, const std::size_t y)
        {
            this->requireIndex_(x, y);
            return this->_matrix[x][y];
        };
        _hot _nodiscard inline const T& at(const std::size_t x, const std::size_t y) const
        {
            this->requireIndex_(x, y);
            return this->_matrix[x][y];
        };
        _cold _nodiscard std::vector<T> col(const std::size_t n) const
        {
            if (n >= this->_size.second) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid column index on the matrix");
            }
            std::vector<T> column;

            column.reserve(this->_size.first);
            for (const std::vector<T>& line: this->_matrix)
                column.push_back(line[n]);
            return column;
        };
        _cold _nodiscard std::vector<T> row(const std::size_t n) const
        {
            if (n >= this->_size.first) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::OutOfBounds, "Invalid row index on the matrix");
            }
            return this->_matrix[n];
        };
        _hot _nodiscard inline std::pair<std::size_t, std::size_t> size(void) const {return this->_size;};
        _hot _nodiscard inline std::size_t col(void) const                          {return this->_size.second;};
        _hot _nodiscard inline std::size_t row(void) const                          {return this->_size.first;};

        // ------------ Operator ---------- //
        _hot _nodiscard inline T& operator()(const std::size_t x, const std::size_t y)             {return this->at(x, y);};
        _hot _nodiscard inline const T& operator()(const std::size_t x, const std::size_t y) const {return this->at(x, y);};

        // -------- Basic-Operator -------- //
        /* matrix */
        template<typename U>
        _hot _nodiscard utils::type::Matrix<std::common_type_t<T, U>> operator+(const utils::type::Matrix<U>& m) const
        requires utils::concepts::AddableWith<T, U>
        {
            this->requireSameSize_(m);
            utils::type::Matrix<std::common_type_t<T, U>> result(this->_size.first, this->_size.second);

            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t j = 0; j < this->_size.second; ++j)
                    result._matrix[i][j] = this->_matrix[i][j] + m._matrix[i][j];
            return result;
        };

        template<typename U>
        _hot _nodiscard utils::type::Matrix<std::common_type_t<T, U>> operator-(const utils::type::Matrix<U>& m) const
        requires utils::concepts::SubtractableWith<T, U>
        {
            this->requireSameSize_(m);
            utils::type::Matrix<std::common_type_t<T, U>> result(this->_size.first, this->_size.second);

            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t j = 0; j < this->_size.second; ++j)
                    result._matrix[i][j] = this->_matrix[i][j] - m._matrix[i][j];
            return result;
        };

        template<typename U>
        _hot _nodiscard utils::type::Matrix<std::common_type_t<T, U>> operator*(const utils::type::Matrix<U>& m) const
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::AddableWith<T, U>
        {
            this->requireProduct_(m);
            utils::type::Matrix<std::common_type_t<T, U>> result(this->_size.first, m._size.second);

            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t k = 0; k < this->_size.second; ++k)
                    for (std::size_t j = 0; j < m._size.second; ++j)
                        result._matrix[i][j] = result._matrix[i][j] + this->_matrix[i][k] * m._matrix[k][j];
            return result;
        };

        template<typename U>
        _hot _nodiscard utils::type::Matrix<std::common_type_t<T, U>> operator/(const utils::type::Matrix<U>& m) const // this * m^-1
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::AddableWith<T, U> && (!std::is_integral_v<std::common_type_t<T, U>>)
        {
            utils::type::Matrix<std::common_type_t<T, U>> inverse(m);

            inverse.invert();
            return (*this) * inverse;
        };

        /* value */
        template<typename U>
        _hot _nodiscard utils::type::Matrix<std::common_type_t<T, U>> operator*(const U& v) const
        requires utils::concepts::MultipliableWith<T, U>
        {
            utils::type::Matrix<std::common_type_t<T, U>> result(this->_size.first, this->_size.second);

            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t j = 0; j < this->_size.second; ++j)
                    result._matrix[i][j] = this->_matrix[i][j] * v;
            return result;
        };

        template<typename U>
        _hot _nodiscard utils::type::Matrix<std::common_type_t<T, U>> operator/(const U& v) const
        requires utils::concepts::DivisibleWith<T, U>
        {
            utils::type::Matrix<std::common_type_t<T, U>> result(this->_size.first, this->_size.second);

            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t j = 0; j < this->_size.second; ++j)
                    result._matrix[i][j] = this->_matrix[i][j] / v;
            return result;
        };

        // ----- Assignment-Operator ----- //
        Matrix& operator=(const Matrix& other) = default;
        Matrix& operator=(Matrix&& other)
        {
            if (this != &other) _likely {
                this->_size = std::exchange(other._size, {0, 0});
                this->_matrix = std::move(other._matrix);
            }
            return *this;
        };

        /* matrix */
        template<typename U>
        _hot Matrix& operator+=(const utils::type::Matrix<U>& m)
        requires utils::concepts::AddAssignableWith<T, U>
        {
            this->requireSameSize_(m);
            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t j = 0; j < this->_size.second; ++j)
                    this->_matrix[i][j] += m._matrix[i][j];
            return *this;
        };

        template<typename U>
        _hot Matrix& operator-=(const utils::type::Matrix<U>& m)
        requires utils::concepts::SubtractAssignableWith<T, U>
        {
            this->requireSameSize_(m);
            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t j = 0; j < this->_size.second; ++j)
                    this->_matrix[i][j] -= m._matrix[i][j];
            return *this;
        };

        template<typename U>
        _hot Matrix& operator*=(const utils::type::Matrix<U>& m)
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::AddAssignable<T>
        {
            this->requireProduct_(m);
            Matrix result(this->_size.first, m._size.second);

            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t k = 0; k < this->_size.second; ++k)
                    for (std::size_t j = 0; j < m._size.second; ++j)
                        result._matrix[i][j] += this->_matrix[i][k] * m._matrix[k][j];
            *this = std::move(result);
            return *this;
        };

        template<typename U>
        _hot Matrix& operator/=(const utils::type::Matrix<U>& m) // this * m^-1
        requires utils::concepts::MultipliableWith<T, U> && utils::concepts::AddAssignable<T> && (!std::is_integral_v<std::common_type_t<T, U>>)
        {
            utils::type::Matrix<std::common_type_t<T, U>> inverse(m);

            inverse.invert();
            return (*this) *= inverse;
        };

        /* value */
        template<typename U>
        _hot Matrix& operator*=(const U& v)
        requires utils::concepts::MultiplyAssignableWith<T, U>
        {
            for (std::vector<T>& line: this->_matrix)
                for (T& value: line)
                    value *= v;
            return *this;
        };

        template<typename U>
        _hot Matrix& operator/=(const U& v)
        requires utils::concepts::DivideAssignableWith<T, U>
        {
            for (std::vector<T>& line: this->_matrix)
                for (T& value: line)
                    value /= v;
            return *this;
        };

        // ---------- Comparison ---------- //
        template<typename U>
        _hot _nodiscard bool operator==(const utils::type::Matrix<U>& m) const
        requires utils::concepts::EqualityComparableWith<T, U>
        {
            if (this->_size != m._size)
                return false;
            for (std::size_t i = 0; i < this->_size.first; ++i)
                for (std::size_t j = 0; j < this->_size.second; ++j)
                    if (this->_matrix[i][j] != m._matrix[i][j])
                        return false;
            return true;
        };
        template<typename U>
        _hot _nodiscard bool operator!=(const utils::type::Matrix<U>& m) const
        requires utils::concepts::EqualityComparableWith<T, U> {return !(*this == m);};

        // ------------ Unary ------------- //
        _hot _nodiscard Matrix operator-(void) const
        requires utils::concepts::Negatable<T>
        {
            Matrix result(*this);

            for (std::vector<T>& line: result._matrix)
                for (T& value: line)
                    value = -value;
            return result;
        };

        // ---------- Constructor --------- //
        explicit Matrix(const std::size_t n): Matrix(n, n) {};
        Matrix(const std::size_t x, const std::size_t y): _size{x, y}, _matrix(x, std::vector<T>(y))
        {
            if (x == 0 || y == 0) _unlikely {
                throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "A matrix can't have a null size");
            }
        };
        template<typename U>
        Matrix(const utils::type::Matrix<U>& m)
        requires std::constructible_from<T, U>: _size(m._size)
        {
            this->_matrix.reserve(m._size.first);
            for (const std::vector<U>& line: m._matrix)
                this->_matrix.emplace_back(line.begin(), line.end());
        };
        Matrix(const Matrix& other) = default;
        Matrix(Matrix&& other): _size(std::exchange(other._size, {0, 0})), _matrix(std::move(other._matrix)) {};

        // ----------- Destructor --------- //
        ~Matrix() = default;
};

// -------- Basic-Operator (reverse) -------- //
template<typename T, typename U>
_hot _nodiscard utils::type::Matrix<std::common_type_t<T, U>> operator*(const T& lhs, const utils::type::Matrix<U>& rhs)
requires utils::concepts::MultipliableWith<T, U>
{
    utils::type::Matrix<std::common_type_t<T, U>> result(rhs.row(), rhs.col());

    for (std::size_t i = 0; i < rhs.row(); ++i)
        for (std::size_t j = 0; j < rhs.col(); ++j)
            result.at(i, j) = lhs * rhs.at(i, j);
    return result;
}

// ------------ Stream ------------ //
template<typename T>
_cold std::ostream& operator<<(std::ostream& os, const utils::type::Matrix<T>& m)
{
    for (std::size_t i = 0; i < m.row(); ++i) {
        os << (i == 0 ? "[(" : " (");
        for (std::size_t j = 0; j < m.col(); ++j)
            os << m.at(i, j) << (j + 1 < m.col() ? ", " : ")");
        os << (i + 1 < m.row() ? "\n" : "]");
    }
    return os;
}

} // namespace end
#endif /* MATRIX_H */
