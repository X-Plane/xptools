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
                minimum_enclosing_parallelogram_2$(EXE_EXT) \
                minimum_enclosing_rectangle_2$(EXE_EXT) \
                minimum_enclosing_strip_2$(EXE_EXT) 

minimum_enclosing_parallelogram_2$(EXE_EXT): minimum_enclosing_parallelogram_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)minimum_enclosing_parallelogram_2 minimum_enclosing_parallelogram_2$(OBJ_EXT) $(LDFLAGS)

minimum_enclosing_rectangle_2$(EXE_EXT): minimum_enclosing_rectangle_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)minimum_enclosing_rectangle_2 minimum_enclosing_rectangle_2$(OBJ_EXT) $(LDFLAGS)

minimum_enclosing_strip_2$(EXE_EXT): minimum_enclosing_strip_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)minimum_enclosing_strip_2 minimum_enclosing_strip_2$(OBJ_EXT) $(LDFLAGS)

clean: \
                   minimum_enclosing_parallelogram_2.clean \
                   minimum_enclosing_rectangle_2.clean \
                   minimum_enclosing_strip_2.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

