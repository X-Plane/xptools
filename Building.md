The X-Plane Scenery Tools are available as source code, as well as binaries. This article describes how to get, compile, and modify the scenery tools code. See also the [Scenery Tools Bug Database](http://developer.x-plane.com/scenery-tools-bug-database/ "Scenery Tools Bug Database").

## Contents

- [Setting Up Your Build Environment](#setting-up-your-build-environment)
    - [macOS](#macos)
    - [Windows](#windows)
    - [Linux](#linux)
- [Getting the Source Code](#getting-the-source-code)
- [Compiling the Program](#compiling-the-program)
    - [Generating projects](#generating-projects)
    - [Building from the command line](#building-from-the-command-line)
    - [Build configurations and DEV](#build-configurations-and-dev)
    - [Available tools](#available-tools)


## Setting Up Your Build Environment

The X-Plane scenery tools code (XPTools) can be compiled for Mac, Windows, or Linux. Before you can work on the tools, you may need to get/update your development environment.

You will need a command-line version of [CMake](http://www.cmake.org/) installed. Beside downloading a binary from the cmake website, it can also be installed via [Homebrew](https://brew.sh): `$ brew install cmake` on macOS.

*Note: CMake 4.0 removed backward compatibility with version older than 3.5. This causes issues with some third-party libraries we are using. Please install a CMake version prior 4.x*

### macOS

To build on macOS, you’ll need at least macOS 10.11 (El Capitan) and Xcode 8.3 or higher ([free in the Mac App Store](https://apps.apple.com/us/app/xcode/id497799835?mt=12)).

### Windows

Building on Windows requires [Visual Studio](https://visualstudio.microsoft.com/vs/features/cplusplus/) 2017 or later (the free Community edition is fine).

In addition to the standard installation of Microsoft Visual Studio Community, you’ll also need some kind of Git client; [Git GUI](http://msysgit.github.io/) is a simple choice, and the command-line syntax listed here will work in the “GIT Bash” shell that comes with it.

### Linux

You will need the gcc compiler, version 5.4 or newer, which should be installed by default on pretty much any system. In addition you will need developer files for a few libraries installed:

* libc and make tools, package gcc-?-dev (the ? denotes the gcc version you want to use)

* X11 and openGL. When the binary AMD or Nvida video drivers are installed - these all come with a full set of developer bindings. When using MESA drivers, package libglu-mesa and its dependencies will provide all these.

* FTTK toolkit version 1.3, package libfltk1.3-dev
* cURL, package libcurl4-openssl-dev

## Getting the Source Code

The source code now lives on [GitHub](https://github.com/X-Plane/xptools)! You can browse the code online, download it, or clone it using all of the standard GitHub techniques. Clone the complete repo like this:

    git clone https://github.com/X-Plane/xptools.git

If you don’t want a complete clone of the code, you can of course use GitHub to just download a ZIP of the most recent code, or download any major release; binary tools releases have matching tags in the repo.

## Compiling the Program

The scenery tools source code depends on a large number of third party libraries; to make cross-platform development easier, we are using Conan to install those for you.

### Generating projects

Go to the Scenery Tools root directory (same dir as these instructions) and run:

Linux / macOS:

    ./cmake.sh

Windows (PowerShell):

    ./cmake.ps1

`cmake.sh` runs `conan install` and CMake for three configurations, each in its own directory:
`build_Debug`, `build_RelWithDebInfo` and `build_Release`. The generator defaults to Xcode on
macOS and Ninja on Linux; override it with `GENERATOR=Ninja ./cmake.sh` (or pass it as the first
argument).

`cmake.ps1` installs Conan dependencies for all three configurations and generates a single
Visual Studio 2022 solution in `vs_build`. Pass `-BuildType Debug` to configure a debug build
(the default is `Release`), `-Clean` to start over, and `-ConanProfile <name>` to use a non-default
Conan profile.

### Building from the command line

    cmake --build build_Release --config Release --target WED       # macOS / Linux
    cmake --build vs_build --config Release --target WED            # Windows

Omit `--target` to build every tool. On macOS you can instead open the generated Xcode project in
`build_<Config>`; on Windows, open the solution in `vs_build`.

### Build configurations and DEV

`DEV=1` (debug checks and asserts) is set when the *configured* `CMAKE_BUILD_TYPE` is `Debug`;
every other configuration gets `DEV=0`. On macOS and Linux that means `build_Debug` is the DEV
build. On Windows `vs_build` is configured once with `-BuildType`, so choosing "Debug" inside
Visual Studio does not turn on `DEV` — re-run `./cmake.ps1 -BuildType Debug` for that.

### Available tools

* `WED`
* `DSFTool`
* `DDSTool`
* `ObjView`
* `XGrinder`
* the `OneOffs` utilities (see `cmake/OneOffs.cmake`)
