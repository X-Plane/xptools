#!/usr/bin/env bash
#set -euo pipefail

# --- Configuration ---
BUILD_DIR="build"
PROFILE="default"   # Change to your Conan profile name if needed
BUILD_TYPE="Release" # Or "Debug"

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

# --- Step 3: Run CMake to generate Xcode project ---
echo "Generating Xcode project..."
cmake -S . -B "${BUILD_DIR}" \
    -G Xcode \
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

echo "✅ Xcode project generated in ${BUILD_DIR}"

