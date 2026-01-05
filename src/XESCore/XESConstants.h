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
#define EARTH_MEAN_RADIUS	6371008.0
#define DEG_TO_MTR_LAT		(DEG_TO_RAD * EARTH_MEAN_RADIUS)
#define MTR_TO_DEG_LAT		(RAD_TO_DEG / EARTH_MEAN_RADIUS)

// X-Plane 12+ uses a GRS80 ellipsoid, there are 2 relevant radii
// https://en.wikipedia.org/wiki/Geographical_distance#Ellipsoidal_Earth_projected_to_a_plane
// https://en.wikipedia.org/wiki/Earth_radius#/media/File:EarthEllipRadii.jpg

#define EARTH_EQ_RADIUS		6378137.0
#define EARTH_EPS2			0.0066945

enum rf_region {
	rf_usa = 0,
	rf_eu = 1
};


#endif
