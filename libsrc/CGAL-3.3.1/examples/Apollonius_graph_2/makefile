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
                ag2_exact_traits$(EXE_EXT) \
                ag2_exact_traits_sqrt$(EXE_EXT) \
                ag2_filtered_traits_no_hidden$(EXE_EXT) \
                ag2_hierarchy$(EXE_EXT) 

ag2_exact_traits$(EXE_EXT): ag2_exact_traits$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ag2_exact_traits ag2_exact_traits$(OBJ_EXT) $(LDFLAGS)

ag2_exact_traits_sqrt$(EXE_EXT): ag2_exact_traits_sqrt$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ag2_exact_traits_sqrt ag2_exact_traits_sqrt$(OBJ_EXT) $(LDFLAGS)

ag2_filtered_traits_no_hidden$(EXE_EXT): ag2_filtered_traits_no_hidden$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ag2_filtered_traits_no_hidden ag2_filtered_traits_no_hidden$(OBJ_EXT) $(LDFLAGS)

ag2_hierarchy$(EXE_EXT): ag2_hierarchy$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ag2_hierarchy ag2_hierarchy$(OBJ_EXT) $(LDFLAGS)

clean: \
                   ag2_exact_traits.clean \
                   ag2_exact_traits_sqrt.clean \
                   ag2_filtered_traits_no_hidden.clean \
                   ag2_hierarchy.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

