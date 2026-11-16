+++
title = "Math"
summary = "Types for 2D / 3D coordinates and angles, point rotation, look vector and degree / radian conversion."
keywords = "math coordinate angle direction CFrame chunk geometry rotate_point_2D rotate_point_3D to_look trigo deg_to_rad rad_to_deg utils_math"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Math` | `_Tools` | `utils::math` | `utils/math/` |
| `_Geometry` (from `_Math`) | `_Math` | `utils::math::geometry` | `utils/math/geometry/` |
| `_Trigo` (from `_Math`) | `_Math` | `utils::math::trigo` | `utils/math/trigo/Convertion.hpp` |

Types and functions for 2D / 3D coordinates and angles, built on the [vector](page:type#vector) types.

## Types {#types}

`utils/math/MathType.hpp`:

| Type | Definition |
|---|---|
| `Type` / `UType` | `double` / `std::uint64_t`, used for every coordinate and angle |
| `Coord2D` (`V2Type`), `UCoord2D` (`V2UType`) | `OVector2<Type>`, `OVector2<UType>` |
| `Coord` (`V3Type`), `UCoord` (`V3UType`) | `OVector3<Type>`, `OVector3<UType>` |
| `Angle` | `Type`, in degrees by default |
| `Direction` | `OVector3<Angle>` (orientation or normalized direction) |
| `Chunk` | `OVector3<std::int32_t>`, for spatial partitioning |
| `CFrame` | `{Coord position, Direction orientation (deg), Direction look (normalized)}` |

## Geometry {#geometry}

| Function | Description |
|---|---|
| `rotate_point_2D(origin, point, angle, rad = false)` | Rotate `point` around `origin` by `angle` (degrees, radians with `rad = true`) |
| `rotate_point_3D(origin, point, orientation, rad = false)` | Rotate `point` around `origin` by the `orientation` angles `{pitch (x), yaw (y), roll (z)}` |
| `to_look(orientation)` | Normalized look vector of an orientation |

> [!NOTE]
> Convention: **y up, forward = +z**. `rotate_point_3D` and `to_look` use the same one: rotating the forward vector `(0, 0, 1)` by an orientation gives `to_look(orientation)`.

```cpp
#define _Geometry
#include <utils/utils.hpp>

utils::math::Coord2D p = utils::math::geometry::rotate_point_2D({0, 0}, {1, 0}, 90); // ~(0, 1)
utils::math::Direction look = utils::math::geometry::to_look({0, 90, 0});           // ~(1, 0, 0)
```

## Trigo {#trigo}

| Function | Description |
|---|---|
| `deg_to_rad(deg)` | Degrees to radians |
| `rad_to_deg(rad)` | Radians to degrees |

```cpp
#define _Trigo
#include <utils/utils.hpp>

utils::math::Type rad = utils::math::trigo::deg_to_rad(180); // pi
```

## In C {#in-c}

The math types are plain C structures passed by value (`utils_math_type_t` = `double`).

| C++ | C |
|---|---|
| `Coord2D` / `UCoord2D` | `utils_math_coord2d_t` / `utils_math_ucoord2d_t` `{x, y}` |
| `Coord` / `UCoord` / `Direction` | `utils_math_coord_t` / `utils_math_ucoord_t` / `utils_math_direction_t` `{x, y, z}` |
| `Chunk`, `CFrame` | `utils_math_chunk_t`, `utils_math_cframe_t {position, orientation, look}` |
| `rotate_point_2D` / `rotate_point_3D` / `to_look` | `utils_math_rotate_point_2d` / `utils_math_rotate_point_3d` / `utils_math_to_look` |
| `deg_to_rad` / `rad_to_deg` | `utils_math_deg_to_rad` / `utils_math_rad_to_deg` |

```c
#define _Geometry
#define _Trigo
#include <utils.h>
#include <stdio.h>

int main(void)
{
    utils_math_coord2d_t origin = {0, 0};
    utils_math_coord2d_t point = {1, 0};
    utils_math_coord2d_t p = utils_math_rotate_point_2d(origin, point, 90, false);    // ~(0, 1)
    utils_math_direction_t look = utils_math_to_look((utils_math_direction_t){0, 90, 0}); // ~(1, 0, 0)

    printf("(%.2f, %.2f) (%.2f, %.2f, %.2f) %f\n", p.x, p.y, look.x, look.y, look.z, utils_math_deg_to_rad(180));
    return 0;
}
```
