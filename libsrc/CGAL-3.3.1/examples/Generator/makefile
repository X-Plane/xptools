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
                random_convex_set$(EXE_EXT) \
                random_degenerate_point_set$(EXE_EXT) \
                random_grid$(EXE_EXT) \
                random_polygon$(EXE_EXT) \
                random_polygon2$(EXE_EXT) \
                random_segments1$(EXE_EXT) \
                random_segments2$(EXE_EXT) 

random_convex_set$(EXE_EXT): random_convex_set$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)random_convex_set random_convex_set$(OBJ_EXT) $(LDFLAGS)

random_degenerate_point_set$(EXE_EXT): random_degenerate_point_set$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)random_degenerate_point_set random_degenerate_point_set$(OBJ_EXT) $(LDFLAGS)

random_grid$(EXE_EXT): random_grid$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)random_grid random_grid$(OBJ_EXT) $(LDFLAGS)

random_polygon$(EXE_EXT): random_polygon$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)random_polygon random_polygon$(OBJ_EXT) $(LDFLAGS)

random_polygon2$(EXE_EXT): random_polygon2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)random_polygon2 random_polygon2$(OBJ_EXT) $(LDFLAGS)

random_segments1$(EXE_EXT): random_segments1$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)random_segments1 random_segments1$(OBJ_EXT) $(LDFLAGS)

random_segments2$(EXE_EXT): random_segments2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)random_segments2 random_segments2$(OBJ_EXT) $(LDFLAGS)

clean: \
                   random_convex_set.clean \
                   random_degenerate_point_set.clean \
                   random_grid.clean \
                   random_polygon.clean \
                   random_polygon2.clean \
                   random_segments1.clean \
                   random_segments2.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

