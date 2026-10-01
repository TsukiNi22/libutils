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
    # Exception
    exception/Exception.cpp

    # Concepts
    concepts/Concepts.cpp

    # Verbose
    verbose/Verbose.cpp

    # IO - Manip
    manip/iomanip/ANSI.cpp

    # S - Manip
    manip/smanip/Format.cpp
    manip/smanip/Base64Codec.cpp
    manip/smanip/EETPParser.cpp

    # Cli
    cli/Cli.cpp

    # Arguments
    arguments/ArgParser.cpp
    arguments/Settings.cpp

    # Network
    network/Socket.cpp
    network/ClientServer.cpp

    # Encapsulation
    encapsulation/Encapsulation.cpp
    encapsulation/SharedMemory.cpp

    # Math
    math/Geometry.cpp

    # Type
    type/Vector.cpp
    type/Matrix.cpp
    type/Freezable.cpp

    # Pool
    pool/Pool.cpp

    # System
    system/IdHandler.cpp
    system/Scheduler.cpp

    # Algorithms
    algorithms/C2dmp.cpp
    algorithms/Sos.cpp

    # Security (sub-part: encryption)
    security/encryption/RSAKey.cpp
    security/encryption/AESKey.cpp
    security/encryption/CommonRSAKey.cpp

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
