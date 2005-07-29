# Created by the script create_makefile
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
           $(CGAL_CXXFLAGS) \
           $(LONG_NAME_PROBLEM_CXXFLAGS) \
           $(DEBUG_OPT)

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
                benchmark$(EXE_EXT) \
                min_sphere_of_spheres_d_example_2$(EXE_EXT) \
                min_sphere_of_spheres_d_example_3$(EXE_EXT) \
                min_sphere_of_spheres_d_example_d$(EXE_EXT) 

benchmark$(EXE_EXT): benchmark$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)benchmark benchmark$(OBJ_EXT) $(LDFLAGS)

min_sphere_of_spheres_d_example_2$(EXE_EXT): min_sphere_of_spheres_d_example_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)min_sphere_of_spheres_d_example_2 min_sphere_of_spheres_d_example_2$(OBJ_EXT) $(LDFLAGS)

min_sphere_of_spheres_d_example_3$(EXE_EXT): min_sphere_of_spheres_d_example_3$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)min_sphere_of_spheres_d_example_3 min_sphere_of_spheres_d_example_3$(OBJ_EXT) $(LDFLAGS)

min_sphere_of_spheres_d_example_d$(EXE_EXT): min_sphere_of_spheres_d_example_d$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)min_sphere_of_spheres_d_example_d min_sphere_of_spheres_d_example_d$(OBJ_EXT) $(LDFLAGS)

clean: \
                   benchmark.clean \
                   min_sphere_of_spheres_d_example_2.clean \
                   min_sphere_of_spheres_d_example_3.clean \
                   min_sphere_of_spheres_d_example_d.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

