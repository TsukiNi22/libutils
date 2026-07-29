cmake_minimum_required(VERSION 3.20)
set(TARGET utils)
project(${TARGET} VERSION 2.1.0 LANGUAGES CXX)
option(UTILS_DEV_PACKAGE "Build development package only" OFF)
option(UTILS_PACKAGE "Build global package" OFF)

# =========================
# Compilateur & Standard
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Moderne compilation -fPIE
add_definitions(-D_NoWarning -DNO_LINKER_WARNING -DNO_USAGE_WARNING) # Remove warnings from utils (usless for hard import)

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
if (CMAKE_BUILD_TYPE STREQUAL "Optimized" OR UTILS_DEV_PACKAGE)
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT headers
    )
endif()

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
    file(GLOB_RECURSE UTILS_EXPORT_FILES
        "${CMAKE_BINARY_DIR}/${EXPORT_PATH}/*/${TARGET}Targets.cmake"
        "${CMAKE_BINARY_DIR}/${EXPORT_PATH}/*/${TARGET}Targets-*.cmake"
    )
    install(
        FILES ${UTILS_EXPORT_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
        COMPONENT headers
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

# Setup the package name
if(UTILS_PACKAGE)
    set(CPACK_PACKAGE_NAME "libutils")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (global)")
elseif(UTILS_DEV_PACKAGE)
    set(CPACK_PACKAGE_NAME "libutils-dev")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (headers)")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_PACKAGE_NAME "libutils-db")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (debug)")
elseif(CMAKE_BUILD_TYPE STREQUAL "Asan")
    set(CPACK_PACKAGE_NAME "libutils-as")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (asan)")
else()
    set(CPACK_PACKAGE_NAME "libutils-op")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 utility library (optimized)")
endif()

# Separate headers & lib
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
if(UTILS_PACKAGE)
    set(CPACK_COMPONENTS_ALL "")
    set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF) # In case~
elseif(UTILS_DEV_PACKAGE)
    set(CPACK_COMPONENTS_ALL headers)
else()
    set(CPACK_COMPONENTS_ALL lib)
endif()

# Dependencies
if(UTILS_PACKAGE)
    set(CPACK_PACKAGE_REQUIRES "libutils-dev >= ${PROJECT_VERSION}, libutils-op >= ${PROJECT_VERSION}")
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_PACKAGE_REQUIRES}")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_PACKAGE_REQUIRES}")
elseif(UTILS_DEV_PACKAGE)
    set(CPACK_RPM_PACKAGE_REQUIRES "")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
else()
    set(CPACK_RPM_PACKAGE_REQUIRES "libutils-dev >= ${PROJECT_VERSION}, openssl-libs")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libutils-dev (>= ${PROJECT_VERSION}), libssl3")
endif()

# RPM
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_LIB_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_RPM_HEADERS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})

# Debian
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "TsukiNi22")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/TsukiNi22/libutils")
set(CPACK_DEB_LIB_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_DEB_HEADERS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})

include(CPack)
