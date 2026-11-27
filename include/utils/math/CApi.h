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
##  C API of utils::math: the math types (utils::math::Type = double), geometry & trigonometry (C & C++ compatible)
##  The structures are given by value (same fields as the utils::math types), the math functions can't fail
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef MATH_CAPI_C_H
    #define MATH_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _nodiscard, _nothrow, _const
    #include <stdbool.h>                    // bool
    #include <stdint.h>                     // int32_t, uint64_t

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

typedef double utils_math_type_t; // utils::math::Type: coord, angle, direction, ...
typedef uint64_t utils_math_utype_t; // utils::math::UType: unsigned coord, angle, direction, ...
typedef utils_math_type_t utils_math_angle_t; // utils::math::Angle: generaly in deg

//----------------------------------------------------------------//
/* STRUCT */

/* 2D (utils::math::Coord2D / V2Type, UCoord2D / V2UType) */
typedef struct utils_math_coord2d {
    utils_math_type_t x;
    utils_math_type_t y;
} utils_math_coord2d_t;

typedef struct utils_math_ucoord2d {
    utils_math_utype_t x;
    utils_math_utype_t y;
} utils_math_ucoord2d_t;

/* 3D (utils::math::Coord / V3Type / Direction, UCoord / V3UType) */
typedef struct utils_math_coord {
    utils_math_type_t x;
    utils_math_type_t y;
    utils_math_type_t z;
} utils_math_coord_t;

typedef struct utils_math_ucoord {
    utils_math_utype_t x;
    utils_math_utype_t y;
    utils_math_utype_t z;
} utils_math_ucoord_t;

typedef utils_math_coord_t utils_math_direction_t; // utils::math::Direction: generaly normalized

/* spacial partitionning (utils::math::Chunk) */
typedef struct utils_math_chunk {
    int32_t x;
    int32_t y;
    int32_t z;
} utils_math_chunk_t;

/* utils::math::CFrame */
typedef struct utils_math_cframe {
    utils_math_coord_t position;
    utils_math_direction_t orientation; // in deg
    utils_math_direction_t look; // normalized
} utils_math_cframe_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* trigonometry */
_nodiscard _nothrow _const utils_math_type_t utils_math_deg_to_rad(utils_math_angle_t deg);
_nodiscard _nothrow _const utils_math_angle_t utils_math_rad_to_deg(utils_math_type_t rad);

/* geometry (rad: the angle / orientation is in radian, else in deg) */
_nodiscard _nothrow _const utils_math_direction_t utils_math_to_look(utils_math_direction_t orientation); // orientation in deg -> look vector normalized
_nodiscard _nothrow _const utils_math_coord2d_t utils_math_rotate_point_2d(utils_math_coord2d_t origin, utils_math_coord2d_t point, utils_math_angle_t angle, bool rad);
_nodiscard _nothrow _const utils_math_coord_t utils_math_rotate_point_3d(utils_math_coord_t origin, utils_math_coord_t point, utils_math_direction_t orientation, bool rad);

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* MATH_CAPI_C_H */
