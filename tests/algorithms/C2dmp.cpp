/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/10/2026 by @author Tsukini

File Name:
##  @file C2dmp.cpp

File Description:
##  Unit tests of the c2dmp-hsm heuristic string matching algorithms
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>

using utils::algorithms::c2dmp::c2dmp;
using utils::algorithms::c2dmp::c2dmp_optimized;
using utils::algorithms::c2dmp::c2dmp_foptimized;

// Return the closest candidate of the given input (like the cli hint)
static std::string closest(const std::string& input, const std::vector<std::string>& candidates)
{
    return *std::min_element(candidates.begin(), candidates.end(), [&](const std::string& a, const std::string& b) {
        return c2dmp(input, a) < c2dmp(input, b);
    });
}

TEST(C2dmp, IdenticalIsBetterThanDifferent) {
    EXPECT_LT(c2dmp("hello", "hello"), c2dmp("hello", "world"));
    EXPECT_LT(c2dmp("abc", "abc"), c2dmp("abc", "abd"));
}

TEST(C2dmp, OneTypoIsBetterThanUnrelated) {
    EXPECT_LT(c2dmp("hellp", "hello"), c2dmp("hellp", "quit"));
    EXPECT_LT(c2dmp("exot", "exit"), c2dmp("exot", "help"));
}

TEST(C2dmp, CaseInsensitive) {
    EXPECT_FLOAT_EQ(c2dmp("Hello", "hello"), c2dmp("hello", "hello"));
    EXPECT_LT(c2dmp("HELLO", "hello"), c2dmp("hellp", "hello"));
}

TEST(C2dmp, AccentInsensitive) {
    // Latin-1 accents are normalized (é -> e)
    EXPECT_LT(c2dmp("caf\xE9", "cafe"), c2dmp("cafx", "cafe"));
}

TEST(C2dmp, SwappedCharsBetterThanDifferent) {
    EXPECT_LT(c2dmp("hlelo", "hello"), c2dmp("hxyzo", "hello"));
}

TEST(C2dmp, LengthDifference) {
    EXPECT_LT(c2dmp("hell", "hello"), c2dmp("he", "hello"));
}

TEST(C2dmp, EmptyStrings) {
    EXPECT_FLOAT_EQ(c2dmp("", ""), 0.0f);
}

TEST(C2dmp, ClosestCommand) {
    const std::vector<std::string> commands = {"help", "exit", "quit", "bye", "history", "clear"};
    EXPECT_EQ(closest("hepl", commands), "help");
    EXPECT_EQ(closest("exti", commands), "exit");
    EXPECT_EQ(closest("qiut", commands), "quit");
    EXPECT_EQ(closest("histroy", commands), "history");
    EXPECT_EQ(closest("claer", commands), "clear");
}

TEST(C2dmp, RedirectionMatchesImplementations) {
    const std::vector<std::pair<std::string, std::string>> pairs = {
        {"hello", "hello"}, {"hello", "world"}, {"abc", "abcdef"}, {"abcdef", "abc"}, {"Test", "tset"}
    };
    for (const auto& [a, b]: pairs) {
        EXPECT_FLOAT_EQ((c2dmp<3>(a, b)), (c2dmp_optimized<3>(a, b))) << a << " / " << b;
        EXPECT_FLOAT_EQ((c2dmp<3, std::uint_fast8_t, true>(a, b)), (c2dmp_foptimized<3>(a, b))) << a << " / " << b;
    }
}

TEST(C2dmp, FullVersionEqualOnSameLength) {
    // The full version only differ by the misplaced chars of the extra part of 'a'
    EXPECT_FLOAT_EQ(c2dmp_optimized("hello", "hlelo"), c2dmp_foptimized("hello", "hlelo"));
}

TEST(C2dmp, PrefixDepthVariants) {
    for (const auto& [a, b]: std::vector<std::pair<std::string, std::string>>{{"hello", "help"}, {"exit", "exot"}}) {
        EXPECT_LE((c2dmp<1>(a, a)), (c2dmp<1>(a, b)));
        EXPECT_LE((c2dmp<5>(a, a)), (c2dmp<5>(a, b)));
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
TEST(C2dmp, SimplifiedMatchesOptimized) {
    const std::vector<std::pair<std::string, std::string>> pairs = {
        {"hello", "hello"}, {"hello", "world"}, {"abc", "abcdef"}, {"abcdef", "abc"}, {"Test", "tset"}, {"exit", "exot"}
    };
    for (const auto& [a, b]: pairs) {
        EXPECT_FLOAT_EQ(utils::algorithms::c2dmp::c2dmp_simplified(a, b), c2dmp_optimized(a, b)) << a << " / " << b;
        EXPECT_FLOAT_EQ(utils::algorithms::c2dmp::c2dmp_fsimplified(a, b), c2dmp_foptimized(a, b)) << a << " / " << b;
    }
}
#pragma clang diagnostic pop
