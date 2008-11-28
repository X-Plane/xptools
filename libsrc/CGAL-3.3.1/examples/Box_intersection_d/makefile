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
                box_grid$(EXE_EXT) \
                custom_box_grid$(EXE_EXT) \
                minimal$(EXE_EXT) \
                minimal_self$(EXE_EXT) \
                proximity_custom_box_traits$(EXE_EXT) \
                triangle_self_intersect$(EXE_EXT) \
                triangle_self_intersect_pointers$(EXE_EXT) 

box_grid$(EXE_EXT): box_grid$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)box_grid box_grid$(OBJ_EXT) $(LDFLAGS)

custom_box_grid$(EXE_EXT): custom_box_grid$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)custom_box_grid custom_box_grid$(OBJ_EXT) $(LDFLAGS)

minimal$(EXE_EXT): minimal$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)minimal minimal$(OBJ_EXT) $(LDFLAGS)

minimal_self$(EXE_EXT): minimal_self$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)minimal_self minimal_self$(OBJ_EXT) $(LDFLAGS)

proximity_custom_box_traits$(EXE_EXT): proximity_custom_box_traits$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)proximity_custom_box_traits proximity_custom_box_traits$(OBJ_EXT) $(LDFLAGS)

triangle_self_intersect$(EXE_EXT): triangle_self_intersect$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)triangle_self_intersect triangle_self_intersect$(OBJ_EXT) $(LDFLAGS)

triangle_self_intersect_pointers$(EXE_EXT): triangle_self_intersect_pointers$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)triangle_self_intersect_pointers triangle_self_intersect_pointers$(OBJ_EXT) $(LDFLAGS)

clean: \
                   box_grid.clean \
                   custom_box_grid.clean \
                   minimal.clean \
                   minimal_self.clean \
                   proximity_custom_box_traits.clean \
                   triangle_self_intersect.clean \
                   triangle_self_intersect_pointers.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

