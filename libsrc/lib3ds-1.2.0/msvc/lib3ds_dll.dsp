# Microsoft Developer Studio Project File - Name="lib3ds_dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=lib3ds_dll - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "lib3ds_dll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lib3ds_dll.mak" CFG="lib3ds_dll - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lib3ds_dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "lib3ds_dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "lib3ds_dll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "lib3ds_dll___Win32_Release"
# PROP BASE Intermediate_Dir "lib3ds_dll___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release\dll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIB3DS_DLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_LIB3DS_DLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /out:"Build\Release/lib3ds-120.dll"

!ELSEIF  "$(CFG)" == "lib3ds_dll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "lib3ds_dll___Win32_Debug"
# PROP BASE Intermediate_Dir "lib3ds_dll___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug\dll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIB3DS_DLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_LIB3DS_DLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"Build\Debug\lib3ds-120d.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "lib3ds_dll - Win32 Release"
# Name "lib3ds_dll - Win32 Debug"
# Begin Source File

SOURCE=..\lib3ds\atmosphere.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\atmosphere.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\background.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\background.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\camera.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\camera.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\chunk.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\chunk.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\chunktable.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\ease.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\ease.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\file.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\file.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\float.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\float.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\io.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\io.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\light.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\light.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\material.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\material.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\matrix.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\matrix.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\mesh.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\mesh.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\node.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\node.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\quat.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\quat.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\shadow.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\shadow.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\tcb.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\tcb.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\tracks.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\tracks.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\types.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\vector.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\vector.h
# End Source File
# Begin Source File

SOURCE=..\lib3ds\viewport.c
# End Source File
# Begin Source File

SOURCE=..\lib3ds\viewport.h
# End Source File
# End Target
# End Project
