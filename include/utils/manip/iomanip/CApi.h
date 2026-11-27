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
##  @file CApi.h

File Description:
##  C API of utils::iomanip: the ANSI sequences as strings, the reports of the terminal (C & C++ compatible)
##  Sequences without argument: static strings (always valid), with arguments: valid until the next call of the same function in the thread (NULL on error), _r: copied in the buffer of the user
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef IOMANIP_CAPI_C_H
    #define IOMANIP_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../../attribute/Attribute.hpp"   // _nodiscard, _nothrow, _write_only, _null_terminated_string_arg
    #include "../../exception/CApi.h"          // utils_exception, UTILS_OK, UTILS_KO
    #include <stdbool.h>                       // bool
    #include <stddef.h>                        // size_t
    #include <stdint.h>                        // uint8_t

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* ENUM */

/* special chars (utils::iomanip::Char) */
typedef enum utils_iomanip_char {
    C_API_IOMANIP_CHAR_NUL = 0,   // Null
    C_API_IOMANIP_CHAR_SOH = 1,   // Start of Header
    C_API_IOMANIP_CHAR_STX = 2,   // Start of Text
    C_API_IOMANIP_CHAR_ETX = 3,   // End of Text
    C_API_IOMANIP_CHAR_EOT = 4,   // End of Transmission
    C_API_IOMANIP_CHAR_ENQ = 5,   // Enquiry
    C_API_IOMANIP_CHAR_ACK = 6,   // Acknowledge
    C_API_IOMANIP_CHAR_BEL = 7,   // Bell
    C_API_IOMANIP_CHAR_BS  = 8,   // Backspace
    C_API_IOMANIP_CHAR_HT  = 9,   // Horizontal Tab
    C_API_IOMANIP_CHAR_LF  = 10,  // Line Feed
    C_API_IOMANIP_CHAR_VT  = 11,  // Vertical Tab
    C_API_IOMANIP_CHAR_FF  = 12,  // Form Feed
    C_API_IOMANIP_CHAR_CR  = 13,  // Carriage Return
    C_API_IOMANIP_CHAR_SO  = 14,  // Shift Out
    C_API_IOMANIP_CHAR_SI  = 15,  // Shift In
    C_API_IOMANIP_CHAR_DLE = 16,  // Data Link Escape
    C_API_IOMANIP_CHAR_DC1 = 17,  // Device Control 1
    C_API_IOMANIP_CHAR_DC2 = 18,  // Device Control 2
    C_API_IOMANIP_CHAR_DC3 = 19,  // Device Control 3
    C_API_IOMANIP_CHAR_DC4 = 20,  // Device Control 4
    C_API_IOMANIP_CHAR_NAK = 21,  // Negative Acknowledge
    C_API_IOMANIP_CHAR_SYN = 22,  // Synchronous Idle
    C_API_IOMANIP_CHAR_ETB = 23,  // End of Transmission Block
    C_API_IOMANIP_CHAR_CAN = 24,  // Cancel
    C_API_IOMANIP_CHAR_EM  = 25,  // End of Medium
    C_API_IOMANIP_CHAR_SUB = 26,  // Substitute
    C_API_IOMANIP_CHAR_ESC = 27,  // Escape
    C_API_IOMANIP_CHAR_FS  = 28,  // File Separator
    C_API_IOMANIP_CHAR_GS  = 29,  // Group Separator
    C_API_IOMANIP_CHAR_RS  = 30,  // Record Separator
    C_API_IOMANIP_CHAR_US  = 31,  // Unit Separator
    C_API_IOMANIP_CHAR_DEL = 127  // Delete
} utils_iomanip_char_t;

