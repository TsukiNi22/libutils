+++
title = "Type"
summary = "Custom types: 2D / 3D vectors, matrices, the bidirectional lookup table, and the Freezable / Worker base classes."
keywords = "type vector Vector2 Vector3 OVector2 OVector3 matrix Matrix OMatrix det invert transpose BLT BidirectionalLookupTable Freezable freeze Worker utils_vector2 utils_matrix utils_blt utils_freezable utils_worker"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_CustomType` | `_Tools` | `utils::type` | `utils/type/` |
| `_Vector` / `_Matrix` / `_BLT` (from `_CustomType`) | `_CustomType` | `utils::type` | `utils/type/vector/`, `utils/type/matrix/`, `utils/type/blt/` |

Custom types used by the library and usable by your project.

| Part | Define | Content |
|---|---|---|
| [Vector](#vector) | `_Vector` | `Vector2<T>`, `Vector3<T>` (checked) and `OVector2<T>`, `OVector3<T>` (optimized) |
| [Matrix](#matrix) | `_Matrix` | `Matrix<T>` (checked) and `OMatrix<T>` (optimized) |
| [BLT](#blt) | `_BLT` | `BidirectionalLookupTable<L, R>` |
| [Freezable](#freezable), [Worker](#worker) | `_CustomType` | Base classes |

## Vector {#vector}

Vectors of 2 (`x`, `y`) or 3 (`x`, `y`, `z`) values of any type, with every operator.

| Class | Description |
|---|---|
| `Vector2<T>`, `Vector3<T>` | **Checked**: derive `IVector<T>` (instance tracking), every operation is constrained by a [concept](page:concepts), the operations between 2 types return the common type (`Vector2<int> + Vector2<double>` → `Vector2<double>`) |
| `OVector2<T>`, `OVector3<T>` | **Optimized**: plain struct, no tracking, the operations keep `T` (used by the [Math](page:math) types) |

| Method | Description |
|---|---|
| `x`, `y` (, `z`) | Public members |
| `get(i)` / `operator[](i)` | Value by index (`VectorInvalidIndex` when out of range) |
| `min(v)`, `max(v)`, `clamp(min, max)` | Per component |
| `dot(v)` | Dot product |
| `cross(v)` | `Vector2`: scalar `x1 * y2 - y1 * x2`; `Vector3`: vector |
| `length()`, `lengthSquared()` | Norm (truncated for the integer types) |
| `normalize()` | Unit vector (a zero vector stays zero) |
| `sign()` | `-1`, `0` or `1` per component |

Operators: `+ - * /` (with a vector or a scalar, on both sides), `+= -= *= /=`, `& | ^`, `++ --`, unary `-`, `== !=`, `< <= > >=` (all the components), `<<` (stream: `(x, y)`).

```cpp
#define _Vector
#include <utils/utils.hpp>

utils::type::Vector2<int> a(1, 2);
utils::type::Vector2<double> b = a + utils::type::Vector2<double>(0.5, 0.5); // (1.5, 2.5)
utils::type::Vector3<double> up = utils::type::Vector3<double>(1, 0, 0).cross(utils::type::Vector3<double>(0, 1, 0)); // (0, 0, 1)
std::cout << b << " " << b.length() << std::endl;
```

## Matrix {#matrix}

Matrices of `x` rows and `y` columns of any type (`m(x, y)`), with the usual operations.

| Class | Storage | Description |
|---|---|---|
| `Matrix<T>` | `std::vector<std::vector<T>>` | **Checked**: every operation is constrained by a [concept](page:concepts), the operations between 2 types return the common type, `invert` / `/` disabled for integers |
| `OMatrix<T>` | contiguous, row-major | **Optimized**: no concept and no tracking, the operations keep `T` (`invert` of an integer matrix truncates) |

| Method | Description |
|---|---|
| `Matrix(n)` / `Matrix(x, y)` | Square `n x n` / `x` rows, `y` columns, default values |
| `at(x, y)` / `operator()(x, y)` | Value (`OutOfBounds` out of the matrix) |
| `set(x, y, value)` | Set one value |
| `set(begin, end, x = 0, y = 0)` | Fill row by row from `(x, y)` |
| `row(n)`, `col(n)` | A row / column as `std::vector<T>` |
| `row()`, `col()`, `size()` | Number of rows / columns / both |
| `swap(a, b)`, `swapRow(a, b)`, `swapCol(a, b)` | Swap values / rows / columns |
| `transpose()` | Transpose in place |
| `det()`, `trace()` | Determinant / trace (square matrix) |
| `invert()` | Invert in place (`MatrixSingular` when the determinant is 0) |
| `clear()` | Reset every value |

Operators: `+ -` (same size), `*` (matrix product, or scalar on both sides), `/` (`this * m^-1`, or scalar), the assignment versions, unary `-`, `== !=`, `<<` (stream).

> [!WARNING]
> Sizes that don't match throw `InvalidArgument`; `det`, `trace` and `invert` need a square matrix.

```cpp
#define _Matrix
#include <utils/utils.hpp>

