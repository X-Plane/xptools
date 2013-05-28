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

/==[1.0.0] Introduction=============================================

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

 
 * expat appears to only need the .h, no other files. .h is located ..\..\msvc_libs\expat-2.0.1\lib otherwise a collection of other opt
 ions 
 
 geotiff
 resave the geo_config.h.vc as geo_config.h
 
 tiff
 resave tiffconf.vc.h to tiffconf.h
 
 jpeg
 resave jconfig.vc to jconfig.h
 
 expat
 just build with the included vsproject file <-all linker errors gone
 
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

 jpeg 
 for now use Visual Studio 2010's Command Promt tool and cd to your jpeg9 directory
 use the following command
 nmake -f set
 
 http://stackoverflow.com/questions/12644343/configuring-libjpeg-in-visual-studio-2010 <- do this or cry
 then use the jpeg .sln and make sure to
 
 libpng
 http://www.leptonica.org/vs2008doc/building-image-libraries.html <- follow this link
 
libtiff
C:\tiff-4.0.0> nmake /f makefile.vc clean
C:\tiff-4.0.0> nmake /f makefile.vc

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

Copied geotiff.lib to msvc_compiled_libs
==============================================================/
 
 
 
/==[3.0.0] Working With this Project====================================
 
		[3.X.0] Special Pre Processor Directives
			[3.X.1] Debug Mode
			[3.X.2] Release Mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

==============================================================/
 
 
 
 /== [4.0.0] Troubleshooting==========================================

		[4.X.0] Visual Studio Issues
		[4.X.0] Compiling Issues
		[4.X.0] Liberary Issues
		[4.X.0] Git Issues
		[4.X.0] Other Issues

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 I keep getting error C1083, what is that and how can I make it stop?
	A liberary is out of place
ErrorC4496
http://nndung179.wordpress.com/2012/10/14/fix-error-c4996/
make sure to put in _CRT_SECURE_NO_WARNINGS


openGL<-put in manually.
