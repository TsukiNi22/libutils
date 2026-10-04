/**************************************************************\
Edition:
##  @date 27/07/2026 by @author Tsukini

File Name:
##  @file ANSI.hpp

File Description:
##  Definition of ANSI escape sequences
\**************************************************************/

#ifndef ANSI_H
    #define ANSI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "Color.hpp"                        // utils::iomanip::Color, utils::iomanip::BackColor
    #include "Char.hpp"                         // utils::iomanip::Char
    #include "Style.hpp"                        // utils::iomanip::Style, utils::iomanip::ResetStyle
    #include "../../attribute/Attribute.hpp"    // _cold, _hot, _nodiscard, _migration
    #include <initializer_list>                 // std::initializer_list
    #include <cstddef>                          // std::size_t
    #include <cstdint>                          // std::uint8_t
    #include <utility>                          // std::pair
    #include <string>                           // std::string, std::to_string

namespace utils::iomanip { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* function */
std::string set_style(std::initializer_list<utils::iomanip::Style> styles);
std::string reset_style(std::initializer_list<utils::iomanip::ResetStyle> styles);

/* internal */
_hot _nodiscard constexpr inline std::string esc(void)                    {return std::string(1, static_cast<char>(utils::iomanip::Char::ESC));};
_hot _nodiscard constexpr inline std::string csi(const std::string& code) {return utils::iomanip::esc() + "[" + code;};

/* special */
_hot _nodiscard constexpr inline std::string file_hyperlink(const std::string& display, const std::string& path) {return utils::iomanip::esc() + "]8;;file://" + path + utils::iomanip::esc() + "\\" + display + utils::iomanip::esc() + "]8;;" + utils::iomanip::esc() + "\\";};
_hot _nodiscard constexpr inline std::string hyperlink(const std::string& display, const std::string& link)      {return utils::iomanip::esc() + "]8;;" + link + utils::iomanip::esc() + "\\" + display + utils::iomanip::esc() + "]8;;" + utils::iomanip::esc() + "\\";};

/* reset */
_hot _nodiscard constexpr inline std::string reset(void)                  {return utils::iomanip::csi("0m");};
_hot _nodiscard constexpr inline std::string strong_reset(void)           {return utils::iomanip::csi("22m");};
_hot _nodiscard constexpr inline std::string dark_reset(void)             {return utils::iomanip::csi("22m");};
_hot _nodiscard constexpr inline std::string italic_reset(void)           {return utils::iomanip::csi("23m");};
_hot _nodiscard constexpr inline std::string underlined_reset(void)       {return utils::iomanip::csi("24m");};
_hot _nodiscard constexpr inline std::string flashing_fast_reset(void)    {return utils::iomanip::csi("25m");};
_hot _nodiscard constexpr inline std::string flashing_slow_reset(void)    {return utils::iomanip::csi("25m");};
_hot _nodiscard constexpr inline std::string reversed_reset(void)         {return utils::iomanip::csi("27m");};
_hot _nodiscard constexpr inline std::string hide_reset(void)             {return utils::iomanip::csi("28m");};
_hot _nodiscard constexpr inline std::string bar_reset(void)              {return utils::iomanip::csi("29m");};
_hot _nodiscard constexpr inline std::string framed_encircled_reset(void) {return utils::iomanip::csi("54m");};
_hot _nodiscard constexpr inline std::string overlined_reset(void)        {return utils::iomanip::csi("55m");};
_hot _nodiscard constexpr inline std::string underline_color_reset(void)  {return utils::iomanip::csi("59m");};
_hot _nodiscard constexpr inline std::string exposant_indice_reset(void)  {return utils::iomanip::csi("75m");};
/* args */
_hot _nodiscard constexpr inline std::string reset_style(utils::iomanip::ResetStyle style) {return utils::iomanip::csi(std::to_string(static_cast<std::uint8_t>(style)) + "m");};

/* style */
_hot _nodiscard constexpr inline std::string strong(void)        {return utils::iomanip::csi("1m");};
_hot _nodiscard constexpr inline std::string dark(void)          {return utils::iomanip::csi("2m");};
_hot _nodiscard constexpr inline std::string italic(void)        {return utils::iomanip::csi("3m");};
_hot _nodiscard constexpr inline std::string underlined(void)    {return utils::iomanip::csi("4m");};
_hot _nodiscard constexpr inline std::string flashing_fast(void) {return utils::iomanip::csi("6m");};
_hot _nodiscard constexpr inline std::string flashing_slow(void) {return utils::iomanip::csi("5m");};
_hot _nodiscard constexpr inline std::string reversed(void)      {return utils::iomanip::csi("7m");};
_hot _nodiscard constexpr inline std::string hide(void)          {return utils::iomanip::csi("8m");};
_hot _nodiscard constexpr inline std::string bar(void)           {return utils::iomanip::csi("9m");};
_hot _nodiscard constexpr inline std::string monospace(void)     {return utils::iomanip::csi("50m");};
_hot _nodiscard constexpr inline std::string framed(void)        {return utils::iomanip::csi("51m");}; // Rarely supported
_hot _nodiscard constexpr inline std::string encircled(void)     {return utils::iomanip::csi("52m");}; // Rarely supported
_hot _nodiscard constexpr inline std::string overlined(void)     {return utils::iomanip::csi("53m");};
_hot _nodiscard constexpr inline std::string exposant(void)      {return utils::iomanip::csi("73m");}; // Rarely supported
_hot _nodiscard constexpr inline std::string indice(void)        {return utils::iomanip::csi("74m");}; // Rarely supported
/* args */
_hot _nodiscard constexpr inline std::string set_style(utils::iomanip::Style style)                              {return utils::iomanip::csi(std::to_string(static_cast<std::uint8_t>(style)) + "m");};
_hot _nodiscard constexpr inline std::string color(utils::iomanip::Color c)                                      {return utils::iomanip::csi(std::to_string(static_cast<std::uint8_t>(c)) + "m");};
_hot _nodiscard constexpr inline std::string color(utils::iomanip::BackColor c)                                  {return utils::iomanip::csi(std::to_string(static_cast<std::uint8_t>(c)) + "m");};
_hot _nodiscard constexpr inline std::string color_id(std::uint8_t id)                                           {return utils::iomanip::csi("38;5;" + std::to_string(id) + "m");};
_hot _nodiscard constexpr inline std::string back_color_id(std::uint8_t id)                                      {return utils::iomanip::csi("48;5;" + std::to_string(id) + "m");};
_hot _nodiscard constexpr inline std::string underline_color_id(std::uint8_t id)                                 {return utils::iomanip::csi("58;5;" + std::to_string(id) + "m");};
_hot _nodiscard constexpr inline std::string color_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)           {return utils::iomanip::csi("38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m");};
_hot _nodiscard constexpr inline std::string back_color_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)      {return utils::iomanip::csi("48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m");};
_hot _nodiscard constexpr inline std::string underline_color_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) {return utils::iomanip::csi("58;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m");};

