/**************************************************************\
Edition:
##  @date 06/07/2026 by @author Tsukini

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
    #include "Color.hpp"        // utils::iomanip::Color, utils::iomanip::BackColor
    #include "Char.hpp"         // utils::iomanip::Char
    #include "Style.hpp"        // utils::iomanip::Style, utils::iomanip::ResetStyle
    #include <initializer_list> // std::initializer_list
    #include <cstdint>          // std::uint8_t, std::size_t
    #include <string>           // std::string

namespace utils::iomanip { // namespace start
    //----------------------------------------------------------------//
    /* ANSI */

    /* --------- function ---------- */
    std::string setStyle(std::initializer_list<utils::iomanip::Style> styles);
    std::string resetStyle(std::initializer_list<utils::iomanip::ResetStyle> styles);

    /* --------- internal ---------- */
    constexpr inline std::string csi(const std::string& code) {return std::string(1, static_cast<char>(utils::iomanip::Char::ESC)) + "[" + code;}

    /* ----------- reset ----------- */
    constexpr inline std::string reset(void)                    {return csi("0m");}
    constexpr inline std::string strong_reset(void)             {return csi("21m");}
    constexpr inline std::string dark_reset(void)               {return csi("22m");}
    constexpr inline std::string italic_reset(void)             {return csi("23m");}
    constexpr inline std::string underlined_reset(void)         {return csi("24m");}
    constexpr inline std::string flashing_fast_reset(void)      {return csi("25m");}
    constexpr inline std::string flashing_slow_reset(void)      {return csi("26m");}
    constexpr inline std::string reversed_reset(void)           {return csi("27m");}
    constexpr inline std::string hide_reset(void)               {return csi("28m");}
    constexpr inline std::string bar_reset(void)                {return csi("29m");}
    constexpr inline std::string framed_encircled_reset(void)   {return csi("54m");}
    constexpr inline std::string overlined_reset(void)          {return csi("55m");}
    constexpr inline std::string underline_color_reset(void)    {return csi("59m");}
    constexpr inline std::string exposant_indice_reset(void)    {return csi("75m");}
    /* args */
    constexpr inline std::string resetStyle(utils::iomanip::ResetStyle style) {return csi(std::to_string(static_cast<std::uint8_t>(style)) + "m");}

    /* ----------- style ----------- */
    constexpr inline std::string strong(void)           {return csi("1m");}
    constexpr inline std::string dark(void)             {return csi("2m");}
    constexpr inline std::string italic(void)           {return csi("3m");}
    constexpr inline std::string underlined(void)       {return csi("4m");}
    constexpr inline std::string flashing_fast(void)    {return csi("5m");}
    constexpr inline std::string flashing_slow(void)    {return csi("6m");}
    constexpr inline std::string reversed(void)         {return csi("7m");}
    constexpr inline std::string hide(void)             {return csi("8m");}
    constexpr inline std::string bar(void)              {return csi("9m");}
    constexpr inline std::string monospace(void)        {return csi("50m");}
    constexpr inline std::string framed(void)           {return csi("51m");} // Rarely supported
    constexpr inline std::string encircled(void)        {return csi("52m");} // Rarely supported
    constexpr inline std::string overlined(void)        {return csi("53m");}
    constexpr inline std::string exposant(void)         {return csi("73m");} // Rarely supported
    constexpr inline std::string indice(void)           {return csi("74m");} // Rarely supported
    /* args */
    constexpr inline std::string setStyle(utils::iomanip::Style style)                                    {return csi(std::to_string(static_cast<std::uint8_t>(style)) + "m");}
    constexpr inline std::string color(utils::iomanip::Color c)                                           {return csi(std::to_string(static_cast<std::uint8_t>(c)) + "m");}
    constexpr inline std::string color(utils::iomanip::BackColor c)                                       {return csi(std::to_string(static_cast<std::uint8_t>(c)) + "m");}
    constexpr inline std::string color_id(std::uint8_t id)                                              {return csi("38;5;" + std::to_string(id) + "m");}
    constexpr inline std::string back_color_id(std::uint8_t id)                                         {return csi("48;5;" + std::to_string(id) + "m");}
    constexpr inline std::string underline_color_id(std::uint8_t id)                                    {return csi("58;5;" + std::to_string(id) + "m");}
    constexpr inline std::string color_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)              {return csi("38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m");}
    constexpr inline std::string back_color_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)         {return csi("48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m");}
    constexpr inline std::string underline_color_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)    {return csi("58;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m");}

    /* ---------- cursor ----------- */
    constexpr inline std::string load_cur(void) {return csi("u");}
    constexpr inline std::string save_cur(void) {return csi("s");}
    /* args */
    constexpr inline std::string up(std::size_t n)                      {return csi(std::to_string(n) + "A");}
    constexpr inline std::string down(std::size_t n)                    {return csi(std::to_string(n) + "B");}
    constexpr inline std::string right(std::size_t n)                   {return csi(std::to_string(n) + "C");}
    constexpr inline std::string left(std::size_t n)                    {return csi(std::to_string(n) + "D");}
    constexpr inline std::string next_line(std::size_t n)               {return csi(std::to_string(n) + "E");}
    constexpr inline std::string previous_line(std::size_t n)           {return csi(std::to_string(n) + "F");}
    constexpr inline std::string column(std::size_t col)                {return csi(std::to_string(col) + "G");}
    constexpr inline std::string pos(std::size_t row, std::size_t col)  {return csi(std::to_string(row) + ";" + std::to_string(col) + "H");}
    constexpr inline std::string scroll_up(std::size_t n)               {return csi(std::to_string(n) + "S");}
    constexpr inline std::string scroll_down(std::size_t n)             {return csi(std::to_string(n) + "T");}

    /* ----------- erase ----------- */
    constexpr inline std::string screen_end(void)           {return csi("0J");}
    constexpr inline std::string screen_start(void)         {return csi("1J");}
    constexpr inline std::string screen(void)               {return csi("2J");}
    constexpr inline std::string scrollback_buffer(void)    {return csi("3J");} // Can delete the term history
    constexpr inline std::string line_end(void)             {return csi("0K");}
    constexpr inline std::string line_start(void)           {return csi("1K");}
    constexpr inline std::string line(void)                 {return csi("2K");}

    /* ------ private-modes ------- */
    constexpr inline std::string inverted_color_enable(void)    {return csi("?5h");}
    constexpr inline std::string inverted_color_disable(void)   {return csi("?5l");}
    constexpr inline std::string wrapping_enable(void)          {return csi("?7h");}
    constexpr inline std::string wrapping_disable(void)         {return csi("?7l");}
    constexpr inline std::string show_cur(void)                 {return csi("?25h");}
    constexpr inline std::string hide_cur(void)                 {return csi("?25l");}
    constexpr inline std::string save_screen(void)              {return csi("?1049h");}
    constexpr inline std::string load_screen(void)              {return csi("?1049l");}
    
    /* --------- reports ---------- */
    constexpr inline std::string get_pos(void)                      {return csi("6n");} // Reports cursor position as "ESC[row;colR"
    constexpr inline std::string mouse_tracking_enable(void)        {return csi("?1000h");}
    constexpr inline std::string mouse_tracking_disable(void)       {return csi("?1000l");}
    constexpr inline std::string mouse_move_tracking_enable(void)   {return csi("?1002h");}
    constexpr inline std::string mouse_move_tracking_disable(void)  {return csi("?1002l");}
    // Reports mouse action as (each x&y value as a '+ 32') "ESC[Mb;x;y" -> 'b' is the boutton & modifier
    // Boutton: 0 left, 1 mid, 2 right, 3 release
    // Modifer: +4 shift, +8 alt, +16 ctrl
    constexpr inline std::string mouse_adv_tracking_enable(void)    {return csi("?1006h");}
    constexpr inline std::string mouse_adv_tracking_disable(void)   {return csi("?1006l");}
    // Reports mouse action as "ESC[<b;x;y(M|m)" -> 'b' is the boutton & modifier -> M = pressed, m = released
    // Boutton: 0 left, 1 mid, 2 right, 3 release
    // Modifer: +4 shift, +8 alt, +16 ctrl
    constexpr inline std::string report_focus_enable(void)          {return csi("?1004h");} // Focus in: "ESC[I" | Focus out: "ESC[O"
    constexpr inline std::string report_focus_disabled(void)        {return csi("?1004l");}
    constexpr inline std::string report_past_enable(void)           {return csi("?2004h");} // Reports for pasted data: "ESC[200~{data}ESC[201~"
    constexpr inline std::string report_past_disable(void)          {return csi("?2004l");}

//----------------------------------------------------------------//
/* TYPDEF */

/* mousse button */
enum class MouseButton {
    Left,
    Right,
    Middle,
    Release,
    Unknown
};

/* mouse event -> "ESC[Mb;x;y" */
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

/* reports */
std::pair<int, int> readCursorPosition(void);
utils::iomanip::MouseEvent readMouseEvent(void);
utils::iomanip::AdvancedMouseEvent readAdvancedMouseEvent(void);

} // namespace ends
#endif /* ANSI_H */
