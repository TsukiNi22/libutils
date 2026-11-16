+++
title = "Custom exception codes"
summary = "Add the exception codes of your project: a JSON config, the generator script, the CMake rule, then throw them as ExternalCode."
section = "guide"
order = 30
keywords = "exception code custom external ExternalCode json config generate_exception_header.py const.py scripts.zip cmake restrictions"
+++

The codes of libutils are **internal** (`InternalCode`); the ones of your project are **external** (`ExternalCode`). Both are written in JSON files and turned into a header by a Python script at the build. The classes and the codes of the library are on [Exceptions](page:exception).

> [!IMPORTANT]
> An external code that duplicates an internal one only points to the internal one: its setup information is ignored.

> [!TIP]
> A project made from the [cpp_project_template](https://github.com/TsukiNi22/cpp_project_template) already has everything below. You can also [install libutils from the sources](page:installation#sources) with your codes added to its own JSON files: they become internal ones.

## JSON config {#json}

Every `.json` under `cmake/config/exceptions/` (recursively, any number of files) is read. Another folder works too: change `const.py` and the path of the CMake.

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

| Key | Use |
|---|---|
| `code` | The name: `utils::exception::ExternalCode::<code_name>` |
| `message` | Returned by `what()` |
| `info` | Returned by `info()` when none is given (optional, `"[None]"` by default) |
| `restrictions` | The types the code accepts: `None`, `Warning`, `Error`, `Fatal` (optional, no restriction by default) |

The libutils config itself, for example:

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

The scripts are in the project template, [`cmake/scripts/`](https://github.com/TsukiNi22/cpp_project_template/tree/main/cmake/scripts): copy the folder, or download only the useful files as [scripts.zip](https://github.com/TsukiNi22/libutils/wiki/scripts.zip) and extract it at the root of the project.

```bash "Terminal"
sha256sum scripts.zip
f969642ec52bb9bf0491ee215dae9b64f5e5775a574d87287fbd55146c4435b3  scripts.zip
```

| File | Role |
|---|---|
| `generate_exception_header.py` | Reads every `.json` of `cmake/config/exceptions/` (recursively) and writes `include/exception/generated_external_exception_header.hpp` |
| `const.py` | Paths (`CONFIG_EXCEPTION`, `GENERATED_EXCEPTION_HEADER`), the names of the generated sections (`ExternalCode`...) and the exception types |
| `requirements.txt` | Python packages to install before the generation (none for now: nothing is installed when the file is empty, nor when run as root) |
| `check_files.sh` | Optional, used by the `unregistered` CMake target to list the `src/*.cpp` missing from the `SRC` list |

The generator stops with an error (and no header) on a `code` that isn't a C++ identifier, that is `Undefined` (reserved), or that is defined twice (in one or two files). A `restrictions` value other than `None`, `Warning`, `Error`, `Fatal` is reported and ignored.

## CMake {#cmake}

Run the script before the build, when the config changes:

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

And link libutils as usual (`find_package(utils 5.0.0 REQUIRED)` + `target_link_libraries(${TARGET} PRIVATE utils::utils)`).

## Use {#use}

With an `Exit` code allowing the type `None` in your config:

```cpp
throw utils::exception::NoneException(utils::exception::InternalCode::Exit); // internal
throw utils::exception::NoneException(utils::exception::ExternalCode::Exit); // external
```

Here both throw the same exception, the external `Exit` being a duplicate of the internal one. The output of `formated()`:

```text
[Error(fatal)] .../src/utils/exception/AException.cpp:39 (utils) -> Error during the setup of an exception
-------------------------------------------
void utils::exception::AException::subinit(void) = Restriction trigerred on a code & type combination
-------------------------------------------
```

In C, an external code is reported with `utils_exception.origin == C_API_EXCEPTION_ORIGIN_EXTERNAL`; a C function sets its own code with `utils_fail(code, info)` (see [Use libutils from C](page:c-api#errors)).
