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
##  Unit tests of the C API of the algorithms (c2dmp-hsm, s.o.s)
\**************************************************************/

#include "utils/algorithms/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <cmath>

class AlgorithmsCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

// Audio like carrier (sine + noise) on 16 bits
static std::vector<uint16_t> makeCarrier(const std::size_t size)
{
    std::mt19937 gen(42);
    std::normal_distribution<double> noise(0.0, 800.0);
    std::vector<uint16_t> carrier(size);

    for (std::size_t i = 0; i < size; ++i)
        carrier[i] = static_cast<uint16_t>(std::clamp(32768.0 + 18000.0 * std::sin(static_cast<double>(i) * 0.01) + noise(gen), 0.0, 65535.0));
    return carrier;
}

//----------------------------------------------------------------//
/* C2DMP */

TEST_F(AlgorithmsCApiTest, C2dmpIsTheCppOne)
{
    EXPECT_FLOAT_EQ(utils_c2dmp_default("hello", "hello"), utils::algorithms::c2dmp::c2dmp("hello", "hello"));
    EXPECT_FLOAT_EQ(utils_c2dmp_default("hello", "world"), utils::algorithms::c2dmp::c2dmp("hello", "world"));
    EXPECT_LT(utils_c2dmp_default("hellp", "hello"), utils_c2dmp_default("hellp", "quit"));
    EXPECT_FLOAT_EQ(utils_c2dmp("exot", 4, "exit", 4, 5, 8, true), (utils::algorithms::c2dmp::c2dmp<5, std::uint64_t, true>("exot", "exit")));
    EXPECT_FLOAT_EQ(utils_c2dmp("", 0, "abc", 3, 1, 2, false), (utils::algorithms::c2dmp::c2dmp<1, std::uint16_t>("", "abc")));
}

TEST_F(AlgorithmsCApiTest, C2dmpErrors)
{
    EXPECT_EQ(utils_c2dmp("a", 1, "b", 1, 6, 1, false), -1.0f);
    EXPECT_STREQ(utils_exception.info, "The prefix depth must be from 1 to 5");
    EXPECT_EQ(utils_c2dmp("a", 1, "b", 1, 3, 3, false), -1.0f);
    EXPECT_STREQ(utils_exception.info, "The counter size must be 1, 2, 4 or 8 bytes");
    EXPECT_EQ(utils_c2dmp_default(nullptr, "b"), -1.0f);
    EXPECT_STREQ(utils_exception.info, "The a is NULL");
}

//----------------------------------------------------------------//
/* S.O.S */

TEST_F(AlgorithmsCApiTest, SosEmbedExtract)
{
    std::vector<uint16_t> carrier = makeCarrier(200000);
    const std::vector<uint16_t> payload = {'s', 'e', 'c', 'r', 'e', 't'};
    const std::vector<uint16_t> key = {'k', 'e', 'y'};
    std::vector<uint16_t> out(16);

    ASSERT_EQ(utils_sos_embed_u16(carrier.data(), carrier.size(), payload.data(), payload.size(), key.data(), key.size(), C_API_SOS_OPTION_NONE), UTILS_OK) << utils_exception.info;
    const size_t size = utils_sos_extract_u16(carrier.data(), carrier.size(), key.data(), key.size(), out.data(), out.size());
    ASSERT_EQ(size, payload.size());
    EXPECT_TRUE(std::equal(payload.begin(), payload.end(), out.begin()));
    EXPECT_EQ(utils_sos_extract_u16(carrier.data(), carrier.size(), key.data(), key.size(), nullptr, 0), payload.size());
    EXPECT_EQ(utils_sos_embed_u16(carrier.data(), carrier.size(), payload.data(), payload.size(), nullptr, 0, 7), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The options of s.o.s are invalid");
    EXPECT_EQ(utils_sos_embed_u16(nullptr, 3, payload.data(), payload.size(), nullptr, 0, C_API_SOS_OPTION_NONE), UTILS_KO);
}
