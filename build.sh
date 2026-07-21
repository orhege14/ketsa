#!/bin/bash
# =========================================================
# Ketsa Build System - Unix (Linux/macOS)
# =========================================================
# Usage: ./build.sh [--release|--debug|--clean|--test]

set -e

BUILD_DIR="build"
CONFIG="Debug"
GENERATOR="Ninja"

# Parse arguments
for arg in "$@"; do
    case "$arg" in
        --release) CONFIG="Release" ;;
        --debug) CONFIG="Debug" ;;
        --clean) rm -rf "$BUILD_DIR" ketsa ketsa.exe 2>/dev/null
                 echo "[KETSA] Clean complete."
                 exit 0 ;;
        --test) CONFIG="Debug"; RUN_TESTS=1 ;;
    esac
done

# Check prerequisites
command -v cmake >/dev/null 2>&1 || { echo "[KETSA] ERROR: CMake not found. Install CMake 3.20+"; exit 1; }
command -v ninja >/dev/null 2>&1 || { echo "[KETSA] WARNING: Ninja not found, using Make"; GENERATOR="Unix Makefiles"; }

# Detect platform
OS="$(uname -s)"
ARCH="$(uname -m)"
echo "[KETSA] Platform: $OS ($ARCH)"
echo "[KETSA] Building Ketsa ($CONFIG)..."

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$CONFIG"
cmake --build . --config "$CONFIG" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
cd ..

# Copy binary
if [ -f "$BUILD_DIR/ketsa" ]; then
    cp "$BUILD_DIR/ketsa" .
elif [ -f "$BUILD_DIR/ketsa.exe" ]; then
    cp "$BUILD_DIR/ketsa.exe" .
fi

echo "[KETSA] Build successful."
echo "[KETSA] Run: ./ketsa --version"

# Run tests if requested
if [ -n "$RUN_TESTS" ]; then
    echo "[KETSA] Running tests..."
    ./ketsa --check examples/comprehensive.ketsa
    echo "[KETSA] All tests passed!"
fi
