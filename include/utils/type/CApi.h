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
##  @file CApi.h

File Description:
##  C API of utils::type: Freezable / Worker of any handle, vectors, matrices & bidirectional lookup tables (C & C++ compatible)
##  The templates are given for the fixed types of type/CApiTypes.h (suffix: i8 ... u64, size, f32, f64, f128, bool, char, str) & as bytes (void* + size)
##  Every function that can fail returns UTILS_OK / UTILS_KO (or NULL / 0), details in utils_exception
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef TYPE_CAPI_C_H
    #define TYPE_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _nodiscard, _nothrow, _read_only, _write_only
    #include "../exception/CApi.h"          // utils_exception, UTILS_OK, UTILS_KO
    #include "CApiTypes.h"                  // C_API_*_TYPES
    #include <stdbool.h>                    // bool
    #include <stddef.h>                     // size_t
    #include <stdint.h>                     // uint64_t

    //----------------------------------------------------------------//
    /* MACRO */

    /* vector2 (by value, utils::type::OVector2) */
    #define C_API_VECTOR2_DECL(name, type) \
        typedef struct utils_vector2_##name { \
            type x; \
            type y; \
        } utils_vector2_##name##_t; \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_add(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_sub(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_mul(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_div(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_add_scalar(utils_vector2_##name##_t v, type s); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_sub_scalar(utils_vector2_##name##_t v, type s); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_mul_scalar(utils_vector2_##name##_t v, type s); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_div_scalar(utils_vector2_##name##_t v, type s); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_neg(utils_vector2_##name##_t v); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_min(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_max(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_clamp(utils_vector2_##name##_t v, utils_vector2_##name##_t min, utils_vector2_##name##_t max); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_sign(utils_vector2_##name##_t v); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_normalize(utils_vector2_##name##_t v); /* zero vector: kept */ \
        _nodiscard _nothrow type utils_vector2_##name##_dot(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow type utils_vector2_##name##_cross(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow type utils_vector2_##name##_length(utils_vector2_##name##_t v); /* truncated for the integer types */ \
        _nodiscard _nothrow type utils_vector2_##name##_length_squared(utils_vector2_##name##_t v); \
        _nodiscard _nothrow bool utils_vector2_##name##_equal(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow bool utils_vector2_##name##_less(utils_vector2_##name##_t a, utils_vector2_##name##_t b); /* like operator< of the c++ */ \
        _nodiscard _nothrow bool utils_vector2_##name##_less_equal(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow bool utils_vector2_##name##_greater(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow bool utils_vector2_##name##_greater_equal(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow type utils_vector2_##name##_get(utils_vector2_##name##_t v, size_t index); /* 0 on error (index >= 2) */ \
        _nodiscard _nothrow int utils_vector2_##name##_set(utils_vector2_##name##_t* v, size_t index, type value);

    /* vector2 of the integers (bitwise) */
    #define C_API_VECTOR2_INT_DECL(name, type) \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_and(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_or(utils_vector2_##name##_t a, utils_vector2_##name##_t b); \
        _nodiscard _nothrow utils_vector2_##name##_t utils_vector2_##name##_xor(utils_vector2_##name##_t a, utils_vector2_##name##_t b);

    /* vector3 (by value, utils::type::OVector3) */
    #define C_API_VECTOR3_DECL(name, type) \
        typedef struct utils_vector3_##name { \
            type x; \
            type y; \
            type z; \
        } utils_vector3_##name##_t; \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_add(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_sub(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_mul(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_div(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_add_scalar(utils_vector3_##name##_t v, type s); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_sub_scalar(utils_vector3_##name##_t v, type s); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_mul_scalar(utils_vector3_##name##_t v, type s); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_div_scalar(utils_vector3_##name##_t v, type s); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_neg(utils_vector3_##name##_t v); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_min(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_max(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_clamp(utils_vector3_##name##_t v, utils_vector3_##name##_t min, utils_vector3_##name##_t max); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_sign(utils_vector3_##name##_t v); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_normalize(utils_vector3_##name##_t v); /* zero vector: kept */ \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_cross(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow type utils_vector3_##name##_dot(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow type utils_vector3_##name##_length(utils_vector3_##name##_t v); /* truncated for the integer types */ \
        _nodiscard _nothrow type utils_vector3_##name##_length_squared(utils_vector3_##name##_t v); \
        _nodiscard _nothrow bool utils_vector3_##name##_equal(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow bool utils_vector3_##name##_less(utils_vector3_##name##_t a, utils_vector3_##name##_t b); /* like operator< of the c++ */ \
        _nodiscard _nothrow bool utils_vector3_##name##_less_equal(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow bool utils_vector3_##name##_greater(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow bool utils_vector3_##name##_greater_equal(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow type utils_vector3_##name##_get(utils_vector3_##name##_t v, size_t index); /* 0 on error (index >= 3) */ \
        _nodiscard _nothrow int utils_vector3_##name##_set(utils_vector3_##name##_t* v, size_t index, type value);

    /* vector3 of the integers (bitwise) */
    #define C_API_VECTOR3_INT_DECL(name, type) \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_and(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_or(utils_vector3_##name##_t a, utils_vector3_##name##_t b); \
        _nodiscard _nothrow utils_vector3_##name##_t utils_vector3_##name##_xor(utils_vector3_##name##_t a, utils_vector3_##name##_t b);

    /* operation of a matrix: new handle (NULL on error, destroy it), _r: in out (replaced, can be a / b), _assign: in a */
    #define C_API_MATRIX_OP2_DECL(name, op, arg) \
        _nodiscard _nothrow utils_matrix_##name##_t* utils_matrix_##name##_##op(const utils_matrix_##name##_t* a, arg); \
        _nodiscard _nothrow int utils_matrix_##name##_##op##_r(const utils_matrix_##name##_t* a, arg, utils_matrix_##name##_t* out); \
        _nodiscard _nothrow int utils_matrix_##name##_##op##_assign(utils_matrix_##name##_t* a, arg);
    #define C_API_MATRIX_OP1_DECL(name, op) \
        _nodiscard _nothrow utils_matrix_##name##_t* utils_matrix_##name##_##op(const utils_matrix_##name##_t* a); \
        _nodiscard _nothrow int utils_matrix_##name##_##op##_r(const utils_matrix_##name##_t* a, utils_matrix_##name##_t* out); \
        _nodiscard _nothrow int utils_matrix_##name##_##op##_assign(utils_matrix_##name##_t* a);

    /* matrix (handle, utils::type::OMatrix, (x, y) = (row, col)) */
    #define C_API_MATRIX_DECL(name, type) \
        typedef struct utils_matrix_##name utils_matrix_##name##_t; \
        _nodiscard _nothrow utils_matrix_##name##_t* utils_matrix_##name##_create(size_t rows, size_t cols); /* every value at 0, NULL on error */ \
        _nodiscard _nothrow utils_matrix_##name##_t* utils_matrix_##name##_copy(const utils_matrix_##name##_t* m); /* NULL on error */ \
        _nothrow void utils_matrix_##name##_destroy(utils_matrix_##name##_t* m); /* NULL: nothing */ \
        _nodiscard _nothrow size_t utils_matrix_##name##_rows(const utils_matrix_##name##_t* m); \
        _nodiscard _nothrow size_t utils_matrix_##name##_cols(const utils_matrix_##name##_t* m); \
        _nodiscard _nothrow int utils_matrix_##name##_get(const utils_matrix_##name##_t* m, size_t x, size_t y, type* value); \
        _nodiscard _nothrow int utils_matrix_##name##_set(utils_matrix_##name##_t* m, size_t x, size_t y, type value); \
        _nodiscard _nothrow _read_only(2, 3) int utils_matrix_##name##_set_values(utils_matrix_##name##_t* m, const type* values, size_t count, size_t x, size_t y); /* row by row from (x, y) */ \
        _nodiscard _nothrow _write_only(3, 4) size_t utils_matrix_##name##_row_r(const utils_matrix_##name##_t* m, size_t n, type* buf, size_t count); /* copied in buf (count max), returns the full length (0 on error) */ \
        _nodiscard _nothrow _write_only(3, 4) size_t utils_matrix_##name##_col_r(const utils_matrix_##name##_t* m, size_t n, type* buf, size_t count); \
        _nodiscard _nothrow int utils_matrix_##name##_swap(utils_matrix_##name##_t* m, size_t ax, size_t ay, size_t bx, size_t by); \
        _nodiscard _nothrow int utils_matrix_##name##_swap_row(utils_matrix_##name##_t* m, size_t a, size_t b); \
        _nodiscard _nothrow int utils_matrix_##name##_swap_col(utils_matrix_##name##_t* m, size_t a, size_t b); \
        _nodiscard _nothrow int utils_matrix_##name##_clear(utils_matrix_##name##_t* m); /* every value at 0 */ \
        _nodiscard _nothrow int utils_matrix_##name##_det(const utils_matrix_##name##_t* m, type* value); /* square only */ \
        _nodiscard _nothrow int utils_matrix_##name##_trace(const utils_matrix_##name##_t* m, type* value); /* square only */ \
        _nodiscard _nothrow bool utils_matrix_##name##_equal(const utils_matrix_##name##_t* a, const utils_matrix_##name##_t* b); \
        C_API_MATRIX_OP2_DECL(name, add, const utils_matrix_##name##_t* b) \
        C_API_MATRIX_OP2_DECL(name, sub, const utils_matrix_##name##_t* b) \
        C_API_MATRIX_OP2_DECL(name, mul, const utils_matrix_##name##_t* b) /* product */ \
        C_API_MATRIX_OP2_DECL(name, mul_scalar, type s) \
        C_API_MATRIX_OP2_DECL(name, div_scalar, type s) \
        C_API_MATRIX_OP1_DECL(name, neg) \
        C_API_MATRIX_OP1_DECL(name, transpose)

    /* matrix of the floating points (the integers would be truncated) */
    #define C_API_MATRIX_FLOAT_DECL(name, type) \
        C_API_MATRIX_OP2_DECL(name, div, const utils_matrix_##name##_t* b) /* a * b^-1 */ \
        C_API_MATRIX_OP1_DECL(name, invert)

    /* bidirectional lookup table of one type (utils::type::BidirectionalLookupTable<T, T>, an element is found on both sides) */
    #define C_API_BLT_SAME_DECL(name, type) \
        typedef struct utils_blt_##name utils_blt_##name##_t; \
        _nodiscard _nothrow utils_blt_##name##_t* utils_blt_##name##_create(void); /* NULL on error */ \
        _nothrow void utils_blt_##name##_destroy(utils_blt_##name##_t* blt); /* NULL: nothing */ \
        _nodiscard _nothrow int utils_blt_##name##_clear(utils_blt_##name##_t* blt); \
        _nodiscard _nothrow int utils_blt_##name##_set(utils_blt_##name##_t* blt, type left, type right, bool force); /* force: override an existing one */ \
        _nodiscard _nothrow int utils_blt_##name##_add(utils_blt_##name##_t* blt, type left, type right); \
        _nodiscard _nothrow int utils_blt_##name##_remove(utils_blt_##name##_t* blt, type element); \
        _nodiscard _nothrow int utils_blt_##name##_remove_all(utils_blt_##name##_t* blt, type const* elements, size_t count); \
        _nodiscard _nothrow int utils_blt_##name##_get(const utils_blt_##name##_t* blt, type element, type* other); /* the other side of element */

    /* bidirectional lookup table of two types (utils::type::BidirectionalLookupTable<L, R>) */
    #define C_API_BLT_DECL(name, left_type, right_type) \
        typedef struct utils_blt_##name utils_blt_##name##_t; \
        _nodiscard _nothrow utils_blt_##name##_t* utils_blt_##name##_create(void); /* NULL on error */ \
        _nothrow void utils_blt_##name##_destroy(utils_blt_##name##_t* blt); /* NULL: nothing */ \
        _nodiscard _nothrow int utils_blt_##name##_clear(utils_blt_##name##_t* blt); \
        _nodiscard _nothrow int utils_blt_##name##_set(utils_blt_##name##_t* blt, left_type left, right_type right, bool force); /* force: override an existing one */ \
        _nodiscard _nothrow int utils_blt_##name##_add(utils_blt_##name##_t* blt, left_type left, right_type right); \
        _nodiscard _nothrow int utils_blt_##name##_remove_left(utils_blt_##name##_t* blt, left_type left); \
        _nodiscard _nothrow int utils_blt_##name##_remove_right(utils_blt_##name##_t* blt, right_type right); \
        _nodiscard _nothrow int utils_blt_##name##_remove_lefts(utils_blt_##name##_t* blt, left_type const* lefts, size_t count); \
        _nodiscard _nothrow int utils_blt_##name##_remove_rights(utils_blt_##name##_t* blt, right_type const* rights, size_t count); \
        _nodiscard _nothrow int utils_blt_##name##_get_right(const utils_blt_##name##_t* blt, left_type left, right_type* right); \
        _nodiscard _nothrow int utils_blt_##name##_get_left(const utils_blt_##name##_t* blt, right_type right, left_type* left);

    /* string <-> integer, both orders (str_i32 & i32_str, ...) */
    #define C_API_BLT_STR_INT_DECL(name, type) \
        C_API_BLT_DECL(str_##name, const char*, type) \
        C_API_BLT_DECL(name##_str, type, const char*)

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* opaque types */
typedef struct utils_worker utils_worker_t; // utils::type::Worker + data of the C side (element of a load balancer)
typedef struct utils_blt utils_blt_t; // utils::type::BidirectionalLookupTable of bytes (an element is found on both sides)

//----------------------------------------------------------------//
/* PROTOTYPE */

/* Freezable of any handle (utils::type::Freezable in its class, else UTILS_KO / false: "This handle has no Freezable") */
_nodiscard _nothrow int utils_freezable_freeze(void* handle);
_nodiscard _nothrow bool utils_freezable_is_frozen(const void* handle);
_nodiscard _nothrow int utils_freezable_require_frozen(const void* handle); // UTILS_KO (ShouldBeFrozen) when not frozen
_nodiscard _nothrow int utils_freezable_require_unfrozen(const void* handle); // UTILS_KO (Frozen) when frozen

/* Worker of any handle (utils::type::Worker in its class, else UTILS_KO / false / 0: "This handle has no Worker") */
_nodiscard _nothrow utils_worker_t* utils_worker_create(void* data); // NULL on error
_nothrow void utils_worker_destroy(utils_worker_t* worker); // NULL: nothing
_nodiscard _nothrow void* utils_worker_get_data(const utils_worker_t* worker);
_nodiscard _nothrow int utils_worker_set_data(utils_worker_t* worker, void* data);
_nodiscard _nothrow int utils_worker_set_working_status(void* handle, bool status);
_nodiscard _nothrow bool utils_worker_is_working(const void* handle);
_nodiscard _nothrow uint64_t utils_worker_get_stoped_working_timestamp(const void* handle); // steady clock in ns (when the status was set to false), 0 on error

/* vectors */
C_API_NUMERIC_TYPES(C_API_VECTOR2_DECL)
C_API_INT_TYPES(C_API_VECTOR2_INT_DECL)
C_API_NUMERIC_TYPES(C_API_VECTOR3_DECL)
C_API_INT_TYPES(C_API_VECTOR3_INT_DECL)

/* matrices */
C_API_NUMERIC_TYPES(C_API_MATRIX_DECL)
C_API_FLOAT_TYPES(C_API_MATRIX_FLOAT_DECL)

/* bidirectional lookup tables of bytes (generic: an element = data + size, copied) */
_nodiscard _nothrow utils_blt_t* utils_blt_create(void); // NULL on error
_nothrow void utils_blt_destroy(utils_blt_t* blt); // NULL: nothing
_nodiscard _nothrow int utils_blt_clear(utils_blt_t* blt);
_nodiscard _nothrow _read_only(2, 3) _read_only(4, 5) int utils_blt_set(utils_blt_t* blt, const void* left, size_t left_size, const void* right, size_t right_size, bool force); // force: override an existing one
_nodiscard _nothrow _read_only(2, 3) _read_only(4, 5) int utils_blt_add(utils_blt_t* blt, const void* left, size_t left_size, const void* right, size_t right_size);
_nodiscard _nothrow _read_only(2, 3) int utils_blt_remove(utils_blt_t* blt, const void* element, size_t size);
_nodiscard _nothrow _read_only(2, 3) const void* utils_blt_get(const utils_blt_t* blt, const void* element, size_t size, size_t* other_size); // the other side of element, valid until the table is changed (NULL on error), other_size: optional

/* bidirectional lookup tables of the fixed types (str: const char* copied, the str given back are valid until the table is changed) */
C_API_BLT_SAME_DECL(str_str, const char*)
C_API_BLT_SAME_DECL(size_size, size_t)
C_API_BLT_SAME_DECL(i64_i64, int64_t)
C_API_INT_TYPES(C_API_BLT_STR_INT_DECL)

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* TYPE_CAPI_C_H */
