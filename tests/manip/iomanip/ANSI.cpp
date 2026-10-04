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
    EXPECT_EQ(utils::iomanip::set_style({utils::iomanip::Style::Strong, utils::iomanip::Style::Italic}), ESC "[1;3m");
    EXPECT_EQ(utils::iomanip::set_style(utils::iomanip::Style::Underlined), ESC "[4m");
}

TEST(ANSI, ResetStyleList) {
    EXPECT_EQ(utils::iomanip::reset_style({}), ESC "[0m");
    EXPECT_EQ(utils::iomanip::reset_style({utils::iomanip::ResetStyle::Strong, utils::iomanip::ResetStyle::Overlined}), ESC "[22;55m");
    EXPECT_EQ(utils::iomanip::reset_style(utils::iomanip::ResetStyle::Italic), ESC "[23m");
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
    std::pair<int, int> pos = utils::iomanip::read_cursor_position();
    EXPECT_EQ(pos.first, 12);
    EXPECT_EQ(pos.second, 34);
}

TEST(ANSIReports, ReadCursorPositionInvalid) {
    StdinRedirect redirect("garbage");
    std::pair<int, int> pos = utils::iomanip::read_cursor_position();
    EXPECT_EQ(pos.first, -1);
    EXPECT_EQ(pos.second, -1);
}

// Classic report: ESC [ M Cb Cx Cy (raw bytes, value + 32)
static std::string classicMouse(int b, int x, int y)
{
    return std::string(ESC "[M") + static_cast<char>(b + 32) + static_cast<char>(x + 32) + static_cast<char>(y + 32);
}

TEST(ANSIReports, ReadMouseEvent) {
    CinRedirect redirect(classicMouse(0, 10, 20));
    utils::iomanip::MouseEvent event = utils::iomanip::read_mouse_event();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Left);
    EXPECT_EQ(event.x, 10u);
    EXPECT_EQ(event.y, 20u);
}

TEST(ANSIReports, ReadMouseEventRelease) {
    CinRedirect redirect("noise" + classicMouse(3, 1, 2));
    utils::iomanip::MouseEvent event = utils::iomanip::read_mouse_event();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Release);
}

TEST(ANSIReports, ReadMouseEventInvalid) {
    CinRedirect redirect(ESC "[X0;1;2\n");
    try {
        (void)utils::iomanip::read_mouse_event();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ANSIMouseEvent);
    }
}

TEST(ANSIReports, ReadMouseEventHighCoordinates) {
    CinRedirect redirect(classicMouse(2, 150, 200)); // bytes >= 0x80
    utils::iomanip::MouseEvent event = utils::iomanip::read_mouse_event();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Right);
    EXPECT_EQ(event.x, 150u);
    EXPECT_EQ(event.y, 200u);
}

TEST(ANSIReports, ReadMouseEventTooShort) {
    CinRedirect redirect(ESC "[M!");
    try {
        (void)utils::iomanip::read_mouse_event();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ANSIMouseEvent);
    }
}

TEST(ANSIReports, ReadAdvancedMouseEventPressed) {
    CinRedirect redirect(ESC "[<2;5;6M");
    utils::iomanip::AdvancedMouseEvent event = utils::iomanip::read_advanced_mouse_event();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Right);
    EXPECT_EQ(event.x, 5u);
    EXPECT_EQ(event.y, 6u);
    EXPECT_TRUE(event.pressed);
}

TEST(ANSIReports, ReadAdvancedMouseEventReleased) {
    CinRedirect redirect(ESC "[<1;100;200m");
    utils::iomanip::AdvancedMouseEvent event = utils::iomanip::read_advanced_mouse_event();
    EXPECT_EQ(event.button, utils::iomanip::MouseButton::Middle);
    EXPECT_EQ(event.x, 100u);
    EXPECT_EQ(event.y, 200u);
    EXPECT_FALSE(event.pressed);
}

TEST(ANSIReports, ReadAdvancedMouseEventEmpty) {
    CinRedirect redirect("");
    try {
        (void)utils::iomanip::read_advanced_mouse_event();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::ANSIMouseEvent);
    }
}

/* Sequences (ECMA-48 / xterm codes) */
TEST(ANSI, StyleSequences) {
    EXPECT_EQ(utils::iomanip::dark(), ESC "[2m");
    EXPECT_EQ(utils::iomanip::underlined(), ESC "[4m");
    EXPECT_EQ(utils::iomanip::flashing_slow(), ESC "[5m");
    EXPECT_EQ(utils::iomanip::flashing_fast(), ESC "[6m");
    EXPECT_EQ(utils::iomanip::reversed(), ESC "[7m");
    EXPECT_EQ(utils::iomanip::hide(), ESC "[8m");
    EXPECT_EQ(utils::iomanip::bar(), ESC "[9m");
    EXPECT_EQ(utils::iomanip::monospace(), ESC "[50m");
    EXPECT_EQ(utils::iomanip::framed(), ESC "[51m");
    EXPECT_EQ(utils::iomanip::encircled(), ESC "[52m");
    EXPECT_EQ(utils::iomanip::overlined(), ESC "[53m");
    EXPECT_EQ(utils::iomanip::exposant(), ESC "[73m");
    EXPECT_EQ(utils::iomanip::indice(), ESC "[74m");
    EXPECT_EQ(utils::iomanip::underline_color_id(196), ESC "[58;5;196m");
}

