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
#ifndef XROADDEFS_H
#define XROADDEFS_H

enum {
	seg_LeftPylons = 0,
	seg_LeftFenceOutside,
	seg_LeftFenceTop,
	seg_LeftFenceInside,
	seg_LeftRoad,
	seg_LeftMedian,
	seg_TopMedian,
	seg_RightMedian,
	seg_RightRoad,
	seg_RightFenceInside,
	seg_RightFenceTop,
	seg_RightFenceOutside,
	seg_RightPylons,
	seg_Total
};

struct	RoadSegInfo_t {
	double	length;					// Length of one segment in meters
	double	pixel_left;				// Left pixel position as frac
	double	width_meters[seg_Total];// Width of each segment in meters in x-plane
	double	width_pixels[seg_Total];// Width of each segment in pixels on bitmap
	// Height in meters of each seg?
};

struct	QuadPavementInfo_t {
	double	s1;
	double	s2;
	double	t1;
	double	t2;
};

typedef	long	RoadTypeID;
typedef	long	RoadSegID;

typedef	map<RoadTypeID, RoadSegID>	RoadTypeSegMap;

struct	CrossingInfo_t {
	QuadPavementInfo_t	pavement;	// For a given quad of intersection pavement, these types
	RoadTypeID			left;		// Connect to the four sides.
	RoadTypeID			top;
	RoadTypeID			right;
	RoadTypeID			bottom;
};

struct	RoadTypeInfo_t {
	// These segments define the basic road.
	RoadSegID		begin_cap;
	RoadSegID		main_stretch;
	RoadSegID		end_cap;

	// These are used if this pavement starts or ends at an N-way intersection
	// By convention, the lower right half of the triangle is used for starting
	// a road, and the upper left is used for ending it.
	QuadPavementInfo_t	multi_join;

	// For now our layering is simple: a height for each stack of this type.
	double			height_per_layer;

	// A mapping for an incoming T intersection from our left or right for each type.
	RoadTypeSegMap	left_joins;
	RoadTypeSegMap	right_joins;
};

typedef	map<RoadTypeID, RoadTypeInfo_t>	RoadMap;
typedef	vector<CrossingInfo_t>			CrossingVector;


#endif