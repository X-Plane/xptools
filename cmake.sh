#!/usr/bin/env bash
set -euo pipefail

# --- Configuration ---
PROFILE="default"   # Change to your Conan profile name if needed

# --- Determine default generator ---
OS_NAME="$(uname -s)"
if [[ "$OS_NAME" == "Darwin" ]]; then
    DEFAULT_GENERATOR="Xcode"
else [[ "$OS_NAME" == "Linux" ]];
    DEFAULT_GENERATOR="Unix Makefiles"
fi

# Allow override via env var or first CLI argument
GENERATOR="${GENERATOR:-${1:-$DEFAULT_GENERATOR}}"

echo "Using CMake generator: $GENERATOR"

mkdir -p "build_Debug"
mkdir -p "build_RelWithDebInfo"
mkdir -p "build_Release"

# --- Step 1: Install Conan dependencies ---
echo "Installing Conan Debug build type dependencies..."

conan install . \
    --profile "${PROFILE}" \
    --build=missing \
    --output-folder="build_Debug" \
    --settings build_type="Debug"

echo "Installing Conan RelWithDebInfo build type dependencies..."
conan install . \
    --profile "${PROFILE}" \
    --build=missing \
    --output-folder="build_RelWithDebInfo" \
    --settings build_type="RelWithDebInfo"

echo "Installing Conan Release build type dependencies..."
conan install . \
    --profile "${PROFILE}" \
    --build=missing \
    --output-folder="build_Release" \
    --settings build_type="Release"

# --- Step 3: Run CMake ---
echo "Generating project with ${GENERATOR}..."
cmake -S . -B "build_Debug" \
    -G "${GENERATOR}" \
    -DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_CONFIGURATION_TYPES=Debug \
    -DCMAKE_TOOLCHAIN_FILE=build_Debug/build/Debug/generators/conan_toolchain.cmake

cmake -S . -B "build_RelWithDebInfo" \
    -G "${GENERATOR}" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_CONFIGURATION_TYPES=RelWithDebInfo \
    -DCMAKE_TOOLCHAIN_FILE=build_RelWithDebInfo/build/RelWithDebInfo/generators/conan_toolchain.cmake

cmake -S . -B "build_Release" \
    -G "${GENERATOR}" \
    -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_CONFIGURATION_TYPES=Release \
    -DCMAKE_TOOLCHAIN_FILE=build_Release/build/Release/generators/conan_toolchain.cmake

echo "Project generated using ${GENERATOR}"
