+++
title = "Lier et inclure"
summary = "Lier libutils avec CMake, inclure son header racine et choisir les sections nécessaires avec des defines."
keywords = "usage cmake find_package utils::utils include utils.hpp define _Utils _Handling _Tools _Network sections type de build debug asan version"
+++

Une fois [installée](page:installation), un projet lie la cible CMake `utils::utils` et inclut un seul header racine, avec les sections nécessaires choisies par des defines.

## CMake {#cmake}

```cmake "CMakeLists.txt"
find_package(utils 5.0.0 REQUIRED)                       # minimal version
target_link_libraries(${TARGET} PRIVATE utils::utils)    # includes, static library & OpenSSL
```

La bibliothèque liée suit le `CMAKE_BUILD_TYPE` de votre projet :

| Votre type de build | Bibliothèque | Paquet (Linux) | Paquet (Windows) |
|---|---|---|---|
| `Debug` | `libutils_debug.a` | `libutils-db` | `mingw64-libutils-db` |
| `Asan` | `libutils_asan.a` (compilez aussi votre projet avec `-fsanitize=address`) | `libutils-as` | aucun (pas d'AddressSanitizer avec MinGW, utilisez `Debug`) |
| tout autre | `libutils.a` (optimisée) | `libutils-op` (dans `libutils`) | `mingw64-libutils-op` (dans `mingw64-libutils`) |

Un paquet manquant arrête la configuration avec le nom du paquet à installer (mêmes noms pour RPM, DEB et Arch). Le même code CMake fonctionne pour Windows : voir [Windows](page:windows#your-project).

## Inclusion {#include}

```cpp "main.cpp"
#define _Network        // only the needed sections (nothing defined = everything)
#include <utils/utils.hpp>

int main(void)
{
    std::cout << utils::version << std::endl; // "v5.0.0"
    return 0;
}
```

`#include <utils.hpp>` équivaut à `<utils/utils.hpp>` (une redirection, avec un message quand `utils/utils.hpp` est introuvable). Depuis C, le header racine est `<utils.h>` : voir [Utiliser libutils depuis C](page:c-api).

> [!IMPORTANT]
> Incluez toujours le header racine, jamais un header de libutils un par un. La seule exception : les [observers](page:security#observer) (`<utils/security/observer/Observer.hpp>`...).

## Sections {#sections}

Un define placé avant l'include active une section ; aucun define signifie tout (`_Utils`). L'articulation des sections est dessinée dans [Architecture](page:overview#sections).

| Define | Contenu |
|---|---|
| `_Utils` | Tout (défini quand aucun autre define n'est posé) : `_Handling`, `_Tools`, `_Attribute` |
| `_Handling` | `_Exception`, `_Verbose`, `_Pool`, `_Cli`, `_Arguments`, `_Network` |
| `_Tools` | `_Math`, `_Concepts`, `_Encapsulation`, `_System`, `_CustomType`, `_Manip`, `_Algorithms`, `_Security`, `_Platform` |

| Define | Contenu | Page |
|---|---|---|
| `_Exception` | Exceptions avec un type et un code, sortie formatée | [Exceptions](page:exception) |
| `_Verbose` | Niveau de verbose global et macros de sortie | [Verbose](page:verbose) |
| `_Pool` (`_Middleware`) | Clusters d'entités, chaînes de middlewares | [Pool](page:pool) |
| `_Cli` | Ligne de commande interactive personnalisable | [Cli](page:cli) |
| `_Arguments` | Parsing de `argc / argv` et réglages typés | [Arguments](page:arguments) |
| `_Network` (`_Socket`) | Client / serveur TCP, sockets | [Network](page:network) |
| `_Math` (`_Geometry`, `_Trigo`) | Coordonnées, angles, rotations | [Math](page:math) |
| `_Concepts` | Concepts C++20 des opérations | [Concepts](page:concepts) |
| `_Encapsulation` | Pipe, dup, processus, poll, objet partagé, mémoire partagée | [Encapsulation](page:encapsulation) |
| `_System` | Gestionnaire d'id, scheduler, load balancer, mémoire GPU | [System](page:system) |
| `_CustomType` (`_Vector`, `_Matrix`, `_BLT`) | Vecteurs, matrices, table de correspondance bidirectionnelle, freezable, worker | [Type](page:type) |
| `_Manip` (`_IOManip`, `_SManip`) | Séquences ANSI, outils de chaînes, codecs, parsers | [IOManip](page:iomanip), [SManip](page:smanip) |
| `_Algorithms` (`_C2DMP`, `_SOS`) | Distance entre chaînes, stéganographie | [Algorithms](page:algorithms) |
| `_Security` (`_Encryption`, `_Observer`) | AES / RSA, observers | [Security](page:security) |
| `_Platform` | Appels POSIX pour Unix et Windows | [Platform](page:platform) |
| `_Attribute` | Macros d'attributs du compilateur | [Attribute](page:attribute) |

Les avertissements de la bibliothèque se choisissent aussi avec des defines : voir [Avertissements](page:warnings).

## Règles {#rules}

> [!WARNING]
> `utils.hpp` n'a pas de garde d'inclusion globale et les defines restent posés pour le reste du fichier : placez les defines **avant chaque** include de `utils.hpp` (dans un `.hpp` comme dans un `.cpp`). Un include nu après un include partiel n'obtient que les sections déjà définies.

Tout est dans le namespace `utils::`. La plupart des méthodes qui peuvent échouer prennent un booléen `failsafe` (avertir au lieu de lever une exception) ou un booléen `safe_mode` (verrou, thread safe) ; voir [Architecture](page:overview#conventions).

## Version {#version}

`utils::version` (`"vX.Y.Z"`) et la macro `__LIBUTILS_VERSION__` donnent la version installée (`"[unknown]"` quand `utils/version.hpp` est absent). En C : `utils_version` et `__LIBUTILS_VERSION__` depuis `utils/version.h`.
