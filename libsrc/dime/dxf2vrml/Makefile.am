## Process this file with automake to generate Makefile.in.

INCLUDES = -I$(top_srcdir)/include

noinst_PROGRAMS = dxf2vrml

dxf2vrml_SOURCES = dxf2vrml.cpp

if BUILD_WITH_MSVC
dxf2vrml_LDADD = $(top_builddir)/src/dime0.lib
else
dxf2vrml_LDADD = $(top_builddir)/src/libdime.la
endif

