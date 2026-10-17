cmake_minimum_required(VERSION 3.20)
set(TARGET utils)
if(NOT CMAKE_TOOLCHAIN_FILE AND NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER clang++)
endif()
project(${TARGET} VERSION 4.2.0 LANGUAGES CXX)

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
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Moderne compilation -fPIE
add_compile_definitions(BACKWARD_COMPATIBILITY_WARNING) # To check internal usage for potential depreacted function

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
    ## Platform
    src/utils/platform/Unistd.cpp
    src/utils/platform/Socket.cpp
    src/utils/platform/Epoll.cpp
    src/utils/platform/Dlfcn.cpp
    src/utils/platform/Termios.cpp

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
    src/utils/encapsulation/Poll.cpp
    src/utils/encapsulation/SharedMemory.cpp

    # System
    src/utils/system/Scheduler.cpp
)

# Windows only (internal Win32 helpers)
set(SRC_WINDOWS
    ## Platform
    src/utils/platform/Windows.cpp
)
if(WIN32)
    list(APPEND SRC ${SRC_WINDOWS})
endif()
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
# Asan packages: not with MinGW (Windows packages mingw64-libutils*, built with the MinGW toolchain)
set(PACKAGE_ASAN_COMMANDS "")
if(NOT WIN32)
    list(APPEND PACKAGE_ASAN_COMMANDS
        COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Asan -DUTILS_DEV_PACKAGE=OFF -DUTILS_PACKAGE=OFF
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    )
endif()

add_custom_target(package_release
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Debug -DUTILS_DEV_PACKAGE=OFF -DUTILS_PACKAGE=OFF
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    ${PACKAGE_ASAN_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized -DUTILS_DEV_PACKAGE=OFF -DUTILS_PACKAGE=OFF
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized -DUTILS_DEV_PACKAGE=ON -DUTILS_PACKAGE=OFF
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized -DUTILS_DEV_PACKAGE=OFF -DUTILS_PACKAGE=ON
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMENT "Building the debug versions Debug & Asan (libutils-db, libutils-as) and the Optimized one (libutils-op) with the header (libutils-dev), mingw64-libutils* without Asan with the MinGW toolchain"
    VERBATIM
)

# =========================
# Dependencies
# =========================
add_dependencies(${TARGET} generated_internal_exception_header)
target_link_libraries(${TARGET} PRIVATE OpenSSL::SSL OpenSSL::Crypto) # Network & Codec
if(WIN32)
    target_link_libraries(${TARGET} PUBLIC ws2_32) # Winsock (sockets)
endif()

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

if(WIN32)
    # MinGW: no AddressSanitizer (Asan = Debug), no PLT (not ELF)
    target_compile_options(${TARGET} PRIVATE
        $<$<CONFIG:Debug,Asan>:-g -ggdb3>
        $<$<CONFIG:Optimized>:-O3 -ffast-math -funroll-loops -pipe -DNDEBUG>
    )
else()
    target_compile_options(${TARGET} PRIVATE
        $<$<CONFIG:Debug>:-g -ggdb3>
        $<$<CONFIG:Asan>:-fsanitize=address -fno-omit-frame-pointer -g -ggdb3>
        $<$<CONFIG:Optimized>:-O3 -ffast-math -funroll-loops -fno-plt -pipe -DNDEBUG>
    )
    target_link_options(${TARGET} PRIVATE
        $<$<CONFIG:Asan>:-fsanitize=address>
    )
endif()

# =========================
# Installation
# =========================
if(NOT UTILS_PACKAGE)
include(GNUInstallDirs) # load variable used for installation
include(CMakePackageConfigHelpers)

# licenses
install(
    FILES ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_SOURCE_DIR}/THIRD_PARTY_NOTICES.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/lib${TARGET} # CPACK_PACKAGE_NAME is not set yet here
    COMPONENT headers
)
install(
    FILES ${CMAKE_SOURCE_DIR}/include/utils/algorithms/LICENSE.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/lib${TARGET} # CPACK_PACKAGE_NAME is not set yet here
    RENAME LICENSE-algorithms.md
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

set(CONFIG_INSTALL_DIR
    ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/package/${TARGET}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}ConfigVersion.cmake
    DESTINATION ${CONFIG_INSTALL_DIR}
    COMPONENT headers
)

