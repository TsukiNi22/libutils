+++
title = "IOManip"
summary = "Functions returning the ANSI escape sequences (styles, colors, cursor, screen, modes, links) and readers of the terminal reports."
keywords = "iomanip ansi escape sequence color style bold underline cursor screen clear hyperlink mouse report enable_ansi utils_iomanip"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_IOManip` (from `_Manip`) | `_Tools` | `utils::iomanip` | `utils/manip/iomanip/` |

Functions returning the ANSI escape sequences (to write on a terminal) and readers of the terminal reports. The string tools of the same group are in [SManip](page:smanip).

## Styles {#styles}

| Function | Sequence |
|---|---|
| `reset()` | `ESC[0m` |
| `strong()`, `dark()`, `italic()`, `underlined()` | `1`, `2`, `3`, `4` |
| `flashing_slow()`, `flashing_fast()` | `5`, `6` |
| `reversed()`, `hide()`, `bar()` | `7`, `8`, `9` |
| `monospace()`, `framed()`, `encircled()`, `overlined()` | `50`, `51`, `52`, `53` |
| `exposant()`, `indice()` | `73`, `74` (rarely supported) |
| `<style>_reset()` | the matching reset (`strong_reset()` / `dark_reset()` = `22`, `flashing_*_reset()` = `25`...) |
| `set_style(Style)` / `set_style({Style, ...})` | one sequence for several `Style` values |
| `reset_style(ResetStyle)` / `reset_style({ResetStyle, ...})` | same for the resets |

> [!NOTE]
> The codes follow ECMA-48 / xterm since `v3.0.0` (the slow / fast blink were swapped, the bold reset was `21`).

## Colors {#colors}

| Function | Description |
|---|---|
| `color(Color)` / `color(BackColor)` | 16 colors (`Black`... `BrightWhite`, `Default`) for the text / background |
| `color_id(id)`, `back_color_id(id)`, `underline_color_id(id)` | 256 colors palette |
| `color_rgb(r, g, b)`, `back_color_rgb(r, g, b)`, `underline_color_rgb(r, g, b)` | True colors |

## Cursor and screen {#cursor-screen}

| Function | Description |
|---|---|
| `up(n)`, `down(n)`, `right(n)`, `left(n)` | Move the cursor |
| `next_line(n)`, `previous_line(n)`, `column(col)`, `pos(row, col)` | Move to a line / column / position |
| `save_cur()`, `load_cur()`, `show_cur()`, `hide_cur()` | Cursor state |
| `screen()`, `screen_start()`, `screen_end()`, `scrollback_buffer()` | Clear the screen (all / to the start / to the end / history) |
| `line()`, `line_start()`, `line_end()` | Clear the line |
| `scroll_up(n)`, `scroll_down(n)` | Scroll |
| `save_screen()`, `load_screen()` | Alternate screen |
| `hyperlink(display, link)`, `file_hyperlink(display, path)` | Clickable links (OSC 8) |
| `esc()`, `csi(code)` | Raw `ESC` / `ESC[` + code |

## Modes {#modes}

| Functions | Description |
|---|---|
| `wrapping_enable()` / `wrapping_disable()` | Line wrapping |
| `inverted_color_enable()` / `inverted_color_disable()` | Inverted screen colors |
| `mouse_tracking_enable()` / `_disable()` | Mouse clicks reports (classic) |
| `mouse_move_tracking_enable()` / `_disable()` | Mouse moves reports |
| `mouse_adv_tracking_enable()` / `_disable()` | SGR mouse reports |
| `report_focus_enable()` / `report_focus_disabled()` | Focus in / out (`ESC[I` / `ESC[O`) |
| `report_past_enable()` / `report_past_disable()` | Bracketed paste (`ESC[200~ ... ESC[201~`) |
| `enable_ansi()` | Windows: enable the virtual terminal mode of the console (the sequences are interpreted), call it once at the start; `true` on Unix (nothing to do) |

## Reports {#reports}

| Function | Description |
|---|---|
| `get_pos()` | Ask the cursor position (the terminal answers `ESC[row;colR`) |
| `read_cursor_position()` | Read that answer on stdin, `{row, col}` (`{-1, -1}` when invalid) |
| `read_mouse_event()` | Read a classic mouse report (`ESC[M` + 3 bytes), returns `MouseEvent{button, x, y}` |
| `read_advanced_mouse_event()` | Read an SGR report (`ESC[<b;x;yM`), returns `AdvancedMouseEvent{button, x, y, pressed}` |

`MouseButton`: `Left`, `Right`, `Middle`, `Release`, `Unknown`. Invalid reports throw `ANSIMouseEvent`.

## Special characters {#special-characters}

`utils::iomanip::Char` (`enum class: char`): the ASCII control characters `NUL`, `SOH`... `ESC`, `DEL` and `EOF` (`static_cast<char>(utils::iomanip::Char::ESC)`).

## Example {#example}

```cpp
#define _IOManip
#include <utils/utils.hpp>

std::cout << utils::iomanip::set_style({utils::iomanip::Style::Strong, utils::iomanip::Style::Underlined})
          << utils::iomanip::color(utils::iomanip::Color::Red) << "error" << utils::iomanip::reset()
          << " see " << utils::iomanip::hyperlink("the docs", "https://tsukini22.github.io/libutils/docs/") << std::endl;
```

## In C {#in-c}

Every sequence is a string: `utils_iomanip_<name>` with the C++ name. A sequence without argument is a static string; a sequence with arguments is kept in a thread local buffer valid until the next call of the same function in the thread, and its `_r` version copies it in a buffer of yours (returns the full length like `snprintf`). The enums are `utils_iomanip_color_t` (`C_API_IOMANIP_COLOR_*`), `utils_iomanip_back_color_t`, `utils_iomanip_style_t` (`C_API_IOMANIP_STYLE_*`), `utils_iomanip_reset_style_t`, `utils_iomanip_char_t` (`C_API_IOMANIP_CHAR_*`), `utils_iomanip_mouse_button_t`.

| C++ | C |
|---|---|
| `set_style({...})` | `utils_iomanip_set_styles(styles, count)` (`utils_iomanip_set_style` for one) |
| `color(Color::Red)` | `utils_iomanip_color(C_API_IOMANIP_COLOR_RED)` |
| `pos(row, col)` | `utils_iomanip_pos(row, col)` / `utils_iomanip_pos_r(row, col, buf, size)` |
| `read_cursor_position()` | `utils_iomanip_read_cursor_position(&row, &col)` |
| `read_mouse_event()` | `utils_iomanip_read_mouse_event(&event)` (`utils_iomanip_mouse_event_t`) |

```c
#define _IOManip
#include <utils.h>
#include <stdio.h>

int main(void)
{
    utils_iomanip_style_t styles[] = {C_API_IOMANIP_STYLE_STRONG, C_API_IOMANIP_STYLE_UNDERLINED};
    char link[256];

    utils_iomanip_enable_ansi(); // Windows console: interpret the sequences
    utils_iomanip_hyperlink_r("the docs", "https://tsukini22.github.io/libutils/docs/", link, sizeof(link));
    printf("%s%serror%s see %s\n", utils_iomanip_set_styles(styles, 2), utils_iomanip_color(C_API_IOMANIP_COLOR_RED),
        utils_iomanip_reset(), link);
    return 0;
}
```