/* color (utils::iomanip::Color) */
typedef enum utils_iomanip_color {
    C_API_IOMANIP_COLOR_BLACK = 30,
    C_API_IOMANIP_COLOR_RED = 31,
    C_API_IOMANIP_COLOR_GREEN = 32,
    C_API_IOMANIP_COLOR_YELLOW = 33,
    C_API_IOMANIP_COLOR_BLUE = 34,
    C_API_IOMANIP_COLOR_MAGENTA = 35,
    C_API_IOMANIP_COLOR_CYAN = 36,
    C_API_IOMANIP_COLOR_WHITE = 37,
    C_API_IOMANIP_COLOR_BRIGHT_BLACK = 90,
    C_API_IOMANIP_COLOR_BRIGHT_RED = 91,
    C_API_IOMANIP_COLOR_BRIGHT_GREEN = 92,
    C_API_IOMANIP_COLOR_BRIGHT_YELLOW = 93,
    C_API_IOMANIP_COLOR_BRIGHT_BLUE = 94,
    C_API_IOMANIP_COLOR_BRIGHT_MAGENTA = 95,
    C_API_IOMANIP_COLOR_BRIGHT_CYAN = 96,
    C_API_IOMANIP_COLOR_BRIGHT_WHITE = 97,
    C_API_IOMANIP_COLOR_DEFAULT = 39
} utils_iomanip_color_t;

/* back color (utils::iomanip::BackColor) */
typedef enum utils_iomanip_back_color {
    C_API_IOMANIP_BACK_COLOR_BLACK = 40,
    C_API_IOMANIP_BACK_COLOR_RED = 41,
    C_API_IOMANIP_BACK_COLOR_GREEN = 42,
    C_API_IOMANIP_BACK_COLOR_YELLOW = 43,
    C_API_IOMANIP_BACK_COLOR_BLUE = 44,
    C_API_IOMANIP_BACK_COLOR_MAGENTA = 45,
    C_API_IOMANIP_BACK_COLOR_CYAN = 46,
    C_API_IOMANIP_BACK_COLOR_WHITE = 47,
    C_API_IOMANIP_BACK_COLOR_BRIGHT_BLACK = 100,
    C_API_IOMANIP_BACK_COLOR_BRIGHT_RED = 101,
    C_API_IOMANIP_BACK_COLOR_BRIGHT_GREEN = 102,
    C_API_IOMANIP_BACK_COLOR_BRIGHT_YELLOW = 103,
    C_API_IOMANIP_BACK_COLOR_BRIGHT_BLUE = 104,
    C_API_IOMANIP_BACK_COLOR_BRIGHT_MAGENTA = 105,
    C_API_IOMANIP_BACK_COLOR_BRIGHT_CYAN = 106,
    C_API_IOMANIP_BACK_COLOR_BRIGHT_WHITE = 107,
    C_API_IOMANIP_BACK_COLOR_DEFAULT = 49
} utils_iomanip_back_color_t;

/* style (utils::iomanip::Style) */
typedef enum utils_iomanip_style {
    C_API_IOMANIP_STYLE_STRONG = 1,
    C_API_IOMANIP_STYLE_DARK = 2,
    C_API_IOMANIP_STYLE_ITALIC = 3,
    C_API_IOMANIP_STYLE_UNDERLINED = 4,
    C_API_IOMANIP_STYLE_FLASHING_SLOW = 5,
    C_API_IOMANIP_STYLE_FLASHING_FAST = 6,
    C_API_IOMANIP_STYLE_REVERSED = 7,
    C_API_IOMANIP_STYLE_HIDE = 8,
    C_API_IOMANIP_STYLE_BAR = 9,
    C_API_IOMANIP_STYLE_MONOSPACE = 50,
    C_API_IOMANIP_STYLE_FRAMED = 51,
    C_API_IOMANIP_STYLE_ENCIRCLED = 52,
    C_API_IOMANIP_STYLE_OVERLINED = 53,
    C_API_IOMANIP_STYLE_EXPOSANT = 73,
    C_API_IOMANIP_STYLE_INDICE = 74
} utils_iomanip_style_t;

