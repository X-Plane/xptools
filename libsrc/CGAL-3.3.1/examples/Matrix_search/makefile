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
                all_furthest_neighbors_2$(EXE_EXT) \
                extremal_polygon_2_area$(EXE_EXT) \
                extremal_polygon_2_perimeter$(EXE_EXT) \
                rectangular_p_center_2$(EXE_EXT) \
                sorted_matrix_search$(EXE_EXT) 

all_furthest_neighbors_2$(EXE_EXT): all_furthest_neighbors_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)all_furthest_neighbors_2 all_furthest_neighbors_2$(OBJ_EXT) $(LDFLAGS)

extremal_polygon_2_area$(EXE_EXT): extremal_polygon_2_area$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)extremal_polygon_2_area extremal_polygon_2_area$(OBJ_EXT) $(LDFLAGS)

extremal_polygon_2_perimeter$(EXE_EXT): extremal_polygon_2_perimeter$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)extremal_polygon_2_perimeter extremal_polygon_2_perimeter$(OBJ_EXT) $(LDFLAGS)

rectangular_p_center_2$(EXE_EXT): rectangular_p_center_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)rectangular_p_center_2 rectangular_p_center_2$(OBJ_EXT) $(LDFLAGS)

sorted_matrix_search$(EXE_EXT): sorted_matrix_search$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sorted_matrix_search sorted_matrix_search$(OBJ_EXT) $(LDFLAGS)

clean: \
                   all_furthest_neighbors_2.clean \
                   extremal_polygon_2_area.clean \
                   extremal_polygon_2_perimeter.clean \
                   rectangular_p_center_2.clean \
                   sorted_matrix_search.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

