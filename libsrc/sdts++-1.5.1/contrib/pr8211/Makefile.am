## Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.7 2001/05/07 19:59:18 mcoletti Exp $
#

INCLUDES = -I$(top_srcdir)

bin_PROGRAMS = pr8211

pr8211_SOURCES = pr8211.cpp

if MAINTAINER_MODE
pr8211_LDADD = $(prefix)/lib/libsdts++.a -L${libdir} -lsysutils
else
pr8211_LDADD = $(prefix)/lib/libsdts++.la -L${libdir} -lsysutils
endif

