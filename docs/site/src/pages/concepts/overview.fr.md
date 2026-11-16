+++
title = "Architecture"
summary = "De quoi est faite libutils : un seul namespace, des sections choisies avec des defines, une API C++ et une API C sur les mêmes objets, et les conventions partagées par toutes les classes."
keywords = "architecture vue d'ensemble namespace utils sections groupes defines bibliothèque statique conventions failsafe safe_mode observer déplaçable"
+++

libutils est une seule bibliothèque statique (`libutils.a`, plus un build debug et un build asan) pour les programmes C++20, avec une API C sur les mêmes objets. Tout est dans le namespace `utils::` ; un programme inclut un seul header racine et choisit les sections dont il a besoin.

## En bref {#in-short}

| Point | Valeur |
|---|---|
| Langage | C++20, compilé avec `clang++` et CMake (>= 3.20) ; API C pour C11 / C23 |
| Bibliothèque | statique : `libutils.a` (optimisée), `libutils_debug.a`, `libutils_asan.a` |
| Plateformes | Unix (Linux, BSD, macOS) et Windows x86_64 (compilé en croisé avec MinGW-w64), même API ([Plateformes](page:platforms)) |
| Dépendance | OpenSSL >= 3.0 (chiffrement, Base64) |
| Points d'entrée | `<utils/utils.hpp>` (C++), `<utils/utils.h>` (C) |
| Version | `utils::version` / `utils_version`, `__LIBUTILS_VERSION__` |

## Sections {#sections}

Une section est un dossier de `include/utils/`, un namespace et un define. Un define de **groupe** active ses sections, un define de section active ses sous-sections ; aucun define signifie tout (`_Utils`). La liste avec leurs pages est sur [Lier et inclure](page:usage#sections).

:::html
<figure>
<svg viewBox="0 0 760 300" role="img" aria-label="utils.hpp and utils.h include the Handling and Tools groups and their sections">
<defs>
<marker id="arr-fig1-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box-alt" x="300" y="16" width="160" height="44" rx="8"/>
<text class="lbl" x="380" y="43" text-anchor="middle">utils.hpp · utils.h</text>
<path class="edge" d="M330 60L190 98" marker-end="url(#arr-fig1-fr)"/>
<path class="edge" d="M430 60L570 98" marker-end="url(#arr-fig1-fr)"/>
<path class="edge dash" d="M380 60V250" marker-end="url(#arr-fig1-fr)"/>
<rect class="group" x="20" y="100" width="330" height="140" rx="10"/>
<text class="title" x="36" y="124">_Handling</text>
<text class="small" x="36" y="150">_Exception · _Verbose · _Pool (_Middleware)</text>
<text class="small" x="36" y="174">_Cli · _Arguments</text>
<text class="small" x="36" y="198">_Network (_Socket)</text>
<text class="small" x="36" y="222">utils::exception, verbose, pool, cli, arguments, network</text>
<rect class="group" x="410" y="100" width="330" height="140" rx="10"/>
<text class="title" x="426" y="124">_Tools</text>
<text class="small" x="426" y="150">_Math (_Geometry, _Trigo) · _Concepts</text>
<text class="small" x="426" y="174">_Encapsulation · _System</text>
<text class="small" x="426" y="198">_CustomType (_Vector, _Matrix, _BLT)</text>
<text class="small" x="426" y="222">_Manip · _Algorithms · _Security · _Platform</text>
<rect class="box" x="300" y="252" width="160" height="38" rx="8"/>
<text class="lbl" x="380" y="276" text-anchor="middle">_Attribute</text>
</svg>
<figcaption>Les defines de <code>utils.hpp</code> et <code>utils.h</code> : un groupe active ses sections, une section ses sous-sections.</figcaption>
</figure>
:::

| Define | Namespace | Contenu |
|---|---|---|
| `_Exception` | `utils::exception` | exceptions avec un type et un code, sortie formatée |
| `_Verbose` | `utils::verbose` | niveau global atomique, macros de sortie verrouillées |
| `_Pool` | `utils::pool` | `Cluster<T>`, `Middlewares<T, U>` |
| `_Cli` | `utils::cli` | ligne de commande interactive : commandes, historique, suggestions, complétion, hooks, middlewares |
| `_Arguments` | `utils::arguments` | `ArgParser` (usages, flags, options, env), `Settings` (valeurs typées, casts) |
| `_Network` | `utils::network` | `Server`, `Client`, `ISocket` / `ASocket` / `TCPSocket` |
| `_Math` | `utils::math` | coordonnées, `CFrame`, rotation de point, look vector, deg / rad |
| `_Concepts` | `utils::concepts` | concepts d'opérations (`AddableWith`, `Comparable`...) |
| `_Encapsulation` | `utils::encapsulation` | `Pipe`, `Dup`, `Process`, `Poll`, `SharedObject`, `SharedMemory` |
| `_System` | `utils::system` | `IdHandler`, `Scheduler`, `LoadBalancer`, `HybridMemory` |
| `_CustomType` | `utils::type` | `Vector2/3`, `OVector2/3`, `Matrix`, `OMatrix`, `BidirectionalLookupTable`, `Freezable`, `Worker` |
| `_Manip` | `utils::iomanip`, `utils::smanip` | séquences ANSI, `format`, `FixedString`, `Base64Codec`, `EETPParser` |
| `_Algorithms` | `utils::algorithms` | c2dmp-hsm (distance entre chaînes), s.o.s (stéganographie) |
| `_Security` | `utils::security` | `AESKey`, `RSAKey`, `CommonRSAKey`, observers / notificateur de fuites mémoire |
| `_Platform` | `utils::platform` | appels POSIX pour Unix et Windows, `UTILS_UNIX` / `UTILS_WINDOWS` |
| `_Attribute` | macros | `_hot`, `_nodiscard`, `_likely`, `_migration(x, y, z)`... |

Les dépendances entre les headers, les classes et les appels sont dessinées sur le [graphe du projet](page:project-graph).

## Deux API, une bibliothèque {#apis}

L'API C n'est pas une seconde implémentation : chaque `CApi.h` déclare des fonctions C (`extern "C"`) dont le `.cpp` appelle les classes C++ dans une garde. Les objets sont les mêmes, donc un programme C obtient le même comportement, les mêmes threads et les mêmes codes d'erreur. Le fonctionnement de la frontière : [Conception de l'API C](page:c-api-design).

