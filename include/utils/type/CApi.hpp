/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.hpp

File Description:
##  Handles of the C API of utils::type (usable in c++ to get the object)
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef TYPE_CAPI_H
    #define TYPE_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "blt/BidirectionalLookupTable_t.hpp"   // utils::type::BidirectionalLookupTable<T, T>
    #include "../exception/CApi.hpp"                // utils_handle
    #include "matrix/OMatrix.hpp"                   // utils::type::OMatrix
    #include "CApiTypes.hpp"                        // utils::type::c_api::cpp_t
    #include "Worker.hpp"                           // utils::type::Worker
    #include "CApi.h"                               // utils_worker_t, utils_blt_t, utils_matrix_*_t, utils_blt_*_t
    #include <cstddef>                              // std::size_t
    #include <cstdint>                              // std::int64_t
    #include <utility>                              // std::move
    #include <string>                               // std::string

    //----------------------------------------------------------------//
    /* MACRO */

    /* handle of a matrix (utils::type::OMatrix) */
    #define C_API_MATRIX_HANDLE(name, ctype) \
        struct utils_matrix_##name: public utils_handle { \
            utils::type::OMatrix<ctype> matrix; \
            utils_matrix_##name(utils::type::OMatrix<ctype> m): matrix{std::move(m)} { /* Nothing */ }; \
        };

    /* handle of a bidirectional lookup table (the table itself: Freezable reachable from the handle) */
    #define C_API_BLT_HANDLE(name, left_type, right_type) \
        struct utils_blt_##name: public utils_handle, public utils::type::BidirectionalLookupTable<utils::type::c_api::cpp_t<left_type>, utils::type::c_api::cpp_t<right_type>> {};
    #define C_API_BLT_SAME_HANDLE(name, ctype) C_API_BLT_HANDLE(name, ctype, ctype)
    #define C_API_BLT_STR_INT_HANDLE(name, ctype) \
        C_API_BLT_HANDLE(str_##name, const char*, ctype) \
        C_API_BLT_HANDLE(name##_str, ctype, const char*)

    /* vector2 */
    #define C_API_VECTOR2_DEF(name, ctype) \
        utils_vector2_##name##_t utils_vector2_##name##_add(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a) + o2<ctype>(b));}; \
        utils_vector2_##name##_t utils_vector2_##name##_sub(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a) - o2<ctype>(b));}; \
        utils_vector2_##name##_t utils_vector2_##name##_mul(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a) * o2<ctype>(b));}; \
        utils_vector2_##name##_t utils_vector2_##name##_div(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a) / o2<ctype>(b));}; \
        utils_vector2_##name##_t utils_vector2_##name##_add_scalar(utils_vector2_##name##_t v, ctype s) {return c2<utils_vector2_##name##_t>(o2<ctype>(v) + s);}; \
        utils_vector2_##name##_t utils_vector2_##name##_sub_scalar(utils_vector2_##name##_t v, ctype s) {return c2<utils_vector2_##name##_t>(o2<ctype>(v) - s);}; \
        utils_vector2_##name##_t utils_vector2_##name##_mul_scalar(utils_vector2_##name##_t v, ctype s) {return c2<utils_vector2_##name##_t>(o2<ctype>(v) * s);}; \
        utils_vector2_##name##_t utils_vector2_##name##_div_scalar(utils_vector2_##name##_t v, ctype s) {return c2<utils_vector2_##name##_t>(o2<ctype>(v) / s);}; \
        utils_vector2_##name##_t utils_vector2_##name##_neg(utils_vector2_##name##_t v) {return c2<utils_vector2_##name##_t>(-o2<ctype>(v));}; \
        utils_vector2_##name##_t utils_vector2_##name##_min(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a).min(o2<ctype>(b)));}; \
        utils_vector2_##name##_t utils_vector2_##name##_max(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a).max(o2<ctype>(b)));}; \
        utils_vector2_##name##_t utils_vector2_##name##_clamp(utils_vector2_##name##_t v, utils_vector2_##name##_t min, utils_vector2_##name##_t max) {return c2<utils_vector2_##name##_t>(o2<ctype>(v).clamp(o2<ctype>(min), o2<ctype>(max)));}; \
        utils_vector2_##name##_t utils_vector2_##name##_sign(utils_vector2_##name##_t v) {return c2<utils_vector2_##name##_t>(o2<ctype>(v).sign());}; \
        utils_vector2_##name##_t utils_vector2_##name##_normalize(utils_vector2_##name##_t v) {return c2<utils_vector2_##name##_t>(o2<ctype>(v).normalize());}; \
        ctype utils_vector2_##name##_dot(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return o2<ctype>(a).dot(o2<ctype>(b));}; \
        ctype utils_vector2_##name##_cross(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return o2<ctype>(a).cross(o2<ctype>(b));}; \
        ctype utils_vector2_##name##_length(utils_vector2_##name##_t v) {return o2<ctype>(v).length();}; \
        ctype utils_vector2_##name##_length_squared(utils_vector2_##name##_t v) {return o2<ctype>(v).lengthSquared();}; \
        bool utils_vector2_##name##_equal(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return o2<ctype>(a) == o2<ctype>(b);}; \
        bool utils_vector2_##name##_less(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return o2<ctype>(a) < o2<ctype>(b);}; \
        bool utils_vector2_##name##_less_equal(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return o2<ctype>(a) <= o2<ctype>(b);}; \
        bool utils_vector2_##name##_greater(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return o2<ctype>(a) > o2<ctype>(b);}; \
        bool utils_vector2_##name##_greater_equal(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return o2<ctype>(a) >= o2<ctype>(b);}; \
        ctype utils_vector2_##name##_get(utils_vector2_##name##_t v, size_t index) {return utils::exception::c_api::guard_fallback(static_cast<ctype>(0), [&](void) -> ctype {return o2<ctype>(v).get(index);});}; \
        int utils_vector2_##name##_set(utils_vector2_##name##_t* v, size_t index, ctype value) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                utils::type::OVector2<ctype> res = o2<ctype>(utils::exception::c_api::not_null_ref(v, "vector")); \
                res[index] = value; \
                *v = c2<utils_vector2_##name##_t>(res); \
            }); \
        }
    #define C_API_VECTOR2_INT_DEF(name, ctype) \
        utils_vector2_##name##_t utils_vector2_##name##_and(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a) & o2<ctype>(b));}; \
        utils_vector2_##name##_t utils_vector2_##name##_or(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a) | o2<ctype>(b));}; \
        utils_vector2_##name##_t utils_vector2_##name##_xor(utils_vector2_##name##_t a, utils_vector2_##name##_t b) {return c2<utils_vector2_##name##_t>(o2<ctype>(a) ^ o2<ctype>(b));}

    /* vector3 */
    #define C_API_VECTOR3_DEF(name, ctype) \
        utils_vector3_##name##_t utils_vector3_##name##_add(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a) + o3<ctype>(b));}; \
        utils_vector3_##name##_t utils_vector3_##name##_sub(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a) - o3<ctype>(b));}; \
        utils_vector3_##name##_t utils_vector3_##name##_mul(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a) * o3<ctype>(b));}; \
        utils_vector3_##name##_t utils_vector3_##name##_div(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a) / o3<ctype>(b));}; \
        utils_vector3_##name##_t utils_vector3_##name##_add_scalar(utils_vector3_##name##_t v, ctype s) {return c3<utils_vector3_##name##_t>(o3<ctype>(v) + s);}; \
        utils_vector3_##name##_t utils_vector3_##name##_sub_scalar(utils_vector3_##name##_t v, ctype s) {return c3<utils_vector3_##name##_t>(o3<ctype>(v) - s);}; \
        utils_vector3_##name##_t utils_vector3_##name##_mul_scalar(utils_vector3_##name##_t v, ctype s) {return c3<utils_vector3_##name##_t>(o3<ctype>(v) * s);}; \
        utils_vector3_##name##_t utils_vector3_##name##_div_scalar(utils_vector3_##name##_t v, ctype s) {return c3<utils_vector3_##name##_t>(o3<ctype>(v) / s);}; \
        utils_vector3_##name##_t utils_vector3_##name##_neg(utils_vector3_##name##_t v) {return c3<utils_vector3_##name##_t>(-o3<ctype>(v));}; \
        utils_vector3_##name##_t utils_vector3_##name##_min(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a).min(o3<ctype>(b)));}; \
        utils_vector3_##name##_t utils_vector3_##name##_max(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a).max(o3<ctype>(b)));}; \
        utils_vector3_##name##_t utils_vector3_##name##_clamp(utils_vector3_##name##_t v, utils_vector3_##name##_t min, utils_vector3_##name##_t max) {return c3<utils_vector3_##name##_t>(o3<ctype>(v).clamp(o3<ctype>(min), o3<ctype>(max)));}; \
        utils_vector3_##name##_t utils_vector3_##name##_sign(utils_vector3_##name##_t v) {return c3<utils_vector3_##name##_t>(o3<ctype>(v).sign());}; \
        utils_vector3_##name##_t utils_vector3_##name##_normalize(utils_vector3_##name##_t v) {return c3<utils_vector3_##name##_t>(o3<ctype>(v).normalize());}; \
        utils_vector3_##name##_t utils_vector3_##name##_cross(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a).cross(o3<ctype>(b)));}; \
        ctype utils_vector3_##name##_dot(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return o3<ctype>(a).dot(o3<ctype>(b));}; \
        ctype utils_vector3_##name##_length(utils_vector3_##name##_t v) {return o3<ctype>(v).length();}; \
        ctype utils_vector3_##name##_length_squared(utils_vector3_##name##_t v) {return o3<ctype>(v).lengthSquared();}; \
        bool utils_vector3_##name##_equal(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return o3<ctype>(a) == o3<ctype>(b);}; \
        bool utils_vector3_##name##_less(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return o3<ctype>(a) < o3<ctype>(b);}; \
        bool utils_vector3_##name##_less_equal(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return o3<ctype>(a) <= o3<ctype>(b);}; \
        bool utils_vector3_##name##_greater(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return o3<ctype>(a) > o3<ctype>(b);}; \
        bool utils_vector3_##name##_greater_equal(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return o3<ctype>(a) >= o3<ctype>(b);}; \
        ctype utils_vector3_##name##_get(utils_vector3_##name##_t v, size_t index) {return utils::exception::c_api::guard_fallback(static_cast<ctype>(0), [&](void) -> ctype {return o3<ctype>(v).get(index);});}; \
        int utils_vector3_##name##_set(utils_vector3_##name##_t* v, size_t index, ctype value) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                utils::type::OVector3<ctype> res = o3<ctype>(utils::exception::c_api::not_null_ref(v, "vector")); \
                res[index] = value; \
                *v = c3<utils_vector3_##name##_t>(res); \
            }); \
        }
    #define C_API_VECTOR3_INT_DEF(name, ctype) \
        utils_vector3_##name##_t utils_vector3_##name##_and(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a) & o3<ctype>(b));}; \
        utils_vector3_##name##_t utils_vector3_##name##_or(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a) | o3<ctype>(b));}; \
        utils_vector3_##name##_t utils_vector3_##name##_xor(utils_vector3_##name##_t a, utils_vector3_##name##_t b) {return c3<utils_vector3_##name##_t>(o3<ctype>(a) ^ o3<ctype>(b));}

    /* matrix operations: new handle, _r (out), _assign (a) */
    #define C_API_MATRIX_OP2_DEF(name, ctype, op, arg_type, arg) \
        utils_matrix_##name##_t* utils_matrix_##name##_##op(const utils_matrix_##name##_t* a, arg_type b) \
        {return matrix_new<utils_matrix_##name##_t>([&](void) {return MatrixOp<ctype>::op(matrix(a), arg);});} \
        int utils_matrix_##name##_##op##_r(const utils_matrix_##name##_t* a, arg_type b, utils_matrix_##name##_t* out) \
        {return matrix_r(out, [&](void) {return MatrixOp<ctype>::op(matrix(a), arg);});} \
        int utils_matrix_##name##_##op##_assign(utils_matrix_##name##_t* a, arg_type b) \
        {return matrix_r(a, [&](void) {return MatrixOp<ctype>::op(matrix(a), arg);});}
    #define C_API_MATRIX_OP1_DEF(name, ctype, op) \
        utils_matrix_##name##_t* utils_matrix_##name##_##op(const utils_matrix_##name##_t* a) \
        {return matrix_new<utils_matrix_##name##_t>([&](void) {return MatrixOp<ctype>::op(matrix(a));});} \
        int utils_matrix_##name##_##op##_r(const utils_matrix_##name##_t* a, utils_matrix_##name##_t* out) \
        {return matrix_r(out, [&](void) {return MatrixOp<ctype>::op(matrix(a));});} \
        int utils_matrix_##name##_##op##_assign(utils_matrix_##name##_t* a) \
        {return matrix_r(a, [&](void) {return MatrixOp<ctype>::op(matrix(a));});}

    /* matrix */
    #define C_API_MATRIX_DEF(name, ctype) \
        utils_matrix_##name##_t* utils_matrix_##name##_create(size_t rows, size_t cols) \
        {return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_matrix_##name##_t* {return new utils_matrix_##name##_t(utils::type::OMatrix<ctype>(rows, cols));});} \
        utils_matrix_##name##_t* utils_matrix_##name##_copy(const utils_matrix_##name##_t* m) \
        {return matrix_new<utils_matrix_##name##_t>([&](void) {return matrix(m);});} \
        void utils_matrix_##name##_destroy(utils_matrix_##name##_t* m) {(void)utils::exception::c_api::guard([&](void) {delete m;});}; \
        size_t utils_matrix_##name##_rows(const utils_matrix_##name##_t* m) {return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return matrix(m).row();});}; \
        size_t utils_matrix_##name##_cols(const utils_matrix_##name##_t* m) {return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return matrix(m).col();});}; \
        int utils_matrix_##name##_get(const utils_matrix_##name##_t* m, size_t x, size_t y, ctype* value) \
        {return utils::exception::c_api::guard([&](void) {ctype& res = utils::exception::c_api::not_null_ref(value, "value"); res = matrix(m).at(x, y);});} \
        int utils_matrix_##name##_set(utils_matrix_##name##_t* m, size_t x, size_t y, ctype value) {return utils::exception::c_api::guard([&](void) {matrix(m).set(x, y, value);});}; \
        int utils_matrix_##name##_set_values(utils_matrix_##name##_t* m, const ctype* values, size_t count, size_t x, size_t y) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                if (count > 0) (void)utils::exception::c_api::not_null(values, "values"); \
                matrix(m).set(values, values + count, x, y); \
            }); \
        } \
        size_t utils_matrix_##name##_row_r(const utils_matrix_##name##_t* m, size_t n, ctype* buf, size_t count) \
        { \
            return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t { \
                const std::vector<ctype> row = matrix(m).row(n); \
                if (count > 0) std::copy_n(row.begin(), std::min(count, row.size()), utils::exception::c_api::not_null(buf, "buffer")); \
                return row.size(); \
            }); \
        } \
        size_t utils_matrix_##name##_col_r(const utils_matrix_##name##_t* m, size_t n, ctype* buf, size_t count) \
        { \
            return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t { \
                const std::vector<ctype> col = matrix(m).col(n); \
                if (count > 0) std::copy_n(col.begin(), std::min(count, col.size()), utils::exception::c_api::not_null(buf, "buffer")); \
                return col.size(); \
            }); \
        } \
        int utils_matrix_##name##_swap(utils_matrix_##name##_t* m, size_t ax, size_t ay, size_t bx, size_t by) \
        {return utils::exception::c_api::guard([&](void) {utils::type::OMatrix<ctype>& self = matrix(m); std::swap(self.at(ax, ay), self.at(bx, by));});} \
        int utils_matrix_##name##_swap_row(utils_matrix_##name##_t* m, size_t a, size_t b) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                utils::type::OMatrix<ctype>& self = matrix(m); \
                for (std::size_t y = 0; y < self.col(); ++y) \
                    std::swap(self.at(a, y), self.at(b, y)); \
            }); \
        } \
        int utils_matrix_##name##_swap_col(utils_matrix_##name##_t* m, size_t a, size_t b) {return utils::exception::c_api::guard([&](void) {matrix(m).swapCol(a, b);});}; \
        int utils_matrix_##name##_clear(utils_matrix_##name##_t* m) \
        {return utils::exception::c_api::guard([&](void) {utils::type::OMatrix<ctype>& self = matrix(m); self = utils::type::OMatrix<ctype>(self.row(), self.col());});} \
        int utils_matrix_##name##_det(const utils_matrix_##name##_t* m, ctype* value) \
        {return utils::exception::c_api::guard([&](void) {ctype& res = utils::exception::c_api::not_null_ref(value, "value"); res = matrix(m).det();});} \
        int utils_matrix_##name##_trace(const utils_matrix_##name##_t* m, ctype* value) \
        {return utils::exception::c_api::guard([&](void) {ctype& res = utils::exception::c_api::not_null_ref(value, "value"); res = matrix(m).trace();});} \
        bool utils_matrix_##name##_equal(const utils_matrix_##name##_t* a, const utils_matrix_##name##_t* b) \
        {return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return matrix(a) == matrix(b);});} \
        C_API_MATRIX_OP2_DEF(name, ctype, add, const utils_matrix_##name##_t*, matrix(b)) \
        C_API_MATRIX_OP2_DEF(name, ctype, sub, const utils_matrix_##name##_t*, matrix(b)) \
        C_API_MATRIX_OP2_DEF(name, ctype, mul, const utils_matrix_##name##_t*, matrix(b)) \
        C_API_MATRIX_OP2_DEF(name, ctype, mul_scalar, ctype, b) \
        C_API_MATRIX_OP2_DEF(name, ctype, div_scalar, ctype, b) \
        C_API_MATRIX_OP1_DEF(name, ctype, neg) \
        C_API_MATRIX_OP1_DEF(name, ctype, transpose)
    #define C_API_MATRIX_FLOAT_DEF(name, ctype) \
        C_API_MATRIX_OP2_DEF(name, ctype, div, const utils_matrix_##name##_t*, matrix(b)) \
        C_API_MATRIX_OP1_DEF(name, ctype, invert)

    /* bidirectional lookup table of one type */
    #define C_API_BLT_SAME_DEF(name, ctype) \
        utils_blt_##name##_t* utils_blt_##name##_create(void) {return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_blt_##name##_t* {return new utils_blt_##name##_t();});}; \
        void utils_blt_##name##_destroy(utils_blt_##name##_t* blt) {(void)utils::exception::c_api::guard([&](void) {delete blt;});}; \
        int utils_blt_##name##_clear(utils_blt_##name##_t* blt) {return utils::exception::c_api::guard([&](void) {table(blt).clear();});}; \
        int utils_blt_##name##_set(utils_blt_##name##_t* blt, ctype left, ctype right, bool force) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                utils_blt_##name##_t& self = table(blt); \
                if (force) self.template setElement<true>(utils::type::c_api::Storage<ctype>::cpp(left), utils::type::c_api::Storage<ctype>::cpp(right)); \
                else self.template setElement<false>(utils::type::c_api::Storage<ctype>::cpp(left), utils::type::c_api::Storage<ctype>::cpp(right)); \
            }); \
        } \
        int utils_blt_##name##_add(utils_blt_##name##_t* blt, ctype left, ctype right) \
        {return utils::exception::c_api::guard([&](void) {table(blt).addElement(utils::type::c_api::Storage<ctype>::cpp(left), utils::type::c_api::Storage<ctype>::cpp(right));});} \
        int utils_blt_##name##_remove(utils_blt_##name##_t* blt, ctype element) \
        {return utils::exception::c_api::guard([&](void) {table(blt).removeElement(utils::type::c_api::Storage<ctype>::cpp(element));});} \
        int utils_blt_##name##_remove_all(utils_blt_##name##_t* blt, ctype const* elements_, size_t count) \
        {return utils::exception::c_api::guard([&](void) {utils_blt_##name##_t& self = table(blt); self.removeElements(elements(elements_, count));});} \
        int utils_blt_##name##_get(const utils_blt_##name##_t* blt, ctype element, ctype* other) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                ctype& res = utils::exception::c_api::not_null_ref(other, "other"); \
                res = utils::type::c_api::Storage<ctype>::c(table(blt)[utils::type::c_api::Storage<ctype>::cpp(element)]); \
            }); \
        }

    /* bidirectional lookup table of two types */
    #define C_API_BLT_DEF(name, left_type, right_type) \
        utils_blt_##name##_t* utils_blt_##name##_create(void) {return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_blt_##name##_t* {return new utils_blt_##name##_t();});}; \
        void utils_blt_##name##_destroy(utils_blt_##name##_t* blt) {(void)utils::exception::c_api::guard([&](void) {delete blt;});}; \
        int utils_blt_##name##_clear(utils_blt_##name##_t* blt) {return utils::exception::c_api::guard([&](void) {table(blt).clear();});}; \
        int utils_blt_##name##_set(utils_blt_##name##_t* blt, left_type left, right_type right, bool force) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                utils_blt_##name##_t& self = table(blt); \
                if (force) self.template setElement<true>(utils::type::c_api::Storage<left_type>::cpp(left), utils::type::c_api::Storage<right_type>::cpp(right)); \
                else self.template setElement<false>(utils::type::c_api::Storage<left_type>::cpp(left), utils::type::c_api::Storage<right_type>::cpp(right)); \
            }); \
        } \
        int utils_blt_##name##_add(utils_blt_##name##_t* blt, left_type left, right_type right) \
        {return utils::exception::c_api::guard([&](void) {table(blt).addElement(utils::type::c_api::Storage<left_type>::cpp(left), utils::type::c_api::Storage<right_type>::cpp(right));});} \
        int utils_blt_##name##_remove_left(utils_blt_##name##_t* blt, left_type left) \
        {return utils::exception::c_api::guard([&](void) {table(blt).removeElement(utils::type::c_api::Storage<left_type>::cpp(left));});} \
        int utils_blt_##name##_remove_right(utils_blt_##name##_t* blt, right_type right) \
        {return utils::exception::c_api::guard([&](void) {table(blt).removeElement(utils::type::c_api::Storage<right_type>::cpp(right));});} \
        int utils_blt_##name##_remove_lefts(utils_blt_##name##_t* blt, left_type const* lefts, size_t count) \
        {return utils::exception::c_api::guard([&](void) {utils_blt_##name##_t& self = table(blt); self.removeElements(elements(lefts, count));});} \
        int utils_blt_##name##_remove_rights(utils_blt_##name##_t* blt, right_type const* rights, size_t count) \
        {return utils::exception::c_api::guard([&](void) {utils_blt_##name##_t& self = table(blt); self.removeElements(elements(rights, count));});} \
        int utils_blt_##name##_get_right(const utils_blt_##name##_t* blt, left_type left, right_type* right) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                right_type& res = utils::exception::c_api::not_null_ref(right, "right"); \
                res = utils::type::c_api::Storage<right_type>::c(table(blt)[utils::type::c_api::Storage<left_type>::cpp(left)]); \
            }); \
        } \
        int utils_blt_##name##_get_left(const utils_blt_##name##_t* blt, right_type right, left_type* left) \
        { \
            return utils::exception::c_api::guard([&](void) { \
                left_type& res = utils::exception::c_api::not_null_ref(left, "left"); \
                res = utils::type::c_api::Storage<left_type>::c(table(blt)[utils::type::c_api::Storage<right_type>::cpp(right)]); \
            }); \
        }
    #define C_API_BLT_STR_INT_DEF(name, ctype) \
        C_API_BLT_DEF(str_##name, const char*, ctype) \
        C_API_BLT_DEF(name##_str, ctype, const char*)

//----------------------------------------------------------------//
/* STRUCT */

/* worker of the C side (element of a load balancer) */
struct utils_worker: public utils_handle, public utils::type::Worker {
    void* data = nullptr;

    // ---------- Constructor --------- //
    utils_worker(void* d = nullptr): data{d} { /* Nothing */ }; // default: a worker of a load balancer
};

/* bidirectional lookup table of bytes */
struct utils_blt: public utils_handle, public utils::type::BidirectionalLookupTable<std::string, std::string> {};

/* matrices */
C_API_NUMERIC_TYPES(C_API_MATRIX_HANDLE)

/* bidirectional lookup tables of the fixed types */
C_API_BLT_SAME_HANDLE(str_str, const char*)
C_API_BLT_SAME_HANDLE(size_size, std::size_t)
C_API_BLT_SAME_HANDLE(i64_i64, std::int64_t)
C_API_INT_TYPES(C_API_BLT_STR_INT_HANDLE)

#endif /* TYPE_CAPI_H */
