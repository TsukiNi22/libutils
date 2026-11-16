+++
title = "Installation"
summary = "Installer libutils depuis les paquets RPM / DEB / Arch signés, depuis les sources avec CMake, ou en copiant la bibliothèque compilée dans votre projet."
keywords = "installer installation setup.sh paquets rpm deb arch pacman dnf apt cmake sources dépendances openssl pré-version release"
+++

Trois façons d'installer libutils : les paquets de son propre dépôt (recommandé), les sources avec CMake, ou une copie de la bibliothèque compilée dans votre projet. Windows a [sa propre page](page:windows).

## Dépendances {#dependencies}

| Nom | Version | Fedora (`dnf`) | Debian / Ubuntu (`apt`) | Arch (`pacman`) |
|---|---|---|---|---|
| `clang++` | C++20 | `clang` | `clang` | `clang` |
| `cmake` | >= 3.20 | `cmake` | `cmake` | `cmake` |
| OpenSSL | >= 3.0 | `openssl-devel` | `libssl-dev` | `openssl` |
| GoogleTest (tests uniquement) | - | `gtest-devel` | `libgtest-dev` | `gtest` |
| `bsdtar` (paquets Arch uniquement) | - | `bsdtar` | `libarchive-tools` | `libarchive` |
| Pilote GPU (`HybridMemory` uniquement) | - | `libcuda` / `ocl-icd` / `vulkan-loader` | `libcuda1` / `ocl-icd-libopencl1` / `libvulkan1` | `nvidia-utils` / `ocl-icd` / `vulkan-icd-loader` |

Les bibliothèques GPU sont chargées à l'exécution : aucune n'est nécessaire pour le build, et une machine sans GPU utilise la RAM. Les licences des dépendances sont sur [Licences](page:licenses).

## Paquets {#packages}

`setup.sh` enregistre le dépôt de paquets de libutils (`https://tsukini22.github.io/libutils`) dans votre gestionnaire de paquets, puis les paquets s'installent comme n'importe quel autre :

```bash "Terminal"
curl -fsSL https://raw.githubusercontent.com/TsukiNi22/libutils/main/setup.sh | bash -s   # or: wget -qO- <same url> | bash -s
sudo dnf install libutils       # Fedora / RHEL
sudo apt install libutils       # Debian / Ubuntu
sudo pacman -S libutils         # Arch Linux
```

| Distribution | Ce que fait `setup.sh` |
|---|---|
| `rpm` | ajoute `/etc/yum.repos.d/libutils.repo` et importe la clé GPG |
| `deb` | ajoute la clé GPG dans `/etc/apt/keyrings/libutils.gpg`, la source dans `/etc/apt/sources.list.d/libutils.list`, puis `apt-get update` |
| `arch` | ajoute la clé GPG au trousseau de pacman (`pacman-key --add` + `--lsign-key`), le dépôt `[libutils]` dans `/etc/pacman.conf` (`SigLevel = Required`), puis `pacman -Sy` |

| Argument | Effet |
|---|---|
| `--no-sudo` | Lance sans `sudo` (le script doit déjà tourner en root) |
| `-h`, `--help` | Affiche l'usage |

| Paquet | Contenu |
|---|---|
| `libutils` | Les paquets par défaut : `libutils-dev` + `libutils-op` |
| `libutils-dev` | Tous les headers et la configuration CMake globale (requiert `openssl-devel` / `libssl-dev`) |
| `libutils-dev-op` / `-dev-db` / `-dev-as` | Configuration CMake de la bibliothèque optimisée / debug / asan |
| `libutils-op` | Bibliothèque compilée avec les options d'optimisation (`libutils.a`) |
| `libutils-db` | Bibliothèque compilée avec les options de debug (`libutils_debug.a`, pour le debug uniquement) |
| `libutils-as` | Bibliothèque compilée avec AddressSanitizer (`libutils_asan.a`, pour le debug uniquement) |

> [!NOTE]
> La plupart des paquets ont une pré-version nommée `<package>-pre`, remplacée par toute release (sans `-pre`) de la même version ou d'une version supérieure. Les mêmes paquets existent en RPM (Fedora), DEB (Debian / Ubuntu) et Arch Linux (`.pkg.tar.zst`, bibliothèques dans `/usr/lib`) : mêmes noms, contenu et dépendances.

> [!WARNING]
> Une erreur `invalid hash` de `dnf` / `apt` : videz le cache de l'outil ou relancez `setup.sh`. Si elle persiste, le dépôt est peut-être cassé : ouvrez une issue.

## Depuis les sources {#sources}

Installé dans `/usr/local` :

```bash "Terminal"
git clone https://github.com/TsukiNi22/libutils.git && cd libutils
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR
sudo cmake --build $BUILD_DIR --target install --parallel $(nproc)
```

Les types de build et les cibles (`release`, `package_release`) sont sur [Build, tests et hooks](page:development).

## Copie dans votre projet {#copy}

Compilez la bibliothèque, puis copiez les headers et l'archive là où votre projet les attend :

```bash "Terminal"
export BUILD_DIR=build
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Optimized
cmake --build $BUILD_DIR --parallel $(nproc)
cp -r include/* build/generated/* <your_project>/include/
cp build/libutils.a <your_project>/libs/
```

Puis compilez avec `-I<your_project>/include/ -L<your_project>/libs/ -lutils -lssl -lcrypto`, ou en CMake :

```cmake
find_package(OpenSSL REQUIRED)
target_include_directories(${TARGET} PRIVATE include)
target_link_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/libs/)
target_link_libraries(${TARGET} PRIVATE utils OpenSSL::SSL OpenSSL::Crypto)
```

Ensuite : [la lier et choisir les sections](page:usage).
