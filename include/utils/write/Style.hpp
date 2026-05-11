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
    #include <iostream>

namespace utils::write { // namespace start
//----------------------------------------------------------------//
/* TYPDEF */

/* style */
enum class Style: std::uint8_t {
    Strong = 1,
    Dark,
    Italic,
    Underlined,
    FlashingFast,
    FlashingSlow,
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
    Strong = 21,
    Dark,
    Italic,
    Underlined,
    FlashingFast,
    FlashingSlow,
    Reversed,
    Hide,
    Bar,
    FramedEncircled = 54,
    Overlined,
    UnderlineColor = 59,
    ExposantIndice = 75,
};

} // namespace end
#endif /* STYLE_H */
