#include "MobileAutogenAlgs.h"
#include "MathUtils.h"
#include "GISTool_Globals.h" // for barf_on_tiny_map_faces()

ag_terrain_style choose_style(int dsf_lon_west, int dsf_lat_south)
{
	// Far east edge of Scotland is roughly -10 lon
	// Far west edge of Poland is +24 lon
	// Far south edge of Spain is 36 lat
	// Farthest north big city I could find is Bergen, Norway at 60 lat
	if(intrange(dsf_lon_west, -11, 31) && intrange(dsf_lat_south, 36, 60))
		return style_europe;
	return style_us;
}

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
	// Ensure none of the points are colocated
	DebugAssert(tri->vertex(0)->point() != tri->vertex(1)->point());
	DebugAssert(tri->vertex(0)->point() != tri->vertex(2)->point());
	DebugAssert(tri->vertex(1)->point() != tri->vertex(2)->point());
	DebugAssert(tri->is_valid());

	Polygon2 out;
	out.reserve(3);
	for(int vert_idx = 0; vert_idx < 3; ++vert_idx)
	{
		CDT::Vertex_handle vert = tri->vertex(vert_idx);
		out.push_back(Point2(doblim(CGAL::to_double(vert->point().x()), containing_dsf.xmin(), containing_dsf.xmax()),
							 doblim(CGAL::to_double(vert->point().y()), containing_dsf.ymin(), containing_dsf.ymax())));
	}
	DebugAssert(out.is_ccw() || out.area() == 0);
#if DEV
	// Tyler says: zero-area tris sometimes happen due to rounding to double... the CGAL version had (super small) positive area, but the double version does not
	if(out.area() == 0)
	{
		static size_t zero_area_tris = 0;
		fprintf(stderr, "Found zero-area tri #%ld\n", zero_area_tris++);
	}
#endif
	return out;
}

grid_coord_desc get_ortho_grid_xy(const Point2 &point, ag_terrain_style style)
{
	const double dsf_min_lon = floor(point.x());
	const double dsf_min_lat = floor(point.y());
	const double dsf_center_lat = dsf_min_lat + 0.5;

	const int divisions_lon = divisions_longitude_per_degree(g_desired_ortho_dim_m[style], dsf_center_lat);
	const int divisions_lat = divisions_latitude_per_degree(g_desired_ortho_dim_m[style]);

	// Note: we use the *tri*'s centroid to decide the grid coords, because any *vertex* might be shared
	//       between multiple tris in *different* grid squares.
	//       (There's gonna be one tri with a vertex is at (1, 1) in UV, and another sharing the same vertex,
	//        but needing UV coords of (0,0).)
	const double delta_lon = dob_abs(dsf_min_lon - point.x());
	const double delta_lat = dob_abs(dsf_min_lat - point.y());
	const int x_grid_coord = delta_lon * divisions_lon;
	const int y_grid_coord = delta_lat * divisions_lat;

	grid_coord_desc out = { x_grid_coord, y_grid_coord, divisions_lon, divisions_lat, intround(dsf_min_lon), intround(dsf_min_lat) };
	DebugAssert(out.x < out.dx);
	DebugAssert(out.y < out.dy);
	return out;
}

#define REALLY_REALLY_CLOSE 0.00000001
bool points_are_real_close(const Point2 &p0, const Point2 &p1)
{
	return  dob_abs(p0.x() - p1.x()) < REALLY_REALLY_CLOSE &&
			dob_abs(p0.y() - p1.y()) < REALLY_REALLY_CLOSE;
}

bool tri_is_sliver(const Polygon2 &ben_tri)
{
	DebugAssert(ben_tri.size() == 3);
	// A sliver can come from *either* having two points of your tri be "really really close,"
	// or from *one* coordinate of *all three* points being "really close", even if the *other*
	// coordinate is not. E.g., the points (0, 1); (0.000000000001, 2); (0.000000000002, 3)
	return  points_are_real_close(ben_tri.at(0), ben_tri.at(1)) ||
			points_are_real_close(ben_tri.at(0), ben_tri.at(2)) ||
			points_are_real_close(ben_tri.at(1), ben_tri.at(2)) ||
			ben_tri.area() < (REALLY_REALLY_CLOSE * REALLY_REALLY_CLOSE);

}

