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
                comparison$(EXE_EXT) \
                complex_construction$(EXE_EXT) \
                construction$(EXE_EXT) \
                exploration_SM$(EXE_EXT) \
                extended_kernel$(EXE_EXT) \
                handling_double_coordinates$(EXE_EXT) \
                interface_polyhedron$(EXE_EXT) \
                nefIO$(EXE_EXT) \
                offIO$(EXE_EXT) \
                point_location$(EXE_EXT) \
                point_set_operations$(EXE_EXT) \
                polygon_construction$(EXE_EXT) \
                polyline_construction$(EXE_EXT) \
                set_operations$(EXE_EXT) \
                shell_exploration$(EXE_EXT) \
                simple$(EXE_EXT) \
                topological_operations$(EXE_EXT) \
                transformation$(EXE_EXT) 

comparison$(EXE_EXT): comparison$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)comparison comparison$(OBJ_EXT) $(LDFLAGS)

complex_construction$(EXE_EXT): complex_construction$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)complex_construction complex_construction$(OBJ_EXT) $(LDFLAGS)

construction$(EXE_EXT): construction$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)construction construction$(OBJ_EXT) $(LDFLAGS)

exploration_SM$(EXE_EXT): exploration_SM$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)exploration_SM exploration_SM$(OBJ_EXT) $(LDFLAGS)

extended_kernel$(EXE_EXT): extended_kernel$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)extended_kernel extended_kernel$(OBJ_EXT) $(LDFLAGS)

handling_double_coordinates$(EXE_EXT): handling_double_coordinates$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)handling_double_coordinates handling_double_coordinates$(OBJ_EXT) $(LDFLAGS)

interface_polyhedron$(EXE_EXT): interface_polyhedron$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)interface_polyhedron interface_polyhedron$(OBJ_EXT) $(LDFLAGS)

nefIO$(EXE_EXT): nefIO$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)nefIO nefIO$(OBJ_EXT) $(LDFLAGS)

offIO$(EXE_EXT): offIO$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)offIO offIO$(OBJ_EXT) $(LDFLAGS)

point_location$(EXE_EXT): point_location$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)point_location point_location$(OBJ_EXT) $(LDFLAGS)

point_set_operations$(EXE_EXT): point_set_operations$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)point_set_operations point_set_operations$(OBJ_EXT) $(LDFLAGS)

polygon_construction$(EXE_EXT): polygon_construction$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)polygon_construction polygon_construction$(OBJ_EXT) $(LDFLAGS)

polyline_construction$(EXE_EXT): polyline_construction$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)polyline_construction polyline_construction$(OBJ_EXT) $(LDFLAGS)

set_operations$(EXE_EXT): set_operations$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)set_operations set_operations$(OBJ_EXT) $(LDFLAGS)

shell_exploration$(EXE_EXT): shell_exploration$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)shell_exploration shell_exploration$(OBJ_EXT) $(LDFLAGS)

simple$(EXE_EXT): simple$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)simple simple$(OBJ_EXT) $(LDFLAGS)

topological_operations$(EXE_EXT): topological_operations$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)topological_operations topological_operations$(OBJ_EXT) $(LDFLAGS)

transformation$(EXE_EXT): transformation$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)transformation transformation$(OBJ_EXT) $(LDFLAGS)

clean: \
                   comparison.clean \
                   complex_construction.clean \
                   construction.clean \
                   exploration_SM.clean \
                   extended_kernel.clean \
                   handling_double_coordinates.clean \
                   interface_polyhedron.clean \
                   nefIO.clean \
                   offIO.clean \
                   point_location.clean \
                   point_set_operations.clean \
                   polygon_construction.clean \
                   polyline_construction.clean \
                   set_operations.clean \
                   shell_exploration.clean \
                   simple.clean \
                   topological_operations.clean \
                   transformation.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

