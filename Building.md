The X-Plane Scenery Tools are available as source code, as well as binaries. This article describes how to get, compile, and modify the scenery tools code. See also the [Scenery Tools Bug Database](http://developer.x-plane.com/scenery-tools-bug-database/ "Scenery Tools Bug Database").

## Contents

- [Setting Up Your Build Environment](#setting-up-your-build-environment)
    - [macOS](#macos)
    - [Windows](#windows)
    - [Linux](#linux)
- [Getting the Source Code](#getting-the-source-code)
- [Compiling the Program](#compiling-the-program)
    - [Building Libraries (Mac, Linux, and MinGW only)](#building-libraries-mac-linux-and-mingw-only)
    - [Getting the libraries (windows-only)](#getting-the-libraries-windows-only)
    - [Building the Applications from the command line on Linux or macOS](#building-the-applications-from-the-command-line-on-linux-or-macos)
    - [Building on Windows Using Visual Studio](#building-on-windows-using-visual-studio)
    - [Building on macOS Using XCode](#building-on-macos-using-xcode)
    - [Building on Linux Using Code::Blocks](#building-on-linux-using-codeblocks)


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

### Building the Applications from the command line on Linux or macOS

Go to the Scenery Tools root directory (same dir as where these instructions can be found) and run 

Windows (Powershell):
    `cmake.ps1`

Linux / macOS:
    `cmake.sh`

This will build the tool using default options for debugging. After awhile, the output can be found under

    [xptools dir]/build/[platform]/[configuration]

The platform is determined automatically (when building on Linux it is Linux of course). The configuration defaults to `debug_opt`. You can specify the configuration when building the tools this way:

    make conf=[configuration]

where `[configuration]` can be one of the following:

* `release`
* `release_opt`
* `debug`
* `debug_opt`

The `release` configuration is built with maximum optimizations `-Ofast -flto`, `debug` with no optimization at all '-O0' and when no configuration is specified, optimizations suitable for most debugging tasks (platform dependent) are used.

The `release` configuration are built with `-DDEV=0` set, while `debug` and default variants have `-DDEV=1`.

To clean the tree you can do:

* `make clean`, this just deletes the `build` directory
* `make distclean`, this deletes the `build` directory and the built 3rd-party libraries located in `libs/local`

You can also build a single tool or a set of tools like this:

    conf=release_opt make [tool_1] [tool_2] [...tool_n]

Available tools are:

* `DDSTool`
* `DSFTool`
* `ObjView`
* `OneOffs`
* `WED`
* `XGrinder`