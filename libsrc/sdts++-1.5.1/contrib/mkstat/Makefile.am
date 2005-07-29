## Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.7 2001/03/26 21:16:59 mcoletti Exp $
#

bin_PROGRAMS = mkstat
mkstat_SOURCES = mkstat.cpp emit.cpp emit.h filename.cpp filename.h

if MAINTAINER_MODE
mkstat_LDADD = ${libdir}/libsdts++.a -L${top_srcdir}/sysutils -lsysutils
else
mkstat_LDADD = ${libdir}/libsdts++.la -L${top_srcdir}/sysutils -lsysutils
endif
 
man_MANS = mkstat.1

EXTRA_DIST = $(man_MANS)

INCLUDES = -I${top_srcdir}
