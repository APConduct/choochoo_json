#!/bin/bash

# Get project directory and name dynamically
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="$(basename "$PROJECT_DIR")"

# Use relative paths and smart detection
cmake \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_MAKE_PROGRAM=ninja \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-G Ninja \
-S . \
-B ./cmake-build-debug \
&& \
ninja -C ./cmake-build-debug \
&& \
./cmake-build-debug/"$PROJECT_NAME"
