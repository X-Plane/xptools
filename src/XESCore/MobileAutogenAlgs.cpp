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

#define AssertLegalOrtho(member) DebugAssert(member == NO_VALUE || member == terrain_PseudoOrthoInner || member == terrain_PseudoOrthoTown || member == terrain_PseudoOrthoOuter || member == terrain_PseudoOrthoIndustrial);

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
	return bottom_left == bottom_right == top_left == top_right;
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
	deg = intwrap(deg, 0, 360);
	while(deg > 0)
	{
		const int old_bl = out.bottom_left;
		out.bottom_left = out.top_left;
		out.top_left = out.top_right;
		out.top_right = out.bottom_right;
		out.bottom_right = old_bl;
		deg -= 90;
	}
	return out;
}

map<ortho_urbanization, int> get_terrain_transition_descriptions()
{
	map<ortho_urbanization, int> ter_with_transitions; // maps desired urb levels in the corners to the terrain enum
	//										Bottom left						Bottom right					Top right						Top left
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner		)] = terrain_PseudoOrthoInner1;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoTown1;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoOuter1;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial	)] = terrain_PseudoOrthoIndustrial1;
	
	ter_with_transitions[ortho_urbanization(NO_VALUE,						NO_VALUE,						terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoTownTransBottom;
	ter_with_transitions[ortho_urbanization(NO_VALUE,						terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		NO_VALUE						)] = terrain_PseudoOrthoTownTransLeft;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		NO_VALUE,						NO_VALUE,						terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoTownTransRight;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		NO_VALUE,						NO_VALUE						)] = terrain_PseudoOrthoTownTransUpper;
	ter_with_transitions[ortho_urbanization(NO_VALUE,						NO_VALUE,						terrain_PseudoOrthoTown,		NO_VALUE						)] = terrain_PseudoOrthoTownTransLL_Half;
	ter_with_transitions[ortho_urbanization(NO_VALUE,						terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoTownTransLL_Full;
	ter_with_transitions[ortho_urbanization(NO_VALUE,						NO_VALUE,						NO_VALUE,						terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoTownTransLR_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		NO_VALUE,						terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoTownTransLR_Full;
	ter_with_transitions[ortho_urbanization(NO_VALUE,						terrain_PseudoOrthoTown,		NO_VALUE,						NO_VALUE						)] = terrain_PseudoOrthoTownTransUL_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		NO_VALUE						)] = terrain_PseudoOrthoTownTransUL_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		NO_VALUE,						NO_VALUE,						NO_VALUE						)] = terrain_PseudoOrthoTownTransUR_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		NO_VALUE,						terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoTownTransUR_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoOuterTransBottom;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoOuterTransLeft;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoOuterTransRight;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoOuterTransUpper;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoOuterTransLL_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoOuterTransLL_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoOuterTransLR_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoOuterTransLR_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoOuterTransUL_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoOuterTransUL_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoTown			)] = terrain_PseudoOrthoOuterTransUR_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoTown,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoOuterTransUR_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial	)] = terrain_PseudoOrthoIndTransBottom;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoIndTransLeft;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial	)] = terrain_PseudoOrthoIndTransRight;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoIndTransUpper;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoIndTransLL_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial	)] = terrain_PseudoOrthoIndTransLL_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial	)] = terrain_PseudoOrthoIndTransLR_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial	)] = terrain_PseudoOrthoIndTransLR_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoIndTransUL_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoIndTransUL_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoIndTransUR_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoIndustrial,	terrain_PseudoOrthoOuter,		terrain_PseudoOrthoIndustrial	)] = terrain_PseudoOrthoIndTransUR_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner		)] = terrain_PseudoOrthoInnerTransBottom;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoInnerTransLeft;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner		)] = terrain_PseudoOrthoInnerTransRight;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoInnerTransUpper;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoInnerTransLL_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner		)] = terrain_PseudoOrthoInnerTransLL_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner		)] = terrain_PseudoOrthoInnerTransLR_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner		)] = terrain_PseudoOrthoInnerTransLR_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoInnerTransUL_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoInnerTransUL_Full;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoOuter		)] = terrain_PseudoOrthoInnerTransUR_Half;
	ter_with_transitions[ortho_urbanization(terrain_PseudoOrthoInner,		terrain_PseudoOrthoInner,		terrain_PseudoOrthoOuter,		terrain_PseudoOrthoInner		)] = terrain_PseudoOrthoInnerTransUR_Full;
	return ter_with_transitions;
}

tile_assignment get_analogous_ortho_terrain(int ter_enum, int x, int y, const map<int, ortho_urbanization> &terrain_desc_by_enum)
{
	if(ter_enum == NO_VALUE)
		return tile_assignment(ter_enum, 0);

	const bool needs_tiling =
			ter_enum == terrain_PseudoOrthoInner1 || ter_enum == terrain_PseudoOrthoTown1 ||
			ter_enum == terrain_PseudoOrthoOuter1 || ter_enum == terrain_PseudoOrthoIndustrial1;
	if(needs_tiling)
	{
		// The variant gives us the perfect checkerboard tiling of the two "normal" variants of each ortho
		const int new_ter = ter_enum + (x + y) % 2;
		// Industrial has big shadows... don't rotate it or we make it look even worse!
		const int rot = ter_enum == terrain_PseudoOrthoIndustrial1 ? 0 : 90 * ((x + x * y) % 4));
		return tile_assignment(new_ter, rot;
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


