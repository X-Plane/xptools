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
                ellipsoid$(EXE_EXT) \
                ellipsoid_for_maple$(EXE_EXT) 

ellipsoid$(EXE_EXT): ellipsoid$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ellipsoid ellipsoid$(OBJ_EXT) $(LDFLAGS)

ellipsoid_for_maple$(EXE_EXT): ellipsoid_for_maple$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ellipsoid_for_maple ellipsoid_for_maple$(OBJ_EXT) $(LDFLAGS)

clean: \
                   ellipsoid.clean \
                   ellipsoid_for_maple.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

