## Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.1 2001/07/02 13:49:00 mcoletti Exp $
#

INCLUDES = -I$(top_srcdir)

bin_PROGRAMS = sdtsdem2xyz

sdtsdem2xyz_SOURCES = sdtsdem2xyz.cpp

if MAINTAINER_MODE
sdtsdem2xyz_LDADD = $(libdir)/libsdts++.a -L${libdir} -lsysutils
else
sdtsdem2xyz_LDADD = $(libdir)/libsdts++.la -L${libdir} -lsysutils
endif
