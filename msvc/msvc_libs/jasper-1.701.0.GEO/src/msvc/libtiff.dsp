# Microsoft Developer Studio Project File - Name="libtiff" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libtiff - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libtiff.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libtiff.mak" CFG="libtiff - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libtiff - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libtiff - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libtiff - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_Release"
# PROP Intermediate_Dir "libtiff___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\libjpeg" /I "..\libjasper\tiffgeo" /I "..\libproj" /I "..\libjasper\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BSDTYPES" /D "HAVE_LIBPROJ" /D "HAVE_PROJECTS_H" /D "USE_WIN32_FILEIO" /D "TIF_PLATFORM_CONSOLE" /FR /YX /FD /O3 -O3 /QaxW -QaxW /c
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libtiff - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_Debug"
# PROP Intermediate_Dir "libtiff___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\libjpeg" /I "..\libjasper\tiffgeo" /I "..\libproj" /I "..\libjasper\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BSDTYPES" /D "HAVE_LIBPROJ" /D "HAVE_PROJECTS_H" /D "USE_WIN32_FILEIO" /D "TIF_PLATFORM_CONSOLE" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libtiff - Win32 Release"
# Name "libtiff - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "csv"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\libjasper\tiffgeo\csv\datum.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\csv\ellipsoid.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\csv\gcs.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\csv\pcs.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\csv\prime_meridian.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\csv\projop_wparm.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\csv\unit_of_measure.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\libjasper\tiffgeo\cpl_csv_incode.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\cpl_serv.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\fax3sm_winnt.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_extra.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_free.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_get.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_names.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_new.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_normalize.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_print.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_set.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_tiffp.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_trans.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geo_write.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\geotiff_proj4.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_aux.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_close.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_codec.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_color.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_compress.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_dir.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_dirinfo.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_dirread.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_dirwrite.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_dumpmode.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_error.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_extension.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_fax3.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_flush.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_getimage.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_jpeg.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_luv.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_lzw.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_next.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_ojpeg.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_open.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_packbits.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_pixarlog.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_predict.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_print.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_read.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_strip.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_swab.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_thunder.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_tile.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_version.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_warning.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_win32.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_write.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_zip.c
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\xtiff.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\libjasper\tiffgeo\port.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\t4.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_dir.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_fax3.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tif_predict.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tiff.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tiffcomp.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tiffconf.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tiffio.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\tiffiop.h
# End Source File
# Begin Source File

SOURCE=..\libjasper\tiffgeo\version.h
# End Source File
# End Group
# End Target
# End Project
