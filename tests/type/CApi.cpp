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
##  @file CApi.cpp

File Description:
##  Unit tests of the C API of utils::type (Freezable / Worker of any handle, vectors, matrices, bidirectional lookup tables)
\**************************************************************/

#include "utils/type/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <string>

class TypeCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

//----------------------------------------------------------------//
/* VECTOR */

TEST_F(TypeCApiTest, Vector2OfDouble)
{
    const utils_vector2_f64_t a = {3.0, 4.0};
    const utils_vector2_f64_t b = {1.0, -2.0};
    const utils_vector2_f64_t sum = utils_vector2_f64_add(a, b);
    const utils_vector2_f64_t n = utils_vector2_f64_normalize(a);

    EXPECT_DOUBLE_EQ(sum.x, 4.0);
    EXPECT_DOUBLE_EQ(sum.y, 2.0);
    EXPECT_DOUBLE_EQ(utils_vector2_f64_length(a), 5.0);
    EXPECT_DOUBLE_EQ(utils_vector2_f64_length_squared(a), 25.0);
    EXPECT_DOUBLE_EQ(utils_vector2_f64_dot(a, b), -5.0);
    EXPECT_DOUBLE_EQ(utils_vector2_f64_cross(a, b), -10.0);
    EXPECT_DOUBLE_EQ(n.x, 0.6);
    EXPECT_DOUBLE_EQ(n.y, 0.8);
    EXPECT_DOUBLE_EQ(utils_vector2_f64_mul_scalar(a, 2.0).y, 8.0);
    EXPECT_DOUBLE_EQ(utils_vector2_f64_neg(b).y, 2.0);
    EXPECT_DOUBLE_EQ(utils_vector2_f64_clamp(a, {0.0, 0.0}, {2.0, 2.0}).x, 2.0);
    EXPECT_TRUE(utils_vector2_f64_equal(utils_vector2_f64_sub(sum, b), a));
}

TEST_F(TypeCApiTest, Vector2GetSetAndIndexError)
{
    utils_vector2_i32_t v = {1, 2};

    EXPECT_EQ(utils_vector2_i32_get(v, 1), 2);
    EXPECT_EQ(utils_vector2_i32_set(&v, 0, 7), UTILS_OK);
    EXPECT_EQ(v.x, 7);
    EXPECT_EQ(utils_vector2_i32_get(v, 2), 0);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::VectorInvalidIndex));
    EXPECT_EQ(utils_vector2_i32_set(nullptr, 0, 1), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The vector is NULL");
}

TEST_F(TypeCApiTest, Vector3OfIntegersAndOtherTypes)
{
    const utils_vector3_i64_t a = {1, 0, 0};
    const utils_vector3_i64_t b = {0, 1, 0};
    const utils_vector3_i64_t c = utils_vector3_i64_cross(a, b);
    const utils_vector3_u8_t mask = utils_vector3_u8_and({0xFF, 0x0F, 0xF0}, {0x0F, 0xFF, 0x0F});
    const utils_vector3_u8_t sign = utils_vector3_u8_sign({0, 5, 9});

    EXPECT_EQ(c.z, 1);
    EXPECT_EQ(utils_vector3_i64_dot(a, b), 0);
    EXPECT_EQ(mask.x, 0x0F);
    EXPECT_EQ(mask.z, 0x00);
    EXPECT_EQ(sign.x, 0);
    EXPECT_EQ(sign.y, 1);
    EXPECT_FLOAT_EQ(utils_vector3_f32_length({2.0f, 3.0f, 6.0f}), 7.0f);
    EXPECT_EQ(utils_vector3_f128_length({2.0L, 3.0L, 6.0L}), 7.0L);
    EXPECT_EQ(utils_vector2_size_length({6, 8}), 10u);
}

//----------------------------------------------------------------//
/* MATRIX */

TEST_F(TypeCApiTest, MatrixValuesDetTraceAndRows)
{
    const double values[] = {1.0, 2.0, 3.0, 4.0};
    utils_matrix_f64_t* m = utils_matrix_f64_create(2, 2);
    double value = 0.0;
    double row[4] = {};

    ASSERT_NE(m, nullptr);
    EXPECT_EQ(utils_matrix_f64_rows(m), 2u);
    EXPECT_EQ(utils_matrix_f64_cols(m), 2u);
    EXPECT_EQ(utils_matrix_f64_set_values(m, values, 4, 0, 0), UTILS_OK);
    EXPECT_EQ(utils_matrix_f64_get(m, 1, 0, &value), UTILS_OK);
    EXPECT_DOUBLE_EQ(value, 3.0);
    EXPECT_EQ(utils_matrix_f64_det(m, &value), UTILS_OK);
    EXPECT_DOUBLE_EQ(value, -2.0);
    EXPECT_EQ(utils_matrix_f64_trace(m, &value), UTILS_OK);
    EXPECT_DOUBLE_EQ(value, 5.0);
    EXPECT_EQ(utils_matrix_f64_row_r(m, 1, row, 4), 2u);
    EXPECT_DOUBLE_EQ(row[1], 4.0);
    EXPECT_EQ(utils_matrix_f64_col_r(m, 0, row, 1), 2u);
    EXPECT_DOUBLE_EQ(row[0], 1.0);
    EXPECT_EQ(utils_matrix_f64_swap_row(m, 0, 1), UTILS_OK);
    EXPECT_EQ(utils_matrix_f64_get(m, 0, 0, &value), UTILS_OK);
    EXPECT_DOUBLE_EQ(value, 3.0);
    EXPECT_EQ(utils_matrix_f64_get(m, 2, 0, &value), UTILS_KO);
    EXPECT_EQ(utils_matrix_f64_clear(m), UTILS_OK);
    EXPECT_EQ(utils_matrix_f64_get(m, 0, 0, &value), UTILS_OK);
    EXPECT_DOUBLE_EQ(value, 0.0);
    utils_matrix_f64_destroy(m);
}

