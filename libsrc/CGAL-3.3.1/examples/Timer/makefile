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
                Profile_counter$(EXE_EXT) \
                Profile_histogram_counter$(EXE_EXT) 

Profile_counter$(EXE_EXT): Profile_counter$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Profile_counter Profile_counter$(OBJ_EXT) $(LDFLAGS)

Profile_histogram_counter$(EXE_EXT): Profile_histogram_counter$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Profile_histogram_counter Profile_histogram_counter$(OBJ_EXT) $(LDFLAGS)

clean: \
                   Profile_counter.clean \
                   Profile_histogram_counter.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

