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
##  Unit tests of the C API of utils::pool (clusters, middlewares)
\**************************************************************/

#include "utils/pool/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <string>

class PoolCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

/* callbacks */
static int doubleIt(int32_t* element, void* data)
{
    *element *= 2;
    *static_cast<int32_t*>(data) += *element;
    return UTILS_OK;
}

static int upper(void* element, size_t size, _unused void* data)
{
    for (size_t i = 0; i < size; ++i)
        static_cast<char*>(element)[i] = static_cast<char>(std::toupper(static_cast<char*>(element)[i]));
    return UTILS_OK;
}

static int stopAtThree(int32_t* element, _unused void* data)
{
    if (*element == 3) return utils_fail(3, "three found");
    return UTILS_OK;
}

static int count(void* data)
{
    ++*static_cast<int*>(data);
    return UTILS_OK;
}

static int concat(const char* arg, void* data)
{
    *static_cast<std::string*>(data) += arg;
    return UTILS_OK;
}

static int refuse(double arg, _unused void* data)
{
    return (arg < 0.0 ? utils_fail(1, "negative value") : UTILS_OK);
}

static int bytesSize(_unused const void* arg, size_t size, void* data)
{
    *static_cast<size_t*>(data) += size;
    return UTILS_OK;
}

//----------------------------------------------------------------//
/* CLUSTER */

TEST_F(PoolCApiTest, ClusterOfAFixedType)
{
    utils_cluster_i32_t* cluster = utils_cluster_i32_create();
    int32_t sum = 0;

    ASSERT_NE(cluster, nullptr);
    EXPECT_EQ(utils_cluster_i32_spawn(cluster, 3, 5), UTILS_OK);
    EXPECT_EQ(utils_cluster_i32_size(cluster), 3u);
    EXPECT_EQ(utils_cluster_i32_apply(cluster, doubleIt, &sum), UTILS_OK);
    EXPECT_EQ(sum, 30);
    EXPECT_EQ(utils_cluster_i32_kill_n(cluster, 1), UTILS_OK);
    EXPECT_EQ(utils_cluster_i32_size(cluster), 2u);
    EXPECT_EQ(utils_cluster_i32_kill_n(cluster, 5), UTILS_KO);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::OutOfBounds));
    EXPECT_EQ(utils_cluster_i32_spawn(cluster, 1, 3), UTILS_OK);
    EXPECT_EQ(utils_cluster_i32_apply(cluster, stopAtThree, nullptr), UTILS_KO);
    EXPECT_STREQ(utils_exception.what, "three found");
    EXPECT_EQ(utils_cluster_i32_kill(cluster), UTILS_OK);
    EXPECT_EQ(utils_cluster_i32_size(cluster), 0u);
    utils_cluster_i32_destroy(cluster);
}

TEST_F(PoolCApiTest, ClusterOfBytes)
{
    utils_cluster_t* cluster = utils_cluster_create();
    utils_cluster_size_t* sizes = utils_cluster_size_create();

    EXPECT_EQ(utils_cluster_spawn(cluster, 2, "ab", 2), UTILS_OK);
    EXPECT_EQ(utils_cluster_apply(cluster, upper, nullptr), UTILS_OK);
    EXPECT_EQ(utils_cluster_size(cluster), 2u);
    EXPECT_EQ(utils_cluster_spawn(cluster, 1, nullptr, 3), UTILS_KO);
    EXPECT_EQ(utils_cluster_size_spawn(sizes, 1, 4), UTILS_OK); // the value 4, not 4 elements
    EXPECT_EQ(utils_cluster_size_size(sizes), 1u);
    utils_cluster_size_destroy(sizes);
    utils_cluster_destroy(cluster);
}

//----------------------------------------------------------------//
/* MIDDLEWARES */

TEST_F(PoolCApiTest, MiddlewaresWithoutArgument)
{
    utils_middlewares_t* middlewares = utils_middlewares_create();
    int calls = 0;

    ASSERT_NE(middlewares, nullptr);
    EXPECT_EQ(utils_middlewares_add_before(middlewares, count, &calls), UTILS_OK);
    EXPECT_EQ(utils_middlewares_add_before(middlewares, count, &calls), UTILS_OK);
    EXPECT_EQ(utils_middlewares_add_after(middlewares, count, &calls), UTILS_OK);
    EXPECT_EQ(utils_middlewares_before_count(middlewares), 2u);
    EXPECT_EQ(utils_middlewares_after_count(middlewares), 1u);
    EXPECT_EQ(utils_middlewares_call_before(middlewares), UTILS_OK);
    EXPECT_EQ(calls, 2);
    EXPECT_EQ(utils_middlewares_call_after(middlewares), UTILS_OK);
    EXPECT_EQ(calls, 3);
    EXPECT_EQ(utils_middlewares_clear(middlewares), UTILS_OK);
    EXPECT_EQ(utils_middlewares_before_count(middlewares), 0u);
    EXPECT_EQ(utils_middlewares_add_before(middlewares, nullptr, nullptr), UTILS_KO);
    utils_middlewares_destroy(middlewares);
}

TEST_F(PoolCApiTest, MiddlewaresOfFixedTypesAndBytes)
{
    utils_middlewares_str_t* strings = utils_middlewares_str_create();
    utils_middlewares_f64_t* values = utils_middlewares_f64_create();
    utils_middlewares_bytes_t* bytes = utils_middlewares_bytes_create();
    std::string text;
    size_t size = 0;

    EXPECT_EQ(utils_middlewares_str_add_before(strings, concat, &text), UTILS_OK);
    EXPECT_EQ(utils_middlewares_str_add_after(strings, concat, &text), UTILS_OK);
    EXPECT_EQ(utils_middlewares_str_call_before(strings, "a"), UTILS_OK);
    EXPECT_EQ(utils_middlewares_str_call_after(strings, "b"), UTILS_OK);
    EXPECT_EQ(text, "ab");
    EXPECT_EQ(utils_middlewares_f64_add_before(values, refuse, nullptr), UTILS_OK);
    EXPECT_EQ(utils_middlewares_f64_call_before(values, 1.0), UTILS_OK);
    EXPECT_EQ(utils_middlewares_f64_call_before(values, -1.0), UTILS_KO);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::MiddlewareCall));
    EXPECT_NE(std::string(utils_exception.info).find("negative value"), std::string::npos);
    EXPECT_EQ(utils_middlewares_bytes_add_after(bytes, bytesSize, &size), UTILS_OK);
    EXPECT_EQ(utils_middlewares_bytes_call_after(bytes, "a\0b", 3), UTILS_OK);
    EXPECT_EQ(size, 3u);
    utils_middlewares_bytes_destroy(bytes);
    utils_middlewares_f64_destroy(values);
    utils_middlewares_str_destroy(strings);
}
