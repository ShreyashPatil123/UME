#!/bin/bash
# UME Development Environment Bootstrap (Linux/macOS)
# Run: bash scripts/bootstrap.sh
#
# This script sets up a complete UME development environment from scratch.
# Prerequisites: Git, CMake 3.28+, Clang 17+ or GCC 13+, Python 3.10+

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "========================================"
echo "  UME Development Environment Setup"
echo "========================================"
echo ""

# ── Check Prerequisites ──
echo "[1/6] Checking prerequisites..."

command_exists() { command -v "$1" &>/dev/null; }

if ! command_exists cmake; then
    echo "  ERROR: cmake not found. Install CMake 3.28+ from https://cmake.org/download/"
    exit 1
fi
echo "  cmake: $(cmake --version | head -1)"

if ! command_exists git; then
    echo "  ERROR: git not found."
    exit 1
fi
echo "  git: $(git --version)"

if ! command_exists ninja; then
    echo "  WARNING: ninja not found. Installing..."
    if command_exists apt-get; then
        sudo apt-get install -y ninja-build
    elif command_exists brew; then
        brew install ninja
    else
        echo "  Please install ninja-build manually."
    fi
fi
echo "  ninja: $(ninja --version 2>/dev/null || echo 'not found')"

# Detect compiler
if command_exists clang++-17; then
    CXX=clang++-17
    CC=clang-17
elif command_exists clang++; then
    CXX=clang++
    CC=clang
elif command_exists g++-13; then
    CXX=g++-13
    CC=gcc-13
elif command_exists g++; then
    CXX=g++
    CC=gcc
else
    echo "  ERROR: No supported C++ compiler found. Install Clang 17+ or GCC 13+."
    exit 1
fi
echo "  compiler: $CXX"

if command_exists python3; then
    echo "  python: $(python3 --version)"
else
    echo "  WARNING: python3 not found. Python bindings will not be available."
fi

# ── Install vcpkg ──
echo ""
echo "[2/6] Setting up vcpkg..."

VCPKG_ROOT="$PROJECT_DIR/vcpkg"
if [ ! -d "$VCPKG_ROOT" ]; then
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
    "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
else
    echo "  vcpkg already present at $VCPKG_ROOT"
fi
export VCPKG_ROOT
echo "  VCPKG_ROOT=$VCPKG_ROOT"

# ── Install System Dependencies ──
echo ""
echo "[3/6] Checking system dependencies..."

if command_exists apt-get; then
    # Ubuntu/Debian
    echo "  Installing build dependencies..."
    sudo apt-get update -qq
    sudo apt-get install -y -qq \
        build-essential \
        pkg-config \
        curl \
        zip \
        unzip \
        tar \
        2>/dev/null || true
fi

# ── Configure Build ──
echo ""
echo "[4/6] Configuring CMake build..."

cmake -B "$BUILD_DIR" \
    -G "Ninja" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="$CC" \
    -DCMAKE_CXX_COMPILER="$CXX" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DUME_BUILD_TESTS=ON \
    -DUME_BUILD_BENCHMARKS=OFF \
    -DUME_BUILD_EXAMPLES=OFF \
    -S "$PROJECT_DIR"

echo "  CMake configure: OK"

# ── Build ──
echo ""
echo "[5/6] Building UME..."

cmake --build "$BUILD_DIR" --parallel "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
echo "  Build: OK"

# ── Run Tests ──
echo ""
echo "[6/6] Running tests..."

cd "$BUILD_DIR"
if ctest --output-on-failure --timeout 30; then
    echo "  Tests: ALL PASSED"
else
    echo "  WARNING: Some tests failed."
fi

# ── Done ──
echo ""
echo "========================================"
echo "  UME development environment is ready!"
echo "========================================"
echo ""
echo "  Build directory: $BUILD_DIR"
echo "  To rebuild:      cmake --build $BUILD_DIR"
echo "  To test:         cd $BUILD_DIR && ctest"
echo ""
