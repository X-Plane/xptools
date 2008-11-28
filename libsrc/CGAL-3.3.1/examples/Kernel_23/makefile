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
                MyKernel$(EXE_EXT) \
                test_delaunay_2$(EXE_EXT) \
                test_polyhedron_prog_incr_builder$(EXE_EXT) \
                test_polyhedron_prog_incr_builder_own_archetype$(EXE_EXT) 

MyKernel$(EXE_EXT): MyKernel$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)MyKernel MyKernel$(OBJ_EXT) $(LDFLAGS)

test_delaunay_2$(EXE_EXT): test_delaunay_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)test_delaunay_2 test_delaunay_2$(OBJ_EXT) $(LDFLAGS)

test_polyhedron_prog_incr_builder$(EXE_EXT): test_polyhedron_prog_incr_builder$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)test_polyhedron_prog_incr_builder test_polyhedron_prog_incr_builder$(OBJ_EXT) $(LDFLAGS)

test_polyhedron_prog_incr_builder_own_archetype$(EXE_EXT): test_polyhedron_prog_incr_builder_own_archetype$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)test_polyhedron_prog_incr_builder_own_archetype test_polyhedron_prog_incr_builder_own_archetype$(OBJ_EXT) $(LDFLAGS)

clean: \
                   MyKernel.clean \
                   test_delaunay_2.clean \
                   test_polyhedron_prog_incr_builder.clean \
                   test_polyhedron_prog_incr_builder_own_archetype.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