Bbox2 get_ortho_grid_square_bounds(const CDT::Face_handle &tri, const Bbox2 &containing_dsf)
{
	DebugAssertWithExplanation(dob_abs(containing_dsf.xspan() - 1) < 0.01, "Your 'DSF' is not 1x1 degree");
	DebugAssertWithExplanation(dob_abs(containing_dsf.yspan() - 1) < 0.01, "Your 'DSF' is not 1x1 degree");
	const ag_terrain_style style = choose_style(containing_dsf.xmin(), containing_dsf.ymin());
	const Polygon2 ben_tri = cgal_tri_to_ben(tri, containing_dsf);
	DebugAssert(containing_dsf.contains(ben_tri.bounds()));
	const Point2 centroid = ben_tri.centroid();

	// Only case when the centroid might not be inside the tri: if the tri is a sliver, and our floating point math blows up
	#if DEV
	static size_t s_slivers = 0;
	if(barf_on_tiny_map_faces() && tri_is_sliver(ben_tri))
	{
		printf("Warning: found sliver #%ld\n", s_slivers++);
		printf("Bounds:\n");
		for(int v = 0; v < 3; ++v)
		{
			printf("- (%0.18f, %0.18f)\n", ben_tri[v].x(), ben_tri[v].y());
		}
		DebugAssert(!tri_is_sliver(ben_tri));
	}
	DebugAssert(!barf_on_tiny_map_faces() || ben_tri.inside(centroid));
	#endif

	const grid_coord_desc grid_pt = get_ortho_grid_xy(ben_tri.inside(centroid) ? centroid : ben_tri.front(), style);

	Bbox2 out(
			containing_dsf.xmin() + ((double)grid_pt.x / grid_pt.dx),
			containing_dsf.ymin() + ((double)grid_pt.y / grid_pt.dy),
			containing_dsf.xmin() + ((double)(grid_pt.x + 1) / grid_pt.dx),
			containing_dsf.ymin() + ((double)(grid_pt.y + 1) / grid_pt.dy));
	DebugAssert(out.xmin() < out.xmax());
	DebugAssert(out.ymin() < out.ymax());
	DebugAssert(containing_dsf.contains(out));
	DebugAssert(!barf_on_tiny_map_faces() || out.contains(centroid));
	DebugAssert(out.area() > 0);

	// Tyler says: We can't actually guarantee the bounds on the point due to both double precision limits.
	//             and the fact that we now allow the ortho to go a little outside its "legal" bounds
	//             for the sake of covering up sub-1-square-meter faces that would have
	//             otherwise made it into the mesh.

	return out;
}


float ortho_urbanization::hash() const
{
	return bottom_left + (3.0f + bottom_right) * (5.0f + top_right) * (7.0f + top_left);
}

bool ortho_urbanization::operator<(const ortho_urbanization &other) const
{
	return hash() < other.hash();
}
bool ortho_urbanization::operator==(const ortho_urbanization &other) const
{
	return bottom_left == other.bottom_left && bottom_right == other.bottom_right &&
			top_left == other.top_left && top_right == other.top_right;
}
bool ortho_urbanization::operator!=(const ortho_urbanization &other) const
{
	return !(*this == other);
}

#define AssertLegalOrtho(member) DebugAssert(member == NO_VALUE || member == terrain_PseudoOrthoInner || member == terrain_PseudoOrthoTown || member == terrain_PseudoOrthoOuter || member == terrain_PseudoOrthoIndustrial || member == terrain_PseudoOrthoEuro || member == terrain_PseudoOrthoEuroSortaIndustrial);

ortho_urbanization::ortho_urbanization(int bl, int br, int tr, int tl) :
		bottom_left(bl),
		bottom_right(br),
		top_right(tr),
		top_left(tl)
{
	AssertLegalOrtho(bottom_left);
	AssertLegalOrtho(bottom_right);
	AssertLegalOrtho(top_right);
	AssertLegalOrtho(top_left);
}

ortho_urbanization::ortho_urbanization(const vector<int> &ccw_vector) :
		bottom_left(ccw_vector[0]),
		bottom_right(ccw_vector[1]),
		top_right(ccw_vector[2]),
		top_left(ccw_vector[3])
{
	DebugAssert(ccw_vector.size() == 4);
	AssertLegalOrtho(bottom_left);
	AssertLegalOrtho(bottom_right);
	AssertLegalOrtho(top_right);
	AssertLegalOrtho(top_left);
}

bool ortho_urbanization::is_uniform() const
{
	return bottom_left == bottom_right &&
			bottom_right == top_left &&
			top_left == top_right;
}

int ortho_urbanization::count_sides(int ter_enum) const
{
	return bottom_left == ter_enum +
		bottom_right == ter_enum +
		top_left == ter_enum +
		top_right == ter_enum;
}

vector<int> ortho_urbanization::to_vector() const
{
	vector<int> out;
	out.reserve(4);
	out.push_back(bottom_left);
	out.push_back(bottom_right);
	out.push_back(top_left);
	out.push_back(top_right);
	return out;
}

