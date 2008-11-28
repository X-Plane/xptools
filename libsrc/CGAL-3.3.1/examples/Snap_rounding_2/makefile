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
                snap_rounding$(EXE_EXT) \
                snap_rounding_to_integer$(EXE_EXT) 

snap_rounding$(EXE_EXT): snap_rounding$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)snap_rounding snap_rounding$(OBJ_EXT) $(LDFLAGS)

snap_rounding_to_integer$(EXE_EXT): snap_rounding_to_integer$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)snap_rounding_to_integer snap_rounding_to_integer$(OBJ_EXT) $(LDFLAGS)

clean: \
                   snap_rounding.clean \
                   snap_rounding_to_integer.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

