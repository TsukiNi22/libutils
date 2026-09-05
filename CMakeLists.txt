cmake_minimum_required(VERSION 3.20)
set(TARGET utils)
project(${TARGET} VERSION 2.12.1 LANGUAGES CXX)

# =========================
# Options
# =========================
option(UTILS_DEV_PACKAGE "Build development package only" OFF)
option(UTILS_PACKAGE "Build global package" OFF)
option(UTILS_STABLE_RELEASE "Channel as stable release" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

# =========================
# Compilateur & Standard
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Moderne compilation -fPIE

# =========================
# Requirement
# =========================
find_package(PkgConfig REQUIRED)
find_package(Python3 REQUIRED) # Used for the special dependencie
find_package(OpenSSL REQUIRED) # Used for encryption purpose
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
    -Wno-c++23-extensions
)

# =========================
# Sources
# =========================
set(SRC
    ## Exception
    src/utils/exception/AException.cpp

    ## IO - Manip
    src/utils/manip/iomanip/ANSI.cpp

    ## S - Manip
    src/utils/manip/smanip/format.cpp

    ## S - Manip (sub-part: codec)
    src/utils/manip/smanip/codec/Base64Codec.cpp

    ## S - Manip (sub-part: parser)
    src/utils/manip/smanip/parser/EETPParser.cpp

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

    ## Network
    src/utils/network/Client.cpp
    src/utils/network/Server.cpp

    ## Network (sub-part: socket)
    src/utils/network/socket/ASocket.cpp
    src/utils/network/socket/TCPSocket.cpp

    ## Security (sub-part: encryption)
    src/utils/security/encryption/AESKey.cpp
    src/utils/security/encryption/RSAKey.cpp
    src/utils/security/encryption/CommonRSAKey.cpp

    ## Security (sub-part: observer)
    src/utils/security/observer/Instances.cpp
    src/utils/security/observer/ANotifier.cpp
    src/utils/security/observer/MemoryLeakNotifier.cpp

    # Encapsulation
    src/utils/encapsulation/SharedObject.cpp
    src/utils/encapsulation/Pipe.cpp
    src/utils/encapsulation/Dup.cpp
    src/utils/encapsulation/Process.cpp
    src/utils/encapsulation/SharedMemory.cpp

    # System
    src/utils/system/Scheduler.cpp
)
add_library(${TARGET} STATIC ${SRC})
#add_library(${TARGET} SHARED ${SRC})

# =========================
# Special Targets
# =========================
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

