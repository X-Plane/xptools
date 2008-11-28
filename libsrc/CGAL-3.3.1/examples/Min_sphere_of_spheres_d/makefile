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
                benchmark$(EXE_EXT) \
                min_sphere_of_spheres_d_2$(EXE_EXT) \
                min_sphere_of_spheres_d_3$(EXE_EXT) \
                min_sphere_of_spheres_d_d$(EXE_EXT) 

benchmark$(EXE_EXT): benchmark$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)benchmark benchmark$(OBJ_EXT) $(LDFLAGS)

min_sphere_of_spheres_d_2$(EXE_EXT): min_sphere_of_spheres_d_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)min_sphere_of_spheres_d_2 min_sphere_of_spheres_d_2$(OBJ_EXT) $(LDFLAGS)

min_sphere_of_spheres_d_3$(EXE_EXT): min_sphere_of_spheres_d_3$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)min_sphere_of_spheres_d_3 min_sphere_of_spheres_d_3$(OBJ_EXT) $(LDFLAGS)

min_sphere_of_spheres_d_d$(EXE_EXT): min_sphere_of_spheres_d_d$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)min_sphere_of_spheres_d_d min_sphere_of_spheres_d_d$(OBJ_EXT) $(LDFLAGS)

clean: \
                   benchmark.clean \
                   min_sphere_of_spheres_d_2.clean \
                   min_sphere_of_spheres_d_3.clean \
                   min_sphere_of_spheres_d_d.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

