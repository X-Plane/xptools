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
                convex_hull_containment$(EXE_EXT) \
                convex_hull_containment2$(EXE_EXT) \
                convex_hull_containment_benchmarks$(EXE_EXT) \
                cycling$(EXE_EXT) \
                first_lp$(EXE_EXT) \
                first_lp_from_iterators$(EXE_EXT) \
                first_lp_from_mps$(EXE_EXT) \
                first_nonnegative_lp$(EXE_EXT) \
                first_nonnegative_lp_from_iterators$(EXE_EXT) \
                first_nonnegative_lp_from_mps$(EXE_EXT) \
                first_nonnegative_qp$(EXE_EXT) \
                first_nonnegative_qp_from_iterators$(EXE_EXT) \
                first_nonnegative_qp_from_mps$(EXE_EXT) \
                first_qp$(EXE_EXT) \
                first_qp_basic_constraints$(EXE_EXT) \
                first_qp_from_iterators$(EXE_EXT) \
                first_qp_from_mps$(EXE_EXT) \
                important_variables$(EXE_EXT) \
                infeasibility_certificate$(EXE_EXT) \
                invert_matrix$(EXE_EXT) \
                optimality_certificate$(EXE_EXT) \
                print_first_lp$(EXE_EXT) \
                print_first_nonnegative_lp$(EXE_EXT) \
                print_first_nonnegative_qp$(EXE_EXT) \
                print_first_qp$(EXE_EXT) \
                unboundedness_certificate$(EXE_EXT) 

convex_hull_containment$(EXE_EXT): convex_hull_containment$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)convex_hull_containment convex_hull_containment$(OBJ_EXT) $(LDFLAGS)

convex_hull_containment2$(EXE_EXT): convex_hull_containment2$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)convex_hull_containment2 convex_hull_containment2$(OBJ_EXT) $(LDFLAGS)

convex_hull_containment_benchmarks$(EXE_EXT): convex_hull_containment_benchmarks$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)convex_hull_containment_benchmarks convex_hull_containment_benchmarks$(OBJ_EXT) $(LDFLAGS)

cycling$(EXE_EXT): cycling$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)cycling cycling$(OBJ_EXT) $(LDFLAGS)

first_lp$(EXE_EXT): first_lp$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_lp first_lp$(OBJ_EXT) $(LDFLAGS)

first_lp_from_iterators$(EXE_EXT): first_lp_from_iterators$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_lp_from_iterators first_lp_from_iterators$(OBJ_EXT) $(LDFLAGS)

first_lp_from_mps$(EXE_EXT): first_lp_from_mps$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_lp_from_mps first_lp_from_mps$(OBJ_EXT) $(LDFLAGS)

first_nonnegative_lp$(EXE_EXT): first_nonnegative_lp$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_nonnegative_lp first_nonnegative_lp$(OBJ_EXT) $(LDFLAGS)

first_nonnegative_lp_from_iterators$(EXE_EXT): first_nonnegative_lp_from_iterators$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_nonnegative_lp_from_iterators first_nonnegative_lp_from_iterators$(OBJ_EXT) $(LDFLAGS)

first_nonnegative_lp_from_mps$(EXE_EXT): first_nonnegative_lp_from_mps$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_nonnegative_lp_from_mps first_nonnegative_lp_from_mps$(OBJ_EXT) $(LDFLAGS)

first_nonnegative_qp$(EXE_EXT): first_nonnegative_qp$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_nonnegative_qp first_nonnegative_qp$(OBJ_EXT) $(LDFLAGS)

first_nonnegative_qp_from_iterators$(EXE_EXT): first_nonnegative_qp_from_iterators$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_nonnegative_qp_from_iterators first_nonnegative_qp_from_iterators$(OBJ_EXT) $(LDFLAGS)

first_nonnegative_qp_from_mps$(EXE_EXT): first_nonnegative_qp_from_mps$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_nonnegative_qp_from_mps first_nonnegative_qp_from_mps$(OBJ_EXT) $(LDFLAGS)

first_qp$(EXE_EXT): first_qp$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_qp first_qp$(OBJ_EXT) $(LDFLAGS)

first_qp_basic_constraints$(EXE_EXT): first_qp_basic_constraints$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_qp_basic_constraints first_qp_basic_constraints$(OBJ_EXT) $(LDFLAGS)

first_qp_from_iterators$(EXE_EXT): first_qp_from_iterators$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_qp_from_iterators first_qp_from_iterators$(OBJ_EXT) $(LDFLAGS)

first_qp_from_mps$(EXE_EXT): first_qp_from_mps$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)first_qp_from_mps first_qp_from_mps$(OBJ_EXT) $(LDFLAGS)

important_variables$(EXE_EXT): important_variables$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)important_variables important_variables$(OBJ_EXT) $(LDFLAGS)

infeasibility_certificate$(EXE_EXT): infeasibility_certificate$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)infeasibility_certificate infeasibility_certificate$(OBJ_EXT) $(LDFLAGS)

invert_matrix$(EXE_EXT): invert_matrix$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)invert_matrix invert_matrix$(OBJ_EXT) $(LDFLAGS)

optimality_certificate$(EXE_EXT): optimality_certificate$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)optimality_certificate optimality_certificate$(OBJ_EXT) $(LDFLAGS)

print_first_lp$(EXE_EXT): print_first_lp$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)print_first_lp print_first_lp$(OBJ_EXT) $(LDFLAGS)

print_first_nonnegative_lp$(EXE_EXT): print_first_nonnegative_lp$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)print_first_nonnegative_lp print_first_nonnegative_lp$(OBJ_EXT) $(LDFLAGS)

print_first_nonnegative_qp$(EXE_EXT): print_first_nonnegative_qp$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)print_first_nonnegative_qp print_first_nonnegative_qp$(OBJ_EXT) $(LDFLAGS)

print_first_qp$(EXE_EXT): print_first_qp$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)print_first_qp print_first_qp$(OBJ_EXT) $(LDFLAGS)

unboundedness_certificate$(EXE_EXT): unboundedness_certificate$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)unboundedness_certificate unboundedness_certificate$(OBJ_EXT) $(LDFLAGS)

clean: \
                   convex_hull_containment.clean \
                   convex_hull_containment2.clean \
                   convex_hull_containment_benchmarks.clean \
                   cycling.clean \
                   first_lp.clean \
                   first_lp_from_iterators.clean \
                   first_lp_from_mps.clean \
                   first_nonnegative_lp.clean \
                   first_nonnegative_lp_from_iterators.clean \
                   first_nonnegative_lp_from_mps.clean \
                   first_nonnegative_qp.clean \
                   first_nonnegative_qp_from_iterators.clean \
                   first_nonnegative_qp_from_mps.clean \
                   first_qp.clean \
                   first_qp_basic_constraints.clean \
                   first_qp_from_iterators.clean \
                   first_qp_from_mps.clean \
                   important_variables.clean \
                   infeasibility_certificate.clean \
                   invert_matrix.clean \
                   optimality_certificate.clean \
                   print_first_lp.clean \
                   print_first_nonnegative_lp.clean \
                   print_first_nonnegative_qp.clean \
                   print_first_qp.clean \
                   unboundedness_certificate.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

