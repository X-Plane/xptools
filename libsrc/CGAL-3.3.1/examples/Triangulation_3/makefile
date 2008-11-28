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
                adding_handles$(EXE_EXT) \
                color$(EXE_EXT) \
                find_conflicts$(EXE_EXT) \
                hierarchy$(EXE_EXT) \
                linking_2d_and_3d$(EXE_EXT) \
                regular$(EXE_EXT) \
                simple$(EXE_EXT) \
                simplex$(EXE_EXT) \
                tds$(EXE_EXT) 

adding_handles$(EXE_EXT): adding_handles$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)adding_handles adding_handles$(OBJ_EXT) $(LDFLAGS)

color$(EXE_EXT): color$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)color color$(OBJ_EXT) $(LDFLAGS)

find_conflicts$(EXE_EXT): find_conflicts$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)find_conflicts find_conflicts$(OBJ_EXT) $(LDFLAGS)

hierarchy$(EXE_EXT): hierarchy$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)hierarchy hierarchy$(OBJ_EXT) $(LDFLAGS)

linking_2d_and_3d$(EXE_EXT): linking_2d_and_3d$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)linking_2d_and_3d linking_2d_and_3d$(OBJ_EXT) $(LDFLAGS)

regular$(EXE_EXT): regular$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)regular regular$(OBJ_EXT) $(LDFLAGS)

simple$(EXE_EXT): simple$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)simple simple$(OBJ_EXT) $(LDFLAGS)

simplex$(EXE_EXT): simplex$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)simplex simplex$(OBJ_EXT) $(LDFLAGS)

tds$(EXE_EXT): tds$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)tds tds$(OBJ_EXT) $(LDFLAGS)

clean: \
                   adding_handles.clean \
                   color.clean \
                   find_conflicts.clean \
                   hierarchy.clean \
                   linking_2d_and_3d.clean \
                   regular.clean \
                   simple.clean \
                   simplex.clean \
                   tds.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

