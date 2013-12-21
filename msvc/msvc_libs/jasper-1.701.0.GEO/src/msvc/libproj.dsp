# Microsoft Developer Studio Project File - Name="libproj" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libproj - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libproj.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libproj.mak" CFG="libproj - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libproj - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libproj - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libproj - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_Release"
# PROP Intermediate_Dir "libproj___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../libproj" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BSDTYPES" /FR /YX /FD /O3 -O3 /QaxW -QaxW /c
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libproj - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_Debug"
# PROP Intermediate_Dir "libproj___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../libproj" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BSDTYPES" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x416 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libproj - Win32 Release"
# Name "libproj - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\libproj\aasincos.c
# End Source File
# Begin Source File

SOURCE=..\libproj\adjlon.c
# End Source File
# Begin Source File

SOURCE=..\libproj\bch2bps.c
# End Source File
# Begin Source File

SOURCE=..\libproj\bchgen.c
# End Source File
# Begin Source File

SOURCE=..\libproj\biveval.c
# End Source File
# Begin Source File

SOURCE=..\libproj\dmstor.c
# End Source File
# Begin Source File

SOURCE=..\libproj\emess.c
# End Source File
# Begin Source File

SOURCE=..\libproj\gen_cheb.c
# End Source File
# Begin Source File

SOURCE=..\libproj\geocent.c
# End Source File
# Begin Source File

SOURCE=..\libproj\geod_for.c
# End Source File
# Begin Source File

SOURCE=..\libproj\geod_inv.c
# End Source File
# Begin Source File

SOURCE=..\libproj\geod_set.c
# End Source File
# Begin Source File

SOURCE=..\libproj\jniproj.c
# End Source File
# Begin Source File

SOURCE=..\libproj\mk_cheby.c
# End Source File
# Begin Source File

SOURCE=..\libproj\nad_cvt.c
# End Source File
# Begin Source File

SOURCE=..\libproj\nad_init.c
# End Source File
# Begin Source File

SOURCE=..\libproj\nad_intr.c
# End Source File
# Begin Source File

SOURCE=..\libproj\p_series.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_aea.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_aeqd.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_airy.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_aitoff.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_apply_gridshift.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_august.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_auth.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_bacon.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_bipc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_boggs.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_bonne.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_cass.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_cc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_cea.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_chamb.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_collg.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_crast.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_datum_set.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_datums.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_denoy.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_deriv.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_eck1.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_eck2.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_eck3.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_eck4.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_eck5.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_ell_set.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_ellps.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_eqc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_eqdc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_errno.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_factors.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_fahey.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_fouc_s.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_fwd.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_gall.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_gauss.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_geocent.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_geos.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_gins8.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_gn_sinu.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_gnom.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_goode.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_gridinfo.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_gridlist.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_hammer.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_hatano.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_imw_p.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_init.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_inv.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_krovak.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_labrd.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_laea.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_lagrng.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_larr.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_lask.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_latlong.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_lcc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_lcca.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_list.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_loxim.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_lsat.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_malloc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_mbt_fps.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_mbtfpp.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_mbtfpq.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_merc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_mill.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_mlfn.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_mod_ster.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_moll.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_mpoly.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_msfn.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_nell.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_nell_h.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_nocol.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_nsper.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_nzmg.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_ob_tran.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_ocea.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_oea.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_omerc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_open_lib.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_ortho.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_param.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_phi2.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_poly.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_pr_list.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_putp2.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_putp3.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_putp4p.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_putp5.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_putp6.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_qsfn.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_release.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_robin.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_rpoly.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_sconics.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_somerc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_stere.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_sterea.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_strerrno.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_sts.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_tcc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_tcea.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_tmerc.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_tpeqd.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_transform.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_tsfn.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_units.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_urm5.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_urmfps.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_utils.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_vandg.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_vandg2.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_vandg4.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_wag2.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_wag3.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_wag7.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_wink1.c
# End Source File
# Begin Source File

SOURCE=..\libproj\PJ_wink2.c
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_zpoly1.c
# End Source File
# Begin Source File

SOURCE=..\libproj\rtodms.c
# End Source File
# Begin Source File

SOURCE=..\libproj\vector1.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\libproj\emess.h
# End Source File
# Begin Source File

SOURCE=..\libproj\geocent.h
# End Source File
# Begin Source File

SOURCE=..\libproj\geodesic.h
# End Source File
# Begin Source File

SOURCE=..\libproj\nad_list.h
# End Source File
# Begin Source File

SOURCE=..\libproj\org_proj4_Projections.h
# End Source File
# Begin Source File

SOURCE=..\libproj\pj_list.h
# End Source File
# Begin Source File

SOURCE=..\libproj\proj_api.h
# End Source File
# Begin Source File

SOURCE=..\libproj\proj_config.h
# End Source File
# Begin Source File

SOURCE=..\libproj\projects.h
# End Source File
# End Group
# End Target
# End Project
