+++
title = "Concepts"
summary = "Concepts C++20 qui contraignent les templates de la bibliothèque (vecteurs, matrices...), utilisables dans vos propres templates."
keywords = "concepts c++20 requires Addable AddableWith Comparable Arithmetic Convertible Streamable Swappable opération"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_Concepts` | `_Tools` | `utils::concepts` | `utils/concepts/` |

Concepts C++20 utilisés pour contraindre les templates de la bibliothèque (vecteurs, matrices...), utilisables dans vos propres templates.

## Globaux {#global}

`utils/concepts/GlobalConcepts.hpp`

| Concept | Satisfait quand |
|---|---|
| `Convertible<T, U>` | `T` peut être converti en `U` |
| `ConvertibleTo<T, U>` | `T` est implicitement convertible en `U` et `static_cast<U>(t)` est valide |
| `Swappable<T>` | `std::swap(a, b)` est valide |
| `Streamable<T>` | `std::ostream << t` est valide |

> [!WARNING]
> `convertible_to` (renommé `ConvertibleTo` en `v3.0.0`) a été supprimé en `v4.0.0`.

## Opérations {#operations}

`utils/concepts/OperationConcepts.hpp` : chacun existe pour un même type (`Addable<T>`) et entre deux types (`AddableWith<T, U>`).

| Concept | Opérateur |
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
| `Arithmetic` / `ArithmeticWith` | `+`, `-`, `*`, `/` (les 4 concepts ci-dessus) |

## Exemple {#example}

```cpp
#define _Concepts
#include <utils/utils.hpp>

template<typename T, typename U>
requires utils::concepts::AddableWith<T, U>
auto sum(const T& a, const U& b) {return a + b;}
```
