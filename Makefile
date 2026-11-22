.PHONY: help configure build test clean debug release debug-gcc debug-clang debug-clang-libcxx debug-msvc release-gcc release-clang release-clang-libcxx release-msvc all

help:
	@echo "Available targets:"
	@echo ""
	@echo "Debug builds:"
	@echo "  make debug-gcc          - Configure & build Debug with GCC"
	@echo "  make debug-clang        - Configure & build Debug with Clang"
	@echo "  make debug-clang-libcxx - Configure & build Debug with Clang + libc++"
	@echo "  make debug-msvc         - Configure & build Debug with MSVC (Windows)"
	@echo ""
	@echo "Release builds:"
	@echo "  make release-gcc        - Configure & build Release with GCC"
	@echo "  make release-clang      - Configure & build Release with Clang"
	@echo "  make release-clang-libcxx - Configure & build Release with Clang + libc++"
	@echo "  make release-msvc       - Configure & build Release with MSVC (Windows)"
	@echo ""
	@echo "Quick shortcuts:"
	@echo "  make debug              - Build Debug (GCC)"
	@echo "  make release            - Build Release (GCC)"
	@echo "  make all                - Build all presets"
	@echo ""
	@echo "Development:"
	@echo "  make configure PRESET=<name> - Configure only"
	@echo "  make build PRESET=<name>     - Build only"
	@echo "  make test PRESET=<name>      - Run tests"
	@echo "  make clean              - Remove all build directories"
	@echo "  make clean-<preset>     - Remove specific build directory"

# Default preset
PRESET ?= debug-gcc

# Configure only
configure:
	cmake --preset $(PRESET)

# Build only
build:
	cmake --build --preset $(PRESET)

# Configure + Build
configure-build: configure build

# Tests
test:
	ctest --preset $(PRESET) --output-on-failure

# Individual preset targets - Debug
debug-gcc: PRESET=debug-gcc
debug-gcc: configure-build test

debug-clang: PRESET=debug-clang
debug-clang: configure-build test

debug-clang-libcxx: PRESET=debug-clang-libcxx
debug-clang-libcxx: configure-build test

debug-msvc: PRESET=debug-msvc
debug-msvc: configure-build test

# Individual preset targets - Release
release-gcc: PRESET=release-gcc
release-gcc: configure-build test

release-clang: PRESET=release-clang
release-clang: configure-build test

release-clang-libcxx: PRESET=release-clang-libcxx
release-clang-libcxx: configure-build test

release-msvc: PRESET=release-msvc
release-msvc: configure-build test

# Quick aliases
debug: debug-gcc
release: release-gcc

# Build all presets
all: debug-gcc release-gcc debug-clang release-clang debug-clang-libcxx release-clang-libcxx

# Clean specific build directory
clean-%:
	rm -rf build/$*

# Clean all
clean:
	rm -rf build/Debug-* build/Release-*

.PHONY: configure build configure-build test help clean all debug release