TEST_F(TypeCApiTest, MatrixOperationsInTheThreeForms)
{
    const double values[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    utils_matrix_f64_t* a = utils_matrix_f64_create(2, 3);
    utils_matrix_f64_t* out = utils_matrix_f64_create(1, 1);
    double value = 0.0;

    ASSERT_EQ(utils_matrix_f64_set_values(a, values, 6, 0, 0), UTILS_OK);
    utils_matrix_f64_t* t = utils_matrix_f64_transpose(a);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(utils_matrix_f64_rows(t), 3u);
    EXPECT_EQ(utils_matrix_f64_transpose_r(a, out), UTILS_OK);
    EXPECT_TRUE(utils_matrix_f64_equal(t, out));
    utils_matrix_f64_t* product = utils_matrix_f64_mul(a, t); // 2x3 * 3x2
    ASSERT_NE(product, nullptr);
    EXPECT_EQ(utils_matrix_f64_get(product, 0, 0, &value), UTILS_OK);
    EXPECT_DOUBLE_EQ(value, 14.0);
    EXPECT_EQ(utils_matrix_f64_mul_scalar_assign(a, 2.0), UTILS_OK);
    EXPECT_EQ(utils_matrix_f64_get(a, 1, 2, &value), UTILS_OK);
    EXPECT_DOUBLE_EQ(value, 12.0);
    EXPECT_EQ(utils_matrix_f64_add_r(a, a, out), UTILS_OK); // out replaced (its size too)
    EXPECT_EQ(utils_matrix_f64_rows(out), 2u);
    EXPECT_EQ(utils_matrix_f64_add(a, t), nullptr); // not the same size
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_EQ(utils_matrix_f64_det(a, &value), UTILS_KO); // not square
    utils_matrix_f64_destroy(product);
    utils_matrix_f64_destroy(t);
    utils_matrix_f64_destroy(out);
    utils_matrix_f64_destroy(a);
}

TEST_F(TypeCApiTest, MatrixInvertOfFloatsAndIntegers)
{
    const double values[] = {4.0, 7.0, 2.0, 6.0};
    const int32_t ints[] = {1, 2, 3, 4};
    utils_matrix_f64_t* m = utils_matrix_f64_create(2, 2);
    utils_matrix_i32_t* i = utils_matrix_i32_create(2, 2);
    double value = 0.0;
    int32_t det = 0;

    ASSERT_EQ(utils_matrix_f64_set_values(m, values, 4, 0, 0), UTILS_OK);
    utils_matrix_f64_t* inv = utils_matrix_f64_invert(m);
    ASSERT_NE(inv, nullptr);
    EXPECT_EQ(utils_matrix_f64_get(inv, 0, 0, &value), UTILS_OK);
    EXPECT_NEAR(value, 0.6, 1e-12);
    EXPECT_EQ(utils_matrix_f64_div_assign(m, m), UTILS_OK); // m * m^-1 = I
    EXPECT_EQ(utils_matrix_f64_get(m, 0, 0, &value), UTILS_OK);
    EXPECT_NEAR(value, 1.0, 1e-12);
    ASSERT_EQ(utils_matrix_i32_set_values(i, ints, 4, 0, 0), UTILS_OK);
    EXPECT_EQ(utils_matrix_i32_neg_assign(i), UTILS_OK);
    EXPECT_EQ(utils_matrix_i32_trace(i, &det), UTILS_OK);
    EXPECT_EQ(det, -5);
    EXPECT_EQ(utils_matrix_i32_set_values(i, nullptr, 2, 0, 0), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The values is NULL");
    utils_matrix_i32_destroy(i);
    utils_matrix_f64_destroy(inv);
    utils_matrix_f64_destroy(m);
}

//----------------------------------------------------------------//
/* BIDIRECTIONAL LOOKUP TABLE */

TEST_F(TypeCApiTest, BltOfStringAndSize)
{
    utils_blt_str_size_t* blt = utils_blt_str_size_create();
    const char* names[] = {"one"};
    const char* name = nullptr;
    size_t id = 0;

    ASSERT_NE(blt, nullptr);
    EXPECT_EQ(utils_blt_str_size_add(blt, "one", 1), UTILS_OK);
    EXPECT_EQ(utils_blt_str_size_add(blt, "two", 2), UTILS_OK);
    EXPECT_EQ(utils_blt_str_size_get_right(blt, "two", &id), UTILS_OK);
    EXPECT_EQ(id, 2u);
    EXPECT_EQ(utils_blt_str_size_get_left(blt, 1, &name), UTILS_OK);
    EXPECT_STREQ(name, "one");
    EXPECT_EQ(utils_blt_str_size_add(blt, "one", 3), UTILS_KO); // already there
    EXPECT_EQ(utils_blt_str_size_set(blt, "one", 3, true), UTILS_OK);
    EXPECT_EQ(utils_blt_str_size_get_right(blt, "one", &id), UTILS_OK);
    EXPECT_EQ(id, 3u);
    EXPECT_EQ(utils_blt_str_size_remove_lefts(blt, names, 1), UTILS_OK);
    EXPECT_EQ(utils_blt_str_size_get_right(blt, "one", &id), UTILS_KO);
    EXPECT_EQ(utils_blt_str_size_remove_right(blt, 2), UTILS_OK);
    EXPECT_EQ(utils_blt_str_size_get_left(blt, 2, &name), UTILS_KO);
    EXPECT_EQ(utils_blt_str_size_add(blt, nullptr, 4), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The string is NULL");
    utils_blt_str_size_destroy(blt);
}

TEST_F(TypeCApiTest, BltOfOneTypeAndBytes)
{
    utils_blt_str_str_t* words = utils_blt_str_str_create();
    utils_blt_t* bytes = utils_blt_create();
    const char* other = nullptr;
    size_t size = 0;

    EXPECT_EQ(utils_blt_str_str_add(words, "yes", "oui"), UTILS_OK);
    EXPECT_EQ(utils_blt_str_str_get(words, "oui", &other), UTILS_OK);
    EXPECT_STREQ(other, "yes");
    EXPECT_EQ(utils_blt_str_str_get(words, "yes", &other), UTILS_OK);
    EXPECT_STREQ(other, "oui");
    EXPECT_EQ(utils_blt_add(bytes, "a\0b", 3, "xyz", 3), UTILS_OK);
    const void* found = utils_blt_get(bytes, "xyz", 3, &size);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(size, 3u);
    EXPECT_EQ(std::memcmp(found, "a\0b", 3), 0);
    EXPECT_EQ(utils_blt_remove(bytes, "a\0b", 3), UTILS_OK);
    EXPECT_EQ(utils_blt_get(bytes, "xyz", 3, nullptr), nullptr);
    utils_blt_destroy(bytes);
    utils_blt_str_str_destroy(words);
}

//----------------------------------------------------------------//
/* FREEZABLE / WORKER */

TEST_F(TypeCApiTest, FreezableOfAnyHandle)
{
    utils_blt_i32_str_t* blt = utils_blt_i32_str_create();
    utils_worker_t* worker = utils_worker_create(nullptr);

    EXPECT_FALSE(utils_freezable_is_frozen(blt));
    EXPECT_EQ(utils_freezable_require_unfrozen(blt), UTILS_OK);
    EXPECT_EQ(utils_freezable_freeze(blt), UTILS_OK);
    EXPECT_TRUE(utils_freezable_is_frozen(blt));
    EXPECT_EQ(utils_blt_i32_str_add(blt, 1, "one"), UTILS_KO);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::Frozen));
    EXPECT_EQ(utils_freezable_require_frozen(blt), UTILS_OK);
    EXPECT_EQ(utils_freezable_freeze(worker), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "This handle has no Freezable");
    utils_worker_destroy(worker);
    utils_blt_i32_str_destroy(blt);
}

TEST_F(TypeCApiTest, WorkerOfAnyHandle)
{
    int data = 0;
    utils_worker_t* worker = utils_worker_create(&data);
    utils_blt_t* blt = utils_blt_create();

    ASSERT_NE(worker, nullptr);
    EXPECT_EQ(utils_worker_get_data(worker), &data);
    EXPECT_EQ(utils_worker_set_data(worker, nullptr), UTILS_OK);
    EXPECT_EQ(utils_worker_get_data(worker), nullptr);
    EXPECT_FALSE(utils_worker_is_working(worker));
    const uint64_t before = utils_worker_get_stoped_working_timestamp(worker);
    EXPECT_EQ(utils_worker_set_working_status(worker, true), UTILS_OK);
    EXPECT_TRUE(utils_worker_is_working(worker));
    EXPECT_EQ(utils_worker_set_working_status(worker, false), UTILS_OK);
    EXPECT_GE(utils_worker_get_stoped_working_timestamp(worker), before);
    EXPECT_FALSE(utils_worker_is_working(blt));
    EXPECT_STREQ(utils_exception.info, "This handle has no Worker");
    utils_blt_destroy(blt);
    utils_worker_destroy(worker);
}
