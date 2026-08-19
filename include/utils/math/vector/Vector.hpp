/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/08/2026 by @author Tsukini

File Name:
##  @file Vector.hpp

File Description:
##  Include for all the different vector
\**************************************************************/

#ifndef VECTOR_H
    #define VECTOR_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* interface */
    #include "IVector.hpp"  // utils::math::vector::IVector

    /* vector */
    #include "Vector2.hpp"  // utils::math::vector::Vector2
    #include "Vector3.hpp"  // utils::math::vector::Vector3
    #include "OVector2.hpp" // utils::math::vector::OVector2
    #include "OVector3.hpp" // utils::math::vector::OVector3

#endif /* VECTOR_H */
