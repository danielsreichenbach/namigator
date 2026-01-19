#!/usr/bin/env bash
# Developer build script with interactive preset selection

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Namigator Developer Build Script ===${NC}"
echo

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed${NC}"
    exit 1
fi

# Check CMake version
CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
CMAKE_MAJOR=$(echo $CMAKE_VERSION | cut -d. -f1)
CMAKE_MINOR=$(echo $CMAKE_VERSION | cut -d. -f2)

if [ "$CMAKE_MAJOR" -lt 3 ] || ([ "$CMAKE_MAJOR" -eq 3 ] && [ "$CMAKE_MINOR" -lt 22 ]); then
    echo -e "${RED}Error: CMake 3.22 or higher is required (found: $CMAKE_VERSION)${NC}"
    exit 1
fi

# Get available presets
echo -e "${GREEN}Available CMake presets:${NC}"
PRESETS=$(cmake --list-presets | grep -E '"[a-z-]+"' | sed 's/.*"\(.*\)".*/\1/')
PS3="Select a preset (or enter number): "
select PRESET in $PRESETS "custom"; do
    if [ -n "$PRESET" ]; then
        break
    fi
done

# If custom, ask for build type and generator
if [ "$PRESET" = "custom" ]; then
    echo
    echo -e "${YELLOW}Custom build configuration:${NC}"
    
    # Build type
    PS3="Select build type: "
    select BUILD_TYPE in Debug Release RelWithDebInfo MinSizeRel; do
        if [ -n "$BUILD_TYPE" ]; then
            break
        fi
    done
    
    # Generator
    PS3="Select generator: "
    select GENERATOR in "Unix Makefiles" "Ninja" "Ninja Multi-Config"; do
        if [ -n "$GENERATOR" ]; then
            break
        fi
    done
    
    BUILD_DIR="build/custom-$BUILD_TYPE"
else
    BUILD_DIR="build/$PRESET"
fi

# Installation options
echo
echo -e "${GREEN}Installation options:${NC}"
read -p "Install prefix [/usr/local]: " INSTALL_PREFIX
INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}

read -p "Use sudo for installation? [Y/n]: " USE_SUDO
USE_SUDO=${USE_SUDO:-Y}

# Build options
echo
echo -e "${GREEN}Build options:${NC}"
read -p "Build Python bindings? [Y/n]: " BUILD_PYTHON
BUILD_PYTHON=${BUILD_PYTHON:-Y}

read -p "Build C API? [Y/n]: " BUILD_C_API
BUILD_C_API=${BUILD_C_API:-Y}

read -p "Build executables? [Y/n]: " BUILD_EXECUTABLES
BUILD_EXECUTABLES=${BUILD_EXECUTABLES:-Y}

read -p "Install tests? [Y/n]: " INSTALL_TESTS
INSTALL_TESTS=${INSTALL_TESTS:-Y}

# Number of parallel jobs
PROCS=$(nproc)
read -p "Number of parallel build jobs [$PROCS]: " JOBS
JOBS=${JOBS:-$PROCS}

# Summary
echo
echo -e "${BLUE}Build configuration summary:${NC}"
if [ "$PRESET" = "custom" ]; then
    echo "  Build type: $BUILD_TYPE"
    echo "  Generator: $GENERATOR"
else
    echo "  Preset: $PRESET"
fi
echo "  Build directory: $BUILD_DIR"
echo "  Install prefix: $INSTALL_PREFIX"
echo "  Python bindings: $BUILD_PYTHON"
echo "  C API: $BUILD_C_API"
echo "  Executables: $BUILD_EXECUTABLES"
echo "  Tests: $INSTALL_TESTS"
echo "  Parallel jobs: $JOBS"
echo

read -p "Continue? [Y/n]: " CONTINUE
CONTINUE=${CONTINUE:-Y}
if [[ ! "$CONTINUE" =~ ^[Yy]$ ]]; then
    echo "Build cancelled."
    exit 0
fi

# Convert Y/n to ON/OFF
[[ "$BUILD_PYTHON" =~ ^[Yy]$ ]] && BUILD_PYTHON=ON || BUILD_PYTHON=OFF
[[ "$BUILD_C_API" =~ ^[Yy]$ ]] && BUILD_C_API=ON || BUILD_C_API=OFF
[[ "$BUILD_EXECUTABLES" =~ ^[Yy]$ ]] && BUILD_EXECUTABLES=ON || BUILD_EXECUTABLES=OFF
[[ "$INSTALL_TESTS" =~ ^[Yy]$ ]] && INSTALL_TESTS=ON || INSTALL_TESTS=OFF

# Configure
echo
echo -e "${YELLOW}Configuring...${NC}"
if [ "$PRESET" = "custom" ]; then
    cmake -B "$BUILD_DIR" \
        -G "$GENERATOR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
        -DNAMIGATOR_BUILD_PYTHON="$BUILD_PYTHON" \
        -DNAMIGATOR_BUILD_C_API="$BUILD_C_API" \
        -DNAMIGATOR_BUILD_EXECUTABLES="$BUILD_EXECUTABLES" \
        -DNAMIGATOR_INSTALL_TESTS="$INSTALL_TESTS"
else
    cmake --preset "$PRESET" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
        -DNAMIGATOR_BUILD_PYTHON="$BUILD_PYTHON" \
        -DNAMIGATOR_BUILD_C_API="$BUILD_C_API" \
        -DNAMIGATOR_BUILD_EXECUTABLES="$BUILD_EXECUTABLES" \
        -DNAMIGATOR_INSTALL_TESTS="$INSTALL_TESTS"
fi

# Build
echo
echo -e "${YELLOW}Building...${NC}"
if [ "$PRESET" = "custom" ]; then
    cmake --build "$BUILD_DIR" -- -j"$JOBS"
else
    cmake --build --preset "$PRESET" -- -j"$JOBS"
fi

# Install
echo
echo -e "${YELLOW}Installing...${NC}"
if [[ "$USE_SUDO" =~ ^[Yy]$ ]]; then
    sudo cmake --install "$BUILD_DIR"
else
    cmake --install "$BUILD_DIR"
fi

echo
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "Namigator has been installed to: ${BLUE}$INSTALL_PREFIX${NC}"
echo
echo -e "${GREEN}To use namigator in your CMake project:${NC}"
echo -e "${YELLOW}find_package(namigator REQUIRED)${NC}"
echo -e "${YELLOW}target_link_libraries(your_target PRIVATE namigator::pathfind namigator::mapbuild)${NC}"