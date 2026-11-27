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
##  C API of utils::iomanip
\**************************************************************/

#include "utils/manip/iomanip/Color.hpp"
#include "utils/manip/iomanip/Style.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/manip/iomanip/ANSI.hpp"
#include "utils/manip/iomanip/Char.hpp"
#include "utils/manip/iomanip/CApi.hpp"
#include "utils/manip/smanip/CApi.hpp"
#include "utils/manip/iomanip/CApi.h"
#include "utils/exception/CApi.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string>

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ enums must stay the same */
static_assert(C_API_IOMANIP_CHAR_NUL == static_cast<char>(utils::iomanip::Char::NUL));
static_assert(C_API_IOMANIP_CHAR_SOH == static_cast<char>(utils::iomanip::Char::SOH));
static_assert(C_API_IOMANIP_CHAR_STX == static_cast<char>(utils::iomanip::Char::STX));
static_assert(C_API_IOMANIP_CHAR_ETX == static_cast<char>(utils::iomanip::Char::ETX));
static_assert(C_API_IOMANIP_CHAR_EOT == static_cast<char>(utils::iomanip::Char::EOT));
static_assert(C_API_IOMANIP_CHAR_ENQ == static_cast<char>(utils::iomanip::Char::ENQ));
static_assert(C_API_IOMANIP_CHAR_ACK == static_cast<char>(utils::iomanip::Char::ACK));
static_assert(C_API_IOMANIP_CHAR_BEL == static_cast<char>(utils::iomanip::Char::BEL));
static_assert(C_API_IOMANIP_CHAR_BS  == static_cast<char>(utils::iomanip::Char::BS));
static_assert(C_API_IOMANIP_CHAR_HT  == static_cast<char>(utils::iomanip::Char::HT));
static_assert(C_API_IOMANIP_CHAR_LF  == static_cast<char>(utils::iomanip::Char::LF));
static_assert(C_API_IOMANIP_CHAR_VT  == static_cast<char>(utils::iomanip::Char::VT));
static_assert(C_API_IOMANIP_CHAR_FF  == static_cast<char>(utils::iomanip::Char::FF));
static_assert(C_API_IOMANIP_CHAR_CR  == static_cast<char>(utils::iomanip::Char::CR));
static_assert(C_API_IOMANIP_CHAR_SO  == static_cast<char>(utils::iomanip::Char::SO));
static_assert(C_API_IOMANIP_CHAR_SI  == static_cast<char>(utils::iomanip::Char::SI));
static_assert(C_API_IOMANIP_CHAR_DLE == static_cast<char>(utils::iomanip::Char::DLE));
static_assert(C_API_IOMANIP_CHAR_DC1 == static_cast<char>(utils::iomanip::Char::DC1));
static_assert(C_API_IOMANIP_CHAR_DC2 == static_cast<char>(utils::iomanip::Char::DC2));
static_assert(C_API_IOMANIP_CHAR_DC3 == static_cast<char>(utils::iomanip::Char::DC3));
static_assert(C_API_IOMANIP_CHAR_DC4 == static_cast<char>(utils::iomanip::Char::DC4));
static_assert(C_API_IOMANIP_CHAR_NAK == static_cast<char>(utils::iomanip::Char::NAK));
static_assert(C_API_IOMANIP_CHAR_SYN == static_cast<char>(utils::iomanip::Char::SYN));
static_assert(C_API_IOMANIP_CHAR_ETB == static_cast<char>(utils::iomanip::Char::ETB));
static_assert(C_API_IOMANIP_CHAR_CAN == static_cast<char>(utils::iomanip::Char::CAN));
static_assert(C_API_IOMANIP_CHAR_EM  == static_cast<char>(utils::iomanip::Char::EM));
static_assert(C_API_IOMANIP_CHAR_SUB == static_cast<char>(utils::iomanip::Char::SUB));
static_assert(C_API_IOMANIP_CHAR_ESC == static_cast<char>(utils::iomanip::Char::ESC));
static_assert(C_API_IOMANIP_CHAR_FS  == static_cast<char>(utils::iomanip::Char::FS));
static_assert(C_API_IOMANIP_CHAR_GS  == static_cast<char>(utils::iomanip::Char::GS));
static_assert(C_API_IOMANIP_CHAR_RS  == static_cast<char>(utils::iomanip::Char::RS));
static_assert(C_API_IOMANIP_CHAR_US  == static_cast<char>(utils::iomanip::Char::US));
static_assert(C_API_IOMANIP_CHAR_DEL == static_cast<char>(utils::iomanip::Char::DEL));

