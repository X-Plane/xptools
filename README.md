X-Plane Scenery Tools README
====================================================================

The X-Plane Scenery Tools (XPTools) code base is the source code tree for all 
of the Laminar Research scenery creation/editing tools.  This code does not 
include X-Plane and the "X-Apps" (PlaneMaker, WorldMaker, AirfoilMaker, and 
Briefer).  It does include source to the ac3d x-plane plugin, WED, the various
tools and our global scenery generator RenderFarm.

The latest info on basic usage of this repository can be found [on the X-Plane
Developer site](https://developer.x-plane.com/code/).


Licensing and Copyright
-------------------------------------------------------------------------------

The code original to Laminar Research lives in the sub-directory "src" and is licensed
under the MIT/X11 license.  If you find a source file with no copyright, or double/
conflicting copyright, please report this (see contact info below)---this is 
probably a clerical error.

The directory "libs" contains tarballs of a number of publicly available open
source libraries---they are included for convenience in building.  I believe
that all of the libsrc libraries are under either an MIT/X11-type or GPL-type
license---if you find a library that is incompatible with WED's licensing,
pleaes report this.

Build materials are in the root directory.

Building
-------------------------------------------------------------------------------

[See the Developer site](https://developer.x-plane.com/code/) for setup,
build instructions, and dev environment setup.

If you have the "latest" version of the code, some projects may not build,
or may not build in release mode.  To get a stable release, use a tag 
associated with some kind of beta or release milestone.

Top Level File Structure
-------------------------------------------------------------------------------

- SceneryTools.xcodeproj
    - Xcode project files for all tools on macOS.
- msvc
    - Microsoft Visual Studio 2017 project files for Windows
- codeblocks
    - code::blocks project files for Linux

- msvc_libs
    - subrepo with pre-compiled 3rd party libraries, only used under Window. 
       The file "howto_build.txt" documents in this directory documents how the
       libraries were originally compiled from the included source.
- libs
    - subrepo with source code for 3rd part libraries, only used under OSX and Linux. 
      The makefile at the toplevel will unpack the tarball, apply patches; when done
      the libs directory contains the static archives we use and headers. The
      XPTools code does not require you to install the libraries globally on your system.
- makerules
    - Master makefile used under Linux, also works to build basic .app bundles under OSX.
- src
    - The main source tree for the various tools and exewcutable of XPtools.

- test
    - Collection of files for regression testing of WED
- scripts
    - A collection of scripts we use to package distros, and other things.
- SDK
    - Third party code required to build the tools that does not come in
      a standard library format.


Documentation
-------------------------------------------------------------------------------

Documentation about sub-modules of the code are typically in a file called
README in the directory being documented.  For example, 
see src/XESCore/README.txt for notes on the XESCore package.

Contact
-------------------------------------------------------------------------------

ben at x-plane.com
