#!/bin/bash
set -e

if [ -z "$VCPKG_ROOT" ]; then
    echo "Error: VCPKG_ROOT is not set. Please set it to your vcpkg installation directory."
    exit 1
fi

echo "Using VCPKG toolchain file: $VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

export PATH="/usr/lib/ccache:$PATH"

if command -v mold >/dev/null 2>&1; then
    export CMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold"
    echo "Using mold linker to speed up linking"
else
    export CMAKE_EXE_LINKER_FLAGS=""
    echo "mold linker not found, using default linker"
fi

cmake -S . -B build \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXE_LINKER_FLAGS="$CMAKE_EXE_LINKER_FLAGS"

cmake --build build --parallel

echo "Build successful! Executable is in build/my_webserver"