endif()
# =========================
# Utils
# =========================
add_custom_target(get_unregistered_files
    COMMAND bash ${CMAKE_SOURCE_DIR}/cmake/scripts/check_files.sh unregistered ${SRC} ${SRC_WINDOWS}
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
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/cmake/package/CPackProjectConfig.cmake") # install prefix per generator

# Arch Linux (pacman): CPack External generator, one .pkg.tar.zst per component written by cmake/package/CPackArch.cmake
find_program(BSDTAR_PROGRAM bsdtar)
if(NOT WIN32 AND BSDTAR_PROGRAM)
    list(APPEND CPACK_GENERATOR "External")
    set(CPACK_EXTERNAL_ENABLE_STAGING ON)
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/package/CPackArch.cmake")
elseif(NOT WIN32)
    message(STATUS "Arch packages skipped: bsdtar (libarchive) not found")
endif()

# Windows (cross compiled): mingw64-libutils*, installed in the MinGW sysroot (found by the MinGW toolchain)
if(WIN32)
    set(PACKAGE_BASE "mingw64-libutils")
    set(CPACK_UTILS_RPM_PREFIX "/usr/x86_64-w64-mingw32/sys-root/mingw") # Fedora (mingw64-filesystem)
    set(CPACK_UTILS_DEB_PREFIX "/usr/x86_64-w64-mingw32") # Debian / Ubuntu (mingw-w64)
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "noarch") # Windows binaries: no host architecture
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
    set(CPACK_RPM_PACKAGE_AUTOREQPROV OFF)
    set(CPACK_RPM_SPEC_MORE_DEFINE "%define __os_install_post %{nil}\n%define debug_package %{nil}") # no host strip / debuginfo
    set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
        /usr/x86_64-w64-mingw32
        /usr/x86_64-w64-mingw32/sys-root
        /usr/x86_64-w64-mingw32/sys-root/mingw
        /usr/x86_64-w64-mingw32/sys-root/mingw/include
        /usr/x86_64-w64-mingw32/sys-root/mingw/lib
        /usr/x86_64-w64-mingw32/sys-root/mingw/lib/cmake
        /usr/x86_64-w64-mingw32/sys-root/mingw/share
        /usr/x86_64-w64-mingw32/sys-root/mingw/share/licenses
    )
else()
    set(PACKAGE_BASE "libutils")