TEST(ANSI, StyleResetSequences) {
    EXPECT_EQ(utils::iomanip::strong_reset(), ESC "[22m"); // normal intensity
    EXPECT_EQ(utils::iomanip::dark_reset(), ESC "[22m");
    EXPECT_EQ(utils::iomanip::italic_reset(), ESC "[23m");
    EXPECT_EQ(utils::iomanip::flashing_fast_reset(), ESC "[25m");
    EXPECT_EQ(utils::iomanip::flashing_slow_reset(), ESC "[25m");
    EXPECT_EQ(utils::iomanip::reversed_reset(), ESC "[27m");
    EXPECT_EQ(utils::iomanip::hide_reset(), ESC "[28m");
    EXPECT_EQ(utils::iomanip::bar_reset(), ESC "[29m");
    EXPECT_EQ(utils::iomanip::framed_encircled_reset(), ESC "[54m");
    EXPECT_EQ(utils::iomanip::overlined_reset(), ESC "[55m");
    EXPECT_EQ(utils::iomanip::underline_color_reset(), ESC "[59m");
    EXPECT_EQ(utils::iomanip::exposant_indice_reset(), ESC "[75m");
}

TEST(ANSI, StyleEnumMatchSequences) {
    EXPECT_EQ(utils::iomanip::set_style(utils::iomanip::Style::FlashingSlow), utils::iomanip::flashing_slow());
    EXPECT_EQ(utils::iomanip::set_style(utils::iomanip::Style::FlashingFast), utils::iomanip::flashing_fast());
    EXPECT_EQ(utils::iomanip::reset_style(utils::iomanip::ResetStyle::Strong), utils::iomanip::strong_reset());
    EXPECT_EQ(utils::iomanip::reset_style(utils::iomanip::ResetStyle::FlashingSlow), utils::iomanip::flashing_slow_reset());
    EXPECT_EQ(utils::iomanip::reset_style(utils::iomanip::ResetStyle::Reversed), utils::iomanip::reversed_reset());
}

TEST(ANSI, ScreenSequences) {
    EXPECT_EQ(utils::iomanip::next_line(2), ESC "[2E");
    EXPECT_EQ(utils::iomanip::previous_line(3), ESC "[3F");
    EXPECT_EQ(utils::iomanip::scroll_up(4), ESC "[4S");
    EXPECT_EQ(utils::iomanip::scroll_down(5), ESC "[5T");
    EXPECT_EQ(utils::iomanip::line_start(), ESC "[1K");
    EXPECT_EQ(utils::iomanip::screen_end(), ESC "[0J");
    EXPECT_EQ(utils::iomanip::screen_start(), ESC "[1J");
    EXPECT_EQ(utils::iomanip::scrollback_buffer(), ESC "[3J");
    EXPECT_EQ(utils::iomanip::load_screen(), ESC "[?1049l");
    EXPECT_EQ(utils::iomanip::get_pos(), ESC "[6n");
}

TEST(ANSI, ModeSequences) {
    EXPECT_EQ(utils::iomanip::wrapping_enable(), ESC "[?7h");
    EXPECT_EQ(utils::iomanip::wrapping_disable(), ESC "[?7l");
    EXPECT_EQ(utils::iomanip::inverted_color_enable(), ESC "[?5h");
    EXPECT_EQ(utils::iomanip::inverted_color_disable(), ESC "[?5l");
    EXPECT_EQ(utils::iomanip::mouse_tracking_enable(), ESC "[?1000h");
    EXPECT_EQ(utils::iomanip::mouse_tracking_disable(), ESC "[?1000l");
    EXPECT_EQ(utils::iomanip::mouse_move_tracking_enable(), ESC "[?1002h");
    EXPECT_EQ(utils::iomanip::mouse_move_tracking_disable(), ESC "[?1002l");
    EXPECT_EQ(utils::iomanip::mouse_adv_tracking_disable(), ESC "[?1006l");
    EXPECT_EQ(utils::iomanip::report_focus_enable(), ESC "[?1004h");
    EXPECT_EQ(utils::iomanip::report_focus_disabled(), ESC "[?1004l");
    EXPECT_EQ(utils::iomanip::report_past_enable(), ESC "[?2004h");
    EXPECT_EQ(utils::iomanip::report_past_disable(), ESC "[?2004l");
}
