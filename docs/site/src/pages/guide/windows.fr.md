+++
title = "Windows"
summary = "Compiler libutils et votre programme pour Windows depuis Linux avec MinGW-w64 : prérequis, paquets, la toolchain, votre projet, les règles de code et les tests."
keywords = "windows mingw mingw-w64 compilation croisée toolchain mingw64-libutils sysroot wine exe winsock windows.h WIN32_LEAN_AND_MEAN statique"
+++

Windows x86_64, compilé en croisé depuis Linux avec MinGW-w64 (toolchain `cmake/toolchain/mingw-w64.cmake`, paquets `mingw64-libutils*` depuis `v4.1.0`). Chaque classe et les appels de [Platform](page:platform) sont implémentés avec Win32 / Winsock / la CRT, donc un programme se compile pour les deux plateformes sans modification ; seuls quelques appels sans équivalent Windows lèvent une `FatalException` (`UndefinedCall`). La conception est expliquée dans [Plateformes](page:platforms).

## Prérequis {#requirements}

| Nom | Fedora (`dnf`) | Debian / Ubuntu (`apt`) |
|---|---|---|
| MinGW-w64 (GCC >= 13) | `mingw64-gcc-c++` | `g++-mingw-w64-x86-64` |
| OpenSSL pour MinGW | `mingw64-openssl` | compilé pour MinGW (pas de paquet) |
| Wine (tests / lancer le `.exe` sous Linux) | `wine` | `wine64` |

> [!NOTE]
> Le **sysroot** MinGW contient les headers et bibliothèques Windows de la cible : `/usr/x86_64-w64-mingw32/sys-root/mingw` sous Fedora, `/usr/x86_64-w64-mingw32` sous Debian / Ubuntu. Les paquets de libutils et d'OpenSSL pour Windows y sont installés.

## Installation {#install}