/* reset style (utils::iomanip::ResetStyle) */
typedef enum utils_iomanip_reset_style {
    C_API_IOMANIP_RESET_STYLE_ALL = 0,
    C_API_IOMANIP_RESET_STYLE_STRONG = 22,
    C_API_IOMANIP_RESET_STYLE_DARK = 22,
    C_API_IOMANIP_RESET_STYLE_ITALIC = 23,
    C_API_IOMANIP_RESET_STYLE_UNDERLINED = 24,
    C_API_IOMANIP_RESET_STYLE_FLASHING_FAST = 25,
    C_API_IOMANIP_RESET_STYLE_FLASHING_SLOW = 25,
    C_API_IOMANIP_RESET_STYLE_REVERSED = 27,
    C_API_IOMANIP_RESET_STYLE_HIDE = 28,
    C_API_IOMANIP_RESET_STYLE_BAR = 29,
    C_API_IOMANIP_RESET_STYLE_FRAMED_ENCIRCLED = 54,
    C_API_IOMANIP_RESET_STYLE_OVERLINED = 55,
    C_API_IOMANIP_RESET_STYLE_UNDERLINE_COLOR = 59,
    C_API_IOMANIP_RESET_STYLE_EXPOSANT_INDICE = 75
} utils_iomanip_reset_style_t;

/* mouse button (utils::iomanip::MouseButton) */
typedef enum utils_iomanip_mouse_button {
    C_API_IOMANIP_MOUSE_BUTTON_LEFT = 0,
    C_API_IOMANIP_MOUSE_BUTTON_RIGHT,
    C_API_IOMANIP_MOUSE_BUTTON_MIDDLE,
    C_API_IOMANIP_MOUSE_BUTTON_RELEASE,
    C_API_IOMANIP_MOUSE_BUTTON_UNKNOWN
} utils_iomanip_mouse_button_t;

//----------------------------------------------------------------//
/* STRUCT */

/* mouse event -> "ESC[M" Cb Cx Cy (utils::iomanip::MouseEvent) */
typedef struct utils_iomanip_mouse_event {
    utils_iomanip_mouse_button_t button;
    size_t x;
    size_t y;
} utils_iomanip_mouse_event_t;

/* mouse event -> "ESC[<b;x;y(M|m)" (utils::iomanip::AdvancedMouseEvent) */
typedef struct utils_iomanip_advanced_mouse_event {
    utils_iomanip_mouse_button_t button;
    size_t x;
    size_t y;
    bool pressed;
} utils_iomanip_advanced_mouse_event_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* terminal */
_nodiscard _nothrow bool utils_iomanip_enable_ansi(void); // Windows: enable the virtual terminal mode of the console (ANSI sequences), Unix: nothing to do (true)

/* reports (read on stdin, after the matching sequence), UTILS_OK / UTILS_KO */
_nodiscard _nothrow int utils_iomanip_read_cursor_position(int* row, int* col); // after utils_iomanip_get_pos (-1, -1 when nothing was read)
_nodiscard _nothrow int utils_iomanip_read_mouse_event(utils_iomanip_mouse_event_t* event); // after utils_iomanip_mouse_tracking_enable
_nodiscard _nothrow int utils_iomanip_read_advanced_mouse_event(utils_iomanip_advanced_mouse_event_t* event); // after utils_iomanip_mouse_adv_tracking_enable