| | C++ | C |
|---|---|---|
| Header racine | `<utils/utils.hpp>` | `<utils/utils.h>` |
| Un objet | une classe (`utils::cli::Cli`) | un handle opaque (`utils_cli_t*`) |
| Une erreur | une exception (`IException`) | `UTILS_KO` / `NULL` + le statut `utils_exception` |
| Un template | `Cluster<T>` | un jeu de fonctions par type fixe (`utils_cluster_i32_*`) + une version en octets |
| Un callback | `std::function` | un pointeur de fonction + `void* data` |

## Conventions {#conventions}

- Chaque erreur est une exception (`utils::exception::*Exception`) avec un code de [Exceptions](page:exception#codes) ; attrapez `const utils::exception::IException&`. Le modèle est sur [Erreurs](page:errors).
- Beaucoup de méthodes prennent un booléen `failsafe` (afficher un avertissement au lieu de lever une exception) ou un booléen `safe_mode` (verrou, thread safe).
- Les hooks de vérification renvoient `std::optional<std::string>` : `std::nullopt` = valide, une chaîne = le message d'erreur.
- La plupart des classes sont déplaçables mais pas copiables, et sont suivies par un [observer](page:security#observer) (rapport des fuites mémoire à la sortie).
- Chaque classe a la même API sous Unix et Windows : seul un appel sans équivalent Windows lève une `FatalException` (`UndefinedCall`).
- Les noms suivent un seul style : classes en `PascalCase`, méthodes en `camelCase`, fonctions outils en `snake_case`, membres `_camelCase`, macros `UPPER_CASE` ; l'API C est `utils_<section>_<name>` et `C_API_<SECTION>_<NAME>`.
