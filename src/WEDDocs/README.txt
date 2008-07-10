-------------------------------------------------------------------------------

                       WORLD-EDITOR 1.0r1 RELEASE NOTES

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
LICENSING
-------------------------------------------------------------------------------

WorldEditor is available under the GPL - see the file COPYING for more 
information.

This is the binary distribution of WED, created for user convenience.  The
source code for WED may also be downloaded - see http://scenery.x-plane.com/
for more info.

(The source code created by Laminar Research is available under the MIT/X11
license.  However, WED requires third party code - available under a variety
of open source licenses - to compile.)

-------------------------------------------------------------------------------
SYSTEM REQUIREMENTS
-------------------------------------------------------------------------------

WED should run on any Windows or Macintosh computer capable of running X-Plane.
WED binaries are not available for Linux, and the code is not fully portable to
Linux.  Users have reported mixed results with WINE.

-------------------------------------------------------------------------------
INSTALLATION INSTRUCTIONS
-------------------------------------------------------------------------------

WED comes as a single application (.app or .exe) file.  Simply unzip it and
place it anywhere you'd like on your hard disk, then double-click to run.  It
does not need to be in an X-Plane folder.  This README file is not required to
run WED.

-------------------------------------------------------------------------------
GETTING ADDITIONAL HELP
-------------------------------------------------------------------------------

WED comes with a manual - to read the documentation, start WED, then pick
"WED User's Guide" from the Help menu.  Windows users must have Adobe Acrobat
Reader or some other PDF viewer to read the manual.

-------------------------------------------------------------------------------
BUG REPORTING
-------------------------------------------------------------------------------

Please report any bugs to bsupnik at xsquawkbox dot net.

-------------------------------------------------------------------------------
KNOWN ISSUES
-------------------------------------------------------------------------------

- The user's manual is still a work in progress.
- Overlay images on Windows must be on the same hard drive as the scenery pack
  you are editing.
- WED may not work right for broken mice.

-------------------------------------------------------------------------------
VERSION HISTORY
-------------------------------------------------------------------------------
1.0r1 - 7/10/07

New Manual

1.0b5 - 3/26/07

Fixed: no crash when duplicating runways
Fixed: images can span drives on Windows
Fixed: no infinite loops of TIFF errors
Fixed: no infinite loops of missing-disk errors
Fixed: bezier nodes snap when the parent polygon is selected
Fixed: terraserver works again

1.0b4 - 10/18/07

Fixed: apt.dat 810 import works
Fixed: some overlay images had wrong aspect ratio
Enhanced: WED supports some DDS formats (only certain compression) for overlay images.

1.0b3 - 9/17/07

Fixed: export codes 55 and 56 swapped
Fixed: snap to locked vertices works.
Fixed: exporting apt.dat for VERY old projects wont leave a zero-vertex airport marking line in the apt.dat file.
Fixed: x-system folder relableed to x-plane folder in startup window
Fixed: smarter parser of apt.dat 
Fixed: hidden vertices are not shown
Fixed: taxiway lights and line were copied to all segments when taxiway was copied
Fixed: mac version number fixed


1.0b2 - 8/10/07

Fixed: split beziers with lights won't cause lights everywhere.

Fixed: all files in x-system folder will show up on windows
Fixed: user's manual will open even if spaces in file name	
Fixed: TIF files now open
Fixed: colors on JPEG fixed
Fixed: same name runway can be in two airports.
Fixed: copy/paste fixed on Mac
Fixed: scroll wheel speed adjusted to be useful
Fixed: signs and light strcuture were imported/exported facing the wrong way


1.0b1 - 8/6/07

Initial release of WED.