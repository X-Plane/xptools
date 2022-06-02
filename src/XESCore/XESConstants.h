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
#ifndef XESCONSTANTS_H
#define XESCONSTANTS_H

// Std lapse rate is 2 degrees C colder per 1000 feet higher, or per 304.8 meters.
const float 	kStdLapseRate = -2.0 / 304.8;

#define NM_TO_MTR			1852.0
#define MTR_TO_NM			0.00054
#define FT_TO_MTR			0.3048
#define MTR_TO_FT			3.28084
#define DEG_TO_NM_LAT		60.0
#define NM_TO_DEG_LAT		(1.0 / 60.0)
#if !defined(PROJ_API_H)
#define RAD_TO_DEG			(180.0 / M_PI)
#define DEG_TO_RAD			(M_PI / 180.0)
#endif

// spherical earth or ellipsoid mean radius
#define DEG_TO_MTR_LAT		(6371100 * M_PI / 180.0)
#define MTR_TO_DEG_LAT		(180.0 / 6371100 * M_1_PI)

// once XP transitions to WGS84 ellipsoid, there are 2 relevant radii and conversions
// https://en.wikipedia.org/wiki/Geographical_distance#Ellipsoidal_Earth_projected_to_a_plane
// https://en.wikipedia.org/wiki/Earth_radius#/media/File:EarthEllipRadii.jpg
//
// to/from lattitude: prime vertical radius N, some 6378 to 6400 km
// to/from longitude: meridional radius M (use *cos(lat) as usual) 6335 to 6400 km
//
// to avoid trigonometric calculations, these two conversions can be aproximated to
// witin 0.06% error (to 60 deg lat) by linear interpolation

#define DEG_LAT_TO_MTR(lat) (6388000.0 + (lat-45.0)/45.0 * 11000.0)
#define DEG_LON_TO_MTR(lat) (6367000.0 + (lat-45.0)/45.0 * 35000.0)

enum rf_region {
	rf_usa = 0,
	rf_eu = 1
};


#endif
