## Process this file with automake to generate Makefile.in.

INCLUDES = -I$(top_srcdir)/include

noinst_PROGRAMS = dxfsphere

dxfsphere_SOURCES = dxfsphere.cpp

if BUILD_WITH_MSVC
dxfsphere_LDADD = $(top_builddir)/src/dime0.lib
else
dxfsphere_LDADD = $(top_builddir)/src/libdime.la
endif

