## Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.11 2003/02/13 23:37:45 mcoletti Exp $
#

bin_PROGRAMS = mksdts
mksdts_SOURCES = parser.yy lexer.ll

mksdts_LDADD = -L${libdir} -lsdts++ ${LEXLIB} -lsysutils


man_MANS = mksdts.1

EXTRA_DIST = $(man_MANS) parser.h

INCLUDES = -I${top_srcdir}

# yes, we'd like that header file, please

AM_YFLAGS = -d
