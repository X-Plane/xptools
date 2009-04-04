-------------------------------------------------------------------------------
                           XPTOOLS SOURCE CODE README
-------------------------------------------------------------------------------

WORLD EDITOR SOURCE CODE ROADMAP

This file contains a brief overview to the source code for WorldEditor and 
friends.  The intended audience is a developer looking to recycle parts of, or 
augment, WorldEditor.

SOURCE TREE NOTES AND DEPENDENCIES

The XPTools source tree and its related derived binaries are generally 
open-source and may be dependent on GPL libraries.  All new code written by 
Laminar for WorldEditor is available under the MIT/X11 license.

Each subtree in this source tree represents a "package" or set of components/
translation units/objects  Each subtree directory will generally have a 
README.txt providing an  overview of the theme for that module.

APPLICATIONS

This source tree builds the following applications, all for Mac and Windows:

	SCENERY EDITING
		WorldEditor		GUI
		RenderFarm		Console (This is LR's internal DSF generator.)
		RenderFarmUI		GUI 	(This is LR's internal DSF generator.)
	XPTOOLS
		ObjConverter		Console
		DSF2Text		Console
		DDSTool			Console
		MeshTool		Console
		ObjView			GUI
		XGrinder		GUI
	AC3D
		XPlaneSupport		GUI
	OneOffs (unrelated command-line tools that can be useful.)
		osm_tile		Console
		osm2shape		Console
		
The code for some legacy projects like ObjEdit are probably floating around too.

EXTERNAL LIBRARIES

All of the libraries used by these apps are linked as external static libraries even
 though they may have DLL/dylib/so versions.  This is to minimize dependencies for
end users.

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
	USE_TIF, USE_JPEG
		These eanble TIFF and JPEG support in BitmapUtils.h and other files.
		At this point all of the apps have these enabled, since libjpeg and
		libtiff are part of the standard library set.

USE OF X-PLANE PLUGIN SYSTEM

The UI version of the RenderFarm uses a modified version of the X-Plane widgets
library (on top of an emulation of part of the XPLM 1.0 APIs) to do UI work.  However
all of the main binary distributed apps use the more portable, modern code in the
UI and GUI directories.

NOTE: while this code is  available under the MIT/X11 license 
the original XPLM code is NOT.

RENDERFARM VS XES VS WED

Some of the naming conventions for the main scenery tools code for creating base
meshes:

- "XES" (X-Plane Editable Scenery) is a high level binary format for GIS data
  that is used by RenderFarm (see below).  The "XESCore" package represent
  the core GIS engine used to make global scenery.  While XES 



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

AC3DPlugins			AC3D plugin
DSF				DSFlib (DSF read/write library)
DSFTools			DSF2Text
Env				ENVlib (ENV read/write library)
GUI				Cross-platform UI framework for WED
Installer			Discontinued early installer code
Interfaces			Abstract Interfaces for WED
MeshTool			MeshTool
Network				Networking Utils
OGLE				OpenGL text Editor - text editing middleware
Obj				OBJlib (OBJ read/write code)
ObjEdit				Discontinued OBJ texturing utility
OneOffs				Misc Command-line tools
RawImport			GIS data import code
RenderFarmUI			Visualization code for RenderFarm	
SDTS				Import code for SDTS file format
Tiger				Import code for TIGER/Line (old ASCII format)
TigerTools			Command-line toosl for processing TIGER/Line (ASCII format)
UI				Lower level cross-platform UI code for XGrinder/ObjView/GUI
Utils				Utility code for all apps
VPF				Vector Product Format file importe
WEDCore				WorldEditor - core data model
WEDDocs				WorldEditor - documentation sources
WEDEntities			WorldEditor - class Hierarchy for data model
WEDImportExport			WorldEditor - import/export code
WEDLibrary			WorldEditor - library view for art assets
WEDMap				WorldEditor - map UI
WEDProperties			WorldEditor - property editing/table UI
WEDResources			WorldEditor - graphic assets
WEDTCE				WorldEditor - texture coordinate editor
WEDWindows			WorldEditor - various windows
XESCore				GIS engine for global scenery
XESTools			RenderFarm command-parsing code
XPCompat			Legacy code to help compile code that is in both this tree and X-Plane.*
XPTools				Other XPTools applications (ObjView, ObjConverter, etc.)
XPWidgets			Widgets port and XPLM emulation
linuxinit			Linux boilerplate code

* Any code in this category is copyright LR, and licensed under the MIT/X11 license to save time
in developing the scenery tools.

Generally the vast majority of the WorldEditor code is either in the 
WED_xxx and GUI_ packages.
