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
                Delaunay_triangulation_2$(EXE_EXT) \
                Delaunay_triangulation_3$(EXE_EXT) \
                insert_event$(EXE_EXT) \
                regular_triangulation_3$(EXE_EXT) \
                sort$(EXE_EXT) \
                sweepline$(EXE_EXT) 

Delaunay_triangulation_2$(EXE_EXT): Delaunay_triangulation_2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Delaunay_triangulation_2 Delaunay_triangulation_2$(OBJ_EXT) $(LDFLAGS)

Delaunay_triangulation_3$(EXE_EXT): Delaunay_triangulation_3$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Delaunay_triangulation_3 Delaunay_triangulation_3$(OBJ_EXT) $(LDFLAGS)

insert_event$(EXE_EXT): insert_event$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)insert_event insert_event$(OBJ_EXT) $(LDFLAGS)

regular_triangulation_3$(EXE_EXT): regular_triangulation_3$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)regular_triangulation_3 regular_triangulation_3$(OBJ_EXT) $(LDFLAGS)

sort$(EXE_EXT): sort$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sort sort$(OBJ_EXT) $(LDFLAGS)

sweepline$(EXE_EXT): sweepline$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sweepline sweepline$(OBJ_EXT) $(LDFLAGS)

clean: \
                   Delaunay_triangulation_2.clean \
                   Delaunay_triangulation_3.clean \
                   insert_event.clean \
                   regular_triangulation_3.clean \
                   sort.clean \
                   sweepline.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

