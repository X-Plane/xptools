#!/usr/bin/env bash
set -euo pipefail

# --- Configuration ---
BUILD_DIR="build"
PROFILE="default"   # Change to your Conan profile name if needed
BUILD_TYPE="Release" # Or "Debug"

# --- Determine default generator ---
OS_NAME="$(uname -s)"
if [[ "$OS_NAME" == "Darwin" ]]; then
    DEFAULT_GENERATOR="Xcode"
else [[ "$OS_NAME" == "Linux" ]];
    DEFAULT_GENERATOR="Ninja"
fi

# Allow override via env var or first CLI argument
GENERATOR="${GENERATOR:-${1:-$DEFAULT_GENERATOR}}"

echo "Using CMake generator: $GENERATOR"

# --- Step 1: Install Conan dependencies ---
echo "Installing Conan Debug build type dependencies..."
conan install . \
    --profile "${PROFILE}" \
    --build=missing \
    --output-folder="${BUILD_DIR}" \
    --settings build_type="Debug"

echo "Installing Conan RelWithDebInfo build type dependencies..."
conan install . \
    --profile "${PROFILE}" \
    --build=missing \
    --output-folder="${BUILD_DIR}" \
    --settings build_type="RelWithDebInfo"

echo "Installing Conan Release build type dependencies..."
conan install . \
    --profile "${PROFILE}" \
    --build=missing \
    --output-folder="${BUILD_DIR}" \
    --settings build_type="Release"

# --- Step 2: Create build directory ---
mkdir -p "${BUILD_DIR}"

# --- Step 3: Run CMake ---
echo "Generating project with ${GENERATOR}..."
cmake -S . -B "${BUILD_DIR}" \
    -G "${GENERATOR}" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

echo "✅ Project generated in ${BUILD_DIR} using ${GENERATOR}"
