#!/usr/bin/env bash
# -*- Mode: sh; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Build script for namigator using CMake presets

set -eux

# Default values
PRESET="linux-release"
INSTALL_PREFIX="/usr/local"
INSTALL_CMD="sudo"
PROCS="$(nproc)"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --preset)
            PRESET="$2"
            shift 2
            ;;
        --install-prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --no-sudo)
            INSTALL_CMD=""
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --preset PRESET         CMake preset to use (default: linux-release)"
            echo "  --install-prefix PATH   Installation prefix (default: /usr/local)"
            echo "  --no-sudo              Don't use sudo for installation"
            echo "  --help                 Show this help message"
            echo ""
            echo "Available presets:"
            cmake --list-presets | grep -E '"[a-z-]+"' | sed 's/.*"\(.*\)".*/  \1/'
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Set compiler environment variables if using clang
if command -v clang &> /dev/null && command -v clang++ &> /dev/null; then
    export CC=/usr/bin/clang
    export CXX=/usr/bin/clang++
fi

export CMAKE_EXPORT_COMPILE_COMMANDS=true

BUILD_ROOT="$(pwd)"

# Extract build directory from preset
BUILD_DIR="${BUILD_ROOT}/build/${PRESET}"

echo "Building namigator with preset: ${PRESET}"
echo "Build directory: ${BUILD_DIR}"
echo "Install prefix: ${INSTALL_PREFIX}"

## CMake: configuration using preset
cmake --preset "${PRESET}" \
    -DCMAKE_INSTALL_PREFIX:STRING="${INSTALL_PREFIX}"

## CMake: build
cmake --build --preset "${PRESET}" -- -j"${PROCS}"

## CMake: installation
if [ -n "${INSTALL_CMD}" ]; then
    ${INSTALL_CMD} cmake --install "${BUILD_DIR}" --config Release
else
    cmake --install "${BUILD_DIR}" --config Release
fi

echo "Build completed successfully!"
echo "Namigator has been installed to: ${INSTALL_PREFIX}"
