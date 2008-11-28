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
                approx_convex_partition_2$(EXE_EXT) \
                greene_approx_convex_partition_2$(EXE_EXT) \
                optimal_convex_partition_2$(EXE_EXT) \
                y_monotone_partition_2$(EXE_EXT) 

approx_convex_partition_2$(EXE_EXT): approx_convex_partition_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)approx_convex_partition_2 approx_convex_partition_2$(OBJ_EXT) $(LDFLAGS)

greene_approx_convex_partition_2$(EXE_EXT): greene_approx_convex_partition_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)greene_approx_convex_partition_2 greene_approx_convex_partition_2$(OBJ_EXT) $(LDFLAGS)

optimal_convex_partition_2$(EXE_EXT): optimal_convex_partition_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)optimal_convex_partition_2 optimal_convex_partition_2$(OBJ_EXT) $(LDFLAGS)

y_monotone_partition_2$(EXE_EXT): y_monotone_partition_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)y_monotone_partition_2 y_monotone_partition_2$(OBJ_EXT) $(LDFLAGS)

clean: \
                   approx_convex_partition_2.clean \
                   greene_approx_convex_partition_2.clean \
                   optimal_convex_partition_2.clean \
                   y_monotone_partition_2.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

