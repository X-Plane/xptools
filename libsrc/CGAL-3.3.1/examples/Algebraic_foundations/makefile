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
                algebraic_structure_dispatch$(EXE_EXT) \
                fraction_traits$(EXE_EXT) \
                implicit_interoperable_dispatch$(EXE_EXT) \
                integralize$(EXE_EXT) \
                interoperable$(EXE_EXT) 

algebraic_structure_dispatch$(EXE_EXT): algebraic_structure_dispatch$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)algebraic_structure_dispatch algebraic_structure_dispatch$(OBJ_EXT) $(LDFLAGS)

fraction_traits$(EXE_EXT): fraction_traits$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)fraction_traits fraction_traits$(OBJ_EXT) $(LDFLAGS)

implicit_interoperable_dispatch$(EXE_EXT): implicit_interoperable_dispatch$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)implicit_interoperable_dispatch implicit_interoperable_dispatch$(OBJ_EXT) $(LDFLAGS)

integralize$(EXE_EXT): integralize$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)integralize integralize$(OBJ_EXT) $(LDFLAGS)

interoperable$(EXE_EXT): interoperable$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)interoperable interoperable$(OBJ_EXT) $(LDFLAGS)

clean: \
                   algebraic_structure_dispatch.clean \
                   fraction_traits.clean \
                   implicit_interoperable_dispatch.clean \
                   integralize.clean \
                   interoperable.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

