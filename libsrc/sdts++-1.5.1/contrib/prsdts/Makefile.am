## Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.6 2001/05/02 19:45:38 mcoletti Exp $
#

INCLUDES = -I$(top_srcdir)

bin_PROGRAMS = prsdts

prsdts_SOURCES = prsdts.cpp

if MAINTAINER_MODE
prsdts_LDADD = $(libdir)/libsdts++.a -L${libdir} -lsysutils
else
prsdts_LDADD = $(libdir)/libsdts++.la -L${libdir} -lsysutils
endif
