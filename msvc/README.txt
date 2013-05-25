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


* freetype_libs
* libjpeg
 download jpeg9 http://www.ijg.org/
 unzip
 
 for now use Visual Studio 2010's Command Promt tool and cd to your jpeg9 directory
 use the following command
 nmake -f set
 
 http://stackoverflow.com/questions/12644343/configuring-libjpeg-in-visual-studio-2010 <- do this or cry
 
 once thats done hit build on JPEG first then in Apps if you want those
 
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
 you need to have xtools_wed to have ZLIB_WINAPI
 
 squish
 copied all the C++ files, had to replace std::min,std::max with min and max because windows
 
 commctrl.h
 
 
 sqlite3
added the source and headers directly to the project, add these preprocessor directives

freetype
use the pre built
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