static_assert(C_API_IOMANIP_COLOR_BLACK          == static_cast<std::uint8_t>(utils::iomanip::Color::Black));
static_assert(C_API_IOMANIP_COLOR_RED            == static_cast<std::uint8_t>(utils::iomanip::Color::Red));
static_assert(C_API_IOMANIP_COLOR_GREEN          == static_cast<std::uint8_t>(utils::iomanip::Color::Green));
static_assert(C_API_IOMANIP_COLOR_YELLOW         == static_cast<std::uint8_t>(utils::iomanip::Color::Yellow));
static_assert(C_API_IOMANIP_COLOR_BLUE           == static_cast<std::uint8_t>(utils::iomanip::Color::Blue));
static_assert(C_API_IOMANIP_COLOR_MAGENTA        == static_cast<std::uint8_t>(utils::iomanip::Color::Magenta));
static_assert(C_API_IOMANIP_COLOR_CYAN           == static_cast<std::uint8_t>(utils::iomanip::Color::Cyan));
static_assert(C_API_IOMANIP_COLOR_WHITE          == static_cast<std::uint8_t>(utils::iomanip::Color::White));
static_assert(C_API_IOMANIP_COLOR_BRIGHT_BLACK   == static_cast<std::uint8_t>(utils::iomanip::Color::BrightBlack));
static_assert(C_API_IOMANIP_COLOR_BRIGHT_RED     == static_cast<std::uint8_t>(utils::iomanip::Color::BrightRed));
static_assert(C_API_IOMANIP_COLOR_BRIGHT_GREEN   == static_cast<std::uint8_t>(utils::iomanip::Color::BrightGreen));
static_assert(C_API_IOMANIP_COLOR_BRIGHT_YELLOW  == static_cast<std::uint8_t>(utils::iomanip::Color::BrightYellow));
static_assert(C_API_IOMANIP_COLOR_BRIGHT_BLUE    == static_cast<std::uint8_t>(utils::iomanip::Color::BrightBlue));
static_assert(C_API_IOMANIP_COLOR_BRIGHT_MAGENTA == static_cast<std::uint8_t>(utils::iomanip::Color::BrightMagenta));
static_assert(C_API_IOMANIP_COLOR_BRIGHT_CYAN    == static_cast<std::uint8_t>(utils::iomanip::Color::BrightCyan));
static_assert(C_API_IOMANIP_COLOR_BRIGHT_WHITE   == static_cast<std::uint8_t>(utils::iomanip::Color::BrightWhite));
static_assert(C_API_IOMANIP_COLOR_DEFAULT        == static_cast<std::uint8_t>(utils::iomanip::Color::Default));

