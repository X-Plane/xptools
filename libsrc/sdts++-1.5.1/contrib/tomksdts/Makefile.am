## Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.2 2001/05/18 22:23:05 mcoletti Exp $
#

bin_PROGRAMS = tomksdts
tomksdts_SOURCES = tomksdts.cpp convert.h convert.cpp

if MAINTAINER_MODE
tomksdts_LDADD = -L${libdir} -lsdts++ ${LEXLIB} -lsysutils
else
tomksdts_LDADD = -L${libdir} ${libdir}/libsdts++.la ${LEXLIB} -lsysutils
endif

man_MANS = tomksdts.1

EXTRA_DIST = $(man_MANS) 

INCLUDES = -I${top_srcdir}

