#!/bin/bash

# Get project directory and name dynamically
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="$(basename "$PROJECT_DIR")"

# Use Homebrew LLVM with specific include and lib paths
cmake \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_MAKE_PROGRAM=ninja \
-DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang \
-DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DCMAKE_CXX_FLAGS="-Wall -Wextra -Wshadow -Wpedantic -Wno-variadic-macros -I/opt/homebrew/opt/llvm/include" \
-DCMAKE_EXE_LINKER_FLAGS="-L/opt/homebrew/opt/llvm/lib/c++" \
-DCMAKE_OSX_SYSROOT=$(xcrun --show-sdk-path) \
-G Ninja \
-S . \
-B ./cmake-build-debug \
&& \
ninja -C ./cmake-build-debug \
&& \
ln -sf ./cmake-build-debug/compile_commands.json . \
&& \
./cmake-build-debug/"$PROJECT_NAME"_example
