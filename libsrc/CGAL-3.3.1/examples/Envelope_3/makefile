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
                ex_envelope_planes$(EXE_EXT) \
                ex_envelope_spheres$(EXE_EXT) \
                ex_envelope_triangles$(EXE_EXT) 

ex_envelope_planes$(EXE_EXT): ex_envelope_planes$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ex_envelope_planes ex_envelope_planes$(OBJ_EXT) $(LDFLAGS)

ex_envelope_spheres$(EXE_EXT): ex_envelope_spheres$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ex_envelope_spheres ex_envelope_spheres$(OBJ_EXT) $(LDFLAGS)

ex_envelope_triangles$(EXE_EXT): ex_envelope_triangles$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ex_envelope_triangles ex_envelope_triangles$(OBJ_EXT) $(LDFLAGS)

clean: \
                   ex_envelope_planes.clean \
                   ex_envelope_spheres.clean \
                   ex_envelope_triangles.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

