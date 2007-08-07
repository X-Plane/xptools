-------------------------------------------------------------------------------
                       X-Plane Scenery Tools README
-------------------------------------------------------------------------------

The X-Plane Scenery Tools (XPTools) code base is the source code tree for all 
of the Laminar Research scenery creation/editing tools.  This code does not 
include X-Plane and the "X-Apps" (PlaneMaker, WorldMaker, AirfoilMaker, and 
Briefer).  It does include source to the ac3d x-plane plugin, ObjView, 
ObjConverter, WED, DSFLib, ObjLib, ENV2CSV, and the global scenery generation 
code.

-------------------------------------------------------------------------------
LICENSING AND COPYRIGHT
-------------------------------------------------------------------------------

The LR original code lives in the sub-directory "src" and is licensed under the
MIT/X11 license.  If you find a source file with no copyright, or double/
conflicting copyright, please report this (see contact info below) - this is 
probably a clerical error.

The directory "libsrc" contains copies of a number of publicly available open
source libraries - they are included for convenience in building.  I believe
that all of the libsrc libraries are under either an MIT/X11-type or GPL-type
license - if you find a library that is incompatible with WED's licensing,
pleaes report this.

Build materials are usually in the root directory.

-------------------------------------------------------------------------------
BUILDING
-------------------------------------------------------------------------------

I believe all build materials are included in this distribution.  If you find
a missing build file, please let me know.  However, the installation steps for
the library files are quite complex.   While I have tried to take notes, I
cannot provide tech support for build-related issues.

If you would like to port WED or any other tools to Linux, please let me know.

Mac Note: the WED binary is universal, but the SceneryTools.xcodeproj project
creates only a native executable for the platform it is compiled on.  This is
not a case of missing build materials; when I do a release of WED the script 
that packages WED merges the x86 and ppc versions, which must be compiled on
two separate computers.  (This isn't very good, but a lot of the libraries 
WED uses are built with GNU autoconf, which has a lot of problems building
universal libraries.)

-------------------------------------------------------------------------------
MISSING FILES
-------------------------------------------------------------------------------

If you find a missing file referenced in a build file, it could be due to a
problem with the filters when the archive is zipped - please report this!

-------------------------------------------------------------------------------
ADDITIONAL INFO
-------------------------------------------------------------------------------

The XPTools source tree is provided as-is.  In many cases, README files are 
provided in specific directories, but Laminar Research cannot provide technical
support for either the direct use of this source code or for derived works.

-------------------------------------------------------------------------------
CONTACT
-------------------------------------------------------------------------------

bsupnik at xsquawkbox dot net.