/* sequences without argument (static strings) */
// internal
_nodiscard _nothrow const char* utils_iomanip_esc(void);
// reset
_nodiscard _nothrow const char* utils_iomanip_reset(void);
_nodiscard _nothrow const char* utils_iomanip_strong_reset(void);
_nodiscard _nothrow const char* utils_iomanip_dark_reset(void);
_nodiscard _nothrow const char* utils_iomanip_italic_reset(void);
_nodiscard _nothrow const char* utils_iomanip_underlined_reset(void);
_nodiscard _nothrow const char* utils_iomanip_flashing_fast_reset(void);
_nodiscard _nothrow const char* utils_iomanip_flashing_slow_reset(void);
_nodiscard _nothrow const char* utils_iomanip_reversed_reset(void);
_nodiscard _nothrow const char* utils_iomanip_hide_reset(void);
_nodiscard _nothrow const char* utils_iomanip_bar_reset(void);
_nodiscard _nothrow const char* utils_iomanip_framed_encircled_reset(void);
_nodiscard _nothrow const char* utils_iomanip_overlined_reset(void);
_nodiscard _nothrow const char* utils_iomanip_underline_color_reset(void);
_nodiscard _nothrow const char* utils_iomanip_exposant_indice_reset(void);
// style
_nodiscard _nothrow const char* utils_iomanip_strong(void);
_nodiscard _nothrow const char* utils_iomanip_dark(void);
_nodiscard _nothrow const char* utils_iomanip_italic(void);
_nodiscard _nothrow const char* utils_iomanip_underlined(void);
_nodiscard _nothrow const char* utils_iomanip_flashing_fast(void);
_nodiscard _nothrow const char* utils_iomanip_flashing_slow(void);
_nodiscard _nothrow const char* utils_iomanip_reversed(void);
_nodiscard _nothrow const char* utils_iomanip_hide(void);
_nodiscard _nothrow const char* utils_iomanip_bar(void);
_nodiscard _nothrow const char* utils_iomanip_monospace(void);
_nodiscard _nothrow const char* utils_iomanip_framed(void); // Rarely supported
_nodiscard _nothrow const char* utils_iomanip_encircled(void); // Rarely supported
_nodiscard _nothrow const char* utils_iomanip_overlined(void);
_nodiscard _nothrow const char* utils_iomanip_exposant(void); // Rarely supported
_nodiscard _nothrow const char* utils_iomanip_indice(void); // Rarely supported
// cursor
_nodiscard _nothrow const char* utils_iomanip_load_cur(void);
_nodiscard _nothrow const char* utils_iomanip_save_cur(void);
// erase
_nodiscard _nothrow const char* utils_iomanip_screen_end(void);
_nodiscard _nothrow const char* utils_iomanip_screen_start(void);
_nodiscard _nothrow const char* utils_iomanip_screen(void);
_nodiscard _nothrow const char* utils_iomanip_scrollback_buffer(void); // Can delete the term history
_nodiscard _nothrow const char* utils_iomanip_line_end(void);
_nodiscard _nothrow const char* utils_iomanip_line_start(void);
_nodiscard _nothrow const char* utils_iomanip_line(void);
// private modes
_nodiscard _nothrow const char* utils_iomanip_inverted_color_enable(void);
_nodiscard _nothrow const char* utils_iomanip_inverted_color_disable(void);
_nodiscard _nothrow const char* utils_iomanip_wrapping_enable(void);
_nodiscard _nothrow const char* utils_iomanip_wrapping_disable(void);
_nodiscard _nothrow const char* utils_iomanip_show_cur(void);
_nodiscard _nothrow const char* utils_iomanip_hide_cur(void);
_nodiscard _nothrow const char* utils_iomanip_save_screen(void);
_nodiscard _nothrow const char* utils_iomanip_load_screen(void);
// reports
_nodiscard _nothrow const char* utils_iomanip_get_pos(void); // Reports cursor position as "ESC[row;colR" (utils_iomanip_read_cursor_position)
_nodiscard _nothrow const char* utils_iomanip_mouse_tracking_enable(void);
_nodiscard _nothrow const char* utils_iomanip_mouse_tracking_disable(void);
_nodiscard _nothrow const char* utils_iomanip_mouse_move_tracking_enable(void);
_nodiscard _nothrow const char* utils_iomanip_mouse_move_tracking_disable(void);
_nodiscard _nothrow const char* utils_iomanip_mouse_adv_tracking_enable(void); // Reports "ESC[<b;x;y(M|m)" (utils_iomanip_read_advanced_mouse_event)
_nodiscard _nothrow const char* utils_iomanip_mouse_adv_tracking_disable(void);
_nodiscard _nothrow const char* utils_iomanip_report_focus_enable(void); // Focus in: "ESC[I" | Focus out: "ESC[O"
_nodiscard _nothrow const char* utils_iomanip_report_focus_disabled(void);
_nodiscard _nothrow const char* utils_iomanip_report_past_enable(void); // Reports for pasted data: "ESC[200~{data}ESC[201~"
_nodiscard _nothrow const char* utils_iomanip_report_past_disable(void);

