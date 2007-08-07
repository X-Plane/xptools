-------------------------------------------------------------------------------
                           XPTOOLS SOURCE CODE README
-------------------------------------------------------------------------------

WORLD EDITOR SOURCE CODE ROADMAP

This file contains a brief overview to the source code for WorldEditor and 
friends.  The intended audience is a developer looking to recycle parts of, or 
augment, WorldEditor.

SOURCE TREE NOTES AND DEPENDENCIES

The WorldEditor source tree and its related derived binaries are generally 
open-source and may be dependent on GPL libraries.  All new code written by 
Laminar for WorldEditor is available under the MIT/X11 license.

Certain directories from WorldEditor are also used by X-Plane.  For examle, 
X-plane uses DSFLib internally to read DSF files.  The directories that X-plane 
uses must be free of GPL dependencies because X-Plane is not open source.  I 
will try to note this where possible.

X-Plane's file structure currently includes some global dependencies.  In a few
cases the "XPCompat" headers are used to fake this environment for code that 
must coexist in both environments.

Each subtree in this source tree represents a "package" or set of components/
translation units/objects  Each subtree directory will generally have a 
README.txt providing an  overview of the theme for that module.

APPLICATIONS

This source tree builds the following applications, all for Mac and Windows:

	SCENERY EDITING
		GISTool			Console	
		WorldEditor		GUI
		RenderFarm		GUI/Console (This is LR's internal DSF generator.)
	XPTOOLS
		AddObjects		GUI
		Env2DSF			GUI
		GetImage		GUI
		ObjConverter	GUI
		ObjView			GUI
		XTaxiMaker		GUI
	DSFTOOLS
		DSF2Text		GUI
		DSFTool			Console
	OBJEDIT
		ObjEdit			GUI

EXTERNAL LIBRARIES

All of the libraries used by these apps are included as external static 
libraries even though they are all available in source code - this is done to 
simplify project management.

REQUIRED #DEFINE SYMBOLS

The following symbols are used that may need to be globally defined via the 
-D flag:

	APL	IBM	LIN	
		These flags controls the target platform.  Define oen of them to one 
		and the others to zero.
	DEV
		This turns on the dev build/debugging - define to one or zero.
	OPENGL_MAP
		This adds OpenGL data to the XESCore map classes.  This is needed to 
		build WorldEditor.  This is controlled by a flag because we don't want 
		to  spend memory on this data for the console version.  Define to one
		or zero.
	LIL BIG
		These flags set the endian-ness of the executable (little or big).

USE OF X-PLANE PLUGIN SYSTEM

Some OpenGL GUI apps (RenderFarm, ObjEdit) use the X-Plane plugin system's 
widgets library to draw UI in OpenGL.  However, this code has been 
significantly modified.  The modified widgets lib is included in source.  The 
XPLM has been partly included and partly hacked to not require a host 
application.  NOTE: while this code is  available under the MIT/X11 license 
the original XPLM code is NOT.

Other OpenGL apps do not require a framework (ObjView) or use "GUI", a more 
advanced open source framework, to render UI.

RENDERFARM VS XES VS WED

The current naming convention for packages is in flux and is thus very
confusing.  There are a few separate threads of technology that all exist in
this one code base:

- "XES" (X-Plane Editable Scenery) is a high level binary format for GIS data
  that is used by RenderFarm (see below).  The "XESCore" code represent
  algorithms and data structures for manipulating GIS data, usually saved to
  XES files.
- "RenderFarm" is the name of the internal program we use to generate the 
  default scenery.  It comes as a command-line tool and a UI version (the UI
  can only run command-line transforms and show results on a map - it is meant
  primarily for debugging).
  
  Previously RenderFarm with UI was called "WorldEditor" and RenderFarm on the
  command line was called "GISTool".  So ... most code that is in the 
  "WorldEditor" folder is actually for RenderFarm (UI version), and most code
  labeled "GISTool_XXXX" is actually for the RenderFarm command-line version.
  Careful examination is necessary.
  
- The actual WorldEditor 1.0 application is not based on any of the above code.
  Instead WED saves files to a database via sqlite3.
  
In the long term I expect the XESCore algorithms used by RenderFarm to also be
used by WED, and I expect more code sharing between the UI and non-UI 
renderfarm apps, but I do not expect WED and RenderFarm to become one large app.

PACKAGE OVERVIEW

DSF				*	DSF I/O library.
DSFTools			Code for DSFTool and DSF2Text
Env					ENV7 import/export code
GUI					X-platform OpenGL UI framework, used for WorldEditor.
Installer			Installer utilities
Interfaces			Abstract interfaces for WED.
Mac Infrastructure	Precompiled headers etc. for XPTools.
Mac Resources		Mac specific resource files.
Network				Socket, HTTP, and Terraserver access.
Obj					X-Plane object, facade and road network handling code.
ObjEdit				ObjEdit specific source code, mostly UI.
RawImport			Import code for variosu raw data file formats
SDTS				"Spatial Data Transfer Spec" import/export code.
ShadowDemo			Demo code - per-pixel shadow mapping.
Tiger				Import and processing of TIGER/Line data
TigerTools			Standalone command line tools for TIGER/Line data
UI					UI wrapper classes for grinders and widget-based apps.
Utils			*	A dump for utility function collections
VPF					"Vector Product Format" import/export code
WED*				World-Editor specific code.
WorldEditor			Renderfarm-specific code (mostly UI)
XESTools			Source for Cmd-line GISTool and other misc. RenderFarm code.
XPCompat			Compatibility - simulation of some x-plane headers.
XPTools				Source to the 6 XPTools apps.
XPWidgets		**	A derivation nof the XPLM and XPWidgets DLLs for UI.
XESCore				Core files for XES files - defs, I/O, algorithms, etc.

*	Used by X-Plane.
**	This is a derivation of the X-Plane plugin code with major changes.

Generally the vast majority of the WorldEditor code is either in the WorldEditor
package (for WorldEditor UI code), or XESCore for the core algorithms that are
used in the command line tool and the app.
