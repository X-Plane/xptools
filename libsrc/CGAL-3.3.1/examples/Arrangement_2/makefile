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
                Bezier_curves$(EXE_EXT) \
                aggregated_insertion$(EXE_EXT) \
                batched_point_location$(EXE_EXT) \
                bgl_dual_adapter$(EXE_EXT) \
                bgl_primal_adapter$(EXE_EXT) \
                circles$(EXE_EXT) \
                circular_arcs$(EXE_EXT) \
                circular_line_arcs$(EXE_EXT) \
                conic_multiplicities$(EXE_EXT) \
                conics$(EXE_EXT) \
                consolidated_curve_data$(EXE_EXT) \
                curve_history$(EXE_EXT) \
                dcel_extension$(EXE_EXT) \
                dcel_extension_io$(EXE_EXT) \
                dual_lines$(EXE_EXT) \
                dual_with_data$(EXE_EXT) \
                edge_insertion$(EXE_EXT) \
                edge_manipulation$(EXE_EXT) \
                edge_manipulation_curve_history$(EXE_EXT) \
                face_extension$(EXE_EXT) \
                face_extension_overlay$(EXE_EXT) \
                generic_curve_data$(EXE_EXT) \
                global_insertion$(EXE_EXT) \
                global_removal$(EXE_EXT) \
                incremental_insertion$(EXE_EXT) \
                io$(EXE_EXT) \
                io_curve_history$(EXE_EXT) \
                isolated_vertices$(EXE_EXT) \
                observer$(EXE_EXT) \
                overlay$(EXE_EXT) \
                overlay_unbounded$(EXE_EXT) \
                point_location$(EXE_EXT) \
                polylines$(EXE_EXT) \
                predefined_kernel$(EXE_EXT) \
                predefined_kernel_non_intersecting$(EXE_EXT) \
                rational_functions$(EXE_EXT) \
                special_edge_insertion$(EXE_EXT) \
                sweep_line$(EXE_EXT) \
                unbounded_non_intersecting$(EXE_EXT) \
                unbounded_rational_functions$(EXE_EXT) \
                vertical_decomposition$(EXE_EXT) \
                vertical_ray_shooting$(EXE_EXT) 

Bezier_curves$(EXE_EXT): Bezier_curves$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)Bezier_curves Bezier_curves$(OBJ_EXT) $(LDFLAGS)

aggregated_insertion$(EXE_EXT): aggregated_insertion$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)aggregated_insertion aggregated_insertion$(OBJ_EXT) $(LDFLAGS)

batched_point_location$(EXE_EXT): batched_point_location$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)batched_point_location batched_point_location$(OBJ_EXT) $(LDFLAGS)

bgl_dual_adapter$(EXE_EXT): bgl_dual_adapter$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)bgl_dual_adapter bgl_dual_adapter$(OBJ_EXT) $(LDFLAGS)

bgl_primal_adapter$(EXE_EXT): bgl_primal_adapter$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)bgl_primal_adapter bgl_primal_adapter$(OBJ_EXT) $(LDFLAGS)

circles$(EXE_EXT): circles$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)circles circles$(OBJ_EXT) $(LDFLAGS)

circular_arcs$(EXE_EXT): circular_arcs$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)circular_arcs circular_arcs$(OBJ_EXT) $(LDFLAGS)

circular_line_arcs$(EXE_EXT): circular_line_arcs$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)circular_line_arcs circular_line_arcs$(OBJ_EXT) $(LDFLAGS)

conic_multiplicities$(EXE_EXT): conic_multiplicities$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)conic_multiplicities conic_multiplicities$(OBJ_EXT) $(LDFLAGS)

conics$(EXE_EXT): conics$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)conics conics$(OBJ_EXT) $(LDFLAGS)

consolidated_curve_data$(EXE_EXT): consolidated_curve_data$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)consolidated_curve_data consolidated_curve_data$(OBJ_EXT) $(LDFLAGS)

curve_history$(EXE_EXT): curve_history$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)curve_history curve_history$(OBJ_EXT) $(LDFLAGS)

dcel_extension$(EXE_EXT): dcel_extension$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)dcel_extension dcel_extension$(OBJ_EXT) $(LDFLAGS)

dcel_extension_io$(EXE_EXT): dcel_extension_io$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)dcel_extension_io dcel_extension_io$(OBJ_EXT) $(LDFLAGS)

dual_lines$(EXE_EXT): dual_lines$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)dual_lines dual_lines$(OBJ_EXT) $(LDFLAGS)

dual_with_data$(EXE_EXT): dual_with_data$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)dual_with_data dual_with_data$(OBJ_EXT) $(LDFLAGS)

edge_insertion$(EXE_EXT): edge_insertion$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)edge_insertion edge_insertion$(OBJ_EXT) $(LDFLAGS)

edge_manipulation$(EXE_EXT): edge_manipulation$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)edge_manipulation edge_manipulation$(OBJ_EXT) $(LDFLAGS)

edge_manipulation_curve_history$(EXE_EXT): edge_manipulation_curve_history$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)edge_manipulation_curve_history edge_manipulation_curve_history$(OBJ_EXT) $(LDFLAGS)

