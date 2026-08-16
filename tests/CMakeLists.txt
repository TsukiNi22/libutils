cmake_minimum_required(VERSION 3.20)
set(TARGET unit_tests)

# =========================
# Compilateur & Standard
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}) # TARGET destination
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Moderne compilation -fPIE

# =========================
# Requirement
# =========================
find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)
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
)

# =========================
# Sources
# =========================
set(SRC
    # Security
    security/IdHandler.cpp
)
add_executable(${TARGET} ${SRC})

# =========================
# Dependencies
# =========================
target_include_directories(${TARGET} PRIVATE ../include/)
target_link_libraries(${TARGET} PRIVATE utils GTest::gtest GTest::gtest_main)

# =========================
# Modes
# =========================
target_compile_options(${TARGET} PRIVATE -g -ggdb3)

include(GoogleTest)
gtest_discover_tests(${TARGET})
