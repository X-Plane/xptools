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
                construction$(EXE_EXT) \
                polylines$(EXE_EXT) \
                simple_intersection$(EXE_EXT) 

construction$(EXE_EXT): construction$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)construction construction$(OBJ_EXT) $(LDFLAGS)

polylines$(EXE_EXT): polylines$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)polylines polylines$(OBJ_EXT) $(LDFLAGS)

simple_intersection$(EXE_EXT): simple_intersection$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)simple_intersection simple_intersection$(OBJ_EXT) $(LDFLAGS)

clean: \
                   construction.clean \
                   polylines.clean \
                   simple_intersection.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

