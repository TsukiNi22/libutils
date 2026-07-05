cmake_minimum_required(VERSION 3.20)
set(TARGET utils)
project(${TARGET} LANGUAGES CXX)

# =========================
# Compilateur & Standard
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Moderne compilation -fPIE
add_definitions(-DFORCED_CXX_STANDARD=14) # Used for the attribute definition
add_definitions(-D_NoWarning) # Remove warnings from utils (usless for hard import)
add_definitions(-D__cpp_lib_hardware_interference_size=201703L) # Force the declaration of hardware_interference_size

# =========================
# Requirement
# =========================
find_package(PkgConfig REQUIRED)
find_package(Python3 REQUIRED) # Used for the special dependencie

# =========================
# Warnings
# =========================
add_compile_options(
    -W
    -Wall
    -Wextra
    -Wpedantic
    -Wunused-parameter
    -Wshadow
    -Wuninitialized
    -Wno-c++26-extensions
)

# =========================
# Sources
# =========================
set(SRC
    ## Exception
    src/utils/exception/AException.cpp

    ## Write
    src/utils/write/ANSI.cpp
    src/utils/write/format.cpp

    ## Math (sub-part: geometry)
    src/utils/math/geometry/Point.cpp
    src/utils/math/geometry/Angle.cpp

    ## Cli
    src/utils/cli/Cli.cpp
    src/utils/cli/Setup.cpp
    src/utils/cli/Hooks.cpp

    ## Arguments
    src/utils/arguments/ArgParser.cpp
    src/utils/arguments/Hooks.cpp

    ## Warning
    src/utils/warning/Observer.cpp
    src/utils/warning/UnsafeObserver.cpp
    src/utils/warning/SharedObject.cpp
)
add_library(${TARGET} STATIC ${SRC})
#add_library(${TARGET} SHARED ${SRC})

# =========================
# Special Targets
# =========================
file(GLOB_RECURSE EXCEPTION_CONFIG_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/cmake/config/exceptions/*.json"
)
set(GENERATED_EXCEPTION_HEADER
    "${CMAKE_SOURCE_DIR}/include/utils/exception/generated_exception_header.hpp"
)
add_custom_command(
    OUTPUT ${GENERATED_EXCEPTION_HEADER}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/scripts/generate_exception_header.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${EXCEPTION_CONFIG_FILES} ${CMAKE_SOURCE_DIR}/cmake/scripts/generate_exception_header.py
    COMMENT "Generating exception header (config changed)"
    VERBATIM
)
add_custom_target(generated_exception_header
    DEPENDS ${GENERATED_EXCEPTION_HEADER}
)

# =========================
# Dependencies
# =========================
add_dependencies(${TARGET} generated_exception_header)

# =========================
# Includes
# =========================
target_include_directories(${TARGET} PUBLIC include)
#target_link_libraries(${TARGET} PRIVATE ...)

# =========================
# Modes
# =========================
target_compile_options(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:-g -ggdb3>
    $<$<CONFIG:Optimized>:-O3 -ffast-math -funroll-loops -pipe -DNDEBUG>
)

# =========================
# Utils
# =========================
add_custom_target(get_unregistered_files
    COMMAND bash ${CMAKE_SOURCE_DIR}/cmake/scripts/check_files.sh unregistered ${SRC}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