static_assert(C_API_IOMANIP_BACK_COLOR_BLACK          == static_cast<std::uint8_t>(utils::iomanip::BackColor::Black));
static_assert(C_API_IOMANIP_BACK_COLOR_RED            == static_cast<std::uint8_t>(utils::iomanip::BackColor::Red));
static_assert(C_API_IOMANIP_BACK_COLOR_GREEN          == static_cast<std::uint8_t>(utils::iomanip::BackColor::Green));
static_assert(C_API_IOMANIP_BACK_COLOR_YELLOW         == static_cast<std::uint8_t>(utils::iomanip::BackColor::Yellow));
static_assert(C_API_IOMANIP_BACK_COLOR_BLUE           == static_cast<std::uint8_t>(utils::iomanip::BackColor::Blue));
static_assert(C_API_IOMANIP_BACK_COLOR_MAGENTA        == static_cast<std::uint8_t>(utils::iomanip::BackColor::Magenta));
static_assert(C_API_IOMANIP_BACK_COLOR_CYAN           == static_cast<std::uint8_t>(utils::iomanip::BackColor::Cyan));
static_assert(C_API_IOMANIP_BACK_COLOR_WHITE          == static_cast<std::uint8_t>(utils::iomanip::BackColor::White));
static_assert(C_API_IOMANIP_BACK_COLOR_BRIGHT_BLACK   == static_cast<std::uint8_t>(utils::iomanip::BackColor::BrightBlack));
static_assert(C_API_IOMANIP_BACK_COLOR_BRIGHT_RED     == static_cast<std::uint8_t>(utils::iomanip::BackColor::BrightRed));
static_assert(C_API_IOMANIP_BACK_COLOR_BRIGHT_GREEN   == static_cast<std::uint8_t>(utils::iomanip::BackColor::BrightGreen));
static_assert(C_API_IOMANIP_BACK_COLOR_BRIGHT_YELLOW  == static_cast<std::uint8_t>(utils::iomanip::BackColor::BrightYellow));
static_assert(C_API_IOMANIP_BACK_COLOR_BRIGHT_BLUE    == static_cast<std::uint8_t>(utils::iomanip::BackColor::BrightBlue));
static_assert(C_API_IOMANIP_BACK_COLOR_BRIGHT_MAGENTA == static_cast<std::uint8_t>(utils::iomanip::BackColor::BrightMagenta));
static_assert(C_API_IOMANIP_BACK_COLOR_BRIGHT_CYAN    == static_cast<std::uint8_t>(utils::iomanip::BackColor::BrightCyan));
static_assert(C_API_IOMANIP_BACK_COLOR_BRIGHT_WHITE   == static_cast<std::uint8_t>(utils::iomanip::BackColor::BrightWhite));
static_assert(C_API_IOMANIP_BACK_COLOR_DEFAULT        == static_cast<std::uint8_t>(utils::iomanip::BackColor::Default));

static_assert(C_API_IOMANIP_STYLE_STRONG        == static_cast<std::uint8_t>(utils::iomanip::Style::Strong));
static_assert(C_API_IOMANIP_STYLE_DARK          == static_cast<std::uint8_t>(utils::iomanip::Style::Dark));
static_assert(C_API_IOMANIP_STYLE_ITALIC        == static_cast<std::uint8_t>(utils::iomanip::Style::Italic));
static_assert(C_API_IOMANIP_STYLE_UNDERLINED    == static_cast<std::uint8_t>(utils::iomanip::Style::Underlined));
static_assert(C_API_IOMANIP_STYLE_FLASHING_SLOW == static_cast<std::uint8_t>(utils::iomanip::Style::FlashingSlow));
static_assert(C_API_IOMANIP_STYLE_FLASHING_FAST == static_cast<std::uint8_t>(utils::iomanip::Style::FlashingFast));
static_assert(C_API_IOMANIP_STYLE_REVERSED      == static_cast<std::uint8_t>(utils::iomanip::Style::Reversed));
static_assert(C_API_IOMANIP_STYLE_HIDE          == static_cast<std::uint8_t>(utils::iomanip::Style::Hide));
static_assert(C_API_IOMANIP_STYLE_BAR           == static_cast<std::uint8_t>(utils::iomanip::Style::Bar));
static_assert(C_API_IOMANIP_STYLE_MONOSPACE     == static_cast<std::uint8_t>(utils::iomanip::Style::Monospace));
static_assert(C_API_IOMANIP_STYLE_FRAMED        == static_cast<std::uint8_t>(utils::iomanip::Style::Framed));
static_assert(C_API_IOMANIP_STYLE_ENCIRCLED     == static_cast<std::uint8_t>(utils::iomanip::Style::Encircled));
static_assert(C_API_IOMANIP_STYLE_OVERLINED     == static_cast<std::uint8_t>(utils::iomanip::Style::Overlined));
static_assert(C_API_IOMANIP_STYLE_EXPOSANT      == static_cast<std::uint8_t>(utils::iomanip::Style::Exposant));
static_assert(C_API_IOMANIP_STYLE_INDICE        == static_cast<std::uint8_t>(utils::iomanip::Style::Indice));