/* sequences with arguments (valid until the next call of the same function in the thread, NULL on error) & _r (copied in buf, truncated & always '\0' terminated, returns the full length like snprintf, 0 on error) */
// internal
_nodiscard _nothrow _null_terminated_string_arg(1) const char* utils_iomanip_csi(const char* code); // ESC[ + code
_nodiscard _nothrow _null_terminated_string_arg(1) _write_only(2, 3) size_t utils_iomanip_csi_r(const char* code, char* buf, size_t size);
// special
_nodiscard _nothrow _null_terminated_string_arg(1) _null_terminated_string_arg(2) const char* utils_iomanip_hyperlink(const char* display, const char* link);
_nodiscard _nothrow _null_terminated_string_arg(1) _null_terminated_string_arg(2) _write_only(3, 4) size_t utils_iomanip_hyperlink_r(const char* display, const char* link, char* buf, size_t size);
_nodiscard _nothrow _null_terminated_string_arg(1) _null_terminated_string_arg(2) const char* utils_iomanip_file_hyperlink(const char* display, const char* path);
_nodiscard _nothrow _null_terminated_string_arg(1) _null_terminated_string_arg(2) _write_only(3, 4) size_t utils_iomanip_file_hyperlink_r(const char* display, const char* path, char* buf, size_t size);
// style
_nodiscard _nothrow const char* utils_iomanip_set_style(utils_iomanip_style_t style);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_set_style_r(utils_iomanip_style_t style, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_set_styles(const utils_iomanip_style_t* styles, size_t count); // every style in one sequence (utils::iomanip::set_style({...}))
_nodiscard _nothrow _write_only(3, 4) size_t utils_iomanip_set_styles_r(const utils_iomanip_style_t* styles, size_t count, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_reset_style(utils_iomanip_reset_style_t style);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_reset_style_r(utils_iomanip_reset_style_t style, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_reset_styles(const utils_iomanip_reset_style_t* styles, size_t count); // every reset in one sequence (utils::iomanip::reset_style({...}))
_nodiscard _nothrow _write_only(3, 4) size_t utils_iomanip_reset_styles_r(const utils_iomanip_reset_style_t* styles, size_t count, char* buf, size_t size);
// color
_nodiscard _nothrow const char* utils_iomanip_color(utils_iomanip_color_t color);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_color_r(utils_iomanip_color_t color, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_back_color(utils_iomanip_back_color_t color);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_back_color_r(utils_iomanip_back_color_t color, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_color_id(uint8_t id); // 256 colors
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_color_id_r(uint8_t id, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_back_color_id(uint8_t id);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_back_color_id_r(uint8_t id, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_underline_color_id(uint8_t id);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_underline_color_id_r(uint8_t id, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_color_rgb(uint8_t r, uint8_t g, uint8_t b);
_nodiscard _nothrow _write_only(4, 5) size_t utils_iomanip_color_rgb_r(uint8_t r, uint8_t g, uint8_t b, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_back_color_rgb(uint8_t r, uint8_t g, uint8_t b);
_nodiscard _nothrow _write_only(4, 5) size_t utils_iomanip_back_color_rgb_r(uint8_t r, uint8_t g, uint8_t b, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_underline_color_rgb(uint8_t r, uint8_t g, uint8_t b);
_nodiscard _nothrow _write_only(4, 5) size_t utils_iomanip_underline_color_rgb_r(uint8_t r, uint8_t g, uint8_t b, char* buf, size_t size);
// cursor
_nodiscard _nothrow const char* utils_iomanip_up(size_t n);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_up_r(size_t n, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_down(size_t n);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_down_r(size_t n, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_right(size_t n);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_right_r(size_t n, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_left(size_t n);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_left_r(size_t n, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_next_line(size_t n);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_next_line_r(size_t n, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_previous_line(size_t n);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_previous_line_r(size_t n, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_column(size_t col);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_column_r(size_t col, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_pos(size_t row, size_t col);
_nodiscard _nothrow _write_only(3, 4) size_t utils_iomanip_pos_r(size_t row, size_t col, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_scroll_up(size_t n);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_scroll_up_r(size_t n, char* buf, size_t size);
_nodiscard _nothrow const char* utils_iomanip_scroll_down(size_t n);
_nodiscard _nothrow _write_only(2, 3) size_t utils_iomanip_scroll_down_r(size_t n, char* buf, size_t size);

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* IOMANIP_CAPI_C_H */
