/**************************************************************\
Edition:
##  @date 03/03/2026 by @author Tsukini

File Name:
##  @file Color.hpp

File Description:
##  Definition of color used in ANSI escape sequences
\**************************************************************/

#ifndef COLOR_H
    #define COLOR_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <cstdint>  // std::uint8_t

namespace utils::iomanip { // namespace start
//----------------------------------------------------------------//
/* TYPEDEF */

/* color */
enum class Color: std::uint8_t {
    Black = 30,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    BrightBlack = 90,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite,
    Default = 39,
};

/* back color */
enum class BackColor: std::uint8_t {
    Black = 40,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    BrightBlack = 100,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite,
    Default = 49,
};

} // namespace end
#endif /* COLOR_H */
