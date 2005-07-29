 #
 # The 3D Studio File Format Library
 # Copyright (C) 1996-2000 by J.E. Hoffmann <je-h@gmx.net>
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
 # $Id: Makefile.am,v 1.14 2002/01/14 12:27:45 jeh Exp $
 #

INCLUDES = \
  -I$(top_srcdir) \
  @GLUT_CFLAGS@ \
  @DMALLOC_CFLAGS@

if GLUT_CHECK
PLAYER = player
else
PLAYER = 
endif

noinst_PROGRAMS = \
  3ds2rib \
  $(PLAYER)

LDADD = \
  $(top_builddir)/lib3ds/lib3ds.a \
  @DMALLOC_LIBS@ -lm

player_SOURCES = player.c
player_LDADD = \
  $(top_builddir)/lib3ds/lib3ds.a \
  @GLUT_LIBS@ \
  @DMALLOC_LIBS@ -lm



