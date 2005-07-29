## Process this file with automake to generate Makefile.in.

DXF2VRMLDIR = dxf2vrml
DXFSPHEREDIR = dxfsphere

if BUILD_WITH_MSVC
EXAMPLEPROGDIRS =
else
EXAMPLEPROGDIRS = $(DXF2VRMLDIR) $(DXFSPHEREDIR)
endif

SUBDIRS = src $(EXAMPLEPROGDIRS)

cvs-update:
	@cd $(top_srcdir) && cvs update -dP

