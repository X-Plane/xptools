#!/usr/bin/env powershell

# C++ Project Build Script with Conan and CMake
# This script sets up a Visual Studio build environment for a C++ project

param(
    [string]$BuildType = "Release",
    [string]$ConanProfile = "default",
    [switch]$Clean = $false,
    [switch]$Verbose = $false
)

# Script configuration
$BuildDir = "vs_build"
$ProjectRoot = Get-Location
$ConanFile = "conanfile.py"
$CMakeLists = "CMakeLists.txt"

# Color output functions
function Write-Info { param($Message) Write-Host "INFO: $Message" -ForegroundColor Green }
function Write-Warning { param($Message) Write-Host "WARNING: $Message" -ForegroundColor Yellow }
function Write-Error { param($Message) Write-Host "ERROR: $Message" -ForegroundColor Red }

# Check prerequisites
function Test-Prerequisites {
    Write-Info "Checking prerequisites..."

    # Check if conan is installed
    try {
        $conanVersion = & conan --version 2>$null
        Write-Info "Found Conan: $conanVersion"
    }
    catch {
        Write-Error "Conan is not installed or not in PATH"
        Write-Host "Please install Conan: pip install conan"
        exit 1
    }

    # Check if cmake is installed
    try {
        $cmakeVersion = & cmake --version 2>$null | Select-Object -First 1
        Write-Info "Found CMake: $cmakeVersion"
    }
    catch {
        Write-Error "CMake is not installed or not in PATH"
        Write-Host "Please install CMake from https://cmake.org/download/"
        exit 1
    }

    # Check if conanfile exists (conanfile.py or conanfile.txt)
    if (-not (Test-Path "conanfile.py") -and -not (Test-Path "conanfile.txt")) {
        Write-Error "conanfile.py or conanfile.txt not found in current directory"
        Write-Host "Please ensure you're in the project root with a Conan recipe file"
        exit 1
    }

    # Check if CMakeLists.txt exists
    if (-not (Test-Path $CMakeLists)) {
        Write-Error "CMakeLists.txt not found in current directory"
        Write-Host "Please ensure you're in the project root with a CMakeLists.txt"
        exit 1
    }

    Write-Info "All prerequisites satisfied"
}

# Clean build directory if requested
function Clear-BuildDirectory {
    if ($Clean -and (Test-Path $BuildDir)) {
        Write-Info "Cleaning build directory: $BuildDir"
        Remove-Item -Recurse -Force $BuildDir
    }
}

# Create build directory
function New-BuildDirectory {
    if (-not (Test-Path $BuildDir)) {
        Write-Info "Creating build directory: $BuildDir"
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    } else {
        Write-Info "Build directory already exists: $BuildDir"
    }
}

# Install dependencies with Conan 2
function Install-ConanDependencies {
    Write-Info "Installing Conan 2 dependencies..."

    Push-Location $BuildDir
    try {
        # Conan 2 install command - this will generate files in the current directory (build dir)
        $conanArgs = @(
            "install",
            "..",
            "--build=missing",
            "--profile:build=$ConanProfile",
            "--profile:host=$ConanProfile",
            "-s", "build_type=$BuildType",
            "--output-folder=."
        )

        if ($Verbose) {
            $conanArgs += "-v"
        }

        Write-Info "Running: conan $($conanArgs -join ' ')"
        & conan @conanArgs

        if ($LASTEXITCODE -ne 0) {
            Write-Error "Conan install failed with exit code: $LASTEXITCODE"
            exit $LASTEXITCODE
        }

        # Verify that the toolchain was generated in the build directory
        if (-not (Test-Path "conan_toolchain.cmake")) {
            Write-Error "conan_toolchain.cmake was not generated in the build directory"
            exit 1
        }

        Write-Info "Conan 2 dependencies installed successfully"
        Write-Info "Generated conan_toolchain.cmake in build directory"
    }
    finally {
        Pop-Location
    }
}

# Generate Visual Studio project files with CMake
function Invoke-CMakeGenerate {
    Write-Info "Generating Visual Studio project files with CMake..."

    Push-Location $BuildDir
    try {
        # CMake arguments for Visual Studio generator with Conan 2 toolchain
        $cmakeArgs = @(
            "..",
            "-G", "Visual Studio 17 2022",
            "-A", "x64",
            "-DCMAKE_BUILD_TYPE=$BuildType",
            "-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake"
        )

        if ($Verbose) {
            $cmakeArgs += "--verbose"
        }

        # Verify Conan toolchain exists
        if (-not (Test-Path "conan_toolchain.cmake")) {
            Write-Error "Conan toolchain file not found. Make sure Conan install completed successfully."
            exit 1
        }

        Write-Info "Using Conan 2 toolchain: conan_toolchain.cmake"
        Write-Info "Running: cmake $($cmakeArgs -join ' ')"
        & cmake @cmakeArgs

        if ($LASTEXITCODE -ne 0) {
            Write-Error "CMake generation failed with exit code: $LASTEXITCODE"
            exit $LASTEXITCODE
        }

        Write-Info "Visual Studio project files generated successfully"
    }
    finally {
        Pop-Location
    }
}

# Display success message with next steps
function Show-CompletionMessage {
    Write-Host "`n" -NoNewline
    Write-Info "Build setup completed successfully!"
    Write-Host "`nNext steps:" -ForegroundColor Cyan
    Write-Host "  1. Open the generated .sln file in Visual Studio:" -ForegroundColor White
    Write-Host "     $BuildDir\*.sln" -ForegroundColor Gray
    Write-Host "  2. Or build from command line:" -ForegroundColor White
    Write-Host "     cmake --build $BuildDir --config $BuildType" -ForegroundColor Gray
    Write-Host "  3. Or use MSBuild directly:" -ForegroundColor White
    Write-Host "     msbuild $BuildDir\*.sln /p:Configuration=$BuildType" -ForegroundColor Gray
}

# Main execution
function Main {
    Write-Info "Starting C++ project build setup..."
    Write-Info "Build Type: $BuildType"
    Write-Info "Conan Profile: $ConanProfile"
    Write-Info "Project Root: $ProjectRoot"

    try {
        Test-Prerequisites
        Clear-BuildDirectory
        New-BuildDirectory
        Install-ConanDependencies
        Invoke-CMakeGenerate
        Show-CompletionMessage
    }
    catch {
        Write-Error "Build setup failed: $_"
        exit 1
    }
}

# Run main function
Main
