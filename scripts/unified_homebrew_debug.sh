#!/bin/bash

# Unified debug script for Homebrew LLVM
# This template works for C++ projects using Homebrew's Clang and libc++
# Copy this to your project root and customize as needed

# Get project directory and name dynamically
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="$(basename "$PROJECT_DIR")"

# Configuration - Homebrew LLVM paths
HOMEBREW_LLVM_ROOT="/opt/homebrew/opt/llvm"
HOMEBREW_LLVM_INCLUDE="$HOMEBREW_LLVM_ROOT/include"
HOMEBREW_LLVM_LIBCXX="$HOMEBREW_LLVM_ROOT/lib/c++"

# Verify Homebrew LLVM is installed
if [ ! -d "$HOMEBREW_LLVM_ROOT" ]; then
    echo "Error: Homebrew LLVM not found at $HOMEBREW_LLVM_ROOT"
    echo "Please install with: brew install llvm"
    exit 1
fi

if [ ! -d "$HOMEBREW_LLVM_LIBCXX" ]; then
    echo "Error: Homebrew libc++ not found at $HOMEBREW_LLVM_LIBCXX"
    echo "Your LLVM installation may be incomplete"
    exit 1
fi

echo "Using Homebrew LLVM toolchain:"
echo "  Compiler: $HOMEBREW_LLVM_ROOT/bin/clang++"
echo "  Headers: $HOMEBREW_LLVM_INCLUDE"
echo "  libc++: $HOMEBREW_LLVM_LIBCXX"
echo "  Project: $PROJECT_NAME"
echo ""

# Build configuration
cmake \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_MAKE_PROGRAM=ninja \
-DCMAKE_C_COMPILER="$HOMEBREW_LLVM_ROOT/bin/clang" \
-DCMAKE_CXX_COMPILER="$HOMEBREW_LLVM_ROOT/bin/clang++" \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DCMAKE_CXX_FLAGS="-Wall -Wextra -Wshadow -Wpedantic -Wno-variadic-macros -I$HOMEBREW_LLVM_INCLUDE" \
-DCMAKE_EXE_LINKER_FLAGS="-L$HOMEBREW_LLVM_LIBCXX -Wl,-rpath,$HOMEBREW_LLVM_LIBCXX" \
-DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)" \
-G Ninja \
-S . \
-B ./cmake-build-debug \
&& \
ninja -C ./cmake-build-debug \
&& \
ln -sf ./cmake-build-debug/compile_commands.json . \
&& \
echo "" \
&& \
echo "Build successful! Running $PROJECT_NAME example..." \
&& \
echo "" \
&& \
./cmake-build-debug/"$PROJECT_NAME"_example

# Optional: Uncomment to show which libc++ is being used
# echo ""
# echo "Verifying libc++ library usage:"
# otool -L ./cmake-build-debug/"$PROJECT_NAME"_example | grep libc++