Depuis les paquets (après le `setup.sh` de [Installation](page:installation#packages)), installés dans le sysroot MinGW :

```bash "Terminal"
sudo dnf install mingw64-libutils       # Fedora: headers, CMake config & optimized library
sudo dnf install mingw64-libutils-db    # Debug library (optional)
sudo apt install mingw64-libutils       # Debian / Ubuntu: OpenSSL for MinGW must be built by hand
```

| Paquet | Contenu |
|---|---|
| `mingw64-libutils` | `mingw64-libutils-dev` + `mingw64-libutils-op` |
| `mingw64-libutils-dev` | Headers et configuration CMake globale (requiert `mingw64-openssl` sous Fedora) |
| `mingw64-libutils-dev-op` / `-dev-db` | Configuration CMake de la bibliothèque optimisée / debug |
| `mingw64-libutils-op` / `-db` | `libutils.a` / `libutils_debug.a` pour Windows (pas d'asan avec MinGW) |

Depuis les sources (n'importe quel préfixe) :

```bash "Terminal"
export BUILD_DIR=build-windows
cmake -S . -B $BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Optimized
cmake --build $BUILD_DIR --parallel $(nproc)
cmake --install $BUILD_DIR --prefix <windows_prefix>    # include/, lib/libutils.a, lib/cmake/utils/
cmake --build $BUILD_DIR --target package                 # mingw64-libutils-* RPM/DEB (MinGW sysroot)
```

## La toolchain {#toolchain}

`cmake/toolchain/mingw-w64.cmake` (dans le dépôt, utilisable par n'importe quel projet) :

| Réglage | Valeur |
|---|---|
| `CMAKE_SYSTEM_NAME` | `Windows` (`WIN32` et `CMAKE_CROSSCOMPILING` sont vrais) |
| Compilateurs | `${MINGW_PREFIX}-gcc` / `-g++` / `-windres`, `MINGW_PREFIX` = `x86_64-w64-mingw32` par défaut (`-DMINGW_PREFIX=<triplet>`) |
| `CMAKE_FIND_ROOT_PATH` | Le sysroot trouvé : `/usr/<triplet>/sys-root/mingw` (Fedora) et / ou `/usr/<triplet>` (Debian) |
| Modes de recherche | Programmes sur l'hôte (`python3`, `ccache`), bibliothèques / headers / paquets uniquement dans le sysroot |
| `CMAKE_CROSSCOMPILING_EMULATOR` | `wine64` ou `wine` quand il est installé, avec `WINEPATH=<sysroot>/bin` et `WINEDEBUG=-all` : `ctest` lance le `.exe` |

> [!TIP]
> Toute toolchain MinGW dont le `CMAKE_FIND_ROOT_PATH` est le sysroot fonctionne pareil : `/usr/share/mingw/toolchain-mingw64.cmake` (ou le wrapper `mingw64-cmake`) sous Fedora.

## Votre projet {#your-project}

**Le même code CMake fonctionne pour Linux et Windows**, seule la commande de configuration change :

```cmake "CMakeLists.txt"
find_package(utils 5.0.0 REQUIRED)
target_link_libraries(${TARGET} PRIVATE utils::utils)   # includes, OpenSSL, ws2_32 on Windows & library
```

```bash "Terminal"
cmake -S . -B build                                                                           # Linux
cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=<libutils>/cmake/toolchain/mingw-w64.cmake  # Windows
```

`Debug` lie `mingw64-libutils-db`, tout le reste `mingw64-libutils-op` ; il n'y a pas d'Asan avec MinGW (compilez en `Debug`). Un paquet manquant arrête la configuration avec son nom.

> [!TIP]
> Un `.exe` autonome (sans la dll du runtime MinGW à côté) : `if(WIN32) target_link_options(${TARGET} PRIVATE -static-libgcc -static-libstdc++ -static) endif()`. OpenSSL reste une dll (`libcrypto-3-x64.dll`, `libssl-3-x64.dll` depuis le `bin/` du sysroot) sauf si ses bibliothèques statiques sont installées.

## Code {#code}

| Règle | Pourquoi |
|---|---|
| Inclure `<windows.h>` **avant** `<utils/utils.hpp>` (ou définir `WIN32_LEAN_AND_MEAN`) | Le `<windows.h>` complet déclare un membre nommé `_unused`, une macro d'attribut de libutils |
| Appeler `utils::iomanip::enable_ansi()` une fois au démarrage | Active le mode terminal virtuel de la console (couleurs, curseur) ; renvoie `true` sous Unix |
| Utiliser `utils::platform::` pour les appels système | Mêmes noms et même contrat que POSIX (`-1` et `errno`), voir [Platform](page:platform) |
| Tester la plateforme avec `UTILS_WINDOWS` / `UTILS_UNIX` | Définis par `utils/platform/Platform.hpp` (toujours inclus), `UTILS_PLATFORM` = `"windows"` / `"unix"` |
| Les structures Winsock (`sockaddr_in`, `addrinfo`, `pollfd`...) | Viennent de `<winsock2.h>` / `<ws2tcpip.h>`, inclus par le programme (pas par libutils) |

Comportements différents sous Windows (détails dans [Platform](page:platform#windows-behavior), [Encapsulation](page:encapsulation#process), [Cli](page:cli#start-stop)) :

- `Poll`, `Client` et `Server` ne gèrent que des sockets (un pipe → erreur `EPoll`), `EPOLLET` est refusé (level triggered uniquement) ;
- un `Process` n'a pas de signal (`Status::sig` = `0`, tué = code de sortie `137`), seuls les fds `0`, `1`, `2` sont transmis à un enfant ;
- une `SharedMemory` est détruite avec son dernier handle (pas de persistance `shm_unlink`) ;
- <kbd>Ctrl</kbd>+<kbd>Z</kbd> est un caractère pour la `Cli` (pas de `SIGTSTP`).

L'API C fonctionne pareil sous Windows (les structures C des appels POSIX viennent de `utils/platform/Types.h`).

## Tests {#tests}

```bash "Terminal"
cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build-windows --parallel $(nproc)     # unit_tests.exe at the root of the repository
ctest --test-dir build-windows --output-on-failure --parallel $(nproc)
```

- GoogleTest : celui compilé pour MinGW dans le sysroot (`find_package(GTest CONFIG)`, ex : l'image `mingw64-unit-tests`), sinon téléchargé et compilé depuis les sources (`FetchContent`).
- `ctest` lance `unit_tests.exe` avec `wine` ; les tests sont listés au démarrage de `ctest` (`DISCOVERY_MODE PRE_TEST`), et le premier démarrage de wine crée son préfixe (`~/.wine`), ce qui peut prendre quelques minutes.
- `tests/platform/Platform.cpp` tourne sur les deux plateformes, `tests/platform/Windows.cpp` uniquement sous Windows.

## Exemple {#example}

```cpp "main.cpp"
#define _Platform
#define _Network
#include <utils/utils.hpp>
#include <iostream>

int main(void)
{
    std::cout << utils::version << " " << UTILS_PLATFORM          // "v5.0.0 windows"
        << " " << utils::network::resolve_hostname("localhost")   // "127.0.0.1" (Winsock)
        << std::endl;
    return 0;
}
```

```bash "Terminal"
cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=<libutils>/cmake/toolchain/mingw-w64.cmake
cmake --build build-windows
WINEPATH=/usr/x86_64-w64-mingw32/sys-root/mingw/bin wine build-windows/<program>.exe
```

| Code | Levé quand |
|---|---|
| `UndefinedCall` | Un appel sans équivalent Windows : `Process::spawn()` (fork), `Process::dup` sur un fd autre que `0`, `1`, `2`, les appels bruts de `ASocket` (`FatalException`) |
| `EPoll` | `Poll` / `Client` / `Server` sur un fd qui n'est pas un socket, ou `EPOLLET` |
