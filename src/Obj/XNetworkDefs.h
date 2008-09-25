/*
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef XNETWORKDEFS_H
#define XNETWORKDEFS_H

#include <vector>
#include <string>
using namespace std;
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"

/************************************************************************
 * NETWORK GRAPHICAL DEFINITIONS
************************************************************************/

// A network segment item is a single quad of road that is repeated.
struct	NetworkSegmentItem_t {
	float	s1;				// Start and end horizontal texture
	float	s2;				// within the road.
	float	delta_lat;		// Width in lateral position in meters
	float	delta_vert;		// Height in vertical meters
};

// A single object to be placed somewhere along the road.
struct	NetworkObjectItem_t {
	string	object_name;	// Name of object
	float	lat_offset;		// Percent placement on road (0=left,1=right)
	float	long_fraction;	// Position as T coordinate
	float	rotation;		// Rotation to apply from road's heading
	bool	on_road;		// true if on road, false if on ground
};

// This represents one version of the below network segment.
struct	NetworkSegmentLOD_t {
	float	lod_near;			// Near and far len for this segment to be visible.
	float	lod_far;
	float	width;				// Total width of segment
	float	vert_offset;		// Start position from 0
	float	scale_lon;			// Length of segment in meters

	float	start_pixel_t;		// Start pixel in texture (always 0 for a non-cap)
	float	end_pixel_t;		// End pixel in texture (always 1 in non-cap)
	float	chop_point_percent;	// Percent through cap to crop at for cropped, -1 for scaled caps or segments

	vector<NetworkSegmentItem_t>	items;		// All of the objects and
	vector<NetworkObjectItem_t>		objects;	// items of pavement in this segment.
};

// A single segment of network, which is tiled to form a straight-away,
// warped to form a curve, or stuck on the end to form a cap.
struct	NetworkSegment_t {

	vector<NetworkSegmentLOD_t>	lod;

	void for_lod(float lod_near, float lod_far, NetworkSegmentLOD_t& lod) const;
};

// A network cap rule is a pair of caps (one for the front and one for the back of
// a chain and a set of intersections that you must match to use this cap.  0
// items in the junction_type means it is a default cap rule, otherwise the junctions
// are listed in CW order from yourself first (so junction_type[0] is always your chain
// type).  caps may be null (e.g. 0 length) to indicate no capping is needed.

struct	NetworkCapRule_t {
	vector<int>			junction_types;
	NetworkSegment_t	front_cap;
	NetworkSegment_t	back_cap;
};

// A set of cap rules is just a vector, usually sorted by priority more or less, e.g. larger
// number rules first, then the default ones at the end.
typedef vector<NetworkCapRule_t> NetworkCapRules_t;

// A junction segment is an N vertex polygon used to fill in the area where a
// series of roads meet.  Junction types can be wild-carded (-1 is valid).
struct	JunctionRule_t {
	float				lod_near;
	float				lod_far;

	vector<int>			junction_types;
	vector<float>		s_coord;
	vector<float>		t_coord;

	void				rotate();
};

// A set of network defs is defined first by the segments for each type of road.
struct	NetworkDef_t {

	string						texture;			// Our texture
	vector<NetworkSegment_t>	segments;			// For each highway type, the main segment
	vector<NetworkCapRules_t>	caps;				// For each hgihway type, the prioritized cap rules
	vector<JunctionRule_t>		junctions;			// Rules for filling in junctions.

	// This routine, given two junction lists, returns the caps for a chain.  It is expected
	// that (1) frontJunction and backJunction have at least one item (no such thing as a 0-item
	// junction!) and (2) the first item of frontJunction and backJunction be the same, since
	// that is the chain we are querying about.
	void	find_caps(
					const vector<int>& 	frontJunction,
					const vector<int>& 	backJunction,
					float					lod_near,
					float					lod_far,
					NetworkSegmentLOD_t&	outFrontCap,
					NetworkSegmentLOD_t&	outBackCap) const;

	// Returns true if we found an all-encompassing rule for the junction.
	bool	find_junction(
					const vector<int>&	junctionPattern,
					JunctionRule_t&		outJunction) const;
	bool	find_junction_part(
					int					roadType,
					JunctionRule_t&		outJunction) const;

};

/************************************************************************
 * ACTUAL NETWORK LOCATIONS
************************************************************************/

// A network junction has a location, and for each level, a series of
// junctions sticking out in all directions.  The junctions are listed
// in clockwise order (looking from above).  a flag of true indicates
// that this spur ends in the junction, rather than starts.
//
// A spur is a half of a chain, (defined by a chain and which end we want)
// used to identify the input into a junction.
struct	NetworkJunction_t {
	typedef	pair<int, bool>				spur;
	Point3								location;
	vector<spur>						spurs;
};

// A shape point - a location and optional secondary point to
// define a bezier curve.
struct	Shape_Point_t {
	Point3								location;
	bool								has_curve;
	Point3								curve;
};

// A network chain - has a type of road, two junctions (by index),
// optional curve points, and zero or more shape points.
struct	Network_Chain_t {
	int									chain_type;
	int									start_junction;
	int									end_junction;
	bool								has_start_curve;
	bool								has_end_curve;
	Point3								start_curve;
	Point3								end_curve;
	vector<Shape_Point_t>				shape_points;
};

// Network data: all junctions (which are defined by index nubmer), and...
struct	NetworkData_t {
	vector<NetworkJunction_t>		junctions;
	vector<Network_Chain_t>			chains;

	void		build_back_links(void);

	// Headings of various chains at the start and end.  Both start and
	// end tangents are in the direction of the chain.  The spur heading
	// is always towards the appropriate junction.
	Vector3		chain_get_start_tangent(int chain) const;
	Vector3		chain_get_end_tangent(int chain) const;
	Vector3		junction_get_spur_heading(NetworkJunction_t::spur chain) const;

	// These routines, given a spur and a junction, give you the next or
	// previous spur.
	NetworkJunction_t::spur
				previous_chain(NetworkJunction_t::spur chain) const;
	NetworkJunction_t::spur
				next_chain(NetworkJunction_t::spur chain) const;

	// Given a spur, return the types of the chains plugged into its junction
	// starting at the spur and going CW.
	void		get_junction_types(int junction					, vector<int>& out_types) const;
	void		get_junction_types(NetworkJunction_t::spur chain, vector<int>& out_types) const;
};

#endif
