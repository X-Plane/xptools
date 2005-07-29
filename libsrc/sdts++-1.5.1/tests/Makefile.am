## Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.38 2003/06/10 20:57:24 mcoletti Exp $
#

INCLUDES = -I$(top_srcdir) -I$(includedir) -I$(BOOST_HEADER)

AM_CXXFLAGS = -ggdb3

SCRIPT_TESTS = sh_Comp_t sh_8211_t sh_ReadWrite_t sh_Writer_t sh_Accessor_t sh_Cell_t sh_Line_t sh_Directory_t

BINARY_TESTS = container_t			\
	sc_Field_t				\
	sc_MultiTypeValue_t			\
	sc_Subfield_t				\
	sio_8211Converter_t			\
	sio_ConverterFactory_t			\
	sio_8211DDRField_t			\
	sio_8211DDRLeader_t			\
	sio_8211DDR_t				\
	sio_8211DRLeader_t			\
	sio_8211DR_t				\
	sio_8211DirEntry_t			\
	sio_8211FieldFormat_t			\
	sio_8211Field_t				\
	sio_8211SubfieldFormat_t		\
	sio_Buffer_t				\
	sio_Error_t                             \
        sb_catd_t				\
        sb_cats_t				\
        sb_ForeignID_t				\
        sb_Spatial_t				\
        sb_utils_t				\
	sl_Point_t				\
	sl_String_t				\
	sl_Chain_t				\
	sl_Ring_t				\
	sl_Polygon_t				\
	sl_Node_t

TESTS = $(BINARY_TESTS) $(SCRIPT_TESTS)


# make sure the test scripts and test data are including in distributions

EXTRA_DIST = $(SCRIPT_TESTS) abcdiden.ddf abcdcatd.ddf Jamfile



noinst_PROGRAMS = $(BINARY_TESTS) 		\
		  sio_8211_t 			\
		  sio_ReadWrite_t 		\
		  sio_Reader_t 			\
		  sio_Writer_t 			\
		  sb_line_t			\
		  sb_comp_t			\
		  sb_Directory_t		\
		  sb_dddf_t			\
		  sb_ddom_t			\
		  sb_ddsh_t			\
	  	  sb_accessor_t			\
		  sb_cell_t			\
	 	  sb_spdm_t			\
		  sb_ring_t			\
		  sb_poly_t			\
                  sb_pnts_t			\
		  sb_xref_t			\
		  sb_iden_t			\
		  sb_iref_t

# if we're in maintainer mode, then we're not running libtool, so just
# link against the plain ole .a

LIBRARY = $(top_builddir)/sdts++/libsdts++.la $(top_builddir)/sysutils/libsysutils.a


container_t_LDADD              = $(LIBRARY)
container_t_SOURCES            = container_t.cpp

sc_Field_t_LDADD               = $(LIBRARY)
sc_Field_t_SOURCES             = sc_Field_t.cpp

sc_MultiTypeValue_t_LDADD      = $(LIBRARY)
sc_MultiTypeValue_t_SOURCES    = sc_MultiTypeValue_t.cpp

sc_Subfield_t_LDADD            = $(LIBRARY)
sc_Subfield_t_SOURCES          = sc_Subfield_t.cpp

sio_8211Converter_t_LDADD      = $(LIBRARY)
sio_8211Converter_t_SOURCES    = sio_8211Converter_t.cpp

sio_ConverterFactory_t_LDADD      = $(LIBRARY)
sio_ConverterFactory_t_SOURCES    = sio_ConverterFactory_t.cpp

sio_8211DDRField_t_LDADD       = $(LIBRARY)
sio_8211DDRField_t_SOURCES     = sio_8211DDRField_t.cpp

sio_8211DDRLeader_t_LDADD      = $(LIBRARY)
sio_8211DDRLeader_t_SOURCES    = sio_8211DDRLeader_t.cpp

sio_8211DDR_t_LDADD            = $(LIBRARY)
sio_8211DDR_t_SOURCES          = sio_8211DDR_t.cpp

sio_8211DRLeader_t_LDADD       = $(LIBRARY)
sio_8211DRLeader_t_SOURCES     = sio_8211DRLeader_t.cpp

sio_8211DR_t_LDADD             = $(LIBRARY)
sio_8211DR_t_SOURCES           = sio_8211DR_t.cpp

sio_8211DirEntry_t_LDADD       = $(LIBRARY)
sio_8211DirEntry_t_SOURCES     = sio_8211DirEntry_t.cpp

sio_8211FieldFormat_t_LDADD    = $(LIBRARY)
sio_8211FieldFormat_t_SOURCES  = sio_8211FieldFormat_t.cpp

sio_8211Field_t_LDADD          = $(LIBRARY)
sio_8211Field_t_SOURCES        = sio_8211Field_t.cpp

sio_8211SubfieldFormat_t_LDADD = $(LIBRARY)
sio_8211SubfieldFormat_t_SOURCES = sio_8211SubfieldFormat_t.cpp

