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
##  @file ANSI.cpp

File Description:
##  Unit tests of the ANSI escape sequences & the terminal reports parsing
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>

#define ESC "\x1b"

/* Sequences */
TEST(ANSI, Basics) {
    EXPECT_EQ(utils::iomanip::esc(), ESC);
    EXPECT_EQ(utils::iomanip::csi("0m"), ESC "[0m");
    EXPECT_EQ(utils::iomanip::reset(), ESC "[0m");
    EXPECT_EQ(utils::iomanip::strong(), ESC "[1m");
    EXPECT_EQ(utils::iomanip::italic(), ESC "[3m");
    EXPECT_EQ(utils::iomanip::underlined_reset(), ESC "[24m");
}

TEST(ANSI, Colors) {
    EXPECT_EQ(utils::iomanip::color(utils::iomanip::Color::Red), ESC "[31m");
    EXPECT_EQ(utils::iomanip::color(utils::iomanip::Color::BrightCyan), ESC "[96m");
    EXPECT_EQ(utils::iomanip::color(utils::iomanip::BackColor::Blue), ESC "[44m");
    EXPECT_EQ(utils::iomanip::color_id(208), ESC "[38;5;208m");
    EXPECT_EQ(utils::iomanip::back_color_id(1), ESC "[48;5;1m");
    EXPECT_EQ(utils::iomanip::color_rgb(1, 2, 3), ESC "[38;2;1;2;3m");
    EXPECT_EQ(utils::iomanip::back_color_rgb(255, 0, 10), ESC "[48;2;255;0;10m");
    EXPECT_EQ(utils::iomanip::underline_color_rgb(0, 0, 0), ESC "[58;2;0;0;0m");
}

TEST(ANSI, Cursor) {
    EXPECT_EQ(utils::iomanip::up(3), ESC "[3A");
    EXPECT_EQ(utils::iomanip::down(1), ESC "[1B");
    EXPECT_EQ(utils::iomanip::right(12), ESC "[12C");
    EXPECT_EQ(utils::iomanip::left(2), ESC "[2D");
    EXPECT_EQ(utils::iomanip::pos(4, 7), ESC "[4;7H");
    EXPECT_EQ(utils::iomanip::column(9), ESC "[9G");
    EXPECT_EQ(utils::iomanip::save_cur(), ESC "[s");
    EXPECT_EQ(utils::iomanip::load_cur(), ESC "[u");
}

TEST(ANSI, Erase) {
    EXPECT_EQ(utils::iomanip::screen(), ESC "[2J");
    EXPECT_EQ(utils::iomanip::line(), ESC "[2K");
    EXPECT_EQ(utils::iomanip::line_end(), ESC "[0K");
}

TEST(ANSI, PrivateModes) {
    EXPECT_EQ(utils::iomanip::hide_cur(), ESC "[?25l");
    EXPECT_EQ(utils::iomanip::show_cur(), ESC "[?25h");
    EXPECT_EQ(utils::iomanip::save_screen(), ESC "[?1049h");
    EXPECT_EQ(utils::iomanip::mouse_adv_tracking_enable(), ESC "[?1006h");
}

TEST(ANSI, Hyperlinks) {
    EXPECT_EQ(utils::iomanip::hyperlink("txt", "https://x.y"), ESC "]8;;https://x.y" ESC "\\txt" ESC "]8;;" ESC "\\");
    EXPECT_EQ(utils::iomanip::file_hyperlink("f", "/tmp/f"), ESC "]8;;file:///tmp/f" ESC "\\f" ESC "]8;;" ESC "\\");
}

TEST(ANSI, SetStyleList) {
    EXPECT_EQ(utils::iomanip::setStyle({utils::iomanip::Style::Strong, utils::iomanip::Style::Italic}), ESC "[1;3m");
    EXPECT_EQ(utils::iomanip::setStyle(utils::iomanip::Style::Underlined), ESC "[4m");
}

