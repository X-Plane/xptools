/**
 * A collection of utilities used in generating the mobile "autogen terrain"
 * (i.e., our pseudo-orthophotos that we drop down in urban areas).
 */

#ifndef MOBILEAUTOGENALGS_H
#define MOBILEAUTOGENALGS_H

#include "CompGeomDefs2.h"

// The width and height of our square pseudo-orthophotos
const int g_ortho_width_m = 1000;

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

/**
 * @return The bounds, in terms of latitude and longitude, for the grid square of width g_ortho_width_m
 *         that contains the specified lon/lat.
 */
Bbox2 get_ortho_grid_square_bounds(double longitude_degrees, double latitude_degrees);



#endif // defined(MOBILEAUTOGENALGS_H)