/* cursor */
_hot _nodiscard constexpr inline std::string load_cur(void) {return utils::iomanip::csi("u");};
_hot _nodiscard constexpr inline std::string save_cur(void) {return utils::iomanip::csi("s");};
/* args */
_hot _nodiscard constexpr inline std::string up(std::size_t n)                     {return utils::iomanip::csi(std::to_string(n) + "A");};
_hot _nodiscard constexpr inline std::string down(std::size_t n)                   {return utils::iomanip::csi(std::to_string(n) + "B");};
_hot _nodiscard constexpr inline std::string right(std::size_t n)                  {return utils::iomanip::csi(std::to_string(n) + "C");};
_hot _nodiscard constexpr inline std::string left(std::size_t n)                   {return utils::iomanip::csi(std::to_string(n) + "D");};
_hot _nodiscard constexpr inline std::string next_line(std::size_t n)              {return utils::iomanip::csi(std::to_string(n) + "E");};
_hot _nodiscard constexpr inline std::string previous_line(std::size_t n)          {return utils::iomanip::csi(std::to_string(n) + "F");};
_hot _nodiscard constexpr inline std::string column(std::size_t col)               {return utils::iomanip::csi(std::to_string(col) + "G");};
_hot _nodiscard constexpr inline std::string pos(std::size_t row, std::size_t col) {return utils::iomanip::csi(std::to_string(row) + ";" + std::to_string(col) + "H");};
_hot _nodiscard constexpr inline std::string scroll_up(std::size_t n)              {return utils::iomanip::csi(std::to_string(n) + "S");};
_hot _nodiscard constexpr inline std::string scroll_down(std::size_t n)            {return utils::iomanip::csi(std::to_string(n) + "T");};

/* erase */
_hot _nodiscard constexpr inline std::string screen_end(void)        {return utils::iomanip::csi("0J");};
_hot _nodiscard constexpr inline std::string screen_start(void)      {return utils::iomanip::csi("1J");};
_hot _nodiscard constexpr inline std::string screen(void)            {return utils::iomanip::csi("2J");};
_hot _nodiscard constexpr inline std::string scrollback_buffer(void) {return utils::iomanip::csi("3J");}; // Can delete the term history
_hot _nodiscard constexpr inline std::string line_end(void)          {return utils::iomanip::csi("0K");};
_hot _nodiscard constexpr inline std::string line_start(void)        {return utils::iomanip::csi("1K");};
_hot _nodiscard constexpr inline std::string line(void)              {return utils::iomanip::csi("2K");};

