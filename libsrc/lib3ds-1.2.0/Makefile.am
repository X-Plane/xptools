 #
 # The 3D Studio File Format Library
 # Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
 # All rights reserved.
 #
 # This program is  free  software;  you can redistribute it and/or modify it
 # under the terms of the  GNU Lesser General Public License  as published by 
 # the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 # your option) any later version.
 #
 # This  program  is  distributed in  the  hope that it will  be useful,  but
 # WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 # or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 # License for more details.
 #
 # You should  have received  a copy of the GNU Lesser General Public License
 # along with  this program;  if not, write to the  Free Software Foundation,
 # Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 #
 # $Id: Makefile.am,v 1.18 2001/07/18 12:17:49 jeh Exp $
 #

SUBDIRS = msvc lib3ds tools examples

bin_SCRIPTS = lib3ds-config

AUTOMAKE_OPTIONS = 1.4

man_MANS = \
  lib3ds-config.1

EXTRA_DIST = \
  lib3ds.spec \
  lib3ds.m4 \
  lib3ds-config.1 \
  AUTHORS \
  COPYING \
  ChangeLog \
  INSTALL \
  README \
  TODO \
  config.h 

m4datadir = $(datadir)/aclocal
m4data_DATA = lib3ds.m4

.PHONY: release snapshot

release:
	rm -rf .deps */.deps
	$(MAKE) distcheck

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)`date +"%y%m%d"`






