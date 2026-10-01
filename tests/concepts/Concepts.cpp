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
##  @file Concepts.cpp

File Description:
##  Unit tests of the global & operation concepts
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace {
    struct NoOp {};
    struct OnlyAdd {
        OnlyAdd operator+(const OnlyAdd&) const {return {};}
    };
}

TEST(GlobalConcepts, Convertible) {
    EXPECT_TRUE((utils::concepts::Convertible<int, double>));
    EXPECT_TRUE((utils::concepts::Convertible<const char*, std::string>));
    EXPECT_FALSE((utils::concepts::Convertible<std::string, int>));
    EXPECT_TRUE((utils::concepts::convertible_to<int, long>));
    EXPECT_FALSE((utils::concepts::convertible_to<NoOp, int>));
}

TEST(GlobalConcepts, SwappableStreamable) {
    EXPECT_TRUE(utils::concepts::Swappable<int>);
    EXPECT_TRUE(utils::concepts::Streamable<int>);
    EXPECT_TRUE(utils::concepts::Streamable<std::string>);
    EXPECT_FALSE(utils::concepts::Streamable<NoOp>);
}

TEST(OperationConcepts, Arithmetic) {
    EXPECT_TRUE(utils::concepts::Arithmetic<int>);
    EXPECT_TRUE(utils::concepts::Arithmetic<double>);
    EXPECT_FALSE(utils::concepts::Arithmetic<std::string>);
    EXPECT_TRUE(utils::concepts::Addable<std::string>);
    EXPECT_TRUE(utils::concepts::Addable<OnlyAdd>);
    EXPECT_FALSE(utils::concepts::Subtractable<OnlyAdd>);
    EXPECT_FALSE(utils::concepts::Addable<NoOp>);
    EXPECT_TRUE((utils::concepts::ArithmeticWith<int, double>));
    EXPECT_TRUE((utils::concepts::AddableWith<std::string, const char*>));
    EXPECT_FALSE((utils::concepts::MultipliableWith<std::string, int>));
}

TEST(OperationConcepts, IncrementDecrement) {
    EXPECT_TRUE(utils::concepts::Incrementable<int>);
    EXPECT_TRUE(utils::concepts::Decrementable<int>);
    EXPECT_FALSE(utils::concepts::Incrementable<std::string>);
}

TEST(OperationConcepts, Bitwise) {
    EXPECT_TRUE(utils::concepts::BitwiseAndable<int>);
    EXPECT_TRUE(utils::concepts::BitwiseOrable<unsigned>);
    EXPECT_TRUE(utils::concepts::BitwiseXorable<long>);
    EXPECT_TRUE(utils::concepts::Shiftable<int>);
    EXPECT_FALSE(utils::concepts::BitwiseAndable<double>);
    EXPECT_FALSE(utils::concepts::Shiftable<double>);
    EXPECT_TRUE((utils::concepts::ShiftableWith<int, unsigned>));
}

TEST(OperationConcepts, Assignment) {
    EXPECT_TRUE(utils::concepts::AddAssignable<int>);
    EXPECT_TRUE(utils::concepts::DivideAssignable<double>);
    EXPECT_TRUE((utils::concepts::MultiplyAssignableWith<double, int>));
    EXPECT_FALSE(utils::concepts::SubtractAssignable<std::string>);
}

TEST(OperationConcepts, Comparison) {
    EXPECT_TRUE(utils::concepts::EqualityComparable<int>);
    EXPECT_TRUE(utils::concepts::Comparable<std::string>);
    EXPECT_FALSE(utils::concepts::Comparable<NoOp>);
    EXPECT_TRUE((utils::concepts::ComparableWith<int, double>));
    EXPECT_TRUE((utils::concepts::EqualityComparableWith<std::string, const char*>));
}

TEST(OperationConcepts, Unary) {
    EXPECT_TRUE(utils::concepts::Negatable<int>);
    EXPECT_FALSE(utils::concepts::Negatable<std::string>);
}
