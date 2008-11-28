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
                adding_a_certificate$(EXE_EXT) \
                defining_a_simulation_traits$(EXE_EXT) \
                listener$(EXE_EXT) \
                pointer_queue$(EXE_EXT) \
                ref_counted$(EXE_EXT) \
                trivial_kds$(EXE_EXT) 

adding_a_certificate$(EXE_EXT): adding_a_certificate$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)adding_a_certificate adding_a_certificate$(OBJ_EXT) $(LDFLAGS)

defining_a_simulation_traits$(EXE_EXT): defining_a_simulation_traits$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)defining_a_simulation_traits defining_a_simulation_traits$(OBJ_EXT) $(LDFLAGS)

listener$(EXE_EXT): listener$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)listener listener$(OBJ_EXT) $(LDFLAGS)

pointer_queue$(EXE_EXT): pointer_queue$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)pointer_queue pointer_queue$(OBJ_EXT) $(LDFLAGS)

ref_counted$(EXE_EXT): ref_counted$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)ref_counted ref_counted$(OBJ_EXT) $(LDFLAGS)

trivial_kds$(EXE_EXT): trivial_kds$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)trivial_kds trivial_kds$(OBJ_EXT) $(LDFLAGS)

clean: \
                   adding_a_certificate.clean \
                   defining_a_simulation_traits.clean \
                   listener.clean \
                   pointer_queue.clean \
                   ref_counted.clean \
                   trivial_kds.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

