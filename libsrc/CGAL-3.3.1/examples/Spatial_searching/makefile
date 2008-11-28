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
                circular_query$(EXE_EXT) \
                distance_browsing$(EXE_EXT) \
                fuzzy_range_query$(EXE_EXT) \
                general_neighbor_searching$(EXE_EXT) \
                iso_rectangle_2_query$(EXE_EXT) \
                nearest_neighbor_searching$(EXE_EXT) \
                searching_with_circular_query$(EXE_EXT) \
                user_defined_point_and_distance$(EXE_EXT) \
                using_fair_splitting_rule$(EXE_EXT) \
                weighted_Minkowski_distance$(EXE_EXT) 

circular_query$(EXE_EXT): circular_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)circular_query circular_query$(OBJ_EXT) $(LDFLAGS)

distance_browsing$(EXE_EXT): distance_browsing$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)distance_browsing distance_browsing$(OBJ_EXT) $(LDFLAGS)

fuzzy_range_query$(EXE_EXT): fuzzy_range_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)fuzzy_range_query fuzzy_range_query$(OBJ_EXT) $(LDFLAGS)

general_neighbor_searching$(EXE_EXT): general_neighbor_searching$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)general_neighbor_searching general_neighbor_searching$(OBJ_EXT) $(LDFLAGS)

iso_rectangle_2_query$(EXE_EXT): iso_rectangle_2_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)iso_rectangle_2_query iso_rectangle_2_query$(OBJ_EXT) $(LDFLAGS)

nearest_neighbor_searching$(EXE_EXT): nearest_neighbor_searching$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)nearest_neighbor_searching nearest_neighbor_searching$(OBJ_EXT) $(LDFLAGS)

searching_with_circular_query$(EXE_EXT): searching_with_circular_query$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)searching_with_circular_query searching_with_circular_query$(OBJ_EXT) $(LDFLAGS)

user_defined_point_and_distance$(EXE_EXT): user_defined_point_and_distance$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)user_defined_point_and_distance user_defined_point_and_distance$(OBJ_EXT) $(LDFLAGS)

using_fair_splitting_rule$(EXE_EXT): using_fair_splitting_rule$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)using_fair_splitting_rule using_fair_splitting_rule$(OBJ_EXT) $(LDFLAGS)

weighted_Minkowski_distance$(EXE_EXT): weighted_Minkowski_distance$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)weighted_Minkowski_distance weighted_Minkowski_distance$(OBJ_EXT) $(LDFLAGS)

clean: \
                   circular_query.clean \
                   distance_browsing.clean \
                   fuzzy_range_query.clean \
                   general_neighbor_searching.clean \
                   iso_rectangle_2_query.clean \
                   nearest_neighbor_searching.clean \
                   searching_with_circular_query.clean \
                   user_defined_point_and_distance.clean \
                   using_fair_splitting_rule.clean \
                   weighted_Minkowski_distance.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

