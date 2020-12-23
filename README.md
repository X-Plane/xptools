X-Plane Scenery Tools README
====================================================================

The X-Plane Scenery Tools (XPTools) code base is the source code tree for all 
of the Laminar Research scenery creation/editing tools. This does not 
include X-Plane, Plane Maker, or Airfoil Maker.  It does include source to WorldEditor (WED),
and our global scenery generator RenderFarm, and other tools.

Contents
-------------------------------------------------------------------------------

- [Licensing and Copyright](#licensing-and-copyright)
- [Building the Applications](#building-the-applications)
- [Contributing Using Git & GitHub](#contributing-using-git--github)
- [Top Level File Structure](#top-level-file-structure)
- [Documentation](#documentation)
- [Mailing List/Contact](#mailing-listcontact)

Licensing and Copyright
-------------------------------------------------------------------------------

The code original to Laminar Research lives in the sub-directory "src" and is licensed
under the MIT/X11 license.  If you find a source file with no copyright, or double/conflicting
copyright, please report this (see contact info below)—this is probably a clerical error.

[The `libs` submodule](https://github.com/X-Plane/xptools_libs) contains tarballs of a number
of publicly available open-source libraries—they are included for convenience in building.
To the best of our knowledge, all libraries are under either an MIT/X11-type or GPL-type
license—if you find a library that is incompatible with WED's licensing,
please report this.

[Building the Applications](Building.md)
-------------------------------------------------------------------------------

See [Building.md](Building.md) for setup, build instructions, and dev environment setup.

We do our best to keep `master` building all projects and in general be release-ready,
but to get a stable release, use a tag associated with some kind of beta or release milestone.


Contributing Using Git & GitHub
-------------------------------------------------------------------------------

If you’d like to contribute to the project, you can do so by forking the repo on GitHub and making a pull request. (If you’re new to Git or GitHub, have a look at [the GitHub guides](https://guides.github.com), especially “Hello World” and the Git Handbook.)

In general, the repo’s `master` branch reflects the current state of development, while release branches are used for staging and patching binary releases (so, for instance, `wed_230_release` is the release branch for WED version 2.3). There are also corresponding tags for public releases (e.g., `wed_231r1`).

Starting a new development branch based on the tip of `master` is probably a good idea to avoid merge conflicts. I encourage the use of `git rebase` after pulling new changes and updating the master branch to have your local development branch up-to-date, unless you have people pulling from your repository. In that case, merging `master` back to your development branch is a better choice because rebasing causes the creation of new commits with new SHA1 checksums which might distort the workflow of users pulling from your repository.

Once you’ve finished your work and you think it’s time to submit your changes, you can use the GitHub UI to submit a pull request.

Top Level File Structure
-------------------------------------------------------------------------------

- SceneryTools.xcodeproj
    - Xcode project files for all tools on macOS.
- msvc
    - Microsoft Visual Studio 2017 project files for Windows
- codeblocks
    - code::blocks project file to build WED for Linux
- msvc_libs
    - subrepo with pre-compiled 3rd party libraries, only used on Windows when compiling with Visual Studio. 
       The file "howto_build.txt" documents in this directory documents how the
       libraries were originally compiled from the included source.
- libs
    - subrepo with source code for 3rd part libraries, only used under OSX and Linux. 
      The makefile at the toplevel will unpack the tarball, apply patches; when done
      the libs directory contains the static archives we use and headers. The
      XPTools code does not require you to install the libraries globally on your system.
- makerules
    - Master makefile to build all tools for Linux, can also build all tools under OSX.
- src
    - The main source tree for the various tools and executable of XPtools.
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

Mailing List/Contact
-------------------------------------------------------------------------------

We have a private Slack, as well as a private “Google Groups” email list to communicate about development of WED and other tools. To get involved, shoot Tyler an email at his first name at X-Plane.com.