ortho_urbanization ortho_urbanization::rotate(int deg) const
{
	DebugAssert(deg % 90 == 0);
	ortho_urbanization out = *this;
	deg = intwrap(deg, 0, 359);
	while(deg > 0)
	{
		const int old_bl = out.bottom_left;
		out.bottom_left = out.bottom_right;
		out.bottom_right = out.top_right;
		out.top_right = out.top_left;
		out.top_left = old_bl;
		deg -= 90;
	}
	return out;
}

// TODO: When we have C++14, make this a frozen::map (constexpr)
// https://github.com/serge-sans-paille/frozen
map<ortho_urbanization, int> get_terrain_transition_descriptions_us()
{
	map<ortho_urbanization, int> ter_with_transitions = { // maps desired urb levels in the corners to the terrain enum
			//							Bottom left						Bottom right					Top right						Top left
			{ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner		), terrain_PseudoOrthoInner1 },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoTown1 },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoOuter1 },
			{ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial	), terrain_PseudoOrthoIndustrial1 },
	
			{ortho_urbanization(NO_VALUE,						NO_VALUE,						terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoTownTransBottom },
			{ortho_urbanization(NO_VALUE,						terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		NO_VALUE						), terrain_PseudoOrthoTownTransLeft },
			{ortho_urbanization(terrain_PseudoOrthoTown,		NO_VALUE,						NO_VALUE,						terrain_PseudoOrthoTown			), terrain_PseudoOrthoTownTransRight },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		NO_VALUE,						NO_VALUE						), terrain_PseudoOrthoTownTransUpper },
			{ortho_urbanization(NO_VALUE,						NO_VALUE,						terrain_PseudoOrthoTown,		NO_VALUE						), terrain_PseudoOrthoTownTransLL_Half },
			{ortho_urbanization(NO_VALUE,						terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoTownTransLL_Full },
			{ortho_urbanization(NO_VALUE,						NO_VALUE,						NO_VALUE,						terrain_PseudoOrthoTown			), terrain_PseudoOrthoTownTransLR_Half },
			{ortho_urbanization(terrain_PseudoOrthoTown,		NO_VALUE,						terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoTownTransLR_Full },
			{ortho_urbanization(NO_VALUE,						terrain_PseudoOrthoTown,		NO_VALUE,						NO_VALUE						), terrain_PseudoOrthoTownTransUL_Half },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		NO_VALUE						), terrain_PseudoOrthoTownTransUL_Full },
			{ortho_urbanization(terrain_PseudoOrthoTown,		NO_VALUE,						NO_VALUE,						NO_VALUE						), terrain_PseudoOrthoTownTransUR_Half },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		NO_VALUE,						terrain_PseudoOrthoTown			), terrain_PseudoOrthoTownTransUR_Full },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoOuterTransBottom },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoOuterTransLeft },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoOuterTransRight },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoOuterTransUpper },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoOuterTransLL_Half },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoOuterTransLL_Full },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoOuterTransLR_Half },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoOuterTransLR_Full },
			{ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoOuterTransUL_Half },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoOuterTransUL_Full },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			), terrain_PseudoOrthoOuterTransUR_Half },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoOuterTransUR_Full },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial	), terrain_PseudoOrthoIndTransBottom },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter		), terrain_PseudoOrthoIndTransLeft },
			{ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial	), terrain_PseudoOrthoIndTransRight },
			{ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoIndTransUpper },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter		), terrain_PseudoOrthoIndTransLL_Half },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial	), terrain_PseudoOrthoIndTransLL_Full },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial	), terrain_PseudoOrthoIndTransLR_Half },
			{ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial	), terrain_PseudoOrthoIndTransLR_Full },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoIndTransUL_Half },
			{ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter		), terrain_PseudoOrthoIndTransUL_Full },
			{ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoIndTransUR_Half },
			{ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial	), terrain_PseudoOrthoIndTransUR_Full },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner		), terrain_PseudoOrthoInnerTransBottom },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoInnerTransLeft },
			{ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner		), terrain_PseudoOrthoInnerTransRight },
			{ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoInnerTransUpper },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoInnerTransLL_Half },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner		), terrain_PseudoOrthoInnerTransLL_Full },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner		), terrain_PseudoOrthoInnerTransLR_Half },
			{ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner		), terrain_PseudoOrthoInnerTransLR_Full },
			{ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoInnerTransUL_Half },
			{ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoInnerTransUL_Full },
			{ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		), terrain_PseudoOrthoInnerTransUR_Half },
			{ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner		), terrain_PseudoOrthoInnerTransUR_Full },
	};
	return ter_with_transitions;
}

