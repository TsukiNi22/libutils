+++
title = "Build, tests et hooks"
summary = "Compiler les trois bibliothèques et les paquets, lancer les tests unitaires (C++ et C pur), et activer le hook pre-commit xstyle."
keywords = "build cmake Debug Asan Optimized release package_release tests unitaires gtest ctest BUILD_TESTS test api c pre-commit hook xstyle site de documentation"
+++

Pour travailler sur libutils elle-même : les types de build et les cibles, les tests et le hook pre-commit. La CI qui les lance est sur [CI / CD](page:ci-cd).

## Build {#build}

| Commande | Résultat |
|---|---|
| `cmake -S . -B build -DCMAKE_BUILD_TYPE=Optimized\|Debug\|Asan` | `libutils.a` / `libutils_debug.a` / `libutils_asan.a` |
| `cmake --build build --target release` | Les 3 bibliothèques |
| `cmake --build build --target package_release` | Tous les paquets RPM / DEB / Arch (`.pkg.tar.zst`, avec `bsdtar`) |
| `-DCMAKE_TOOLCHAIN_FILE=...` | Le build Windows avec `cmake/toolchain/mingw-w64.cmake` ([Windows](page:windows)) |

Les codes d'exception de `cmake/config/exceptions/**/*.json` sont transformés en `generated_internal_exception_header.hpp` au build, et `utils/version.hpp` / `utils/version.h` sont écrits dans `build/generated/`.

## Tests unitaires {#tests}

```bash "Terminal"
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build $BUILD_DIR --parallel $(nproc)
ctest --test-dir $BUILD_DIR --output-on-failure --timeout 30   # or ./unit_tests (written at the root)
```

- GoogleTest, un fichier par module dans `tests/` (l'API C d'une section dans `tests/<section>/CApi.cpp`).
- `tests/c/CApi.c` est un programme en C pur compilé en C11 et C23 (`c_api_c11`, `c_api_c23` dans `build/tests/`), lancé par `ctest` (`CApi.C11`, `CApi.C23`).
- Les bibliothèques GPU sont chargées à l'exécution par `HybridMemory` : les tests des backends sont ignorés sans GPU.

## Hook pre-commit {#hook}

Un hook [xstyle](https://github.com/TsukiNi22/xstyle) partagé (`.githooks/pre-commit`) vérifie les lignes indexées à chaque `git commit` : un problème `minor` (ou pire) refuse le commit, tout le reste est seulement listé. Les sources synchronisées de `sos` et `c2dmp-hsm` sont ignorées.

```bash "Terminal"
git config core.hooksPath .githooks   # once per clone
git commit --no-verify                # skip the hook once (or XSTYLE_HOOK=0 git commit ...)
xstyle --fix --staged                 # fix what can be, then git add -u
```

> [!NOTE]
> Sans `xstyle` installé, le hook avertit seulement et le commit est accepté. Les seuils sont les variables en haut de `.githooks/pre-commit`.

## Documentation {#docs}

Ce site est construit depuis `docs/site/` (pages Markdown, et générateurs qui lisent les headers, les codes d'exception et le changelog) vers `docs/` :

```bash "Terminal"
docs/site/build.sh            # docs/*.html, search-index.js, site.css, site.js, sitemap.xml
docs/site/build.sh --check    # + links, accessibility, style and search checks
```

Le graphe du projet (`docs/graph.html`) est généré à part. Le [wiki GitHub](https://github.com/TsukiNi22/libutils/wiki) contient la même référence en Markdown, tenue à jour avec ce site.
