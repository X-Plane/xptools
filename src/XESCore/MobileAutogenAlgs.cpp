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
	return snap_division(degree_latitude_to_m, desired_division_width_m, exact_division_width_m);
}

int divisions_longitude_per_degree(double desired_division_width_m, double latitude_degrees, double * exact_division_width_m)
{
	return snap_division(degree_longitude_to_m(latitude_degrees), desired_division_width_m, exact_division_width_m);
}

Bbox2 get_ortho_grid_square_bounds(double lon, double lat)
{
	const Bbox2 containing_dsf((int)(lon - 1), (int)lat, (int)lon, (int)(lat + 1));

	const int divisions_lon = divisions_longitude_per_degree(g_ortho_width_m, containing_dsf.centroid().y());
	const int divisions_lat = divisions_latitude_per_degree(g_ortho_width_m);

	const double delta_lon = dob_abs(containing_dsf.xmin() - lon);
	const double delta_lat = dob_abs(containing_dsf.ymin() - lat);
	const int x_grid_coord = delta_lon * divisions_lon;
	const int y_grid_coord = delta_lat * divisions_lat;

	Bbox2 out(
			containing_dsf.xmin() + ((double)x_grid_coord / divisions_lon),
			containing_dsf.ymin() + ((double)y_grid_coord / divisions_lat),
			containing_dsf.xmin() + ((double)(x_grid_coord + 1) / divisions_lon),
			containing_dsf.ymin() + ((double)(y_grid_coord + 1) / divisions_lat));
	DebugAssert(out.xmin() < out.xmax());
	DebugAssert(out.ymin() < out.ymax());
	DebugAssert(out.contains(Point2(lon, lat)));
	DebugAssert(out.area() > 0);
	return out;
}






