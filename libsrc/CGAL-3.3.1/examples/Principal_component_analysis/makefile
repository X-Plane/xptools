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
                barycenter$(EXE_EXT) \
                bounding_box$(EXE_EXT) \
                centroid$(EXE_EXT) \
                linear_least_squares_fitting$(EXE_EXT) 

barycenter$(EXE_EXT): barycenter$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)barycenter barycenter$(OBJ_EXT) $(LDFLAGS)

bounding_box$(EXE_EXT): bounding_box$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)bounding_box bounding_box$(OBJ_EXT) $(LDFLAGS)

centroid$(EXE_EXT): centroid$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)centroid centroid$(OBJ_EXT) $(LDFLAGS)

linear_least_squares_fitting$(EXE_EXT): linear_least_squares_fitting$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)linear_least_squares_fitting linear_least_squares_fitting$(OBJ_EXT) $(LDFLAGS)

clean: \
                   barycenter.clean \
                   bounding_box.clean \
                   centroid.clean \
                   linear_least_squares_fitting.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