/* private modes */
_hot _nodiscard constexpr inline std::string inverted_color_enable(void)  {return utils::iomanip::csi("?5h");};
_hot _nodiscard constexpr inline std::string inverted_color_disable(void) {return utils::iomanip::csi("?5l");};
_hot _nodiscard constexpr inline std::string wrapping_enable(void)        {return utils::iomanip::csi("?7h");};
_hot _nodiscard constexpr inline std::string wrapping_disable(void)       {return utils::iomanip::csi("?7l");};
_hot _nodiscard constexpr inline std::string show_cur(void)               {return utils::iomanip::csi("?25h");};
_hot _nodiscard constexpr inline std::string hide_cur(void)               {return utils::iomanip::csi("?25l");};
_hot _nodiscard constexpr inline std::string save_screen(void)            {return utils::iomanip::csi("?1049h");};
_hot _nodiscard constexpr inline std::string load_screen(void)            {return utils::iomanip::csi("?1049l");};

/* reports */
_hot _nodiscard constexpr inline std::string get_pos(void)                     {return utils::iomanip::csi("6n");}; // Reports cursor position as "ESC[row;colR"
_hot _nodiscard constexpr inline std::string mouse_tracking_enable(void)       {return utils::iomanip::csi("?1000h");};
_hot _nodiscard constexpr inline std::string mouse_tracking_disable(void)      {return utils::iomanip::csi("?1000l");};
_hot _nodiscard constexpr inline std::string mouse_move_tracking_enable(void)  {return utils::iomanip::csi("?1002h");};
_hot _nodiscard constexpr inline std::string mouse_move_tracking_disable(void) {return utils::iomanip::csi("?1002l");};
// Reports mouse action as "ESC[M" Cb Cx Cy (3 raw bytes, each value + 32) -> 'Cb' is the button & modifier
// Button: 0 left, 1 mid, 2 right, 3 release
// Modifier: +4 shift, +8 alt, +16 ctrl
_hot _nodiscard constexpr inline std::string mouse_adv_tracking_enable(void)  {return utils::iomanip::csi("?1006h");};
_hot _nodiscard constexpr inline std::string mouse_adv_tracking_disable(void) {return utils::iomanip::csi("?1006l");};
// Reports mouse action as "ESC[<b;x;y(M|m)" -> 'b' is the button & modifier -> M = pressed, m = released
// Button: 0 left, 1 mid, 2 right, 3 release
// Modifier: +4 shift, +8 alt, +16 ctrl
_hot _nodiscard constexpr inline std::string report_focus_enable(void)   {return utils::iomanip::csi("?1004h");}; // Focus in: "ESC[I" | Focus out: "ESC[O"
_hot _nodiscard constexpr inline std::string report_focus_disabled(void) {return utils::iomanip::csi("?1004l");};
_hot _nodiscard constexpr inline std::string report_past_enable(void)    {return utils::iomanip::csi("?2004h");}; // Reports for pasted data: "ESC[200~{data}ESC[201~"
_hot _nodiscard constexpr inline std::string report_past_disable(void)   {return utils::iomanip::csi("?2004l");};

//----------------------------------------------------------------//
/* ENUM */

/* mouse button */
enum class MouseButton {
    Left,
    Right,
    Middle,
    Release,
    Unknown,
};

//----------------------------------------------------------------//
/* STRUCT */

/* mouse event -> "ESC[M" Cb Cx Cy */
struct MouseEvent {
    utils::iomanip::MouseButton button = utils::iomanip::MouseButton::Unknown;
    std::size_t x = 0;
    std::size_t y = 0;
};

/* mouse event -> "ESC[<b;x;y(M|m)" */
struct AdvancedMouseEvent {
    utils::iomanip::MouseButton button = utils::iomanip::MouseButton::Unknown;
    std::size_t x = 0;
    std::size_t y = 0;
    bool pressed = false;
};

//----------------------------------------------------------------//
/* PROTOTYPE */

/* reports */
std::pair<int, int> read_cursor_position(void);
utils::iomanip::MouseEvent read_mouse_event(void);
utils::iomanip::AdvancedMouseEvent read_advanced_mouse_event(void);

} // namespace end

//----------------------------------------------------------------//
/* MIGRATION */
namespace utils::iomanip {
    _migration(4, 0, 0) inline std::string setStyle(std::initializer_list<utils::iomanip::Style> styles)        {return utils::iomanip::set_style(styles);};
    _migration(4, 0, 0) inline std::string setStyle(utils::iomanip::Style style)                                {return utils::iomanip::set_style(style);};
    _migration(4, 0, 0) inline std::string resetStyle(std::initializer_list<utils::iomanip::ResetStyle> styles) {return utils::iomanip::reset_style(styles);};
    _migration(4, 0, 0) inline std::string resetStyle(utils::iomanip::ResetStyle style)                         {return utils::iomanip::reset_style(style);};
    _migration(4, 0, 0) inline std::pair<int, int> readCursorPosition(void)                                     {return utils::iomanip::read_cursor_position();};
    _migration(4, 0, 0) inline utils::iomanip::MouseEvent readMouseEvent(void)                                  {return utils::iomanip::read_mouse_event();};
    _migration(4, 0, 0) inline utils::iomanip::AdvancedMouseEvent readAdvancedMouseEvent(void)                  {return utils::iomanip::read_advanced_mouse_event();};
}

#endif /* ANSI_H */
