set(TARGET unit_tests)

# =========================
# Compilateur & Standard
# =========================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}) # TARGET destination

# =========================
# Requirement
# =========================
find_package(GTest REQUIRED)

# =========================
# Sources
# =========================
set(SRC
    # Security
    security/IdHandler.cpp

    # Security (sub-part: observer)
    security/observer/MemoryLeakNotifer.cpp
    security/observer/Observer.cpp
    security/observer/UnsafeObserver.cpp
)
add_executable(${TARGET} ${SRC})

# =========================
# Dependencies
# =========================
target_include_directories(${TARGET} PRIVATE ../include/ .)
target_link_libraries(${TARGET} PRIVATE utils GTest::gtest GTest::gtest_main)

# =========================
# Modes
# =========================
target_compile_options(${TARGET} PRIVATE -g -ggdb3)

include(GoogleTest)
gtest_discover_tests(${TARGET})