sio_8211_t_LDADD               = $(LIBRARY)
sio_8211_t_SOURCES             = sio_8211_t.cpp

sio_Buffer_t_LDADD             = $(LIBRARY)
sio_Buffer_t_SOURCES           = sio_Buffer_t.cpp

sio_Error_t_LDADD              = $(LIBRARY)
sio_Error_t_SOURCES            = sio_Error_t.cpp

sio_ReadWrite_t_LDADD          = $(LIBRARY)
sio_ReadWrite_t_SOURCES        = sio_ReadWrite_t.cpp

sio_Reader_t_LDADD             = $(LIBRARY)
sio_Reader_t_SOURCES           = sio_Reader_t.cpp

sio_Writer_t_LDADD             = $(LIBRARY)
sio_Writer_t_SOURCES           = sio_Writer_t.cpp

sb_cell_t_LDADD                = $(LIBRARY)
sb_cell_t_SOURCES              = sb_cell_t.cpp

sb_ForeignID_t_LDADD           = $(LIBRARY)
sb_ForeignID_t_SOURCES         = sb_ForeignID_t.cpp

sb_line_t_LDADD                = $(LIBRARY)
sb_line_t_SOURCES              = sb_line_t.cpp

sb_spdm_t_LDADD                = $(LIBRARY)
sb_spdm_t_SOURCES              = sb_spdm_t.cpp

sb_ring_t_LDADD                = $(LIBRARY)
sb_ring_t_SOURCES              = sb_ring_t.cpp

sb_comp_t_LDADD                = $(LIBRARY)
sb_comp_t_SOURCES              = sb_comp_t.cpp

sb_poly_t_LDADD                = $(LIBRARY)
sb_poly_t_SOURCES              = sb_poly_t.cpp

sb_Spatial_t_LDADD             = $(LIBRARY)
sb_Spatial_t_SOURCES           = sb_Spatial_t.cpp

sb_utils_t_LDADD               = $(LIBRARY)
sb_utils_t_SOURCES             = sb_utils_t.cpp

sb_catd_t_LDADD                = $(LIBRARY)
sb_catd_t_SOURCES              = sb_catd_t.cpp

sb_Directory_t_LDADD           = $(LIBRARY)
sb_Directory_t_SOURCES         = sb_Directory_t.cpp

sb_cats_t_LDADD                = $(LIBRARY)
sb_cats_t_SOURCES              = sb_cats_t.cpp

sb_accessor_t_LDADD            = $(LIBRARY) -L$(libdir) -lboost_filesystem
sb_accessor_t_SOURCES          = sb_accessor_t.cpp

sb_pnts_t_LDADD       	       = $(LIBRARY)
sb_pnts_t_SOURCES              = sb_pnts_t.cpp

sb_xref_t_LDADD       	       = $(LIBRARY)
sb_xref_t_SOURCES              = sb_xref_t.cpp

sb_iden_t_LDADD       	       = $(LIBRARY)
sb_iden_t_SOURCES              = sb_iden_t.cpp

sb_iref_t_LDADD       	       = $(LIBRARY)
sb_iref_t_SOURCES              = sb_iref_t.cpp

sl_Point_t_LDADD               = $(LIBRARY)
sl_Point_t_SOURCES             = sl_Point_t.cpp

sl_Node_t_LDADD                = $(LIBRARY)
sl_Node_t_SOURCES              = sl_Node_t.cpp

sl_String_t_LDADD              = $(LIBRARY)
sl_String_t_SOURCES            = sl_String_t.cpp

sl_Chain_t_LDADD               = $(LIBRARY)
sl_Chain_t_SOURCES             = sl_Chain_t.cpp

sl_Ring_t_LDADD                = $(LIBRARY)
sl_Ring_t_SOURCES              = sl_Ring_t.cpp

sl_Polygon_t_LDADD             = $(LIBRARY)
sl_Polygon_t_SOURCES           = sl_Polygon_t.cpp


## XXX need to add these in

#sb_arc_t_LDADD                 = $(LIBRARY)
#sb_arc_t_SOURCES            = sb_arc_t.cpp

#sb_iden_t_LDADD                = $(LIBRARY)
#sb_iden_t_SOURCES            = sb_iden_t.cpp

#sb_iref_t_LDADD                = $(LIBRARY)
#sb_iref_t_SOURCES            = sb_iref_t.cpp

sb_dddf_t_LDADD                = $(LIBRARY)
sb_dddf_t_SOURCES              = sb_dddf_t.cpp

sb_ddom_t_LDADD                = $(LIBRARY)
sb_ddom_t_SOURCES              = sb_ddom_t.cpp

sb_ddsh_t_LDADD                = $(LIBRARY)
sb_ddsh_t_SOURCES              = sb_ddsh_t.cpp

#sb_xref_t_LDADD                = $(LIBRARY)
#sb_xref_t_SOURCES            = sb_xref_t.cpp



