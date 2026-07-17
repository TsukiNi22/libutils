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

# =========================
# Requirement
# =========================
find_package(PkgConfig REQUIRED)
find_package(Python3 REQUIRED) # Used for the special dependencie
find_program(CCACHE_PROGRAM ccache) # Used for cache during compilation
if (CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

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

    # Verbose
    src/utils/verbose/Verbose.cpp

    ## Cli
    src/utils/cli/Cli.cpp
    src/utils/cli/Setup.cpp
    src/utils/cli/Hooks.cpp

    ## Arguments
    src/utils/arguments/ArgParser.cpp
    src/utils/arguments/Hooks.cpp
    src/utils/arguments/Settings.cpp

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
# Release (build Debug & Optimized)
# =========================
add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Debug
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Asan
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --parallel
    COMMENT "Building both debug versions Debug & Asan (libutils_debug.a, libutils_asan.a) and Optimized (libutils.a) one"
    VERBATIM
)

# =========================
# Dependencies
# =========================
add_dependencies(${TARGET} generated_exception_header)

# =========================
# Includes
# =========================
target_include_directories(${TARGET} PUBLIC include)

# =========================
# Modes
# =========================
set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "utils$<$<CONFIG:Debug>:_debug>$<$<CONFIG:Asan>:_asan>"
)

target_compile_options(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:-g -ggdb3>
    $<$<CONFIG:Asan>:-fsanitize=address -fno-omit-frame-pointer -g -ggdb3>
    $<$<CONFIG:Optimized>:-O3 -ffast-math -funroll-loops -pipe -DNDEBUG>
)

# =========================
# Utils
# =========================
add_custom_target(get_unregistered_files
    COMMAND bash ${CMAKE_SOURCE_DIR}/cmake/scripts/check_files.sh unregistered ${SRC}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
