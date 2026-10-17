set(TARGET unit_tests)

# =========================
# Compilateur & Standard
# =========================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}) # TARGET destination

# =========================
# Requirement
# =========================
if(CMAKE_CROSSCOMPILING)
    find_package(GTest CONFIG QUIET) # built for the target in the sysroot (ex: mingw64-unit-tests image)
endif()
if(CMAKE_CROSSCOMPILING AND NOT GTest_FOUND)
    # No GTest package for the target (ex: MinGW): built from the sources
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
elseif(NOT CMAKE_CROSSCOMPILING)
    find_package(GTest REQUIRED)
endif()

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

    # S - Manip
    manip/smanip/Format.cpp
    manip/smanip/Base64Codec.cpp
    manip/smanip/EETPParser.cpp

    # Cli
    cli/Cli.cpp

    # Arguments
    arguments/Settings.cpp

    # Network
    network/ClientServer.cpp

    # Encapsulation
    encapsulation/SharedMemory.cpp

    # Platform
    platform/Platform.cpp

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
    security/encryption/Key.cpp

    # Security (sub-part: observer)
    security/observer/MemoryLeakNotifer.cpp
    security/observer/Observer.cpp
    security/observer/UnsafeObserver.cpp
    security/observer/ANotifier.cpp
)

# Unix test fixtures (fork, socketpair, epoll, mkfifo...): the Windows versions are in platform/Windows.cpp
set(SRC_UNIX
    # IO - Manip
    manip/iomanip/ANSI.cpp

    # Arguments
    arguments/ArgParser.cpp

    # Network
    network/Socket.cpp

    # Encapsulation
    encapsulation/Encapsulation.cpp
)

# Windows only (ports of Pipe, Dup, SharedObject, Process, Poll, TCPSocket, Client, Server, ANSI)
set(SRC_WINDOWS
    # Platform
    platform/Windows.cpp
)

if(WIN32)
    list(APPEND SRC ${SRC_WINDOWS})
else()
    list(APPEND SRC ${SRC_UNIX})
endif()
add_executable(${TARGET} ${SRC})

# =========================
# Dependencies
# =========================
target_include_directories(${TARGET} PRIVATE ../include/ .)
target_link_libraries(${TARGET} PRIVATE utils GTest::gtest GTest::gtest_main)
if(WIN32)
    target_link_options(${TARGET} PRIVATE -static-libgcc -static-libstdc++ -static) # standalone .exe (no MinGW runtime dll)
endif()

# =========================
# Modes
# =========================
target_compile_options(${TARGET} PRIVATE -g -ggdb3)
if(NOT WIN32)
    # libutils_asan.a needs the AddressSanitizer runtime at the link
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Asan>:-fsanitize=address -fno-omit-frame-pointer>)
    target_link_options(${TARGET} PRIVATE $<$<CONFIG:Asan>:-fsanitize=address>)
endif()

include(GoogleTest)
if(CMAKE_CROSSCOMPILING)
    # Listed when ctest runs (CMAKE_CROSSCOMPILING_EMULATOR, ex: wine), not at the build
    gtest_discover_tests(${TARGET}
        DISCOVERY_MODE PRE_TEST
        DISCOVERY_TIMEOUT 120 # first start of wine
    )
else()
    gtest_discover_tests(${TARGET})
endif()
