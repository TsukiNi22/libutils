+++
title = "Math"
summary = "Types pour les coordonnées et angles 2D / 3D, rotation de points, vecteur de visée et conversion degrés / radians."
keywords = "math coordonnée angle direction CFrame chunk géométrie rotate_point_2D rotate_point_3D to_look trigo deg_to_rad rad_to_deg utils_math"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_Math` | `_Tools` | `utils::math` | `utils/math/` |
| `_Geometry` (depuis `_Math`) | `_Math` | `utils::math::geometry` | `utils/math/geometry/` |
| `_Trigo` (depuis `_Math`) | `_Math` | `utils::math::trigo` | `utils/math/trigo/Convertion.hpp` |

Types et fonctions pour les coordonnées et angles 2D / 3D, construits sur les types [vecteur](page:type#vector).

## Types {#types}

`utils/math/MathType.hpp` :

| Type | Définition |
|---|---|
| `Type` / `UType` | `double` / `std::uint64_t`, utilisés pour chaque coordonnée et chaque angle |
| `Coord2D` (`V2Type`), `UCoord2D` (`V2UType`) | `OVector2<Type>`, `OVector2<UType>` |
| `Coord` (`V3Type`), `UCoord` (`V3UType`) | `OVector3<Type>`, `OVector3<UType>` |
| `Angle` | `Type`, en degrés par défaut |
| `Direction` | `OVector3<Angle>` (orientation ou direction normalisée) |
| `Chunk` | `OVector3<std::int32_t>`, pour le partitionnement spatial |
| `CFrame` | `{Coord position, Direction orientation (deg), Direction look (normalized)}` |

## Géométrie {#geometry}

| Fonction | Description |
|---|---|
| `rotate_point_2D(origin, point, angle, rad = false)` | Fait tourner `point` autour de `origin` de `angle` (degrés, radians avec `rad = true`) |
| `rotate_point_3D(origin, point, orientation, rad = false)` | Fait tourner `point` autour de `origin` des angles de `orientation` `{pitch (x), yaw (y), roll (z)}` |
| `to_look(orientation)` | Vecteur de visée normalisé d'une orientation |

> [!NOTE]
> Convention : **y vers le haut, avant = +z**. `rotate_point_3D` et `to_look` utilisent la même : faire tourner le vecteur avant `(0, 0, 1)` d'une orientation donne `to_look(orientation)`.

```cpp
#define _Geometry
#include <utils/utils.hpp>

utils::math::Coord2D p = utils::math::geometry::rotate_point_2D({0, 0}, {1, 0}, 90); // ~(0, 1)
utils::math::Direction look = utils::math::geometry::to_look({0, 90, 0});           // ~(1, 0, 0)
```

## Trigo {#trigo}

| Fonction | Description |
|---|---|
| `deg_to_rad(deg)` | Degrés vers radians |
| `rad_to_deg(rad)` | Radians vers degrés |

```cpp
#define _Trigo
#include <utils/utils.hpp>

utils::math::Type rad = utils::math::trigo::deg_to_rad(180); // pi
```

## En C {#in-c}

Les types math sont de simples structures C passées par valeur (`utils_math_type_t` = `double`).

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