utils::type::Matrix<double> m(2);
std::vector<double> values = {4, 7, 2, 6};
m.set(values.begin(), values.end());   // [[4, 7], [2, 6]]
std::cout << m.det() << std::endl;      // 10
utils::type::Matrix<double> inv = m;
inv.invert();
std::cout << m * inv << std::endl;      // identity
```

## BLT {#blt}

`BidirectionalLookupTable<L, R>`: a map where each pair can be found from both sides (`table[left] → right`, `table[right] → left`). It is a [Freezable](#freezable): once frozen it can't be edited anymore.

| Method | Description |
|---|---|
| `addElement(left, right)` | Add a pair (`Override` when one side already exists) |
| `setElement<force>(left, right)` | Same, `force = true` replaces the existing pairs of both sides |
| `operator[](left)` / `operator[](right)` | The other side of the pair (`UnknownKey` when missing) |
| `removeElement(side)` / `removeElements(sides)` | Remove the pairs (unknown key: warning on `std::cerr`) |
| `clear()` | Remove everything |
| `freeze()`, `isFrozen()` | From `Freezable`; editing a frozen table throws `Frozen` |

Custom hash / equality: `BidirectionalLookupTable<L, R, HashL, HashR, EqualL, EqualR>`. When both sides have the same type, use `BLT_TYPE(T)` (`BidirectionalLookupTable<BLT_TYPE(std::string)>`): an element is looked up on both sides with `operator[](element)`.

```cpp
#define _BLT
#include <utils/utils.hpp>

utils::type::BidirectionalLookupTable<int, std::string> codes;
codes.addElement(404, std::string("Not Found"));
codes.addElement(500, std::string("Internal Server Error"));
codes.freeze();
std::cout << codes[404] << " / " << codes[std::string("Not Found")] << std::endl; // Not Found / 404
```

## Freezable {#freezable}

Base class of an object that can be locked against edition (used by `BidirectionalLookupTable`).

| Method | Description |
|---|---|
| `Freezable(frozen = false)` | Initial state |
| `freeze()` / `isFrozen()` | Freeze for good / state |
| `requireUnfrozen()` | Throw `Frozen` when frozen (call it at the start of an edition method) |
| `requireFrozen()` | Throw `ShouldBeFrozen` when not frozen |

## Worker {#worker}

Base class of the workers of a [LoadBalancer](page:system#loadbalancer).

| Method | Description |
|---|---|
| `setWorkingStatus(bool)` | Mark the worker working / free (give it back to the balancer with `false`) |
| `isWorking()` | State |
| `getStopedWorkingTimestamp()` | When it became free (used by the lifespan of the balancer) |

## In C {#in-c}

Each template is generated for the [fixed types](page:c-api-design#fixed-types) (`<name>` = `i8` ... `u64`, `size`, `f32`, `f64`, `f128`).

| C++ | C |
|---|---|
| `OVector2<T>` / `OVector3<T>` | `utils_vector2_<name>_t` / `utils_vector3_<name>_t` **by value** (`{x, y}` / `{x, y, z}`): `utils_vector2_<name>_add(a, b)`, `_dot`, `_cross`, `_length`, `_normalize`... (`_and` / `_or` / `_xor` for the integers) |
| `OMatrix<T>` | handle `utils_matrix_<name>_t*`: each operation exists 3 times, `utils_matrix_<name>_add(a, b)` (a new handle), `_add_r(a, b, out)` (in a given handle), `_add_assign(a, b)` (in place); `_div` / `_invert` for the floating points |
| `BidirectionalLookupTable` | `utils_blt_str_str_t*`, `utils_blt_size_size_t*`, `utils_blt_i64_i64_t*` (one type), `utils_blt_str_<name>_t*` / `utils_blt_<name>_str_t*` (string ↔ integer), `utils_blt_t*` (bytes) |
| `Freezable` / `Worker` of any handle | `utils_freezable_*(handle)`, `utils_worker_*(handle)` (an error when the handle has no such base) |

```c
#define _Vector
#define _Matrix
#define _BLT
#include <utils.h>
#include <stdio.h>

int main(void)
{
    utils_vector3_f64_t up = utils_vector3_f64_cross((utils_vector3_f64_t){1, 0, 0}, (utils_vector3_f64_t){0, 1, 0}); // (0, 0, 1)
    utils_matrix_f64_t* m = utils_matrix_f64_create(2, 2);
    utils_matrix_f64_t* inv = NULL;
    utils_blt_i32_str_t* codes = utils_blt_i32_str_create();
    double values[] = {4, 7, 2, 6};
    const char* text = NULL;

    if (!m || !codes)
        return 1;
    utils_matrix_f64_set_values(m, values, 4, 0, 0);          // [[4, 7], [2, 6]]
    inv = utils_matrix_f64_invert(m);                         // a new handle
    utils_blt_i32_str_add(codes, 404, "Not Found");
    utils_freezable_freeze(codes);                            // Freezable of any handle
    utils_blt_i32_str_get_right(codes, 404, &text);
    printf("(%g, %g, %g) %s frozen: %d\n", up.x, up.y, up.z, text, utils_freezable_is_frozen(codes));
    utils_matrix_f64_destroy(inv);
    utils_matrix_f64_destroy(m);
    utils_blt_i32_str_destroy(codes);
    return 0;
}
```
