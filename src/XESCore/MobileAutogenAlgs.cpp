#include "MobileAutogenAlgs.h"
#include "MathUtils.h"

/**
 * Suppose to you have some available area, and you want to divide it into sections of a certain size.
 * You want to know both:
 *  a) how many divisions to make, and
 *  b) the exact width of each division that makes it fit perfectly into the available area.
 * E.g., you have 1000 m, and you want to divide it into sections of 33 m.
 * You would naively have 30.30303030... divisions, but if you'll accept a little fudge factor
 * (the "snapping" behavior here), you could instead have exactly 30 divisions, each of size 33.333333 m.
 */
int snap_division(double available_width, double desired_division_width, double * out_exact_division_width)
{
	int out = intround(available_width / desired_division_width);
	if(out_exact_division_width)
	{
		*out_exact_division_width = available_width / out;
	}
	return out;
}

int divisions_latitude_per_degree(double desired_division_width_m, double * exact_division_width_m)
{
	return int_abs(snap_division(degree_latitude_to_m, desired_division_width_m, exact_division_width_m));
}

int divisions_longitude_per_degree(double desired_division_width_m, double latitude_degrees, double * exact_division_width_m)
{
	return int_abs(snap_division(degree_longitude_to_m(latitude_degrees), desired_division_width_m, exact_division_width_m));
}

Polygon2 cgal_tri_to_ben(const CDT::Face_handle &tri, const Bbox2 &containing_dsf)
{
	Polygon2 out;
	out.reserve(3);
	for(int vert_idx = 0; vert_idx < 3; ++vert_idx)
	{
		CDT::Vertex_handle vert = tri->vertex(vert_idx);
		out.push_back(Point2(doblim(CGAL::to_double(vert->point().x()), containing_dsf.xmin(), containing_dsf.xmax()),
							 doblim(CGAL::to_double(vert->point().y()), containing_dsf.ymin(), containing_dsf.ymax())));
	}
	DebugAssert(out.is_ccw());
	DebugAssert(out.area() > 0);
	return out;
}

grid_coord_desc get_orth_grid_xy(const Point2 &point)
{
	const double dsf_min_lon = floor(point.x());
	const double dsf_min_lat = floor(point.y());
	const double dsf_center_lat = dsf_min_lat + 0.5;

	const int divisions_lon = divisions_longitude_per_degree(g_ortho_width_m, dsf_center_lat);
	const int divisions_lat = divisions_latitude_per_degree(g_ortho_width_m);

	// Note: we use the *tri*'s centroid to decide the grid coords, because any *vertex* might be shared
	//       between multiple tris in *different* grid squares.
	//       (There's gonna be one tri with a vertex is at (1, 1) in UV, and another sharing the same vertex,
	//        but needing UV coords of (0,0).)
	const double delta_lon = dob_abs(dsf_min_lon - point.x());
	const double delta_lat = dob_abs(dsf_min_lat - point.y());
	const int x_grid_coord = delta_lon * divisions_lon;
	const int y_grid_coord = delta_lat * divisions_lat;

	grid_coord_desc out = { x_grid_coord, y_grid_coord, divisions_lon, divisions_lat, dsf_min_lon, dsf_min_lat };
	DebugAssert(out.x < out.dx);
	DebugAssert(out.y < out.dy);
	return out;
}

Bbox2 get_ortho_grid_square_bounds(const CDT::Face_handle &tri, const Bbox2 &containing_dsf)
{
	DebugAssertWithExplanation(dob_abs(containing_dsf.xspan() - 1) < 0.01, "Your 'DSF' is not 1x1 degree");
	DebugAssertWithExplanation(dob_abs(containing_dsf.yspan() - 1) < 0.01, "Your 'DSF' is not 1x1 degree");
	const Polygon2 ben_tri = cgal_tri_to_ben(tri, containing_dsf);
	DebugAssert(containing_dsf.contains(ben_tri.bounds()));
	const Point2 centroid = ben_tri.centroid();
	DebugAssert(ben_tri.inside(centroid));

	const grid_coord_desc grid_pt = get_orth_grid_xy(centroid);

	Bbox2 out(
			containing_dsf.xmin() + ((double)grid_pt.x / grid_pt.dx),
			containing_dsf.ymin() + ((double)grid_pt.y / grid_pt.dy),
			containing_dsf.xmin() + ((double)(grid_pt.x + 1) / grid_pt.dx),
			containing_dsf.ymin() + ((double)(grid_pt.y + 1) / grid_pt.dy));
	DebugAssert(out.xmin() < out.xmax());
	DebugAssert(out.ymin() < out.ymax());
	DebugAssert(containing_dsf.contains(out));
	DebugAssert(out.contains(centroid));
	DebugAssert(out.area() > 0);

#if DEV
	vector<Point2> ensure_within_bounds = ben_tri;
	for(vector<Point2>::const_iterator pt = ensure_within_bounds.begin(); pt != ensure_within_bounds.end(); ++pt)
	{
		const Point2 &p = *pt;
		// Tyler says: We can't actually guarantee this due to double precision limits.
		//             If the vertex is on the edge of the grid square, it may be "outside"
		//             the grid square's bounds by, say, 3 x 10^-10.
		//             This appears not to end up mattering...
		//DebugAssert(out.contains(*pt));
		DebugAssert(out.xmin() - pt->x() <  0.001);
		DebugAssert(out.xmax() - pt->x() > -0.001);
		DebugAssert(out.ymin() - pt->y() <  0.001);
		DebugAssert(out.ymax() - pt->y() > -0.001);
	}
#endif

	return out;
}






