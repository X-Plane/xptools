==============================================================
				Microsoft Visual Studio Project
==============================================================
[Chapter.Section.Subsection] Title: Content is shown here in sentances. The
bracket system makes for very easy searching. Try it out!
		* Content can also be shown here in bullet points.
		* Like this.
			* Also, like this.
		1.) Content can also be shown in a numbered list.
		2.) In this format as you can see.
		3.) It can also have a numbered list.
		
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[1.0.0] Introduction
[2.0.0] Getting Started
[3.0.0] Working With This Project
[4.0.0] Troubleshooting and Tips
[5.0.0] Appendix

/*==[1.0.0] Introduction==============================================

		[1.1.0] Abstract
		[1.2.0] Requirments
		[1.3.0] Terms

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[1.1.0] Abstract: The purpose of this project is to let standard developement tools
such as Microsoft Visual Studio 2012 and Git be used in the place of numours linux
style developement tools. It is also designed to decrease the amount of work in
obtaining, compiling, and implementing libereraries. Overall this project is designed
to decrease the amount of set up time needed. It was first created by Theodore
"Ted" Greene (theodorengreene at gmail dot com) and 
Ben Supnik (bsupnik at xsquawkbox dot net).

[1.2.0] Requirments: To use this software have the following tools

* Microsoft Visual Studio Express 2012 for Desktop
	* MSVC Studio 2010 has not been fully tested though it appears to be compatable
* The latest version of Git

[1.3.0] Terms: Follow all licenses and restrictions of their owners
==============================================================*/
 
/*
 
 
/*==[3.0.0] Working With this Project==================================
 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

==============================================================*/
 
 
 
 /*== [4.0.0] Troubleshooting==========================================

		[4.X.0] Visual Studio Issues
		[4.X.0] Compiling Issues
		[4.X.0] Liberary Issues
		[4.X.0] Git Issues
		[4.X.0] Other Issues

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		[4.X.0] Compiling Issues
			[4.X.0] Common Error Codes
		I keep getting error C1083, what is that and how can I make it stop?
			A liberary is out of place or you have changed the settings inside the projects settings
			
		ErrorC4496
			http://nndung179.wordpress.com/2012/10/14/fix-error-c4996/
			make sure to put in _CRT_SECURE_NO_WARNINGS
		
		[4.X.0] Liberary Issues
			[4.1.0] How to get the libraries prepared for MSVC
				Please note that you MUST make sure that you are using the VS2010 tools or you will most likely have a REALLY BAD TIME
			
				expat
					 files taken from the the latest expat 2.1.0 installer
					 just build with the included vsproject file <-all linker errors gone
					 make sure that a preprocessor called XML_STATIC is put in there.
				 
				zlib  
					you need to have xtools_wed to NOT have ZLIB_WINAPI
				 
				squish
					copied all the C++ files, had to replace std::min,std::max with min and max because windows
				 
				commctrl.h
					Just make sure its from windows SDK
				 
				sqlite3
					added the source and headers directly to the project, add these preprocessor directives

				freetype
					in the msvc project Property Pages-> C/C++->Code Generation-> Change the Runtime Library to MT
					/GS to /GS- to try and remove remote_rangecheckerror security buffer check
				jasper-1.701.0.GEO
					This contains Jasper and the extention for Geo data. From the msvc folder open geojasper.dsw. Visual Studio 2010/2012 will convert these projects to the latest new form. Build the whole solution. Make sure that the toolset matches the Visual Studio project! (This uses 2010 currently) You must also have the following preprocessor definition: JAS_WIN_MSVC_BUILD
					
					Due to the fact of weird Enable Intrinsic Functions that can be diffrent for Debug and Release, it could run into area's of weirdness if very intense floating point operations is required - 12/20/2013
				jpeg 
					for now use Visual Studio 2010's Command Promt tool and cd to your jpeg9 directory
					use the following command
					nmake -f set
					resave jconfig.vc to jconfig.h
					 http://stackoverflow.com/questions/12644343/configuring-libjpeg-in-visual-studio-2010 <- do this or cry
					 then use the jpeg .sln and make sure to
				 
				libpng
					http://www.leptonica.org/vs2008doc/building-image-libraries.html <- follow this link
				 
				libtiff
					C:\tiff-4.0.0> nmake /f makefile.vc clean
					C:\tiff-4.0.0> nmake /f makefile.vc
					 tiff
					 resave tiffconf.vc.h to tiffconf.h
				 
				proj-4.7.0
					C:\PROJ> nmake /f makefile.vc
					C:\PROJ> nmake /f makefile.vc install-all
					Cut from its default location to inside msvc_libs/proj-4.7.0, copy .lib to compiled libs

				geotiff
					change makefile.vc
					
					
					
					FROM
					TIFF_DIR = ..\libtiff\libtiff

					TIFF_INC = -I$(TIFF_DIR)
					TIFF_LIB = $(TIFF_DIR)\libtiff.lib
					TIFF_LIB_DLL = $(TIFF_DIR)\libtiff_i.lib

					TO
					TIFF_DIR = ..\tiff-4.0.0beta5\libtiff
					TIFF_INC = -I$(TIFF_DIR)
					TIFF_LIB = ..\..\msvc_compiled_libs\libtiff.lib
					TIFF_LIB_DLL = $(TIFF_DIR)\libtiff_i.lib

					FROM
					CFLAGS  = $(INCL) /MD /Ox /nologo
					TO
					CFLAGS  = $(INCL) /MT /Ox /nologo

					FROM
					# Installation locations (with install, or devinstall targets)
					PREFIX =	c:\usr
					TO
					PREFIX = .
					
					add
					/* Added to force LIB_PROJ compilation */
					#define HAVE_LIBPROJ 1
					#define HAVE_PROJECTS_H 1
					#define PROJECTS_DIR "..\proj-4.7.0\src\projects.h"
					to geo_config.h.vc
					
					in geotiff_proj4.c commment out line 1402,add this to line 1403
					#include PROJECTS_DIR
					
					run nmake /f makefile.vc all
					
					You will get linker errors but they will be solved when linking together WED
					
					Copy geotiff.lib to msvc_compiled_libs
						openGL<-put in manually (if not already in)
					
