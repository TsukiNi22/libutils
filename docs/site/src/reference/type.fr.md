+++
title = "Type"
summary = "Types personnalisés : vecteurs 2D / 3D, matrices, la table de correspondance bidirectionnelle et les classes de base Freezable / Worker."
keywords = "type vecteur Vector2 Vector3 OVector2 OVector3 matrice Matrix OMatrix det invert transpose BLT BidirectionalLookupTable Freezable freeze Worker utils_vector2 utils_matrix utils_blt utils_freezable utils_worker"
+++

| Define | Groupe | Espace de noms | En-têtes |
|---|---|---|---|
| `_CustomType` | `_Tools` | `utils::type` | `utils/type/` |
| `_Vector` / `_Matrix` / `_BLT` (inclus par `_CustomType`) | `_CustomType` | `utils::type` | `utils/type/vector/`, `utils/type/matrix/`, `utils/type/blt/` |

Types personnalisés utilisés par la bibliothèque et utilisables par votre projet.

| Partie | Define | Contenu |
|---|---|---|
| [Vector](#vector) | `_Vector` | `Vector2<T>`, `Vector3<T>` (vérifiés) et `OVector2<T>`, `OVector3<T>` (optimisés) |
| [Matrix](#matrix) | `_Matrix` | `Matrix<T>` (vérifiée) et `OMatrix<T>` (optimisée) |
| [BLT](#blt) | `_BLT` | `BidirectionalLookupTable<L, R>` |
| [Freezable](#freezable), [Worker](#worker) | `_CustomType` | Classes de base |

## Vector {#vector}

Vecteurs de 2 (`x`, `y`) ou 3 (`x`, `y`, `z`) valeurs de n'importe quel type, avec tous les opérateurs.

| Classe | Description |
|---|---|
| `Vector2<T>`, `Vector3<T>` | **Vérifiés** : dérivent d'`IVector<T>` (suivi des instances), chaque opération est contrainte par un [concept](page:concepts), les opérations entre 2 types renvoient le type commun (`Vector2<int> + Vector2<double>` → `Vector2<double>`) |
| `OVector2<T>`, `OVector3<T>` | **Optimisés** : struct simple, sans suivi, les opérations gardent `T` (utilisés par les types de [Math](page:math)) |

| Méthode | Description |
|---|---|
| `x`, `y` (, `z`) | Membres publics |
| `get(i)` / `operator[](i)` | Valeur par indice (`VectorInvalidIndex` hors limites) |
| `min(v)`, `max(v)`, `clamp(min, max)` | Par composante |
| `dot(v)` | Produit scalaire |
| `cross(v)` | `Vector2` : scalaire `x1 * y2 - y1 * x2` ; `Vector3` : vecteur |
| `length()`, `lengthSquared()` | Norme (tronquée pour les types entiers) |
| `normalize()` | Vecteur unitaire (un vecteur nul reste nul) |
| `sign()` | `-1`, `0` ou `1` par composante |

Opérateurs : `+ - * /` (avec un vecteur ou un scalaire, des deux côtés), `+= -= *= /=`, `& | ^`, `++ --`, `-` unaire, `== !=`, `< <= > >=` (sur toutes les composantes), `<<` (flux : `(x, y)`).

```cpp
#define _Vector
#include <utils/utils.hpp>

utils::type::Vector2<int> a(1, 2);
utils::type::Vector2<double> b = a + utils::type::Vector2<double>(0.5, 0.5); // (1.5, 2.5)
utils::type::Vector3<double> up = utils::type::Vector3<double>(1, 0, 0).cross(utils::type::Vector3<double>(0, 1, 0)); // (0, 0, 1)
std::cout << b << " " << b.length() << std::endl;
```

## Matrix {#matrix}

Matrices de `x` lignes et `y` colonnes de n'importe quel type (`m(x, y)`), avec les opérations usuelles.

| Classe | Stockage | Description |
|---|---|---|
| `Matrix<T>` | `std::vector<std::vector<T>>` | **Vérifiée** : chaque opération est contrainte par un [concept](page:concepts), les opérations entre 2 types renvoient le type commun, `invert` / `/` désactivés pour les entiers |
| `OMatrix<T>` | contigu, row-major | **Optimisée** : ni concept ni suivi, les opérations gardent `T` (`invert` d'une matrice d'entiers tronque) |

| Méthode | Description |
|---|---|
| `Matrix(n)` / `Matrix(x, y)` | Carrée `n x n` / `x` lignes, `y` colonnes, valeurs par défaut |
| `at(x, y)` / `operator()(x, y)` | Valeur (`OutOfBounds` hors de la matrice) |
| `set(x, y, value)` | Définit une valeur |
| `set(begin, end, x = 0, y = 0)` | Remplit ligne par ligne à partir de `(x, y)` |
| `row(n)`, `col(n)` | Une ligne / colonne en `std::vector<T>` |
| `row()`, `col()`, `size()` | Nombre de lignes / colonnes / les deux |
| `swap(a, b)`, `swapRow(a, b)`, `swapCol(a, b)` | Échange des valeurs / lignes / colonnes |
| `transpose()` | Transpose sur place |
| `det()`, `trace()` | Déterminant / trace (matrice carrée) |
| `invert()` | Inverse sur place (`MatrixSingular` quand le déterminant vaut 0) |
| `clear()` | Réinitialise toutes les valeurs |

Opérateurs : `+ -` (même taille), `*` (produit matriciel, ou scalaire des deux côtés), `/` (`this * m^-1`, ou scalaire), les versions avec affectation, `-` unaire, `== !=`, `<<` (flux).

> [!WARNING]
> Des tailles qui ne correspondent pas lancent `InvalidArgument` ; `det`, `trace` et `invert` demandent une matrice carrée.

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

`BidirectionalLookupTable<L, R>` : une map où chaque paire peut être trouvée depuis ses deux côtés (`table[left] → right`, `table[right] → left`). C'est un [Freezable](#freezable) : une fois gelée, elle ne peut plus être modifiée.

| Méthode | Description |
|---|---|
| `addElement(left, right)` | Ajoute une paire (`Override` quand un côté existe déjà) |
| `setElement<force>(left, right)` | Pareil, `force = true` remplace les paires existantes des deux côtés |
| `operator[](left)` / `operator[](right)` | L'autre côté de la paire (`UnknownKey` s'il est absent) |
| `removeElement(side)` / `removeElements(sides)` | Supprime les paires (clé inconnue : avertissement sur `std::cerr`) |
| `clear()` | Supprime tout |
| `freeze()`, `isFrozen()` | Hérités de `Freezable` ; modifier une table gelée lance `Frozen` |

Hash / égalité personnalisés : `BidirectionalLookupTable<L, R, HashL, HashR, EqualL, EqualR>`. Quand les deux côtés ont le même type, utilisez `BLT_TYPE(T)` (`BidirectionalLookupTable<BLT_TYPE(std::string)>`) : un élément est cherché des deux côtés avec `operator[](element)`.

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

Classe de base d'un objet qui peut être verrouillé contre la modification (utilisée par `BidirectionalLookupTable`).

| Méthode | Description |
|---|---|
| `Freezable(frozen = false)` | État initial |
| `freeze()` / `isFrozen()` | Gèle définitivement / état |
| `requireUnfrozen()` | Lance `Frozen` si l'objet est gelé (à appeler au début d'une méthode de modification) |
| `requireFrozen()` | Lance `ShouldBeFrozen` si l'objet n'est pas gelé |

## Worker {#worker}

Classe de base des workers d'un [LoadBalancer](page:system#loadbalancer).

| Méthode | Description |
|---|---|
| `setWorkingStatus(bool)` | Marque le worker occupé / libre (le rend au balancer avec `false`) |
| `isWorking()` | État |
| `getStopedWorkingTimestamp()` | Moment où il est devenu libre (utilisé par la durée de vie du balancer) |

## En C {#in-c}

Chaque template est généré pour les [types fixes](page:c-api-design#fixed-types) (`<name>` = `i8` ... `u64`, `size`, `f32`, `f64`, `f128`).

| C++ | C |
|---|---|
| `OVector2<T>` / `OVector3<T>` | `utils_vector2_<name>_t` / `utils_vector3_<name>_t` **par valeur** (`{x, y}` / `{x, y, z}`) : `utils_vector2_<name>_add(a, b)`, `_dot`, `_cross`, `_length`, `_normalize`... (`_and` / `_or` / `_xor` pour les entiers) |
| `OMatrix<T>` | handle `utils_matrix_<name>_t*` : chaque opération existe en 3 versions, `utils_matrix_<name>_add(a, b)` (un nouveau handle), `_add_r(a, b, out)` (dans un handle donné), `_add_assign(a, b)` (sur place) ; `_div` / `_invert` pour les flottants |
| `BidirectionalLookupTable` | `utils_blt_str_str_t*`, `utils_blt_size_size_t*`, `utils_blt_i64_i64_t*` (un seul type), `utils_blt_str_<name>_t*` / `utils_blt_<name>_str_t*` (chaîne ↔ entier), `utils_blt_t*` (octets) |
| `Freezable` / `Worker` de n'importe quel handle | `utils_freezable_*(handle)`, `utils_worker_*(handle)` (une erreur quand le handle n'a pas cette base) |

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
