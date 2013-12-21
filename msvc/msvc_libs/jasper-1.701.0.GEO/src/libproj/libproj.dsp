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
# ADD CPP /nologo /W3 /GX /O2 /I "./" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BSDTYPES" /FR /YX /FD /c
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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BSDTYPES" /FR /YX /FD /GZ /c
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

SOURCE=.\aasincos.c
# End Source File
# Begin Source File

SOURCE=.\adjlon.c
# End Source File
# Begin Source File

SOURCE=.\bch2bps.c
# End Source File
# Begin Source File

SOURCE=.\bchgen.c
# End Source File
# Begin Source File

SOURCE=.\biveval.c
# End Source File
# Begin Source File

SOURCE=.\dmstor.c
# End Source File
# Begin Source File

SOURCE=.\emess.c
# End Source File
# Begin Source File

SOURCE=.\gen_cheb.c
# End Source File
# Begin Source File

SOURCE=.\geocent.c
# End Source File
# Begin Source File

SOURCE=.\geod_for.c
# End Source File
# Begin Source File

SOURCE=.\geod_inv.c
# End Source File
# Begin Source File

SOURCE=.\geod_set.c
# End Source File
# Begin Source File

SOURCE=.\jniproj.c
# End Source File
# Begin Source File

SOURCE=.\mk_cheby.c
# End Source File
# Begin Source File

SOURCE=.\nad_cvt.c
# End Source File
# Begin Source File

SOURCE=.\nad_init.c
# End Source File
# Begin Source File

SOURCE=.\nad_intr.c
# End Source File
# Begin Source File

SOURCE=.\p_series.c
# End Source File
# Begin Source File

SOURCE=.\PJ_aea.c
# End Source File
# Begin Source File

SOURCE=.\PJ_aeqd.c
# End Source File
# Begin Source File

SOURCE=.\PJ_airy.c
# End Source File
# Begin Source File

SOURCE=.\PJ_aitoff.c
# End Source File
# Begin Source File

SOURCE=.\pj_apply_gridshift.c
# End Source File
# Begin Source File

SOURCE=.\PJ_august.c
# End Source File
# Begin Source File

SOURCE=.\pj_auth.c
# End Source File
# Begin Source File

SOURCE=.\PJ_bacon.c
# End Source File
# Begin Source File

SOURCE=.\PJ_bipc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_boggs.c
# End Source File
# Begin Source File

SOURCE=.\PJ_bonne.c
# End Source File
# Begin Source File

SOURCE=.\PJ_cass.c
# End Source File
# Begin Source File

SOURCE=.\PJ_cc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_cea.c
# End Source File
# Begin Source File

SOURCE=.\PJ_chamb.c
# End Source File
# Begin Source File

SOURCE=.\PJ_collg.c
# End Source File
# Begin Source File

SOURCE=.\PJ_crast.c
# End Source File
# Begin Source File

SOURCE=.\pj_datum_set.c
# End Source File
# Begin Source File

SOURCE=.\pj_datums.c
# End Source File
# Begin Source File

SOURCE=.\PJ_denoy.c
# End Source File
# Begin Source File

SOURCE=.\pj_deriv.c
# End Source File
# Begin Source File

SOURCE=.\PJ_eck1.c
# End Source File
# Begin Source File

SOURCE=.\PJ_eck2.c
# End Source File
# Begin Source File

SOURCE=.\PJ_eck3.c
# End Source File
# Begin Source File

SOURCE=.\PJ_eck4.c
# End Source File
# Begin Source File

SOURCE=.\PJ_eck5.c
# End Source File
# Begin Source File

SOURCE=.\pj_ell_set.c
# End Source File
# Begin Source File

SOURCE=.\pj_ellps.c
# End Source File
# Begin Source File

SOURCE=.\PJ_eqc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_eqdc.c
# End Source File
# Begin Source File

SOURCE=.\pj_errno.c
# End Source File
# Begin Source File