file(GLOB_RECURSE EXCEPTION_CONFIG_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/cmake/config/exceptions/*.json"
)
set(GENERATED_EXCEPTION_HEADER
    "${CMAKE_SOURCE_DIR}/include/utils/exception/generated_internal_exception_header.hpp"
)
add_custom_command(
    OUTPUT ${GENERATED_EXCEPTION_HEADER}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/scripts/generate_exception_header.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${EXCEPTION_CONFIG_FILES} ${CMAKE_SOURCE_DIR}/cmake/scripts/generate_exception_header.py
    COMMENT "Generating exception header (config changed)"
    VERBATIM
)
add_custom_target(generated_internal_exception_header
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
add_custom_target(install_release
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Debug
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Asan
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install --parallel
    COMMENT "installing both debug versions Debug & Asan (libutils_debug.a, libutils_asan.a) and Optimized (libutils.a) one"
    VERBATIM
)
add_custom_target(package_release
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Debug
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Asan
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized -DUTILS_DEV_PACKAGE=ON
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized -DUTILS_PACKAGE=ON
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMENT "Building both debug versions Debug & Asan (libutils-db, libutils-as) and Optimized (libutils-op) one with the header (libutils-dev)"
    VERBATIM
)

# =========================
# Dependencies
# =========================
add_dependencies(${TARGET} generated_internal_exception_header)
target_link_libraries(${TARGET} PRIVATE OpenSSL::SSL OpenSSL::Crypto) # Network & Codec

# =========================
# Includes
# =========================
target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# =========================
# Headers configuration
# =========================

# version injection using @VAR@
set(LIBUTILS_VERSION "v${PROJECT_VERSION}")
configure_file(
    ${CMAKE_SOURCE_DIR}/include/utils/version.hpp.in
    ${CMAKE_BINARY_DIR}/generated/utils/version.hpp
    @ONLY
)

# =========================
# Modes
# =========================
set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "utils$<$<CONFIG:Debug>:_debug>$<$<CONFIG:Asan>:_asan>"
)

target_compile_options(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:-g -ggdb3>
    $<$<CONFIG:Asan>:-fsanitize=address -fno-omit-frame-pointer -g -ggdb3>
    $<$<CONFIG:Optimized>:-O3 -ffast-math -funroll-loops -fno-plt -pipe -DNDEBUG>
)
target_link_options(${TARGET} PRIVATE
    $<$<CONFIG:Asan>:-fsanitize=address>
)

# =========================
# Installation
# =========================
if(NOT UTILS_PACKAGE)
include(GNUInstallDirs) # load variable used for installation
include(CMakePackageConfigHelpers)

# licenses
install(
    FILES ${CMAKE_SOURCE_DIR}/LICENSE
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/${CPACK_PACKAGE_NAME}
    COMPONENT headers
)

# library
install(TARGETS ${TARGET}
    EXPORT ${TARGET}Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT lib
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# header
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers
    PATTERN "*.in" EXCLUDE
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/generated/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers
)

# CMake setup for find_package(utils CONFIG REQUIRED)
install(EXPORT ${TARGET}Targets
    FILE ${TARGET}Targets.cmake
    NAMESPACE ${TARGET}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
)

# Not SECURE!!!!!!!!!! Depends on the 'CMakeFiles/Export' path
# Fuck EXPORT, Fuck Export/<hash>/, Fuck!!!!!!
set(EXPORT_PATH "CMakeFiles/Export")
if(UTILS_DEV_PACKAGE)
    # main-headers
    file(GLOB_RECURSE UTILS_EXPORT_FILES "${CMAKE_BINARY_DIR}/${EXPORT_PATH}/*/${TARGET}Targets.cmake")
    install(
        FILES ${UTILS_EXPORT_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
        COMPONENT headers
    )

    # sub-headers
    file(GLOB_RECURSE UTILS_EXPORT_FILES "${CMAKE_BINARY_DIR}/${EXPORT_PATH}/*/${TARGET}Targets-debug.cmake")
    install(
        FILES ${UTILS_EXPORT_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
        COMPONENT headers-db
    )
    file(GLOB_RECURSE UTILS_EXPORT_FILES "${CMAKE_BINARY_DIR}/${EXPORT_PATH}/*/${TARGET}Targets-asan.cmake")
    install(
        FILES ${UTILS_EXPORT_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
        COMPONENT headers-as
    )
    file(GLOB_RECURSE UTILS_EXPORT_FILES
        "${CMAKE_BINARY_DIR}/${EXPORT_PATH}/*/${TARGET}Targets-optimized.cmake"
        "${CMAKE_BINARY_DIR}/${EXPORT_PATH}/*/${TARGET}Targets-noconfig.cmake" # No config select optimized by default
    )
    install(
        FILES ${UTILS_EXPORT_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
        COMPONENT headers-op
    )
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
    COMPONENT headers
)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/package/${TARGET}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
)

endif()
# =========================
# Utils
# =========================
add_custom_target(get_unregistered_files
    COMMAND bash ${CMAKE_SOURCE_DIR}/cmake/scripts/check_files.sh unregistered ${SRC}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# =========================
# Package Building
# =========================
set(CPACK_PACKAGE_VENDOR "TsukiNi22")
set(CPACK_PACKAGE_CONTACT "xartania.contact@gmail.com")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_GENERATOR "RPM;DEB")

# Setup the channel name
if(UTILS_STABLE_RELEASE)
    set(CHANNEL "")
    set(MIRROR_CHANNEL "-pre")
else()
    set(CHANNEL "-pre")
    set(MIRROR_CHANNEL "")
endif()

# Setup the package name
if(UTILS_PACKAGE)
    set(CPACK_PACKAGE_NAME "libutils${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "libutils${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (global)")
elseif(UTILS_DEV_PACKAGE)
    set(CPACK_PACKAGE_NAME "libutils-dev${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "libutils-dev${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (headers/cmake)")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_PACKAGE_NAME "libutils-db${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "libutils-db${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (debug)")
    set(PREFIX "db")
elseif(CMAKE_BUILD_TYPE STREQUAL "Asan")
    set(CPACK_PACKAGE_NAME "libutils-as${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "libutils-as${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (asan)")
    set(PREFIX "as")
else()
    set(CPACK_PACKAGE_NAME "libutils-op${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "libutils-op${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (optimized)")
    set(PREFIX "op")
endif()

# Setup the conflict between stable & unstable packages
if(UTILS_STABLE_RELEASE)
    # If a recent stable version exist the libutils-*-pre installation will result in the stable version
    set(CPACK_RPM_PACKAGE_OBSOLETES "${MIRROR_PACKAGE_NAME} <= ${PROJECT_VERSION}")
    set(CPACK_DEBIAN_PACKAGE_REPLACES "${MIRROR_PACKAGE_NAME} (<= ${PROJECT_VERSION})")
endif()

set(CPACK_RPM_PACKAGE_CONFLICTS "${MIRROR_PACKAGE_NAME}")
set(CPACK_DEBIAN_PACKAGE_BREAKS "${MIRROR_PACKAGE_NAME}")
set(CPACK_DEBIAN_PACKAGE_CONFLICTS "${MIRROR_PACKAGE_NAME}")

# Separate headers & lib
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
if(UTILS_PACKAGE)
    set(CPACK_COMPONENTS_ALL "")
    set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF) # In case~
elseif(UTILS_DEV_PACKAGE)
    set(CPACK_COMPONENTS_ALL headers headers-db headers-as headers-op)

    set(CPACK_RPM_HEADERS-DB_PACKAGE_NAME "libutils-dev-db${CHANNEL}")
    set(CPACK_RPM_HEADERS-AS_PACKAGE_NAME "libutils-dev-as${CHANNEL}")
    set(CPACK_RPM_HEADERS-OP_PACKAGE_NAME "libutils-dev-op${CHANNEL}")

    set(CPACK_DEBIAN_HEADERS-DB_PACKAGE_NAME "libutils-dev-db${CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-AS_PACKAGE_NAME "libutils-dev-as${CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-OP_PACKAGE_NAME "libutils-dev-op${CHANNEL}")

    # Setup the conflict between stable & unstable packages
    if(UTILS_STABLE_RELEASE)
        set(CPACK_RPM_HEADERS-DB_PACKAGE_OBSOLETES "libutils-dev-db${MIRROR_CHANNEL} <= ${PROJECT_VERSION}")
        set(CPACK_RPM_HEADERS-AS_PACKAGE_OBSOLETES "libutils-dev-as${MIRROR_CHANNEL} <= ${PROJECT_VERSION}")
        set(CPACK_RPM_HEADERS-OP_PACKAGE_OBSOLETES "libutils-dev-op${MIRROR_CHANNEL} <= ${PROJECT_VERSION}")

        set(CPACK_DEBIAN_HEADERS-DB_PACKAGE_REPLACES "libutils-dev-db${MIRROR_CHANNEL} (<= ${PROJECT_VERSION})")
        set(CPACK_DEBIAN_HEADERS-AS_PACKAGE_REPLACES "libutils-dev-as${MIRROR_CHANNEL} (<= ${PROJECT_VERSION})")
        set(CPACK_DEBIAN_HEADERS-OP_PACKAGE_REPLACES "libutils-dev-op${MIRROR_CHANNEL} (<= ${PROJECT_VERSION})")
    endif()

    set(CPACK_RPM_HEADERS-DB_PACKAGE_CONFLICTS "libutils-dev-db${MIRROR_CHANNEL}")
    set(CPACK_RPM_HEADERS-AS_PACKAGE_CONFLICTS "libutils-dev-as${MIRROR_CHANNEL}")
    set(CPACK_RPM_HEADERS-OP_PACKAGE_CONFLICTS "libutils-dev-op${MIRROR_CHANNEL}")

    set(CPACK_DEBIAN_HEADERS-DB_PACKAGE_BREAKS "libutils-dev-db${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-DB_PACKAGE_CONFLICTS "libutils-dev-db${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-AS_PACKAGE_BREAKS "libutils-dev-as${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-AS_PACKAGE_CONFLICTS "libutils-dev-as${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-OP_PACKAGE_BREAKS "libutils-dev-op${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-OP_PACKAGE_CONFLICTS "libutils-dev-op${MIRROR_CHANNEL}")
else()
    set(CPACK_COMPONENTS_ALL lib)
endif()

# Edit release value (run count)
if(UTILS_STABLE_RELEASE)
    # A stable version should never have to be rebuild and should always be removed or marked as depreacted
    set(CPACK_RPM_PACKAGE_RELEASE "1")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "1")
elseif(DEFINED PACKAGE_RELEASE)
    set(CPACK_RPM_PACKAGE_RELEASE "${PACKAGE_RELEASE}")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "${PACKAGE_RELEASE}")
else() # 0 is impossible for github.run_number
    set(CPACK_RPM_PACKAGE_RELEASE "0")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "0")
endif()

# Dependencies
if(UTILS_PACKAGE)
    if(UTILS_STABLE_RELEASE)
        set(CPACK_RPM_PACKAGE_REQUIRES "libutils-op${CHANNEL} >= ${PROJECT_VERSION}")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libutils-op${CHANNEL} (>= ${PROJECT_VERSION})")
    else()
        set(CPACK_RPM_PACKAGE_REQUIRES "libutils-op${CHANNEL} = ${PROJECT_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libutils-op${CHANNEL} (= ${PROJECT_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE})")
    endif()
elseif(UTILS_DEV_PACKAGE)
    set(CPACK_RPM_PACKAGE_REQUIRES "")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
else()
    if(UTILS_STABLE_RELEASE)
        set(CPACK_RPM_PACKAGE_REQUIRES "libutils-dev${CHANNEL} >= ${PROJECT_VERSION}, libutils-dev-${PREFIX}${CHANNEL} >= ${PROJECT_VERSION}, openssl-libs")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libutils-dev${CHANNEL} (>= ${PROJECT_VERSION}), libutils-dev-${PREFIX}${CHANNEL} (>= ${PROJECT_VERSION}), libssl3")
    else()
        set(CPACK_RPM_PACKAGE_REQUIRES "libutils-dev${CHANNEL} = ${PROJECT_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}, libutils-dev-${PREFIX}${CHANNEL} = ${PROJECT_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}, openssl-libs")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libutils-dev${CHANNEL} (= ${PROJECT_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}), libutils-dev-${PREFIX}${CHANNEL} (= ${PROJECT_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}), libssl3")
    endif()

    # Suggests the installation of gdb along side the debug version
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CPACK_RPM_PACKAGE_SUGGESTS "gdb")
        set(CPACK_DEBIAN_PACKAGE_SUGGESTS "gdb")
    endif()
endif()

# RPM
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_LIB_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_RPM_HEADERS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})

# Debian
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "TsukiNi22")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/TsukiNi22/libutils")
set(CPACK_DEBIAN_LIB_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_DEBIAN_HEADERS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})

include(CPack)
