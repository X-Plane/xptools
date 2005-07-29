# Microsoft Developer Studio Project File - Name="sdtsxx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sdtsxx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sdtsxx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sdtsxx.mak" CFG="sdtsxx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sdtsxx - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sdtsxx - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sdtsxx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sdtsxx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /ML /W3 /GR /GX /ZI /Od /I "../../Windows" /I ".." /I "." /I "../../sdts++" /I "../.." /I "c://cygwin/usr/local/include/boost_1_30_0" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /TP /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "sdtsxx - Win32 Release"
# Name "sdtsxx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\..\sdts++\io\FormatLexer.c"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\FormatParser.c"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Accessor.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_At.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Catd.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Cats.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Cell.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Comp.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Dddf.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Ddom.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Ddsh.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Dq.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_ForeignID.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Iden.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Iref.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Ldef.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Line.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Module.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Poly.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Ring.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Rsdf.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Spdm.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Stat.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Utils.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Xref.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_Field.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_Module.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_MultiTypeValue.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_Record.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_Subfield.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Converter.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DDR.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DDRField.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DDRLeader.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Directory.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DirEntry.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DR.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DRLeader.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Field.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211FieldArea.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211FieldFormat.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Leader.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Record.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211SubfieldFormat.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Utils.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Buffer.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Converter.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_ConverterFactory.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Error.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Reader.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Utils.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Writer.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\..\sdts++\io\FormatParser.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Accessor.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_At.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Catd.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Cats.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Cell.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Comp.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Dddf.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Ddom.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Ddsh.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Dq.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_ForeignID.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Iden.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Iref.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Ldef.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Line.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Module.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Poly.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Ring.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Rsdf.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Spdm.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Stat.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Utils.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\builder\sb_Xref.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_Field.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_Module.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_MultiTypeValue.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_Record.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\container\sc_Subfield.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211_yy.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Converter.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DDR.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DDRField.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DDRLeader.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Directory.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DirEntry.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DR.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211DRLeader.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Field.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211FieldArea.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211FieldFormat.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Leader.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Record.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211SubfieldFormat.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_8211Utils.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Buffer.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Converter.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_ConverterFactory.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Error.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Reader.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Utils.h"
# End Source File
# Begin Source File

SOURCE="..\..\sdts++\io\sio_Writer.h"
# End Source File
# End Group
# End Target
# End Project