map<ortho_urbanization, int> get_terrain_transition_descriptions_euro()
{
	map<ortho_urbanization, int> ter_with_transitions = { // maps desired urb levels in the corners to the terrain enum
			//						Bottom left								Bottom right							Top right								Top left
			{ortho_urbanization(terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro					), terrain_PseudoOrthoEuro1},
			{ortho_urbanization(terrain_PseudoOrthoEuroSortaIndustrial,	terrain_PseudoOrthoEuroSortaIndustrial,	terrain_PseudoOrthoEuroSortaIndustrial,	terrain_PseudoOrthoEuroSortaIndustrial	), terrain_PseudoOrthoEuroSemiInd},
			{ortho_urbanization(NO_VALUE,								NO_VALUE,								terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro					), terrain_PseudoOrthoEuroTransBottom},
			{ortho_urbanization(NO_VALUE,								terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro,				NO_VALUE								), terrain_PseudoOrthoEuroTransLeft},
			{ortho_urbanization(terrain_PseudoOrthoEuro,				NO_VALUE,								NO_VALUE,								terrain_PseudoOrthoEuro					), terrain_PseudoOrthoEuroTransRight},
			{ortho_urbanization(terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro,				NO_VALUE,								NO_VALUE								), terrain_PseudoOrthoEuroTransUpper},
			{ortho_urbanization(NO_VALUE,								NO_VALUE,								terrain_PseudoOrthoEuro,				NO_VALUE								), terrain_PseudoOrthoEuroTransLL_Half},
			{ortho_urbanization(NO_VALUE,								terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro					), terrain_PseudoOrthoEuroTransLL_Full},
			{ortho_urbanization(NO_VALUE,								NO_VALUE,								NO_VALUE,								terrain_PseudoOrthoEuro					), terrain_PseudoOrthoEuroTransLR_Half},
			{ortho_urbanization(terrain_PseudoOrthoEuro,				NO_VALUE,								terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro					), terrain_PseudoOrthoEuroTransLR_Full},
			{ortho_urbanization(NO_VALUE,								terrain_PseudoOrthoEuro,				NO_VALUE,								NO_VALUE								), terrain_PseudoOrthoEuroTransUL_Half},
			{ortho_urbanization(terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro,				NO_VALUE								), terrain_PseudoOrthoEuroTransUL_Full},
			{ortho_urbanization(terrain_PseudoOrthoEuro,				NO_VALUE,								NO_VALUE,								NO_VALUE								), terrain_PseudoOrthoEuroTransUR_Half},
			{ortho_urbanization(terrain_PseudoOrthoEuro,				terrain_PseudoOrthoEuro,				NO_VALUE,								terrain_PseudoOrthoEuro					), terrain_PseudoOrthoEuroTransUR_Full},
	};
	return ter_with_transitions;
}
map<ortho_urbanization, int> get_terrain_transition_descriptions(ag_terrain_style style)
{
	return style == style_europe ? get_terrain_transition_descriptions_euro() : get_terrain_transition_descriptions_us();
}

tile_assignment get_analogous_ortho_terrain(int ter_enum, int x, int y, const map<int, ortho_urbanization> &terrain_desc_by_enum)
{
	if(ter_enum == NO_VALUE)
		return tile_assignment(ter_enum, 0);

	const bool needs_tiling =
			ter_enum == terrain_PseudoOrthoInner1 || ter_enum == terrain_PseudoOrthoTown1 ||
			ter_enum == terrain_PseudoOrthoOuter1 || ter_enum == terrain_PseudoOrthoIndustrial1 ||
			ter_enum == terrain_PseudoOrthoEuro1;
	if(needs_tiling)
	{
		// The variant gives us the perfect checkerboard tiling of the two "normal" variants of each ortho
		const int new_ter = ter_enum + (x + y) % 2;
		// Industrial has big shadows... don't rotate it or we make it look even worse!
		const int rot = ter_enum == terrain_PseudoOrthoIndustrial1 || ter_enum == terrain_PseudoOrthoEuroSemiInd ? 0 : 90 * ((x + x * y) % 4);
		return tile_assignment(new_ter, rot);
	}
	else
	{
		map<int, ortho_urbanization>::const_iterator to_be_matched = terrain_desc_by_enum.find(ter_enum);
		if(to_be_matched != terrain_desc_by_enum.end())
		{
			vector<tile_assignment> candidates;
			candidates.reserve(4);
			candidates.push_back(tile_assignment(to_be_matched->first, 0));
			for(map<int, ortho_urbanization>::const_iterator rotation_candidate = terrain_desc_by_enum.begin(); rotation_candidate != terrain_desc_by_enum.end(); ++rotation_candidate)
			{
				for(int rotation = 90; rotation < 360; rotation += 90)
				{
					if(rotation_candidate->second.rotate(rotation) == to_be_matched->second)
					{
						candidates.push_back(tile_assignment(rotation_candidate->first, rotation));
					}
				}
			}

			DebugAssert(candidates.size() == 4);
			return candidates[ (x + x * y) % candidates.size() ];
		}
	}
	return tile_assignment(ter_enum, 0);
}


