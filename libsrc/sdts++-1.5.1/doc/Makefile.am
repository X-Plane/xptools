## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.3 1999/07/20 18:02:59 mcoletti Exp $

info_TEXINFOS = sdts++.texi

EXTRA_DIST = SDTS++FAQ sdts.ood README sdts++.html

sdts++.html : sdts++.texi
	texi2html  -expandinfo -menu -monolithic -number -verbose $<
