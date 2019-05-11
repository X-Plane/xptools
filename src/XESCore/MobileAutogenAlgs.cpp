#include "MobileAutogenAlgs.h"
#include "MathUtils.h"
#include "GISTool_Globals.h" // for barf_on_tiny_map_faces()
#include "XObjReadWrite.h"
#include "ObjConvert.h"
#include "ObjUtils.h"

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
			candidates.emplace_back(to_be_matched->first, 0);
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


struct mobile_ag_library_item {
	const char * lib_path;
	const char * disk_path;
};
static constexpr mobile_ag_library_item s_mobile_ag_library[] = {
		{"lib/mobile/autogen/Europe/Euro_Bottomi.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Bottomi.obj"},
		{"lib/mobile/autogen/Europe/Euro_Bottomg.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Bottomh.obj"},
		{"lib/mobile/autogen/Europe/Euro_Bottomf.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Bottomf.obj"},
		{"lib/mobile/autogen/Europe/Euro_Bottome.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Bottome.obj"},
		{"lib/mobile/autogen/Europe/Euro_Bottomd.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Bottomd.obj"},
		{"lib/mobile/autogen/Europe/Euro_Bottomc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Bottomc.obj"},
		{"lib/mobile/autogen/Europe/Euro_Bottomb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Bottomb.obj"},
		{"lib/mobile/autogen/Europe/Euro_Bottoma.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Bottoma.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1p.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1p.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1o.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1o.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1n.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1n.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1m.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1m.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1l.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1l.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1k.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1k.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1j.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1j.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1i.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1i.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1h.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1h.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1g.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1g.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1f.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1f.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1e.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1e.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1d.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1d.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1c.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1c.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1b.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1b.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-1a.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-1a.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_l.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_l.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_k.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_k.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_o.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_o.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_n.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_n.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_h.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_h.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_g.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_g.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_j.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_j.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_i.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_i.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_e.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_e.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2_b.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2_b.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2p.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2p.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2o.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2o.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2n.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2n.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2m.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2m.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2l.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2l.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2k.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2k.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2j.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2j.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2i.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2i.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2h.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2h.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2g.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2g.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2f.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2f.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2e.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2e.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2d.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2d.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2c.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2c.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2b.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2b.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-2a.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-2a.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3p.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3p.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3o.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3o.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3n.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3n.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3m.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3m.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3l.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3l.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3k.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3k.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3i.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3i.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3h.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3h.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3g.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3g.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3f.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3f.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3e.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3e.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3d.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3d.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3c.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3c.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3b.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3b.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-3a.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-3a.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4_n.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4_n.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4_k.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4_k.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4_j.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4_j.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4_i.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4_i.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4_h.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4_h.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4_f.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4_f.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4_e.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4_e.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4p.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4p.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4o.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4o.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4n.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4n.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4m.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4m.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4l.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4l.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4k.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4k.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4j.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4j.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4i.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4i.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4h.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4h.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4g.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4g.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4f.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4f.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4e.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4e.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4d.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4d.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4c.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4c.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4b.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4b.obj"},
		{"lib/mobile/autogen/Europe/Euro_Full-4a.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Full-4a.obj"},
		{"lib/mobile/autogen/Europe/Euro_Leftn.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Leftn.obj"},
		{"lib/mobile/autogen/Europe/Euro_Lefto.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Lefto.obj"},
		{"lib/mobile/autogen/Europe/Euro_Leftp.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Leftp.obj"},
		{"lib/mobile/autogen/Europe/Euro_Leftl.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Leftl.obj"},
		{"lib/mobile/autogen/Europe/Euro_Lefth.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Lefth.obj"},
		{"lib/mobile/autogen/Europe/Euro_Leftg.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Leftg.obj"},
		{"lib/mobile/autogen/Europe/Euro_Leftd.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Leftd.obj"},
		{"lib/mobile/autogen/Europe/Euro_Leftc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Leftc.obj"},
		{"lib/mobile/autogen/Europe/Euro_Leftb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Leftb.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fullp.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fullp.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fullo.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fullo.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fulln.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fulln.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fulll.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fulll.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fulli.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fulli.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fullh.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fullh.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fullg.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fullg.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fulle.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fulle.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fulld.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fulld.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fullc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fullc.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fullb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fullb.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Fulla.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Fulla.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Halfl.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Halfl.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Halfh.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Halfh.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Halfg.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Halfg.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Halfd.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Halfd.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Halfc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Halfc.obj"},
		{"lib/mobile/autogen/Europe/Euro_LL_Halfb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LL_Halfb.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fullo.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fullo.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fulln.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fulln.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fullm.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fullm.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fullj.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fullj.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fulli.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fulli.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fullh.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fullh.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fullg.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fullg.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fullf.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fullf.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fulle.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fulle.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fulld.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fulld.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fullc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fullc.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fullb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fullb.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Fulla.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Fulla.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Halfe.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Half.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Halfd.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Half.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Halfc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Half.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Halfb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Half.obj"},
		{"lib/mobile/autogen/Europe/Euro_LR_Halfa.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_LR_Half.obj"},
		{"lib/mobile/autogen/Europe/Euro_Rightp.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Rightp.obj"},
		{"lib/mobile/autogen/Europe/Euro_Righto.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Righto.obj"},
		{"lib/mobile/autogen/Europe/Euro_Rightn.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Rightn.obj"},
		{"lib/mobile/autogen/Europe/Euro_Rightm.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Rightm.obj"},
		{"lib/mobile/autogen/Europe/Euro_Rightj.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Rightj.obj"},
		{"lib/mobile/autogen/Europe/Euro_Righti.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Righti.obj"},
		{"lib/mobile/autogen/Europe/Euro_Rightg.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Rightg.obj"},
		{"lib/mobile/autogen/Europe/Euro_Rightf.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Rightf.obj"},
		{"lib/mobile/autogen/Europe/Euro_Righte.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Righte.obj"},
		{"lib/mobile/autogen/Europe/Euro_Rightc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Rightc.obj"},
		{"lib/mobile/autogen/Europe/Euro_Rightb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Rightb.obj"},
		{"lib/mobile/autogen/Europe/Euro_Righta.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Righta.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fullp.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fullp.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fullo.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fullo.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fulln.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fulln.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fullm.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fullm.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fulll.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fulll.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fulli.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fulli.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fullh.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fullh.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fullg.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fullg.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fulld.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fulld.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fullc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fullc.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Fullb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Fullb.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Halfp.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Halfp.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Halfo.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Halfo.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Halfn.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Halfn.obj"},
		{"lib/mobile/autogen/Europe/Euro_UL_Halfl.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UL_Halfl.obj"},
		{"lib/mobile/autogen/Europe/Euro_Upperp.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Upperp.obj"},
		{"lib/mobile/autogen/Europe/Euro_Uppero.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Uppero.obj"},
		{"lib/mobile/autogen/Europe/Euro_Uppern.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Uppern.obj"},
		{"lib/mobile/autogen/Europe/Euro_Upperm.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Upperm.obj"},
		{"lib/mobile/autogen/Europe/Euro_Upperl.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Upperl.obj"},
		{"lib/mobile/autogen/Europe/Euro_Upperk.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Upperk.obj"},
		{"lib/mobile/autogen/Europe/Euro_Upperj.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Upperj.obj"},
		{"lib/mobile/autogen/Europe/Euro_Upperi.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_Upperi.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fullp.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fullp.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fullo.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fullo.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fulln.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fulln.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fullm.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fullm.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fulll.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fulll.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fullj.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fullj.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fulli.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fulli.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fullf.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fullf.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fulle.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fulle.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fullc.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fullc.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fullb.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fullb.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Fulla.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Fulla.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Halfo.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Halfo.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Halfn.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Halfn.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Halfm.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Halfm.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Halfj.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Halfj.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Halfi.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Halfi.obj"},
		{"lib/mobile/autogen/Europe/Euro_UR_Halff.obj", "Global Scenery/Mobile_Autogen_Lib/Europe/objects/Euro_UR_Halff.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottom_e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottom_e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottom_a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottom_a.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottom_b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottom_b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottomg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottomg.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottomf.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottomf.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottome.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottome.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottomd.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottomd.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottomc.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottomc.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottomb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottomb.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Bottoma.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Bottoma.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full1_i.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full1_i.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full1_h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full1_h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full1_g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full1_g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full1_f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full1_f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full1_e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full1_e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full1_d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full1_d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full1_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full1_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full1_b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full1_b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1i.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1i.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full1a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full1a.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full2_f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full2_f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full2_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full2_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full2_g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full2_g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full2i.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full2i.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full2h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full2h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full2g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full2g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full2f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full2f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full2d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full2d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full2c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full2c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full2b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full2b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full3_i.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full3_i.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full3_f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full3_f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full3_e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full3_e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full3_d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full3_d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full3_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full3_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full3_b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full3_b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full3_a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full3_a.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full3_h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full3_h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3i.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3i.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full3a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full3a.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full4_g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full4_g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full4_f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full4_f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full4_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full4_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full4_b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full4_b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full4_d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full4_d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4i.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4i.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full4a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full4a.obj"},
		{"lib/mobile/autogen/US/Stadium.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/Stadium.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full5_h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full5_h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full5_g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full5_g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full5_f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full5_f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full5_e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full5_e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full5_d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full5_d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full5_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full5_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full5_b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full5_b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full5_a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full5_a.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5i.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5i.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full5a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full5a.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full6_g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full6_g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full6_e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full6_e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full6_d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full6_d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full6_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full6_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full6_b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full6_b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Full6_a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Full6_a.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6i.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6i.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6h.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6h.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_full6a.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_full6a.obj"},
		{"lib/mobile/autogen/US/ConventionCtr.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/ConventionCtr.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Left_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Left_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Lefti.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Lefti.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Lefth.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Lefth.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Leftf.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Leftf.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Lefte.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Lefte.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Leftc.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Leftc.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Leftb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Leftb.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Full_e.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Full_e.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Full_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Full_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Full_b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Full_b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fullg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fullg.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fulli.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fulli.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fullh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fullh.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fullf.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fullf.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fulle.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fulle.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fulld.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fulld.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fullc.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fullc.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fullb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fullb.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Fulla.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Fulla.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Halfh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Halfh.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Halff.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Halff.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Halfe.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Halfe.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Halfd.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Halfd.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Halfc.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Halfc.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LL_Halfb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LL_Halfb.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Full_f.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Full_f.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Full_d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Full_d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Full_c.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Full_c.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Full_b.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Full_b.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fulli.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fulli.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fullh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fullh.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fullg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fullg.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fullf.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fullf.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fulle.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fulle.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fulld.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fulld.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fullc.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fullc.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fullb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fullb.obj"},
		{"lib/mobile/autogen/US/CitySqIn_LR_Fulla.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Fulla.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halfi.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halfi.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halfh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halfh.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halfg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halfg.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halff.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halff.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halfd.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halfd.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halfc.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halfc.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halfb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halfb.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halfa.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halfa.obj"},
		{"lib/mobile/autogen/US/CitySq_LR_Halfe.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_LR_Halfe.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Right_g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Right_g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Right_d.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Right_d.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Righth.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Righth.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Rightg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Rightg.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Righte.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Righte.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Rightd.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Rightd.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Righta.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Righta.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Rightb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Rightb.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fulli.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fulli.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fullh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fullh.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fullg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fullg.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fullf.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fullf.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fulle.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fulle.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fulld.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fulld.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fullc.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fullc.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fullb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fullb.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UL_Fulla.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Fulla.obj"},
		{"lib/mobile/autogen/US/CitySqIN_UL_Halfi.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Halfi.obj"},
		{"lib/mobile/autogen/US/CitySqIN_UL_Halfh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Halfh.obj"},
		{"lib/mobile/autogen/US/CitySqIN_UL_Halff.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Halff.obj"},
		{"lib/mobile/autogen/US/CitySqIN_UL_Halfe.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UL_Halfe.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Upperi.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Upperi.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Upperh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Upperh.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Upperg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Upperg.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Upperf.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Upperf.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Upperd.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Upperd.obj"},
		{"lib/mobile/autogen/US/CitySqIn_Uppere.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_Uppere.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Fulli.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Fulli.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Fullh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Fullh.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Fullg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Fullg.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Fullf.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Fullf.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Fulld.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Fulld.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Fullb.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Fullb.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Fulla.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Fulla.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Fulle.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Fulle.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Half_g.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Half_g.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Halfh.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Halfh.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Halfg.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Halfg.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Halfd.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Halfd.obj"},
		{"lib/mobile/autogen/US/CitySqIn_UR_Halfe.obj", "Global Scenery/Mobile_Autogen_Lib/US/objects/CitySqIn_UR_Halfe.obj"},
};

map<string, Bbox2> read_mobile_obj_ground_bounds()
{
	map<string, Bbox2> out;
	for(int i = 0; i < sizeof(s_mobile_ag_library) / sizeof(s_mobile_ag_library[0]); ++i)
	{
		XObj8 obj;
		if(!XObj8Read(s_mobile_ag_library[i].disk_path, obj))
		{
			XObj obj7;
			if(XObjRead(s_mobile_ag_library[i].disk_path, obj7))
			{
				Obj7ToObj8(obj7, obj);
			}
			else
			{
				throw "Failed to read required Mobile object";
			}
		}

		array<float, 3> min_coords;
		array<float, 3> max_coords;
		GetObjDimensions8(obj, min_coords.data(), max_coords.data());
		out.emplace(string(s_mobile_ag_library[i].lib_path), Bbox2(min_coords[0], min_coords[2], max_coords[0], max_coords[2]));
	}
	return out;
}

