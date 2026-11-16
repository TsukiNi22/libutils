+++
title = "Codes d'exception personnalisés"
summary = "Ajouter les codes d'exception de votre projet : une config JSON, le script générateur, la règle CMake, puis les lever en ExternalCode."
keywords = "exception code personnalisé externe ExternalCode json config generate_exception_header.py const.py scripts.zip cmake restrictions"
+++

Les codes de libutils sont **internes** (`InternalCode`) ; ceux de votre projet sont **externes** (`ExternalCode`). Les deux sont écrits dans des fichiers JSON et transformés en header par un script Python au build. Les classes et les codes de la bibliothèque sont sur [Exceptions](page:exception).

> [!IMPORTANT]
> Un code externe qui duplique un code interne pointe seulement vers l'interne : ses informations de setup sont ignorées.

> [!TIP]
> Un projet créé depuis le [cpp_project_template](https://github.com/TsukiNi22/cpp_project_template) a déjà tout ce qui suit. Vous pouvez aussi [installer libutils depuis les sources](page:installation#sources) avec vos codes ajoutés à ses propres fichiers JSON : ils deviennent des codes internes.

## Config JSON {#json}

Chaque `.json` sous `cmake/config/exceptions/` (récursivement, autant de fichiers que voulu) est lu. Un autre dossier fonctionne aussi : modifiez `const.py` et le chemin du CMake.

```json "cmake/config/exceptions/project.json"
{
    "errors": [
        {
            "code": "<code_name>",
            "message": "<error_message>",
            "info": "<sub_info>",
            "restrictions": ["<restriction1>", "<restriction2>"]
        }
    ]
}
```

| Clé | Usage |
|---|---|
| `code` | Le nom : `utils::exception::ExternalCode::<code_name>` |
| `message` | Renvoyé par `what()` |
| `info` | Renvoyé par `info()` quand aucune n'est donnée (optionnel, `"[None]"` par défaut) |
| `restrictions` | Les types que le code accepte : `None`, `Warning`, `Error`, `Fatal` (optionnel, aucune restriction par défaut) |

La config de libutils elle-même, par exemple :

```json "cmake/config/exceptions/utils/exception.json + global/dull.json"
{
    "errors": [
        {
            "code": "ExceptionCodeRestriction",
            "message": "Error during the setup of an exception",
            "info": "Restriction trigerred on a code & type combination",
            "restrictions": ["Fatal", "Error"]
        },
        {"code": "Exit", "message": "Exit", "info": "Exit", "restrictions": ["None"]},
        {"code": "Empty", "message": "Empty"}
    ]
}
```

## Script {#script}

Les scripts sont dans le template de projet, [`cmake/scripts/`](https://github.com/TsukiNi22/cpp_project_template/tree/main/cmake/scripts) : copiez le dossier, ou téléchargez seulement les fichiers utiles dans [scripts.zip](https://github.com/TsukiNi22/libutils/wiki/scripts.zip) et extrayez-le à la racine du projet.

```bash "Terminal"
sha256sum scripts.zip
f969642ec52bb9bf0491ee215dae9b64f5e5775a574d87287fbd55146c4435b3  scripts.zip
```

| Fichier | Rôle |
|---|---|
| `generate_exception_header.py` | Lit chaque `.json` de `cmake/config/exceptions/` (récursivement) et écrit `include/exception/generated_external_exception_header.hpp` |
| `const.py` | Chemins (`CONFIG_EXCEPTION`, `GENERATED_EXCEPTION_HEADER`), noms des sections générées (`ExternalCode`...) et types d'exception |
| `requirements.txt` | Paquets Python à installer avant la génération (aucun pour l'instant : rien n'est installé quand le fichier est vide, ni quand il est lancé en root) |
| `check_files.sh` | Optionnel, utilisé par la cible CMake `unregistered` pour lister les `src/*.cpp` absents de la liste `SRC` |

Le générateur s'arrête sur une erreur (et sans header) pour un `code` qui n'est pas un identifiant C++, qui vaut `Undefined` (réservé), ou qui est défini deux fois (dans un ou deux fichiers). Une valeur de `restrictions` autre que `None`, `Warning`, `Error`, `Fatal` est signalée et ignorée.

## CMake {#cmake}

Lancer le script avant le build, quand la config change :

```cmake "CMakeLists.txt"
file(GLOB_RECURSE EXCEPTION_CONFIG_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/cmake/config/exceptions/*.json"
)
set(GENERATED_EXCEPTION_HEADER
    "${CMAKE_SOURCE_DIR}/include/exception/generated_external_exception_header.hpp"
)
add_custom_command(
    OUTPUT ${GENERATED_EXCEPTION_HEADER}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/scripts/generate_exception_header.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${EXCEPTION_CONFIG_FILES} ${CMAKE_SOURCE_DIR}/cmake/scripts/generate_exception_header.py
    COMMENT "Generating exception header (config changed)"
    VERBATIM
)
add_custom_target(generated_external_exception_header ALL # ALL: also generated for a header-only project
    DEPENDS ${GENERATED_EXCEPTION_HEADER}
)
add_dependencies(${TARGET} generated_external_exception_header)
target_include_directories(${TARGET} PRIVATE include/exception)   # where libutils finds your codes
```

Et liez libutils comme d'habitude (`find_package(utils 5.0.0 REQUIRED)` + `target_link_libraries(${TARGET} PRIVATE utils::utils)`).

## Utilisation {#use}

Avec un code `Exit` autorisant le type `None` dans votre config :

```cpp
throw utils::exception::NoneException(utils::exception::InternalCode::Exit); // internal
throw utils::exception::NoneException(utils::exception::ExternalCode::Exit); // external
```

Ici, les deux lèvent la même exception, le `Exit` externe étant un doublon de l'interne. La sortie de `formated()` :

```text
[Error(fatal)] .../src/utils/exception/AException.cpp:39 (utils) -> Error during the setup of an exception
-------------------------------------------
void utils::exception::AException::subinit(void) = Restriction trigerred on a code & type combination
-------------------------------------------
```

En C, un code externe est signalé avec `utils_exception.origin == C_API_EXCEPTION_ORIGIN_EXTERNAL` ; une fonction C pose son propre code avec `utils_fail(code, info)` (voir [Utiliser libutils depuis C](page:c-api#errors)).
