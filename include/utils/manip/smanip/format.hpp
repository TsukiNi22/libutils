/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 18/05/2026 by @author Tsukini

File Name:
##  @file Format.hpp

File Description:
##  Definition of the utils::iomanip::format & explication
\**************************************************************/

#ifndef FORMAT_H
    #define FORMAT_H
    
    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <string>   // std::string

namespace utils::iomanip { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

/* format */
std::string format(const std::string& s);

//----------------------------------------------------------------//
/* DESCRIPTION */
/*
Only edit style for now, other sequence style require manual call

Delimitor: <...>
Empty content: <>
    - reset style
Content: <DATA1|DATA2|...|DATAX>
    - setup the given style
    - ignore the unknow ones

Data (case insensitive):
    reset
    strong
    dark
    italic
    underlined
    flashing_fast
    flashing_slow
    reversed
    hide
    bar
    monospace
    framed
    encircled
    overlined
    exposant
    indice

not for now:
    color(name)
    b_color(name)
    id_color(id)
    id_b_color(id)
    id_ucolor(id)
    color(r, g, b)
    b_color(r, g, b)
    u_color(r, g, b)

*/

} // namespace end
#endif /* FORMAT_H */