==============================================================*/
 
 /*== [5.0.0] Appendix=============================================
		
		[5.1.0] List of Preprocessor Definitions
			[5.1.1] General
			[5.1.2] Debug (x86)
			[5.1.3] Release (x86)
		[5.2.0] List of Other Configuration Properties
			[5.2.1] General
			[5.2.2] Debug (x86)
			[5.2.3] Release (x86)
			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		
 [5.1.1] General: These preprocessor definitions work for all build configurations.

 -------------------------------------------------------------------------------
| CODE | ORIGIN | PURPOSE																|
| NO_CGAL=1 | WED | Turns off CGAL from being used, avoids remaing CGAL errors			|
| MSC=1 | WED | Have Visual C compiler, could be replaced by ifdef _MSC_VER				|
| APL=0 | WED | Platform defined for Apple												|
| IBM=1 | WED | Platform defined for Windows											|
| USE_JPEG=1 | WED | ImgUtils will support libjpeg										|
| USE_TIF=1 | WED | ImgUtils will support libtiff										|
| USE_GEOJPEG2K=1 | WED| ImgUtils will supportGEOJPEG2K									|
| WED=1 | WED | Declares that one is building WED and WED only							|
| LIL=1 | WED | For x86 CPU Arcatecture													|
| BIG=0 | WED | Depricated from supporting PowerPC										|
|JAS_WIN_MSVC_BUILD| GeoJasper | Makes GeoJasper compile, part of its configuration		|
| XML_STATIC | EXPAT | Stops missing .DLL error, see line 57 in expat_external.h		|
| WIN32 | MSVC | Used in Windows SDK													|
| NDEBUG | MSVC | Activates assert.h functionality										|
| _LIB | MSVC | Appears to be neither used nor detrimental								|
| %(PreprocessorDefinitions) | MSVC | Puts in other hidden Preprocessor Definitions		|
| _CRT_SECURE_NO_WARNINGS | MSVC | Stops unneccassary Error C4496             			|
 --------------------------------------------------------------------------------

 [5.1.2] Debug (x86): These definitions only appear in debug configurations.
  -------------------------------------------------------------------------------
| CODE | ORIGIN | PURPOSE                                                              |
| DEV=1 | WED | Activates debug checks inside WED code                                 |
  -------------------------------------------------------------------------------

 [5.1.3] Release (x86): These definitions only appear in debug configurations.
  -------------------------------------------------------------------------------
| CODE | ORIGIN | PURPOSE                                                              |
| DEV=0 | WED | Activates debug checks inside WED code                                 |
  -------------------------------------------------------------------------------

 [5.2.1]  General: These settings apply to all configurations (and are not on by default)
 
 * General
	* Output Directory: $(Configuration)\bin\
	* Intermediate Directory: $(Configuration)\tmp\
	* Platform Toolset: Visual Studio 2010 (v100)
 *  C/C++
	* Additional Include Directories:
	..\..\msvc_libs\
	..\..\msvc_libs\jasper-1.701.0.GEO\src\libjasper\include;
	..\..\msvc_libs\sqlite-3.6.21;
	..\..\msvc_libs\tiff-4.0.0beta5\libtiff;
	..\..\msvc_libs\libgeotiff-1.2.5;
	..\..\msvc_libs\jpeg-8c;
	..\..\msvc_libs\squish-1.10;
	..\..\msvc_libs\proj-4.7.0\src;
	..\..\msvc_libs\tiff-4.0.0beta5\contrib\tags;
	..\..\msvc_libs\libpng_zlib_BFOLDER\libpng-1.2.41;
	..\..\msvc_libs\freetype-2.2.1\include;
	..\..\msvc_libs\libpng_zlib_BFOLDER\zlib; 
	..\..\msvc_libs\Expat2.1.0\Source\lib;
	..\..\..\src\XPCompat\;
	..\..\..\src\Interfaces;
	..\..\..\src\WEDXPlugin;
	..\..\..\src\WEDWindows;
	..\..\..\src\WEDTCE;
	..\..\..\src\WEDResources;
	..\..\..\src\WEDProperties;
	..\..\..\src\WEDNetwork;
	..\..\..\src\WEDMap;
	..\..\..\src\WEDLibrary;
	..\..\..\src\XESCore;
	..\..\..\src\WEDImportExport;
	..\..\..\src\Utils;
	..\..\..\src\UI;
	..\..\..\src\OGLE;
	..\..\..\src\Obj;
	..\..\..\src\WEDCore;
	..\..\..\src\Network;
	..\..\..\src\GUI;
	..\..\..\src\Env;
	..\..\..\src\DSFTools;
	..\..\..\src\DSF\tri_stripper_101;
	..\..\..\src\DSF;
	..\..\..\src\WEDEntities;
	%(AdditionalIncludeDirectories)
	
	* Code Generation
		* Runtime Library: Mutlti-threaded /MT
		* Enable Function-Level Linking: Yes (/Gy)
	
	* Advanced
		* Disable Specific Warnings: 4018;4244;4068;4800;4305;4355;4200;4250;
		* Forced Include File: ..\..\src\Obj\XDefs.h;%(ForcedIncludeFiles)
 
 * Linker
	* Input
		*Additional Dependencies:
		Comctl32.lib;
		Ws2_32.lib;
		opengl32.lib;
		glu32.lib;
		..\..\msvc_libs\msvc_compiled_libs\libjasper.lib;
		..\..\msvc_libs\msvc_compiled_libs\libexpatMT.lib;
		..\..\msvc_libs\msvc_compiled_libs\zlibstat.lib;
		..\..\msvc_libs\msvc_compiled_libs\freetype221.lib;
		..\..\msvc_libs\msvc_compiled_libs\jpeg.lib;
		..\..\msvc_libs\msvc_compiled_libs\libpng.lib;
		..\..\msvc_libs\msvc_compiled_libs\libtiff.lib;
		..\..\msvc_libs\msvc_compiled_libs\proj.lib;
		..\..\msvc_libs\msvc_compiled_libs\geotiff.lib;
		%(AdditionalDependencies)
		*Ignore All Defualt Libraries: No
	* Optimization
		* Refrences: Yes
		* Enable COMDAT Folding: Yes
		* Link Time Code Generation: Default
		
 [5.2.2] Debug (x86): These settings only appear in debug configurations
  *General
	* Whole Program Optimization: No Whole Program Optimization
  *C/C++
	* Optimization
		Optimization: Disabled
		
 [5.2.3] Debug (x86): These settings only appear in debug configurations
  *General
	* Whole Program Optimization: Use Link Time Code Generation
  *C/C++
	* Optimization
		Optimization: /OS2
  *Linker
	*Debugging
		* Generate Debug Info: No
	*Optimization
		* Link Time Code Generation: Use Link Time Code Generation (/LTCG)

==============================================================*/