face_extension$(EXE_EXT): face_extension$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)face_extension face_extension$(OBJ_EXT) $(LDFLAGS)

face_extension_overlay$(EXE_EXT): face_extension_overlay$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)face_extension_overlay face_extension_overlay$(OBJ_EXT) $(LDFLAGS)

generic_curve_data$(EXE_EXT): generic_curve_data$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)generic_curve_data generic_curve_data$(OBJ_EXT) $(LDFLAGS)

global_insertion$(EXE_EXT): global_insertion$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)global_insertion global_insertion$(OBJ_EXT) $(LDFLAGS)

global_removal$(EXE_EXT): global_removal$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)global_removal global_removal$(OBJ_EXT) $(LDFLAGS)

incremental_insertion$(EXE_EXT): incremental_insertion$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)incremental_insertion incremental_insertion$(OBJ_EXT) $(LDFLAGS)

io$(EXE_EXT): io$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)io io$(OBJ_EXT) $(LDFLAGS)

io_curve_history$(EXE_EXT): io_curve_history$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)io_curve_history io_curve_history$(OBJ_EXT) $(LDFLAGS)

isolated_vertices$(EXE_EXT): isolated_vertices$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)isolated_vertices isolated_vertices$(OBJ_EXT) $(LDFLAGS)

observer$(EXE_EXT): observer$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)observer observer$(OBJ_EXT) $(LDFLAGS)

overlay$(EXE_EXT): overlay$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)overlay overlay$(OBJ_EXT) $(LDFLAGS)

overlay_unbounded$(EXE_EXT): overlay_unbounded$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)overlay_unbounded overlay_unbounded$(OBJ_EXT) $(LDFLAGS)

point_location$(EXE_EXT): point_location$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)point_location point_location$(OBJ_EXT) $(LDFLAGS)

polylines$(EXE_EXT): polylines$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)polylines polylines$(OBJ_EXT) $(LDFLAGS)

predefined_kernel$(EXE_EXT): predefined_kernel$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)predefined_kernel predefined_kernel$(OBJ_EXT) $(LDFLAGS)

predefined_kernel_non_intersecting$(EXE_EXT): predefined_kernel_non_intersecting$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)predefined_kernel_non_intersecting predefined_kernel_non_intersecting$(OBJ_EXT) $(LDFLAGS)

rational_functions$(EXE_EXT): rational_functions$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)rational_functions rational_functions$(OBJ_EXT) $(LDFLAGS)

special_edge_insertion$(EXE_EXT): special_edge_insertion$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)special_edge_insertion special_edge_insertion$(OBJ_EXT) $(LDFLAGS)

sweep_line$(EXE_EXT): sweep_line$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)sweep_line sweep_line$(OBJ_EXT) $(LDFLAGS)

unbounded_non_intersecting$(EXE_EXT): unbounded_non_intersecting$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)unbounded_non_intersecting unbounded_non_intersecting$(OBJ_EXT) $(LDFLAGS)

unbounded_rational_functions$(EXE_EXT): unbounded_rational_functions$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)unbounded_rational_functions unbounded_rational_functions$(OBJ_EXT) $(LDFLAGS)

vertical_decomposition$(EXE_EXT): vertical_decomposition$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)vertical_decomposition vertical_decomposition$(OBJ_EXT) $(LDFLAGS)

vertical_ray_shooting$(EXE_EXT): vertical_ray_shooting$(OBJ_EXT)
	$(CGAL_CXX) $(LIBPATH) $(EXE_OPT)vertical_ray_shooting vertical_ray_shooting$(OBJ_EXT) $(LDFLAGS)

clean: \
                   Bezier_curves.clean \
                   aggregated_insertion.clean \
                   batched_point_location.clean \
                   bgl_dual_adapter.clean \
                   bgl_primal_adapter.clean \
                   circles.clean \
                   circular_arcs.clean \
                   circular_line_arcs.clean \
                   conic_multiplicities.clean \
                   conics.clean \
                   consolidated_curve_data.clean \
                   curve_history.clean \
                   dcel_extension.clean \
                   dcel_extension_io.clean \
                   dual_lines.clean \
                   dual_with_data.clean \
                   edge_insertion.clean \
                   edge_manipulation.clean \
                   edge_manipulation_curve_history.clean \
                   face_extension.clean \
                   face_extension_overlay.clean \
                   generic_curve_data.clean \
                   global_insertion.clean \
                   global_removal.clean \
                   incremental_insertion.clean \
                   io.clean \
                   io_curve_history.clean \
                   isolated_vertices.clean \
                   observer.clean \
                   overlay.clean \
                   overlay_unbounded.clean \
                   point_location.clean \
                   polylines.clean \
                   predefined_kernel.clean \
                   predefined_kernel_non_intersecting.clean \
                   rational_functions.clean \
                   special_edge_insertion.clean \
                   sweep_line.clean \
                   unbounded_non_intersecting.clean \
                   unbounded_rational_functions.clean \
                   vertical_decomposition.clean \
                   vertical_ray_shooting.clean 

#---------------------------------------------------------------------#
#                    suffix rules
#---------------------------------------------------------------------#

.C$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<


.cpp$(OBJ_EXT):
	$(CGAL_CXX) $(CXXFLAGS) $(OBJ_OPT) $<