TEST(ANSI, ResetStyleList) {
    EXPECT_EQ(utils::iomanip::resetStyle({}), ESC "[0m");
    EXPECT_EQ(utils::iomanip::resetStyle({utils::iomanip::ResetStyle::Strong, utils::iomanip::ResetStyle::Overlined}), ESC "[21;55m");
    EXPECT_EQ(utils::iomanip::resetStyle(utils::iomanip::ResetStyle::Italic), ESC "[23m");
}

/* Reports */
// Redirect std::cin on a string for the duration of the scope
class CinRedirect {
    private:
        std::istringstream _iss;
        std::streambuf* _old;
    public:
        CinRedirect(const std::string& s): _iss{s}, _old{std::cin.rdbuf(this->_iss.rdbuf())} {};
        ~CinRedirect() {std::cin.rdbuf(this->_old); std::cin.clear();};
};

// Redirect the fd 0 on a pipe filled with a string for the duration of the scope
class StdinRedirect {
    private:
        int _saved = -1;
    public:
        StdinRedirect(const std::string& s)
        {
            int fds[2];
            if (::pipe(fds) == -1) throw std::runtime_error("pipe");
            (void)!::write(fds[1], s.data(), s.size());
            ::close(fds[1]);
            this->_saved = ::dup(STDIN_FILENO);
            ::dup2(fds[0], STDIN_FILENO);
            ::close(fds[0]);
        };
        ~StdinRedirect() {::dup2(this->_saved, STDIN_FILENO); ::close(this->_saved);};
};

TEST(ANSIReports, ReadCursorPosition) {
    StdinRedirect redirect(ESC "[12;34R");
    std::pair<int, int> pos = utils::iomanip::readCursorPosition();
    EXPECT_EQ(pos.first, 12);
    EXPECT_EQ(pos.second, 34);
}

TEST(ANSIReports, ReadCursorPositionInvalid) {
    StdinRedirect redirect("garbage");
    std::pair<int, int> pos = utils::iomanip::readCursorPosition();
    EXPECT_EQ(pos.first, -1);
    EXPECT_EQ(pos.second, -1);
}

TEST(ANSIReports, ReadMouseEvent) {
    CinRedirect redirect(ESC "[M0;10;20\n");
    utils::iomanip::MouseEvent event = utils::iomanip::readMouseEvent();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Left);
    EXPECT_EQ(event.x, 10u);
    EXPECT_EQ(event.y, 20u);
}

TEST(ANSIReports, ReadMouseEventRelease) {
    CinRedirect redirect(ESC "[M3;1;2\n");
    utils::iomanip::MouseEvent event = utils::iomanip::readMouseEvent();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Release);
}

TEST(ANSIReports, ReadMouseEventInvalid) {
    CinRedirect redirect(ESC "[X0;1;2\n");
    try {
        (void)utils::iomanip::readMouseEvent();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ANSIMouseEvent);
    }
}

TEST(ANSIReports, ReadMouseEventTooShort) {
    CinRedirect redirect(ESC "[M\n");
    try {
        (void)utils::iomanip::readMouseEvent();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ANSIMouseEvent);
    }
}

TEST(ANSIReports, ReadAdvancedMouseEventPressed) {
    CinRedirect redirect(ESC "[<2;5;6M");
    utils::iomanip::AdvancedMouseEvent event = utils::iomanip::readAdvancedMouseEvent();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Right);
    EXPECT_EQ(event.x, 5u);
    EXPECT_EQ(event.y, 6u);
    EXPECT_TRUE(event.pressed);
}

TEST(ANSIReports, ReadAdvancedMouseEventReleased) {
    CinRedirect redirect(ESC "[<1;100;200m");
    utils::iomanip::AdvancedMouseEvent event = utils::iomanip::readAdvancedMouseEvent();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Middle);
    EXPECT_EQ(event.x, 100u);
    EXPECT_EQ(event.y, 200u);
    EXPECT_FALSE(event.pressed);
}

TEST(ANSIReports, ReadAdvancedMouseEventEmpty) {
    CinRedirect redirect("");
    try {
        (void)utils::iomanip::readAdvancedMouseEvent();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ANSIMouseEvent);
    }
}
