/*
 * Copyright (c) 2008, Laminar Research.
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

#ifndef ShapeIO_H
#define ShapeIO_H

#include "MapDefs.h"
#include "ProgressUtils.h"
struct DEMGeo;

enum {
		shp_None			= 0,
		shp_Mode_Landuse	= 1,			// Polygon shape file, params go into the land use fields.
		shp_Mode_Feature	= 2,			// Polygon shape file, params go into the feature field
		shp_Mode_Coastline	= 3,			// Line shape file, right-side of vectors become param
		shp_Mode_Road		= 4,			// Line shape file, lines become roads

		shp_Mode_Simple		= 8,			// Use a simple feature desc for features.
		shp_Mode_Map		= 16,			// Use a map file for features.
		shp_Use_Crop		= 32,			// Add the crop-box to the import on the fly.
		shp_Overlay			= 64,			// Do not clear previous data.
		shp_ErrCheck		= 128			// Check for overlapping line segments, and fail if we find any.
};
typedef int shp_Flags;

// io_bounds - on input, this contains a bounding box that is used for cropping AND gridding.  On output, this contains the real bounds of the
// shape file.  Note that if the shape file contains no entities inside the crop box, the crop box is returned unchanged.

// Note: if the shape file is importing faces (landuse/feature) and is NOT in overlay then "contained" flags on the faces describe the area that is
// inside the shapefile.  if the shapefile is in overlay mode...well, I think the same is true but who knows.

bool	ReadShapeFile(
			const char *			in_file, 
			Pmwx&					out_map, 
			shp_Flags				mode, 
			const char *			feature_desc,			// name of feature to use or feature file map to use
			double					io_bounds[4],			// input: cropping box if desired.  output: actual map bounds.
			double					simplify_mtr,			// For line imports: if > 0, apply this many meters maximum erro douglas-peuker to reduce vertex count.
			int						grid_divisions,			// If > 0, granularity of the grid to apply.  This requires io_bounds to be set.
			ProgressFunc			inFunc);

bool	RasterShapeFile(
			const char *			inFile,
			DEMGeo&					outRaster,
			shp_Flags				mode,
			const char *			feature_desc,
			ProgressFunc			inFunc);

bool	WriteShapefile(
			const char *			in_file,
			Pmwx&					in_map,
			int						terrain_type,
			ProgressFunc			inFunc);

#endif /* ShapeIO_H */
