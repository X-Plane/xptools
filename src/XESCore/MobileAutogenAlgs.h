/**
 * A collection of utilities used in generating the mobile "autogen terrain"
 * (i.e., our pseudo-orthophotos that we drop down in urban areas).
 */

#ifndef MOBILEAUTOGENALGS_H
#define MOBILEAUTOGENALGS_H

#include "CompGeomDefs2.h"
#include "MeshDefs.h"

enum ag_terrain_style {
	style_us,
	style_europe,
	ag_terrain_style_DIM
};
ag_terrain_style choose_style(int dsf_lon_west, int dsf_lat_south);

// The "stated" width and height of our square pseudo-orthophotos.
// This size will *not* tile perfectly onto your DSF; you'll have to use the exact dimensions
// output by divisions_xxx_per_degree() for that.
const int g_desired_ortho_dim_m[ag_terrain_style_DIM] = {1000, 2000};
const int g_ortho_width_px[ag_terrain_style_DIM] = {256, 512};

/**
 * Suppose to you have some available area, measured in degrees of latitude or longitude,
 * and you want to divide it into sections of a certain size.
 *
 * You want to know both:
 *  a) how many divisions to make, and
 *  b) the exact width of each division that makes it fit perfectly into the available area.
 *
 * E.g., you have 1 degree of latitude (roughly 111 km), and you want to divide it into sections of 330 m.
 * You would naively have 336.363636... divisions, but if you'll accept a little fudge factor
 * ("snapping" behavior), you could instead have *exactly* 336 divisions, each of size (111000/336) m.
 */
int divisions_latitude_per_degree( double desired_division_width_m,								double * exact_division_width_m=NULL);
int divisions_longitude_per_degree(double desired_division_width_m, double latitude_degrees,	double * exact_division_width_m=NULL);

struct grid_coord_desc {
	int x;
	int y;
	int dx; // the number of columns (x dimension)
	int dy; // the number of rows (y dimension)
	int dsf_lon;
	int dsf_lat;
};
grid_coord_desc get_ortho_grid_xy(const Point2 &point, ag_terrain_style style);

/**
 * @return The bounds, in terms of latitude and longitude, for the grid square of width g_ortho_width_m
 *         that contains the specified tri.
 */
Bbox2 get_ortho_grid_square_bounds(const CDT::Face_handle &tri, const Bbox2 &containing_dsf);



struct ortho_urbanization
{
	ortho_urbanization(int bl, int br, int tr, int tl);
	ortho_urbanization(const vector<int> &from_ccw_vector); // ccw from lower left

	int bottom_left; // the "base" ter enum for the bottom left; one of terrain_PseudoOrthoEuro, terrain_PseudoOrthoEuroSortaIndustrial, terrain_PseudoOrthoInner, terrain_PseudoOrthoTown, terrain_PseudoOrthoOuter, or terrain_PseudoOrthoIndustrial
	int bottom_right;
	int top_right;
	int top_left;

	ortho_urbanization rotate(int clockwise_deg) const;
	bool is_uniform() const;
	int count_sides(int ter_enum) const;
	vector<int> to_vector() const; // ccw from lower left

	float hash() const;
	bool operator<(const ortho_urbanization &other) const;
	bool operator==(const ortho_urbanization &other) const;
	bool operator!=(const ortho_urbanization &other) const;
};

// maps desired urb levels in the corners to the terrain enum
map<ortho_urbanization, int> get_terrain_transition_descriptions(ag_terrain_style style);


struct tile_assignment
{
	tile_assignment(int tile=NO_VALUE, int rotation=0) : ter_enum(tile), rotation_deg(rotation) { }

	int hash() const { return (13 + ter_enum) * (71 + rotation_deg); }
	bool operator<(const tile_assignment &other) const { return hash() < other.hash(); }

	int ter_enum;
	int rotation_deg; // clockwise
};

// Returns a ter enum plus a rotation in degrees for a tile that is interchangable with the one you passed in.
tile_assignment get_analogous_ortho_terrain(int ter_enum, int tiling_seed_1, int tiling_seed_2, const map<int, ortho_urbanization> &terrain_desc_by_enum);

#endif // defined(MOBILEAUTOGENALGS_H)
