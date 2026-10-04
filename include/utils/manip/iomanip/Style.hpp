/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 11/05/2026 by @author Tsukini

File Name:
##  @file Style.hpp

File Description:
##  Define of the different style used in ANSI
\**************************************************************/

#ifndef STYLE_H
    #define STYLE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <cstdint>  // std::uint8_t

namespace utils::iomanip { // namespace start
//----------------------------------------------------------------//
/* ENUM */

/* style */
enum class Style: std::uint8_t {
    Strong = 1,
    Dark,
    Italic,
    Underlined,
    FlashingSlow,   // 5 (ECMA-48: slowly blinking)
    FlashingFast,   // 6 (ECMA-48: rapidly blinking)
    Reversed,
    Hide,
    Bar,
    Monospace = 50,
    Framed,         // Rarely supported
    Encircled,      // Rarely supported
    Overlined,
    Exposant = 73,  // Rarely supported
    Indice,         // Rarely supported
};

/* reset style */
enum class ResetStyle: std::uint8_t {
    All = 0,
    Strong = 22,        // 22 = normal intensity (21 is a double underline on most terminals)
    Dark = 22,
    Italic,
    Underlined,
    FlashingFast,       // 25 = steady (no blinking)
    FlashingSlow = 25,
    Reversed = 27,
    Hide,
    Bar,
    FramedEncircled = 54,
    Overlined,
    UnderlineColor = 59,
    ExposantIndice = 75,
};

} // namespace end
#endif /* STYLE_H */
