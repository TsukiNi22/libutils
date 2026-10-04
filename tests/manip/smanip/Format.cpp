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
##  @file Format.cpp

File Description:
##  Unit tests of the string format function (<style> sequences) & FixedString
\**************************************************************/

#include "utils.hpp"
#include "utils/manip/smanip/FixedString.hpp"
#include <gtest/gtest.h>
#include <string>

using utils::smanip::format;

TEST(Format, PlainText) {
    EXPECT_EQ(format("hello world"), "hello world");
    EXPECT_EQ(format(""), "");
}

TEST(Format, SingleStyle) {
    EXPECT_EQ(format("<strong>txt"), utils::iomanip::strong() + "txt");
}

TEST(Format, Reset) {
    EXPECT_EQ(format("<strong>txt<>"), utils::iomanip::strong() + "txt" + utils::iomanip::reset());
}

TEST(Format, MultipleStyles) {
    EXPECT_EQ(format("<strong|italic|underlined>x"),
        utils::iomanip::strong() + utils::iomanip::italic() + utils::iomanip::underlined() + "x");
}

TEST(Format, CaseInsensitive) {
    EXPECT_EQ(format("<STRONG|Italic>x"), utils::iomanip::strong() + utils::iomanip::italic() + "x");
}

TEST(Format, UnknownStyleIgnored) {
    EXPECT_EQ(format("<unknown>x"), "x");
    EXPECT_EQ(format("<unknown|dark>x"), utils::iomanip::dark() + "x");
}

TEST(Format, EscapedDelimitor) {
    EXPECT_EQ(format("\\<strong\\>"), "<strong>");
    EXPECT_EQ(format("a \\< b"), "a < b");
}

TEST(Format, UnclosedSequence) {
    EXPECT_EQ(format("a < b"), "a < b");
}

TEST(Format, AllStyles) {
    EXPECT_EQ(format("<dark>"), utils::iomanip::dark());
    EXPECT_EQ(format("<flashing_fast>"), utils::iomanip::flashing_fast());
    EXPECT_EQ(format("<flashing_slow>"), utils::iomanip::flashing_slow());
    EXPECT_EQ(format("<reversed>"), utils::iomanip::reversed());
    EXPECT_EQ(format("<hide>"), utils::iomanip::hide());
    EXPECT_EQ(format("<bar>"), utils::iomanip::bar());
    EXPECT_EQ(format("<monospace>"), utils::iomanip::monospace());
    EXPECT_EQ(format("<framed>"), utils::iomanip::framed());
    EXPECT_EQ(format("<encircled>"), utils::iomanip::encircled());
    EXPECT_EQ(format("<overlined>"), utils::iomanip::overlined());
    EXPECT_EQ(format("<exposant>"), utils::iomanip::exposant());
    EXPECT_EQ(format("<indice>"), utils::iomanip::indice());
}

TEST(Format, ResetKeyword) {
    // 'reset' is listed in the documentation of the format function
    EXPECT_EQ(format("<reset>"), utils::iomanip::reset());
}

/* FixedString */
template<utils::smanip::FixedString S>
static constexpr std::string_view viewOf(void) {return S.view();}

TEST(FixedString, ViewAndSize) {
    constexpr utils::smanip::FixedString s("hello");
    EXPECT_EQ(s.size(), 5u);
    EXPECT_EQ(s.view(), "hello");
    EXPECT_EQ(viewOf<"template">(), "template");
}
