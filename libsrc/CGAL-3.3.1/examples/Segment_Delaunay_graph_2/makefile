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
                sdg-count-sites$(EXE_EXT) \
                sdg-filtered-traits$(EXE_EXT) \
                sdg-info-set$(EXE_EXT) \
                sdg-red-blue-info$(EXE_EXT) \
                sdg-voronoi-edges$(EXE_EXT) 

sdg-count-sites$(EXE_EXT): sdg-count-sites$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sdg-count-sites sdg-count-sites$(OBJ_EXT) $(LDFLAGS)

sdg-filtered-traits$(EXE_EXT): sdg-filtered-traits$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sdg-filtered-traits sdg-filtered-traits$(OBJ_EXT) $(LDFLAGS)

sdg-info-set$(EXE_EXT): sdg-info-set$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sdg-info-set sdg-info-set$(OBJ_EXT) $(LDFLAGS)

sdg-red-blue-info$(EXE_EXT): sdg-red-blue-info$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sdg-red-blue-info sdg-red-blue-info$(OBJ_EXT) $(LDFLAGS)

sdg-voronoi-edges$(EXE_EXT): sdg-voronoi-edges$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sdg-voronoi-edges sdg-voronoi-edges$(OBJ_EXT) $(LDFLAGS)

clean: \
                   sdg-count-sites.clean \
                   sdg-filtered-traits.clean \
                   sdg-info-set.clean \
                   sdg-red-blue-info.clean \
                   sdg-voronoi-edges.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