SOURCE=.\pj_factors.c
# End Source File
# Begin Source File

SOURCE=.\PJ_fahey.c
# End Source File
# Begin Source File

SOURCE=.\PJ_fouc_s.c
# End Source File
# Begin Source File

SOURCE=.\pj_fwd.c
# End Source File
# Begin Source File

SOURCE=.\PJ_gall.c
# End Source File
# Begin Source File

SOURCE=.\pj_gauss.c
# End Source File
# Begin Source File

SOURCE=.\pj_geocent.c
# End Source File
# Begin Source File

SOURCE=.\PJ_geos.c
# End Source File
# Begin Source File

SOURCE=.\PJ_gins8.c
# End Source File
# Begin Source File

SOURCE=.\PJ_gn_sinu.c
# End Source File
# Begin Source File

SOURCE=.\PJ_gnom.c
# End Source File
# Begin Source File

SOURCE=.\PJ_goode.c
# End Source File
# Begin Source File

SOURCE=.\pj_gridinfo.c
# End Source File
# Begin Source File

SOURCE=.\pj_gridlist.c
# End Source File
# Begin Source File

SOURCE=.\PJ_hammer.c
# End Source File
# Begin Source File

SOURCE=.\PJ_hatano.c
# End Source File
# Begin Source File

SOURCE=.\PJ_imw_p.c
# End Source File
# Begin Source File

SOURCE=.\pj_init.c
# End Source File
# Begin Source File

SOURCE=.\pj_inv.c
# End Source File
# Begin Source File

SOURCE=.\PJ_krovak.c
# End Source File
# Begin Source File

SOURCE=.\PJ_labrd.c
# End Source File
# Begin Source File

SOURCE=.\PJ_laea.c
# End Source File
# Begin Source File

SOURCE=.\PJ_lagrng.c
# End Source File
# Begin Source File

SOURCE=.\PJ_larr.c
# End Source File
# Begin Source File

SOURCE=.\PJ_lask.c
# End Source File
# Begin Source File

SOURCE=.\pj_latlong.c
# End Source File
# Begin Source File

SOURCE=.\PJ_lcc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_lcca.c
# End Source File
# Begin Source File

SOURCE=.\pj_list.c
# End Source File
# Begin Source File

SOURCE=.\PJ_loxim.c
# End Source File
# Begin Source File

SOURCE=.\PJ_lsat.c
# End Source File
# Begin Source File

SOURCE=.\pj_malloc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_mbt_fps.c
# End Source File
# Begin Source File

SOURCE=.\PJ_mbtfpp.c
# End Source File
# Begin Source File

SOURCE=.\PJ_mbtfpq.c
# End Source File
# Begin Source File

SOURCE=.\PJ_merc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_mill.c
# End Source File
# Begin Source File

SOURCE=.\pj_mlfn.c
# End Source File
# Begin Source File

SOURCE=.\PJ_mod_ster.c
# End Source File
# Begin Source File

SOURCE=.\PJ_moll.c
# End Source File
# Begin Source File

SOURCE=.\PJ_mpoly.c
# End Source File
# Begin Source File

SOURCE=.\pj_msfn.c
# End Source File
# Begin Source File

SOURCE=.\PJ_nell.c
# End Source File
# Begin Source File

SOURCE=.\PJ_nell_h.c
# End Source File
# Begin Source File

SOURCE=.\PJ_nocol.c
# End Source File
# Begin Source File

SOURCE=.\PJ_nsper.c
# End Source File
# Begin Source File

SOURCE=.\PJ_nzmg.c
# End Source File
# Begin Source File

SOURCE=.\PJ_ob_tran.c
# End Source File
# Begin Source File

SOURCE=.\PJ_ocea.c
# End Source File
# Begin Source File

SOURCE=.\PJ_oea.c
# End Source File
# Begin Source File

