#!/bin/bash
set -e

echo "--- Building project and tests... ---"
cmake --build build

echo ""
echo "--- Running all tests via CTest... ---"
cd build
ctest --output-on-failure
cd ..

echo ""
echo "✅ All tests passed successfully!"