endif()

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
    set(CPACK_PACKAGE_NAME "${PACKAGE_BASE}${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "${PACKAGE_BASE}${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (global)")
elseif(UTILS_DEV_PACKAGE)
    set(CPACK_PACKAGE_NAME "${PACKAGE_BASE}-dev${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "${PACKAGE_BASE}-dev${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (headers/cmake)")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_PACKAGE_NAME "${PACKAGE_BASE}-db${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "${PACKAGE_BASE}-db${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (debug)")
    set(PREFIX "db")
elseif(CMAKE_BUILD_TYPE STREQUAL "Asan")
    set(CPACK_PACKAGE_NAME "${PACKAGE_BASE}-as${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "${PACKAGE_BASE}-as${MIRROR_CHANNEL}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (asan)")
    set(PREFIX "as")
else()
    set(CPACK_PACKAGE_NAME "${PACKAGE_BASE}-op${CHANNEL}")
    set(MIRROR_PACKAGE_NAME "${PACKAGE_BASE}-op${MIRROR_CHANNEL}")
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
    if(WIN32)
        set(CPACK_COMPONENTS_ALL headers headers-db headers-op) # no Asan with MinGW
    else()
        set(CPACK_COMPONENTS_ALL headers headers-db headers-as headers-op)
    endif()

    set(CPACK_RPM_HEADERS-DB_PACKAGE_NAME "${PACKAGE_BASE}-dev-db${CHANNEL}")
    set(CPACK_RPM_HEADERS-AS_PACKAGE_NAME "${PACKAGE_BASE}-dev-as${CHANNEL}")
    set(CPACK_RPM_HEADERS-OP_PACKAGE_NAME "${PACKAGE_BASE}-dev-op${CHANNEL}")

    set(CPACK_DEBIAN_HEADERS-DB_PACKAGE_NAME "${PACKAGE_BASE}-dev-db${CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-AS_PACKAGE_NAME "${PACKAGE_BASE}-dev-as${CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-OP_PACKAGE_NAME "${PACKAGE_BASE}-dev-op${CHANNEL}")

    # Setup the conflict between stable & unstable packages
    if(UTILS_STABLE_RELEASE)
        set(CPACK_RPM_HEADERS-DB_PACKAGE_OBSOLETES "${PACKAGE_BASE}-dev-db${MIRROR_CHANNEL} <= ${PROJECT_VERSION}")
        set(CPACK_RPM_HEADERS-AS_PACKAGE_OBSOLETES "${PACKAGE_BASE}-dev-as${MIRROR_CHANNEL} <= ${PROJECT_VERSION}")
        set(CPACK_RPM_HEADERS-OP_PACKAGE_OBSOLETES "${PACKAGE_BASE}-dev-op${MIRROR_CHANNEL} <= ${PROJECT_VERSION}")

        set(CPACK_DEBIAN_HEADERS-DB_PACKAGE_REPLACES "${PACKAGE_BASE}-dev-db${MIRROR_CHANNEL} (<= ${PROJECT_VERSION})")
        set(CPACK_DEBIAN_HEADERS-AS_PACKAGE_REPLACES "${PACKAGE_BASE}-dev-as${MIRROR_CHANNEL} (<= ${PROJECT_VERSION})")
        set(CPACK_DEBIAN_HEADERS-OP_PACKAGE_REPLACES "${PACKAGE_BASE}-dev-op${MIRROR_CHANNEL} (<= ${PROJECT_VERSION})")
    endif()

    set(CPACK_RPM_HEADERS-DB_PACKAGE_CONFLICTS "${PACKAGE_BASE}-dev-db${MIRROR_CHANNEL}")
    set(CPACK_RPM_HEADERS-AS_PACKAGE_CONFLICTS "${PACKAGE_BASE}-dev-as${MIRROR_CHANNEL}")
    set(CPACK_RPM_HEADERS-OP_PACKAGE_CONFLICTS "${PACKAGE_BASE}-dev-op${MIRROR_CHANNEL}")

    set(CPACK_DEBIAN_HEADERS-DB_PACKAGE_BREAKS "${PACKAGE_BASE}-dev-db${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-DB_PACKAGE_CONFLICTS "${PACKAGE_BASE}-dev-db${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-AS_PACKAGE_BREAKS "${PACKAGE_BASE}-dev-as${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-AS_PACKAGE_CONFLICTS "${PACKAGE_BASE}-dev-as${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-OP_PACKAGE_BREAKS "${PACKAGE_BASE}-dev-op${MIRROR_CHANNEL}")
    set(CPACK_DEBIAN_HEADERS-OP_PACKAGE_CONFLICTS "${PACKAGE_BASE}-dev-op${MIRROR_CHANNEL}")
else()
    set(CPACK_COMPONENTS_ALL lib)
endif()

# Edit release value (run count)
if(UTILS_STABLE_RELEASE)
    # A stable version should never have to be rebuild and should always be removed or marked as depreacted
    set(CPACK_RPM_PACKAGE_RELEASE "1")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "1")
    set(CPACK_ARCH_PACKAGE_RELEASE "1")
elseif(DEFINED PACKAGE_RELEASE)
    set(CPACK_RPM_PACKAGE_RELEASE "${PACKAGE_RELEASE}")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "${PACKAGE_RELEASE}")
    set(CPACK_ARCH_PACKAGE_RELEASE "${PACKAGE_RELEASE}")
else() # 0 is impossible for github.run_number
    set(CPACK_RPM_PACKAGE_RELEASE "0")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "0")
    set(CPACK_ARCH_PACKAGE_RELEASE "0")
endif()

# Dependencies
if(WIN32)
    set(RPM_OPENSSL "mingw64-openssl")
    set(DEBIAN_OPENSSL "")
else()
    set(RPM_OPENSSL "openssl-libs")
    set(DEBIAN_OPENSSL ", libssl3")
endif()
if(UTILS_PACKAGE)
    if(UTILS_STABLE_RELEASE)
        set(CPACK_RPM_PACKAGE_REQUIRES "${PACKAGE_BASE}-op${CHANNEL} >= ${PROJECT_VERSION}")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${PACKAGE_BASE}-op${CHANNEL} (>= ${PROJECT_VERSION})")
    else()
        set(CPACK_RPM_PACKAGE_REQUIRES "${PACKAGE_BASE}-op${CHANNEL} = ${PROJECT_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${PACKAGE_BASE}-op${CHANNEL} (= ${PROJECT_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE})")
    endif()
elseif(UTILS_DEV_PACKAGE)
    if(WIN32)
        set(CPACK_RPM_PACKAGE_REQUIRES "mingw64-openssl") # public headers include <openssl/...> (no Debian package of the MinGW OpenSSL)
    else()
        set(CPACK_RPM_PACKAGE_REQUIRES "openssl-devel") # public headers include <openssl/...>
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libssl-dev")
    endif()
else()
    if(UTILS_STABLE_RELEASE)
        set(CPACK_RPM_PACKAGE_REQUIRES "${PACKAGE_BASE}-dev${CHANNEL} >= ${PROJECT_VERSION}, ${PACKAGE_BASE}-dev-${PREFIX}${CHANNEL} >= ${PROJECT_VERSION}, ${RPM_OPENSSL}")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${PACKAGE_BASE}-dev${CHANNEL} (>= ${PROJECT_VERSION}), ${PACKAGE_BASE}-dev-${PREFIX}${CHANNEL} (>= ${PROJECT_VERSION})${DEBIAN_OPENSSL}")
    else()
        set(CPACK_RPM_PACKAGE_REQUIRES "${PACKAGE_BASE}-dev${CHANNEL} = ${PROJECT_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}, ${PACKAGE_BASE}-dev-${PREFIX}${CHANNEL} = ${PROJECT_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}, ${RPM_OPENSSL}")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${PACKAGE_BASE}-dev${CHANNEL} (= ${PROJECT_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}), ${PACKAGE_BASE}-dev-${PREFIX}${CHANNEL} (= ${PROJECT_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE})${DEBIAN_OPENSSL}")
    endif()

    # Suggests the installation of gdb along side the debug version
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
        set(CPACK_RPM_PACKAGE_SUGGESTS "gdb")
        set(CPACK_DEBIAN_PACKAGE_SUGGESTS "gdb")
    endif()
endif()

# Arch (same names, channels and dependencies as the RPM / DEB, read by cmake/package/CPackArch.cmake)
set(CPACK_ARCH_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_ARCH_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
set(CPACK_ARCH_PACKAGE_URL "https://github.com/TsukiNi22/libutils")
set(CPACK_ARCH_PACKAGE_PACKAGER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
set(CPACK_ARCH_PACKAGE_LICENSE "MIT" "CC-BY-NC-SA-4.0")
set(CPACK_ARCH_PACKAGE_ARCHITECTURE "any") # meta & headers
set(CPACK_ARCH_LIB_PACKAGE_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_ARCH_PACKAGE_CONFLICTS "${MIRROR_PACKAGE_NAME}")
if(UTILS_STABLE_RELEASE)
    set(CPACK_ARCH_PACKAGE_REPLACES "${MIRROR_PACKAGE_NAME}<=${PROJECT_VERSION}")
endif()
foreach(sub db as op)
    string(TOUPPER "${sub}" SUB)
    set(CPACK_ARCH_HEADERS-${SUB}_PACKAGE_NAME "${PACKAGE_BASE}-dev-${sub}${CHANNEL}")
    set(CPACK_ARCH_HEADERS-${SUB}_PACKAGE_CONFLICTS "${PACKAGE_BASE}-dev-${sub}${MIRROR_CHANNEL}")
    if(UTILS_STABLE_RELEASE)
        set(CPACK_ARCH_HEADERS-${SUB}_PACKAGE_REPLACES "${PACKAGE_BASE}-dev-${sub}${MIRROR_CHANNEL}<=${PROJECT_VERSION}")
    endif()
endforeach()
if(UTILS_STABLE_RELEASE)
    set(ARCH_VERSION ">=${PROJECT_VERSION}")
else()
    set(ARCH_VERSION "=${PROJECT_VERSION}-${CPACK_ARCH_PACKAGE_RELEASE}")
endif()
if(UTILS_PACKAGE)
    set(CPACK_ARCH_PACKAGE_DEPENDS "${PACKAGE_BASE}-op${CHANNEL}${ARCH_VERSION}")
elseif(UTILS_DEV_PACKAGE)
    set(CPACK_ARCH_PACKAGE_DEPENDS "openssl") # public headers include <openssl/...> (headers & libraries in one package on Arch)
else()
    set(CPACK_ARCH_PACKAGE_DEPENDS
        "${PACKAGE_BASE}-dev${CHANNEL}${ARCH_VERSION}"
        "${PACKAGE_BASE}-dev-${PREFIX}${CHANNEL}${ARCH_VERSION}"
        "openssl"
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CPACK_ARCH_PACKAGE_OPTDEPENDS "gdb: debugging")
    endif()
endif()

# RPM
set(CPACK_RPM_PACKAGE_LICENSE "MIT AND CC-BY-NC-SA-4.0") # MIT for libutils, CC BY-NC-SA 4.0 for include/utils/algorithms
set(CPACK_RPM_LIB_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_RPM_HEADERS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})

# Debian
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "TsukiNi22")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/TsukiNi22/libutils")
set(CPACK_DEBIAN_LIB_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_DEBIAN_HEADERS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})

include(CPack)
