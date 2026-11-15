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
##  Unit tests of the C API of utils::iomanip (sequences as strings, _r, reports errors)
\**************************************************************/

#include "utils/manip/iomanip/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <string>

class IomanipCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

//----------------------------------------------------------------//
/* SEQUENCES */

TEST_F(IomanipCApiTest, StaticSequencesAreTheCppOnes)
{
    EXPECT_EQ(std::string(utils_iomanip_reset()), utils::iomanip::reset());
    EXPECT_EQ(std::string(utils_iomanip_strong()), utils::iomanip::strong());
    EXPECT_EQ(std::string(utils_iomanip_esc()), utils::iomanip::esc());
    EXPECT_EQ(std::string(utils_iomanip_screen()), utils::iomanip::screen());
    EXPECT_EQ(std::string(utils_iomanip_hide_cur()), utils::iomanip::hide_cur());
    EXPECT_EQ(std::string(utils_iomanip_report_past_disable()), utils::iomanip::report_past_disable());
    EXPECT_EQ(utils_iomanip_reset(), utils_iomanip_reset()); // same static string
    EXPECT_FALSE(utils_exception.exception);
}

TEST_F(IomanipCApiTest, SequencesWithArguments)
{
    EXPECT_EQ(std::string(utils_iomanip_color(C_API_IOMANIP_COLOR_RED)), utils::iomanip::color(utils::iomanip::Color::Red));
    EXPECT_EQ(std::string(utils_iomanip_back_color(C_API_IOMANIP_BACK_COLOR_BRIGHT_BLUE)), utils::iomanip::color(utils::iomanip::BackColor::BrightBlue));
    EXPECT_EQ(std::string(utils_iomanip_color_rgb(1, 2, 3)), utils::iomanip::color_rgb(1, 2, 3));
    EXPECT_EQ(std::string(utils_iomanip_underline_color_id(200)), utils::iomanip::underline_color_id(200));
    EXPECT_EQ(std::string(utils_iomanip_pos(4, 5)), utils::iomanip::pos(4, 5));
    EXPECT_EQ(std::string(utils_iomanip_up(3)), utils::iomanip::up(3));
    EXPECT_EQ(std::string(utils_iomanip_set_style(C_API_IOMANIP_STYLE_ITALIC)), utils::iomanip::set_style(utils::iomanip::Style::Italic));
    EXPECT_EQ(std::string(utils_iomanip_reset_style(C_API_IOMANIP_RESET_STYLE_BAR)), utils::iomanip::reset_style(utils::iomanip::ResetStyle::Bar));
    EXPECT_EQ(std::string(utils_iomanip_hyperlink("site", "https://x")), utils::iomanip::hyperlink("site", "https://x"));
    EXPECT_EQ(std::string(utils_iomanip_csi("2J")), utils::iomanip::screen());
}

TEST_F(IomanipCApiTest, SeveralStylesInOneSequence)
{
    const utils_iomanip_style_t styles[] = {C_API_IOMANIP_STYLE_STRONG, C_API_IOMANIP_STYLE_UNDERLINED};
    const utils_iomanip_reset_style_t resets[] = {C_API_IOMANIP_RESET_STYLE_STRONG, C_API_IOMANIP_RESET_STYLE_UNDERLINED};

    EXPECT_EQ(std::string(utils_iomanip_set_styles(styles, 2)), utils::iomanip::set_style({utils::iomanip::Style::Strong, utils::iomanip::Style::Underlined}));
    EXPECT_EQ(std::string(utils_iomanip_reset_styles(resets, 2)), utils::iomanip::reset_style({utils::iomanip::ResetStyle::Strong, utils::iomanip::ResetStyle::Underlined}));
    EXPECT_EQ(std::string(utils_iomanip_set_styles(nullptr, 0)), utils::iomanip::set_style({}));
    EXPECT_EQ(utils_iomanip_set_styles(nullptr, 2), nullptr);
    EXPECT_STREQ(utils_exception.info, "The styles is NULL");
}

TEST_F(IomanipCApiTest, BufferOfTheUser)
{
    const std::string expected = utils::iomanip::color_rgb(10, 20, 30);
    char buf[64] = {};
    char small[4] = {};

    EXPECT_EQ(utils_iomanip_color_rgb_r(10, 20, 30, buf, sizeof(buf)), expected.size());
    EXPECT_EQ(std::string(buf), expected);
    EXPECT_EQ(utils_iomanip_color_rgb_r(10, 20, 30, small, sizeof(small)), expected.size());
    EXPECT_EQ(std::strlen(small), sizeof(small) - 1);
    EXPECT_EQ(utils_iomanip_pos_r(1, 1, nullptr, 0), utils::iomanip::pos(1, 1).size());
}

TEST_F(IomanipCApiTest, NullArgumentsAreReported)
{
    EXPECT_EQ(utils_iomanip_csi(nullptr), nullptr);
    EXPECT_TRUE(utils_exception.exception);
    EXPECT_STREQ(utils_exception.info, "The code is NULL");
    EXPECT_EQ(utils_iomanip_hyperlink_r("x", nullptr, nullptr, 0), 0u);
    EXPECT_STREQ(utils_exception.info, "The link is NULL");
}

//----------------------------------------------------------------//
/* REPORTS */

TEST_F(IomanipCApiTest, ReportsCheckTheirOutputFirst)
{
    int row = 0;

    EXPECT_EQ(utils_iomanip_read_cursor_position(&row, nullptr), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The col is NULL");
    EXPECT_EQ(utils_iomanip_read_mouse_event(nullptr), UTILS_KO);
    EXPECT_EQ(utils_iomanip_read_advanced_mouse_event(nullptr), UTILS_KO);
    EXPECT_STREQ(utils_exception.info, "The event is NULL");
}