static_assert(C_API_IOMANIP_RESET_STYLE_ALL              == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::All));
static_assert(C_API_IOMANIP_RESET_STYLE_STRONG           == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::Strong));
static_assert(C_API_IOMANIP_RESET_STYLE_DARK             == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::Dark));
static_assert(C_API_IOMANIP_RESET_STYLE_ITALIC           == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::Italic));
static_assert(C_API_IOMANIP_RESET_STYLE_UNDERLINED       == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::Underlined));
static_assert(C_API_IOMANIP_RESET_STYLE_FLASHING_FAST    == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::FlashingFast));
static_assert(C_API_IOMANIP_RESET_STYLE_FLASHING_SLOW    == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::FlashingSlow));
static_assert(C_API_IOMANIP_RESET_STYLE_REVERSED         == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::Reversed));
static_assert(C_API_IOMANIP_RESET_STYLE_HIDE             == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::Hide));
static_assert(C_API_IOMANIP_RESET_STYLE_BAR              == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::Bar));
static_assert(C_API_IOMANIP_RESET_STYLE_FRAMED_ENCIRCLED == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::FramedEncircled));
static_assert(C_API_IOMANIP_RESET_STYLE_OVERLINED        == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::Overlined));
static_assert(C_API_IOMANIP_RESET_STYLE_UNDERLINE_COLOR  == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::UnderlineColor));
static_assert(C_API_IOMANIP_RESET_STYLE_EXPOSANT_INDICE  == static_cast<std::uint8_t>(utils::iomanip::ResetStyle::ExposantIndice));

static_assert(C_API_IOMANIP_MOUSE_BUTTON_LEFT    == static_cast<int>(utils::iomanip::MouseButton::Left));
static_assert(C_API_IOMANIP_MOUSE_BUTTON_RIGHT   == static_cast<int>(utils::iomanip::MouseButton::Right));
static_assert(C_API_IOMANIP_MOUSE_BUTTON_MIDDLE  == static_cast<int>(utils::iomanip::MouseButton::Middle));
static_assert(C_API_IOMANIP_MOUSE_BUTTON_RELEASE == static_cast<int>(utils::iomanip::MouseButton::Release));
static_assert(C_API_IOMANIP_MOUSE_BUTTON_UNKNOWN == static_cast<int>(utils::iomanip::MouseButton::Unknown));

//----------------------------------------------------------------//
/* TOOLS */

// Copy of a sequence for the thread (one buffer per call site: valid until the next call of the same function in the thread)
template<int ID>
_hot _nodiscard static const char* kept(std::string&& str)
{
    // xstyle: ignore-next CPP-THIS (false positive: _thread_local is the attribute macro, not a member)
    static _thread_local std::string buffer;

    buffer = std::move(str);
    return buffer.c_str();
}

// Codes of several styles in one sequence (like utils::iomanip::set_style / reset_style of an initializer_list)
template<typename T>
_cold _nodiscard static std::string join(const T* styles, const std::size_t count)
{
    std::string codes;

    if (count > 0) (void)utils::exception::c_api::not_null(styles, "styles");
    for (std::size_t i = 0; i < count; ++i)
        codes += (i > 0 ? ";" : "") + std::to_string(static_cast<int>(styles[i]));
    return utils::iomanip::csi(codes + "m");
}

//----------------------------------------------------------------//
/* TERMINAL */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold _nothrow bool utils_iomanip_enable_ansi(void)
{
    return utils::exception::c_api::guard_fallback(false, [](void) -> bool {return utils::iomanip::enable_ansi();});
}

//----------------------------------------------------------------//
/* REPORTS */

_cold _nothrow int utils_iomanip_read_cursor_position(int* row, int* col)
{
    return utils::exception::c_api::guard([&](void) {
        int* r = utils::exception::c_api::not_null(row, "row");
        int* c = utils::exception::c_api::not_null(col, "col");
        const std::pair<int, int> position = utils::iomanip::read_cursor_position();

        *r = position.first;
        *c = position.second;
    });
}

