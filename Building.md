The X-Plane Scenery Tools are available as source code, as well as binaries. This article describes how to get, compile, and modify the scenery tools code. See also the [Scenery Tools Bug Database](http://developer.x-plane.com/scenery-tools-bug-database/ "Scenery Tools Bug Database").

## Contents

- [Setting Up Your Build Environment](#setting-up-your-build-environment)
    - [macOS](#macos)
    - [Windows](#windows)
        - [Using MinGW on Windows](#using-mingw-on-windows)
    - [Linux](#linux)
- [Getting the Source Code](#getting-the-source-code)
- [Compiling the Program](#compiling-the-program)
    - [Building Libraries (Mac, Linux, and MinGW only)](#building-libraries-mac-linux-and-mingw-only)
    - [Building the Applications on Linux or MinGW](#building-the-applications-on-linux-or-mingw)
    - [Building on Windows Using Visual Studio](#building-on-windows-using-visual-studio)
    - [Building on macOS Using XCode](#building-on-macos-using-xcode)


## Setting Up Your Build Environment

The X-Plane scenery tools code (XPTools) can be compiled for Mac, Windows, or Linux. Before you can work on the tools, you may need to get/update your development environment.

### macOS

To build on macOS, you’ll need at least macOS 10.12 (Sierra) and the latest version of Xcode ([free in the Mac App Store](https://apps.apple.com/us/app/xcode/id497799835?mt=12)) your OS supports.

You also need a command-line version of [CMake](http://www.cmake.org/) installed. The easiest way to get CMake is via [Homebrew](https://brew.sh): `$ brew install cmake`

### Windows

There are two choices for toolchains on Windows.

1. If you want to work on WED 1.3 or later (i.e., any version we’ve shipped for years now) you can use [Visual Studio](https://visualstudio.microsoft.com/vs/features/cplusplus/) 2017 or later (the free Community edition is fine). This is the easier, and recommended option.
2. If you want to work on older versions of WED (pre-1.3) you will need to use MinGW.

In addition to the standard installation of Microsoft Visual Studio Community, you’ll also need some kind of Git client; [Git GUI](http://msysgit.github.io/) is a simple choice, and the command-line syntax listed here will work in the “GIT Bash” shell that comes with it.

The rest of this section covers using MinGW—if you’re using Visual Studio (recommended), jump ahead to [Getting the source code](#getting-the-source-code).

#### Using MinGW on Windows

**WARNING:** Make sure there are no spaces in the paths up to the code on Windows! The Makefile does not handle this case yet. You may want to consider this when installing the tools.

MinGW is the supported compiler—you will need to set up a full mingw environment with GCC 4.2.x or newer. The simplest way to do that is with the XPTools Environment Installer, which can be downloaded [here](http://dev.x-plane.com/download/tools/xptools-env-installer-1.2.0.exe).

The installer is a net-installer—that is, it will download the latest components to the build environment. The build environment is entirely self-contained – simply delete it when done – it does not install any registry keys, COM components, or other global “stuff”.

To start the environment, run `startenv.bat` from the root of the installation directory. It will put you automatically in the Scenery Tools Source directory if you choosed to download the sources during the installation, or into your home directory if you skipped that step.

To update the build environment itself (i.e. when we added a new gcc release for example) just start the environment and execute following commands:

    cd /
    git pull origin master:master

This however might fail when we update bash or git in the environment because Windows locks files which are currently in use. In this case just re-run the Buildenvironment Installer over an existing installation to update the environment (you will need an installer version >= 1.1.1 for this to work).

Similary you can update the Scenery Tools source tree with:

    cd /xptools
    git pull origin master:master

Note that you aren’t on the master branch after a fresh installation, but a specific commit because the Scenery Tools tree is registered as a git submodule in the build environment. Therefore you need to make sure that you switch to the master branch before building the tools after updating the tree (you need to do this only once after a fresh installation):

    cd /xptools
    git checkout master

### Linux

You will need GCC 7, along with a few other packages:

* gnu make
* gnu coreutils
* binutils, binutils-devel if applicable for libbfd.a
* libmpfr-devel
* qt4-devel
* mesa-libGL-devel, mesa-libGLU-devel or the proprietary variants from ATI or NVidia
* libcurl4-openssl-dev

* * *

## Getting the Source Code

The source code now lives on [GitHub](https://github.com/X-Plane/xptools)! You can browse the code online, download it, or clone it using all of the standard GitHub techniques. Clone the complete repo like this:

    git clone --recurse-submodules https://github.com/X-Plane/xptools.git

(Note that `--recurse-submodules` is necessary to also get the libraries to build from source.)

If you don’t want a complete clone of the code, you can of course use GitHub to just download a ZIP of the most recent code, or download any major release; binary tools releases have matching tags in the repo.

* * *

## Compiling the Program

The scenery tools source code depends on a large number of third party libraries; to make cross-platform development easier, they live in a Git sub-module (`libs` for Mac, Linux, and MinGW, `msvc_libs` for Visual Studio on Windows).

### Building Libraries (Mac, Linux, and MinGW only)

(This step is not necessary when using MSVC to build WorldEditor 1.3 or newer on Windows.)

The first time you want to compile, you need to first download and compile your libraries. From your repository you can do this:

    git submodule update --init
    cd libs
    make -j

The libraries can take 20-30 minutes to compile!

### Building the Applications on Linux or MinGW

I tried to make this process as simple as possible. First of all make sure that you have following prerequisites installed:

* development packages of libbfd (mostly included in binutils), libmpfr, libz and boost
* gcc 7
* gnu make

then just do a

    make -j

in the Scenery Tools root directory. After awhile, the output can be found under

    [xptools dir]/build/[platform]/[configuration]

The platform is determined automatically (when building on Linux it is Linux of course). The configuration defaults to `debug_opt`. You can specify the configuration when building the tools this way:

    make conf=[configuration]

where `[configuration]` can be one of the following:

* `release`
* `release_opt`
* `debug`
* `debug_opt`

The `_opt` variants are built with `-O2` set, the others with `-O0`. `release` variants are built with `-DDEV=0` set, while `debug` variants have `-DDEV=1`.

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
* `ObjConverter`
* `ObjView`
* `RenderFarm`
* `RenderFarmUI`
* `WED`
* `XGrinder`

### Building on Windows Using Visual Studio

The MSVC solution file (`.sln`) can be found in `msvc/XPTools.sln`, and it contains projects that build WorldEditor and the reset of the tools. The MSVC project uses the standard debug and release targets.

### Building on macOS Using XCode

The XCode project is in the root of the repo, `SceneryTools.xcodeproj`. There is one target for each of the scenery tools—simply pick a configuration, target, and build.

**Important**: Before building on the Mac for the first time, you must build the libraries as described above.

