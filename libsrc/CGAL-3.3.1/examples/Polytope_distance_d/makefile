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
                polytope_distance_d$(EXE_EXT) \
                polytope_distance_d_fast_exact$(EXE_EXT) 

polytope_distance_d$(EXE_EXT): polytope_distance_d$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)polytope_distance_d polytope_distance_d$(OBJ_EXT) $(LDFLAGS)

polytope_distance_d_fast_exact$(EXE_EXT): polytope_distance_d_fast_exact$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)polytope_distance_d_fast_exact polytope_distance_d_fast_exact$(OBJ_EXT) $(LDFLAGS)

clean: \
                   polytope_distance_d.clean \
                   polytope_distance_d_fast_exact.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

