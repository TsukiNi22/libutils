#**************************************************************#
#Edition:
#**  19/05/2026 by Tsukini
#
#File Name:
#**  Makefile
#
#File Description:
#**  Shortcuts to the CMake targets (libraries, tests, release, packages)
#**************************************************************#

BUILD_DIR := build

all:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --parallel $$(nproc)

# Build in one mode (kept by the next 'make' until another mode is asked)
debug:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR) --parallel $$(nproc)

asan:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Asan
	cmake --build $(BUILD_DIR) --parallel $$(nproc)

optimized:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Optimized
	cmake --build $(BUILD_DIR) --parallel $$(nproc)

# Unit tests (Debug): ./unit_tests at the root, run with ctest
tests:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
	cmake --build $(BUILD_DIR) --parallel $$(nproc)
	cd $(BUILD_DIR) && ctest --output-on-failure

# Debug, Asan & Optimized libraries (libutils_debug.a, libutils_asan.a, libutils.a) / packages of every variant (RPM, DEB, Arch)
release:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --target release --parallel $$(nproc)

package:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --target package_release --parallel $$(nproc)

# Install the current build (sudo make install), every variant: cmake --build build --target install_release
install:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --target install --parallel $$(nproc)

clean:
	-cmake --build $(BUILD_DIR) --target clean 2> /dev/null

fclean: clean
	-rm -rf $(BUILD_DIR) 2> /dev/null

.NOTPARALLEL:
re: fclean all

.PHONY: all debug asan optimized tests release package install clean fclean re
