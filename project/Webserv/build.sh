#!/bin/bash
set -e 

if [ -z "$VCPKG_ROOT" ]; then
    echo "Error: VCPKG_ROOT is not set. Please set it to your vcpkg installation directory."
    exit 1
fi

echo "Using VCPKG toolchain file: $VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

cmake -S . -B build \
    -G "Ninja" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Debug

cmake --build build

echo "Build successful! Executable is in build/my_webserver"