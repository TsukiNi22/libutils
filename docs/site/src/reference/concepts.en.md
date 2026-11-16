+++
title = "Concepts"
summary = "C++20 concepts that constrain the templates of the library (vectors, matrices...), usable in your own templates."
keywords = "concepts c++20 requires Addable AddableWith Comparable Arithmetic Convertible Streamable Swappable operation"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_Concepts` | `_Tools` | `utils::concepts` | `utils/concepts/` |

C++20 concepts used to constrain the templates of the library (vectors, matrices...), usable in your own templates.

## Global {#global}

`utils/concepts/GlobalConcepts.hpp`

| Concept | Satisfied when |
|---|---|
| `Convertible<T, U>` | `T` can be converted to `U` |
| `ConvertibleTo<T, U>` | `T` is implicitly convertible to `U` and `static_cast<U>(t)` is valid |
| `Swappable<T>` | `std::swap(a, b)` is valid |
| `Streamable<T>` | `std::ostream << t` is valid |

> [!WARNING]
> `convertible_to` (renamed `ConvertibleTo` in `v3.0.0`) was removed in `v4.0.0`.

## Operations {#operations}

`utils/concepts/OperationConcepts.hpp`: each one exists for the same type (`Addable<T>`) and between two types (`AddableWith<T, U>`).

| Concept | Operator |
|---|---|
| `Addable` / `AddableWith` | `a + b` |
| `Subtractable` / `SubtractableWith` | `a - b` |
| `Multipliable` / `MultipliableWith` | `a * b` |
| `Divisible` / `DivisibleWith` | `a / b` |
| `BitwiseAndable` / `BitwiseAndableWith` | `a & b` |
| `BitwiseOrable` / `BitwiseOrableWith` | `a \| b` |
| `BitwiseXorable` / `BitwiseXorableWith` | `a ^ b` |
| `Shiftable` / `ShiftableWith` | `a << b`, `a >> b` |
| `AddAssignable` / `AddAssignableWith` | `a += b` |
| `SubtractAssignable` / `SubtractAssignableWith` | `a -= b` |
| `MultiplyAssignable` / `MultiplyAssignableWith` | `a *= b` |
| `DivideAssignable` / `DivideAssignableWith` | `a /= b` |
| `EqualityComparable` / `EqualityComparableWith` | `a == b`, `a != b` |
| `Comparable` / `ComparableWith` | `<`, `<=`, `>`, `>=` |
| `Incrementable` / `Decrementable` | `++a`, `a++` / `--a`, `a--` |
| `Negatable` | `-a` |
| `Arithmetic` / `ArithmeticWith` | `+`, `-`, `*`, `/` (the 4 concepts above) |

## Example {#example}

```cpp
#define _Concepts
#include <utils/utils.hpp>

template<typename T, typename U>
requires utils::concepts::AddableWith<T, U>
auto sum(const T& a, const U& b) {return a + b;}
```
