/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef TensorRoads_H
#define TensorRoads_H

#include "MapDefs.h"
struct	DEMGeo;
struct	ImageInfo;

#include "ProgressUtils.h"

struct	RoadPrefs_t {
	float		elevation_weight;
	float		radial_weight;
	float		density_amp;
	float		slope_amp;
};

extern	RoadPrefs_t gRoadPrefs;

void	BuildRoadsForFace(
					Pmwx&			ioMap,
					const DEMGeo&	inElevation,
					const DEMGeo&	inSlope,
					const DEMGeo&	inUrbanDensity,
					const DEMGeo&	inUrbanRadial,
					const DEMGeo&	inUrbanSquare,
					Face_handle 	inFace,
					ProgressFunc	inProg,
					ImageInfo *		ioTensorImage,
					double			outTensorBounds[4]);

#endif /* TensorRoads_H */