_cold _nothrow int utils_iomanip_read_mouse_event(utils_iomanip_mouse_event_t* event)
{
    return utils::exception::c_api::guard([&](void) {
        utils_iomanip_mouse_event_t& out = utils::exception::c_api::not_null_ref(event, "event");
        const utils::iomanip::MouseEvent read = utils::iomanip::read_mouse_event();

        out.button = static_cast<utils_iomanip_mouse_button_t>(read.button);
        out.x = read.x;
        out.y = read.y;
    });
}

_cold _nothrow int utils_iomanip_read_advanced_mouse_event(utils_iomanip_advanced_mouse_event_t* event)
{
    return utils::exception::c_api::guard([&](void) {
        utils_iomanip_advanced_mouse_event_t& out = utils::exception::c_api::not_null_ref(event, "event");
        const utils::iomanip::AdvancedMouseEvent read = utils::iomanip::read_advanced_mouse_event();

        out.button = static_cast<utils_iomanip_mouse_button_t>(read.button);
        out.x = read.x;
        out.y = read.y;
        out.pressed = read.pressed;
    });
}

//----------------------------------------------------------------//
/* SEQUENCES (without argument) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_esc(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::esc());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_strong_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::strong_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_dark_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::dark_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_italic_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::italic_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_underlined_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::underlined_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_flashing_fast_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::flashing_fast_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_flashing_slow_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::flashing_slow_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_reversed_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::reversed_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_hide_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::hide_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_bar_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::bar_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_framed_encircled_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::framed_encircled_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_overlined_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::overlined_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_underline_color_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::underline_color_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_exposant_indice_reset(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::exposant_indice_reset());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_strong(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::strong());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_dark(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::dark());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_italic(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::italic());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_underlined(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::underlined());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_flashing_fast(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::flashing_fast());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_flashing_slow(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::flashing_slow());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_reversed(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::reversed());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_hide(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::hide());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_bar(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::bar());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_monospace(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::monospace());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_framed(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::framed());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_encircled(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::encircled());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_overlined(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::overlined());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_exposant(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::exposant());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_indice(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::indice());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_load_cur(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::load_cur());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_save_cur(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::save_cur());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_screen_end(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::screen_end());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_screen_start(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::screen_start());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_screen(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::screen());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_scrollback_buffer(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::scrollback_buffer());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_line_end(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::line_end());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_line_start(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::line_start());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_line(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::line());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_inverted_color_enable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::inverted_color_enable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_inverted_color_disable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::inverted_color_disable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_wrapping_enable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::wrapping_enable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_wrapping_disable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::wrapping_disable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_show_cur(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::show_cur());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_hide_cur(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::hide_cur());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_save_screen(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::save_screen());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_load_screen(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::load_screen());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_get_pos(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::get_pos());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_mouse_tracking_enable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::mouse_tracking_enable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_mouse_tracking_disable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::mouse_tracking_disable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_mouse_move_tracking_enable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::mouse_move_tracking_enable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_mouse_move_tracking_disable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::mouse_move_tracking_disable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_mouse_adv_tracking_enable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::mouse_adv_tracking_enable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_mouse_adv_tracking_disable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::mouse_adv_tracking_disable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_report_focus_enable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::report_focus_enable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_report_focus_disabled(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::report_focus_disabled());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_report_past_enable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::report_past_enable());
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_report_past_disable(void)
{
    return C_API_STATIC_SEQUENCE(utils::iomanip::report_past_disable());
}

//----------------------------------------------------------------//
/* SEQUENCES (with arguments) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_csi(const char* code)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::csi(utils::exception::c_api::not_null(code, "code")));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_csi_r(const char* code, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::csi(utils::exception::c_api::not_null(code, "code")), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_hyperlink(const char* display, const char* link)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::hyperlink(utils::exception::c_api::not_null(display, "display"), utils::exception::c_api::not_null(link, "link")));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_hyperlink_r(const char* display, const char* link, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::hyperlink(utils::exception::c_api::not_null(display, "display"), utils::exception::c_api::not_null(link, "link")), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_file_hyperlink(const char* display, const char* path)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::file_hyperlink(utils::exception::c_api::not_null(display, "display"), utils::exception::c_api::not_null(path, "path")));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_file_hyperlink_r(const char* display, const char* path, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::file_hyperlink(utils::exception::c_api::not_null(display, "display"), utils::exception::c_api::not_null(path, "path")), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_set_style(utils_iomanip_style_t style)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::set_style(static_cast<utils::iomanip::Style>(style)));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_set_style_r(utils_iomanip_style_t style, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::set_style(static_cast<utils::iomanip::Style>(style)), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_set_styles(const utils_iomanip_style_t* styles, size_t count)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(join(styles, count));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_set_styles_r(const utils_iomanip_style_t* styles, size_t count, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(join(styles, count), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_reset_style(utils_iomanip_reset_style_t style)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::reset_style(static_cast<utils::iomanip::ResetStyle>(style)));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_reset_style_r(utils_iomanip_reset_style_t style, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::reset_style(static_cast<utils::iomanip::ResetStyle>(style)), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_reset_styles(const utils_iomanip_reset_style_t* styles, size_t count)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(join(styles, count));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_reset_styles_r(const utils_iomanip_reset_style_t* styles, size_t count, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(join(styles, count), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_color(utils_iomanip_color_t color)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::color(static_cast<utils::iomanip::Color>(color)));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_color_r(utils_iomanip_color_t color, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::color(static_cast<utils::iomanip::Color>(color)), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_back_color(utils_iomanip_back_color_t color)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::color(static_cast<utils::iomanip::BackColor>(color)));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_back_color_r(utils_iomanip_back_color_t color, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::color(static_cast<utils::iomanip::BackColor>(color)), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_color_id(uint8_t id)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::color_id(id));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_color_id_r(uint8_t id, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::color_id(id), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_back_color_id(uint8_t id)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::back_color_id(id));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_back_color_id_r(uint8_t id, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::back_color_id(id), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_underline_color_id(uint8_t id)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::underline_color_id(id));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_underline_color_id_r(uint8_t id, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::underline_color_id(id), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::color_rgb(r, g, b));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_color_rgb_r(uint8_t r, uint8_t g, uint8_t b, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::color_rgb(r, g, b), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_back_color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::back_color_rgb(r, g, b));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_back_color_rgb_r(uint8_t r, uint8_t g, uint8_t b, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::back_color_rgb(r, g, b), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_underline_color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::underline_color_rgb(r, g, b));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_underline_color_rgb_r(uint8_t r, uint8_t g, uint8_t b, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::underline_color_rgb(r, g, b), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_up(size_t n)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::up(n));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_up_r(size_t n, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::up(n), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_down(size_t n)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::down(n));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_down_r(size_t n, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::down(n), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_right(size_t n)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::right(n));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_right_r(size_t n, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::right(n), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_left(size_t n)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::left(n));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_left_r(size_t n, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::left(n), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_next_line(size_t n)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::next_line(n));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_next_line_r(size_t n, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::next_line(n), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_previous_line(size_t n)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::previous_line(n));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_previous_line_r(size_t n, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::previous_line(n), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_column(size_t col)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::column(col));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_column_r(size_t col, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::column(col), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_pos(size_t row, size_t col)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::pos(row, col));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_pos_r(size_t row, size_t col, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::pos(row, col), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_scroll_up(size_t n)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::scroll_up(n));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_scroll_up_r(size_t n, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::scroll_up(n), buf, size);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow const char* utils_iomanip_scroll_down(size_t n)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return kept<__LINE__>(utils::iomanip::scroll_down(n));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot _nothrow size_t utils_iomanip_scroll_down_r(size_t n, char* buf, size_t size)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::smanip::c_api::copy_r(utils::iomanip::scroll_down(n), buf, size);});
}
#undef C_API_STATIC_SEQUENCE