SOURCE=.\PJ_omerc.c
# End Source File
# Begin Source File

SOURCE=.\pj_open_lib.c
# End Source File
# Begin Source File

SOURCE=.\PJ_ortho.c
# End Source File
# Begin Source File

SOURCE=.\pj_param.c
# End Source File
# Begin Source File

SOURCE=.\pj_phi2.c
# End Source File
# Begin Source File

SOURCE=.\PJ_poly.c
# End Source File
# Begin Source File

SOURCE=.\pj_pr_list.c
# End Source File
# Begin Source File

SOURCE=.\PJ_putp2.c
# End Source File
# Begin Source File

SOURCE=.\PJ_putp3.c
# End Source File
# Begin Source File

SOURCE=.\PJ_putp4p.c
# End Source File
# Begin Source File

SOURCE=.\PJ_putp5.c
# End Source File
# Begin Source File

SOURCE=.\PJ_putp6.c
# End Source File
# Begin Source File

SOURCE=.\pj_qsfn.c
# End Source File
# Begin Source File

SOURCE=.\pj_release.c
# End Source File
# Begin Source File

SOURCE=.\PJ_robin.c
# End Source File
# Begin Source File

SOURCE=.\PJ_rpoly.c
# End Source File
# Begin Source File

SOURCE=.\PJ_sconics.c
# End Source File
# Begin Source File

SOURCE=.\PJ_somerc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_stere.c
# End Source File
# Begin Source File

SOURCE=.\PJ_sterea.c
# End Source File
# Begin Source File

SOURCE=.\pj_strerrno.c
# End Source File
# Begin Source File

SOURCE=.\PJ_sts.c
# End Source File
# Begin Source File

SOURCE=.\PJ_tcc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_tcea.c
# End Source File
# Begin Source File

SOURCE=.\PJ_tmerc.c
# End Source File
# Begin Source File

SOURCE=.\PJ_tpeqd.c
# End Source File
# Begin Source File

SOURCE=.\pj_transform.c
# End Source File
# Begin Source File

SOURCE=.\pj_tsfn.c
# End Source File
# Begin Source File

SOURCE=.\pj_units.c
# End Source File
# Begin Source File

SOURCE=.\PJ_urm5.c
# End Source File
# Begin Source File

SOURCE=.\PJ_urmfps.c
# End Source File
# Begin Source File

SOURCE=.\pj_utils.c
# End Source File
# Begin Source File

SOURCE=.\PJ_vandg.c
# End Source File
# Begin Source File

SOURCE=.\PJ_vandg2.c
# End Source File
# Begin Source File

SOURCE=.\PJ_vandg4.c
# End Source File
# Begin Source File

SOURCE=.\PJ_wag2.c
# End Source File
# Begin Source File

SOURCE=.\PJ_wag3.c
# End Source File
# Begin Source File

SOURCE=.\PJ_wag7.c
# End Source File
# Begin Source File

SOURCE=.\PJ_wink1.c
# End Source File
# Begin Source File

SOURCE=.\PJ_wink2.c
# End Source File
# Begin Source File

SOURCE=.\pj_zpoly1.c
# End Source File
# Begin Source File

SOURCE=.\rtodms.c
# End Source File
# Begin Source File

SOURCE=.\vector1.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\emess.h
# End Source File
# Begin Source File

SOURCE=.\geocent.h
# End Source File
# Begin Source File

SOURCE=.\geodesic.h
# End Source File
# Begin Source File

SOURCE=.\nad_list.h
# End Source File
# Begin Source File

SOURCE=.\org_proj4_Projections.h
# End Source File
# Begin Source File

SOURCE=.\pj_list.h
# End Source File
# Begin Source File

SOURCE=.\proj_api.h
# End Source File
# Begin Source File

SOURCE=.\proj_config.h
# End Source File
# Begin Source File

SOURCE=.\projects.h
# End Source File
# End Group
# End Target
# End Project
