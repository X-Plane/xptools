# Created by the script cgal_create_makefile
# This is the makefile for compiling a CGAL application.

#---------------------------------------------------------------------#
#                    include platform specific settings
#---------------------------------------------------------------------#
# Choose the right include file from the <cgalroot>/make directory.

# CGAL_MAKEFILE = ENTER_YOUR_INCLUDE_MAKEFILE_HERE
include $(CGAL_MAKEFILE)

#---------------------------------------------------------------------#
#                    compiler flags
#---------------------------------------------------------------------#

CXXFLAGS = \
           -I../../include \
           $(CGAL_CXXFLAGS) \
           $(LONG_NAME_PROBLEM_CXXFLAGS)

#---------------------------------------------------------------------#
#                    linker flags
#---------------------------------------------------------------------#

LIBPATH = \
          $(CGAL_LIBPATH)

LDFLAGS = \
          $(LONG_NAME_PROBLEM_LDFLAGS) \
          $(CGAL_LDFLAGS)

#---------------------------------------------------------------------#
#                    target entries
#---------------------------------------------------------------------#

all:            \
                bezier_traits_adapter$(EXE_EXT) \
                circle_segment$(EXE_EXT) \
                conic_traits_adapter$(EXE_EXT) \
                connect_polygon$(EXE_EXT) \
                do_intersect$(EXE_EXT) \
                dxf_union$(EXE_EXT) \
                sequence$(EXE_EXT) \
                set_union$(EXE_EXT) \
                simple_join_intersect$(EXE_EXT) \
                symmetric_difference$(EXE_EXT) 

bezier_traits_adapter$(EXE_EXT): bezier_traits_adapter$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)bezier_traits_adapter bezier_traits_adapter$(OBJ_EXT) $(LDFLAGS)

circle_segment$(EXE_EXT): circle_segment$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)circle_segment circle_segment$(OBJ_EXT) $(LDFLAGS)

conic_traits_adapter$(EXE_EXT): conic_traits_adapter$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)conic_traits_adapter conic_traits_adapter$(OBJ_EXT) $(LDFLAGS)

connect_polygon$(EXE_EXT): connect_polygon$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)connect_polygon connect_polygon$(OBJ_EXT) $(LDFLAGS)

do_intersect$(EXE_EXT): do_intersect$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)do_intersect do_intersect$(OBJ_EXT) $(LDFLAGS)

dxf_union$(EXE_EXT): dxf_union$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)dxf_union dxf_union$(OBJ_EXT) $(LDFLAGS)

sequence$(EXE_EXT): sequence$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sequence sequence$(OBJ_EXT) $(LDFLAGS)

set_union$(EXE_EXT): set_union$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)set_union set_union$(OBJ_EXT) $(LDFLAGS)

simple_join_intersect$(EXE_EXT): simple_join_intersect$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)simple_join_intersect simple_join_intersect$(OBJ_EXT) $(LDFLAGS)

symmetric_difference$(EXE_EXT): symmetric_difference$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)symmetric_difference symmetric_difference$(OBJ_EXT) $(LDFLAGS)

clean: \
                   bezier_traits_adapter.clean \
                   circle_segment.clean \
                   conic_traits_adapter.clean \
                   connect_polygon.clean \
                   do_intersect.clean \
                   dxf_union.clean \
                   sequence.clean \
                   set_union.clean \
                   simple_join_intersect.clean \
                   symmetric_difference.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

