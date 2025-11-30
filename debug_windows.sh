#!/usr/bin/bash

# Get project directory and name dynamically
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="$(basename "$PROJECT_DIR")"

# Convert Unix-style paths to Windows-style paths
C_COMPILER=$(cygpath -w /ucrt64/bin/clang)
CXX_COMPILER=$(cygpath -w /ucrt64/bin/clang++)
INCLUDE_PATH=$(cygpath -w /ucrt64/include)
LIB_PATH=$(cygpath -w /ucrt64/lib)

# Use MSYS2 UCRT64 LLVM with specific include and lib paths
cmake \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_MAKE_PROGRAM=ninja \
-DCMAKE_C_COMPILER="$C_COMPILER" \
-DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DCMAKE_CXX_FLAGS="-Wall -Wextra -Wshadow -Wpedantic -Wno-variadic-macros -I$INCLUDE_PATH -std=c++23 -v" \
-DCMAKE_EXE_LINKER_FLAGS="-L$LIB_PATH -v" \
-G Ninja \
-S . \
-B ./cmake-build-debug \
&& \
ninja -C ./cmake-build-debug \
&& \
ln -sf ./cmake-build-debug/compile_commands.json . \
&& \
./cmake-build-debug/"$PROJECT_NAME"_example
