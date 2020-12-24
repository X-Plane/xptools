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

### macOS

To build on macOS, you’ll need at least macOS 10.11 (El Capitan) and Xcode 8.3 or higher ([free in the Mac App Store](https://apps.apple.com/us/app/xcode/id497799835?mt=12)).

You also need a command-line version of [CMake](http://www.cmake.org/) installed. Beside downloading a binary from the cmake website, it can also be installed via [Homebrew](https://brew.sh): `$ brew install cmake`

### Windows

Building on Windows requires [Visual Studio](https://visualstudio.microsoft.com/vs/features/cplusplus/) 2017 or later (the free Community edition is fine).

In addition to the standard installation of Microsoft Visual Studio Community, you’ll also need some kind of Git client; [Git GUI](http://msysgit.github.io/) is a simple choice, and the command-line syntax listed here will work in the “GIT Bash” shell that comes with it.

Very old (WED 1.3 and earlier) versions were built using MingW - but this toolchain isn't maintained since.

### Linux

You will need the gcc compiler, version 5.4 or newer, which should be installed by default on pretty much any system. In addition you will need cmake version 3.0+ and developer files for a few libraries installed:

* libc and make tools, package gcc-?-dev (the ? denotes the gcc version you want to use)

* X11 and openGL. When the binary AMD or Nvida video drivers are installed - these all come with a full set of developer bindings. When using MESA drivers, package libglu-mesa and its dependencies will provide all these.

* FTTK toolkit version 1.3, package libfltk1.3-dev
* cURL, package libcurl4-openssl-dev

When compiling WED 2.2 and earlier or XPTools version 15-3 and earlier - the Qt4 toolkit, package Qt4-dev is required instead of the FLTK toolkit.

It is also highly recommended to install the Code::Blocks IDE, version 13 or higher, for which solution files are available for most xptools starting with WED 1.7. But pure command line builds for all tools are fully supported as well.

## Getting the Source Code

The source code now lives on [GitHub](https://github.com/X-Plane/xptools)! You can browse the code online, download it, or clone it using all of the standard GitHub techniques. Clone the complete repo like this:

    git clone https://github.com/X-Plane/xptools.git

If you don’t want a complete clone of the code, you can of course use GitHub to just download a ZIP of the most recent code, or download any major release; binary tools releases have matching tags in the repo.

## Compiling the Program

The scenery tools source code depends on a large number of third party libraries; to make cross-platform development easier, they live in a Git sub-module (`libs` for Mac, Linux and MinGW, `msvc_libs` for Visual Studio on Windows).

### Building Libraries (Mac, Linux and MinGW only)

(This step is not necessary on Windows using MSVC)

The first time you want to compile, you need to first download and compile your libraries. These libraries are updated infrequently. From your repository you can do this:

    git submodule init
    git submodule update libs
    cd libs
    make -j

The libraries can take 5-10 minutes to compile!

### Getting the Libraries (Windows only)

(This step is not necessary on macOS or Linux)

Compiling the required libraries requires a number of manual steps - so a precompiled set of libraries along with the patched source code is provided in the msvc_libs subdirectory. To get this from the repository do this:

    git submodule init
    git submodule update msvc_libs

Note that WED versions 1.X and xptools before version 19-4 are using 32bit tools and MSVC 2010, while WED 2.x and xptools 19-4 and later are 64bit binaries and all libraries are created for Win10 / MSVC 2017 toolchains, only. So the `submodule update` step needs to be repeated anytime a different branch with changes to the submodule pointer is checked out.

### Building the Applications from the command line on Linux or macOS

Go to the Scenery Tools root directory (same dir as where these instructions can be found) and just do a 

    make -j

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

* `ac3d`
* `DDSTool`
* `DSFTool`
* `MeshTool`
* `ObjView`
* `RenderFarm`
* `RenderFarmUI`
* `WED`
* `XGrinder`

### Building on Windows Using Visual Studio

The MSVC solution file (`.sln`) can be found in `msvc/XPTools.sln`, and it contains projects that build WorldEditor and the reset of the tools.

### Building on macOS Using XCode

The XCode project is in the root of the repo, `SceneryTools.xcodeproj`. There is one target for each of the scenery tools—simply pick a configuration, target, and build.

### Building on Linux Using Code::Blocks

The project files (`.cbp`) for most xptools can be found in the `codeblocks` directory. The IDE is set up to build using the regular command line makefiles and not its internal build tools - so the results are guaranteed identical to command line builds.

