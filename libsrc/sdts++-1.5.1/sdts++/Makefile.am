## Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.33 2003/06/10 16:24:47 mcoletti Exp $

SHELL := /bin/sh

AUTOMAKE_OPTIONS := subdir-objects

AM_CPPFLAGS := -I. -I.. -Icontainer -Iio -Ibuilder -Ilogical -I${prefix}/include -I${BOOST_HEADER}


container_sources := container/sc_Subfield.cpp		\
                     container/sc_Field.cpp		\
                     container/sc_Record.cpp		\
                     container/sc_Module.cpp		\
                     container/sc_MultiTypeValue.cpp

io_sources :=	     io/sio_8211Converter.cpp		\
		     io/sio_ConverterFactory.cpp	\
		     io/sio_8211DDR.cpp			\
		     io/sio_8211DDRField.cpp		\
		     io/sio_8211DDRLeader.cpp		\
		     io/sio_8211DR.cpp			\
		     io/sio_8211DRLeader.cpp		\
		     io/sio_8211DirEntry.cpp		\
		     io/sio_8211Directory.cpp		\
		     io/sio_8211Field.cpp		\
		     io/sio_8211FieldArea.cpp		\
		     io/sio_8211FieldFormat.cpp		\
		     io/sio_8211Leader.cpp		\
		     io/sio_8211Record.cpp		\
		     io/sio_8211SubfieldFormat.cpp	\
		     io/sio_8211Utils.cpp		\
		     io/sio_Buffer.cpp			\
		     io/sio_Converter.cpp		\
		     io/sio_Error.cpp			\
		     io/sio_Reader.cpp			\
		     io/sio_Utils.cpp			\
		     io/sio_Writer.cpp			\
		     io/FormatParser.c			\
		     io/FormatLexer.c


builder_sources :=   builder/sb_At.cpp		\
		     builder/sb_Catd.cpp	\
		     builder/sb_Cats.cpp	\
		     builder/sb_Cell.cpp	\
		     builder/sb_Clrx.cpp	\
		     builder/sb_Dddf.cpp	\
		     builder/sb_Ddsh.cpp	\
		     builder/sb_Ddom.cpp	\
		     builder/sb_Dq.cpp		\
		     builder/sb_ForeignID.cpp	\
		     builder/sb_Iden.cpp	\
		     builder/sb_Iref.cpp	\
		     builder/sb_Ldef.cpp	\
		     builder/sb_Line.cpp	\
		     builder/sb_Pnts.cpp	\
		     builder/sb_Module.cpp	\
		     builder/sb_Rsdf.cpp	\
		     builder/sb_Stat.cpp	\
		     builder/sb_Utils.cpp	\
		     builder/sb_Xref.cpp        \
		     builder/sb_Spdm.cpp	\
		     builder/sb_Poly.cpp	\
		     builder/sb_Ring.cpp        \
		     builder/sb_Comp.cpp	\
		     builder/sb_Accessor.cpp

logical_sources :=    	logical/sl_Point.cpp		\
			logical/sl_Node.cpp		\
			logical/sl_String.cpp 		\
			logical/sl_Chain.cpp 		\
			logical/sl_Polygon.cpp


nobase_pkginclude_HEADERS = $(subst cpp,h,  $(container_sources) $(io_sources) $(builder_sources) $(logical_sources) ) builder/sb_Spatial.h logical/sl_Object.h logical/sl_Ring.h


## force use of C++ compiler for C source; this for yacc generated files

CC := $(CXX)



# XXX Ugh.  automake wasn't properly generating these files, so I'm having
# XXX to do them by hand.  Moreover, there's the obvious dependency on 
# XXX GNU bison and flex; mind, since no one should be changing these files
# XXX anyway, this shouldn't be a problem.

io/FormatLexer.c : io/FormatLexer.ll
	flex  -B -Psio_8211_yy $< 
	mv lex.sio_8211_yy.c $@

io/FormatParser.c : io/FormatParser.yy
	bison --defines -p sio_8211_yy $< -o $@



lib_LTLIBRARIES := libsdts++.la

libsdts___la_SOURCES := $(container_sources) $(io_sources) $(builder_sources) $(logical_sources)


EXTRA_DIST = io/FormatParser.yy			\
	     io/FormatLexer.ll 			\
	     io/FormatLexer.c 			\
	     io/FormatParser.c 			\
	     io/FormatParser.h Jamfile



