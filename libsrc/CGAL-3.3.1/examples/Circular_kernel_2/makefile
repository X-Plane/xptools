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
                Circular_kernel_arrangement$(EXE_EXT) \
                Circular_kernel_arrangement_Line_Circular$(EXE_EXT) \
                Circular_kernel_arrangement_Line_arc$(EXE_EXT) \
                Exact_circular_kernel$(EXE_EXT) \
                arrangement_random_circles$(EXE_EXT) \
                arrangement_random_circles_segments$(EXE_EXT) 

Circular_kernel_arrangement$(EXE_EXT): Circular_kernel_arrangement$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Circular_kernel_arrangement Circular_kernel_arrangement$(OBJ_EXT) $(LDFLAGS)

Circular_kernel_arrangement_Line_Circular$(EXE_EXT): Circular_kernel_arrangement_Line_Circular$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Circular_kernel_arrangement_Line_Circular Circular_kernel_arrangement_Line_Circular$(OBJ_EXT) $(LDFLAGS)

Circular_kernel_arrangement_Line_arc$(EXE_EXT): Circular_kernel_arrangement_Line_arc$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Circular_kernel_arrangement_Line_arc Circular_kernel_arrangement_Line_arc$(OBJ_EXT) $(LDFLAGS)

Exact_circular_kernel$(EXE_EXT): Exact_circular_kernel$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Exact_circular_kernel Exact_circular_kernel$(OBJ_EXT) $(LDFLAGS)

arrangement_random_circles$(EXE_EXT): arrangement_random_circles$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)arrangement_random_circles arrangement_random_circles$(OBJ_EXT) $(LDFLAGS)

arrangement_random_circles_segments$(EXE_EXT): arrangement_random_circles_segments$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)arrangement_random_circles_segments arrangement_random_circles_segments$(OBJ_EXT) $(LDFLAGS)

clean: \
                   Circular_kernel_arrangement.clean \
                   Circular_kernel_arrangement_Line_Circular.clean \
                   Circular_kernel_arrangement_Line_arc.clean \
                   Exact_circular_kernel.clean \
                   arrangement_random_circles.clean \
                   arrangement_random_circles_segments.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

