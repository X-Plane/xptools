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
                approx_offset$(EXE_EXT) \
                exact_offset$(EXE_EXT) \
                sum_by_decomposition$(EXE_EXT) \
                sum_triangles$(EXE_EXT) \
                sum_with_holes$(EXE_EXT) 

approx_offset$(EXE_EXT): approx_offset$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)approx_offset approx_offset$(OBJ_EXT) $(LDFLAGS)

exact_offset$(EXE_EXT): exact_offset$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)exact_offset exact_offset$(OBJ_EXT) $(LDFLAGS)

sum_by_decomposition$(EXE_EXT): sum_by_decomposition$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sum_by_decomposition sum_by_decomposition$(OBJ_EXT) $(LDFLAGS)

sum_triangles$(EXE_EXT): sum_triangles$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sum_triangles sum_triangles$(OBJ_EXT) $(LDFLAGS)

sum_with_holes$(EXE_EXT): sum_with_holes$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sum_with_holes sum_with_holes$(OBJ_EXT) $(LDFLAGS)

clean: \
                   approx_offset.clean \
                   exact_offset.clean \
                   sum_by_decomposition.clean \
                   sum_triangles.clean \
                   sum_with_holes.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

