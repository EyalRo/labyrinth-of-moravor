#!/bin/bash
# Build script for Labyrinth of Moravor (out-of-source, in ./build)
set -e

BUILD_DIR="build"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake and Make
cmake ..
make

# Optionally, print success message and location of executable
if [ -f "moravor" ]; then
    echo "Build successful! Run ./build/moravor to start the game."
else
    echo "Build failed: moravor executable not found."
    exit 1
fi
