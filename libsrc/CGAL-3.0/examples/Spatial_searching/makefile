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
                Circular_query$(EXE_EXT) \
                Distance_browsing$(EXE_EXT) \
                Fuzzy_range_query$(EXE_EXT) \
                General_neighbor_searching$(EXE_EXT) \
                Iso_rectangle_2_query$(EXE_EXT) \
                Nearest_neighbor_searching$(EXE_EXT) \
                Searching_with_box_query$(EXE_EXT) \
                Searching_with_circular_query$(EXE_EXT) \
                User_defined_point_and_distance$(EXE_EXT) \
                Using_fair_splitting_rule$(EXE_EXT) \
                Weighted_Minkowski_distance$(EXE_EXT) 

Circular_query$(EXE_EXT): Circular_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Circular_query Circular_query$(OBJ_EXT) $(LDFLAGS)

Distance_browsing$(EXE_EXT): Distance_browsing$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Distance_browsing Distance_browsing$(OBJ_EXT) $(LDFLAGS)

Fuzzy_range_query$(EXE_EXT): Fuzzy_range_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Fuzzy_range_query Fuzzy_range_query$(OBJ_EXT) $(LDFLAGS)

General_neighbor_searching$(EXE_EXT): General_neighbor_searching$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)General_neighbor_searching General_neighbor_searching$(OBJ_EXT) $(LDFLAGS)

Iso_rectangle_2_query$(EXE_EXT): Iso_rectangle_2_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Iso_rectangle_2_query Iso_rectangle_2_query$(OBJ_EXT) $(LDFLAGS)

Nearest_neighbor_searching$(EXE_EXT): Nearest_neighbor_searching$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Nearest_neighbor_searching Nearest_neighbor_searching$(OBJ_EXT) $(LDFLAGS)

Searching_with_box_query$(EXE_EXT): Searching_with_box_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Searching_with_box_query Searching_with_box_query$(OBJ_EXT) $(LDFLAGS)

Searching_with_circular_query$(EXE_EXT): Searching_with_circular_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Searching_with_circular_query Searching_with_circular_query$(OBJ_EXT) $(LDFLAGS)

User_defined_point_and_distance$(EXE_EXT): User_defined_point_and_distance$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)User_defined_point_and_distance User_defined_point_and_distance$(OBJ_EXT) $(LDFLAGS)

Using_fair_splitting_rule$(EXE_EXT): Using_fair_splitting_rule$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Using_fair_splitting_rule Using_fair_splitting_rule$(OBJ_EXT) $(LDFLAGS)

Weighted_Minkowski_distance$(EXE_EXT): Weighted_Minkowski_distance$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Weighted_Minkowski_distance Weighted_Minkowski_distance$(OBJ_EXT) $(LDFLAGS)

clean: \
                   Circular_query.clean \
                   Distance_browsing.clean \
                   Fuzzy_range_query.clean \
                   General_neighbor_searching.clean \
                   Iso_rectangle_2_query.clean \
                   Nearest_neighbor_searching.clean \
                   Searching_with_box_query.clean \
                   Searching_with_circular_query.clean \
                   User_defined_point_and_distance.clean \
                   Using_fair_splitting_rule.clean \
                   Weighted_Minkowski_distance.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

