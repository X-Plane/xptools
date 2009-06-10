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

enum {
		shp_None			= 0,
		shp_Mode_Landuse	= 1,
		shp_Mode_Feature	= 2,
		shp_Mode_Road		= 4,
		shp_Mode_Simple		= 8,			// Use a simple feature desc for import.
		shp_Mode_Map		= 16,			// Use a map file for features.
		shp_Use_Crop		= 32,			// Add the crop-box to the import on the fly.
		shp_Overlay			= 64,			// Do not clear previous data.
		shp_Fast			= 128			// Assume data is well-formed, use fast path.  Can be dangerous!
};
typedef int shp_Flags;

// io_bounds - input: the crop box (if needed).  output: the shape file bounds.
// Note: if the shape file is importing faces (landuse/feature) and is NOT in overlay then "contained" flags on the faces describe the area that is
// inside the shapefile.  if the shapefile is in overlay mode...well, I think the same is true but who knows.

bool	ReadShapeFile(const char * in_file, Pmwx& out_map, shp_Flags mode, const char * feature_desc, double io_bounds[4], ProgressFunc	inFunc);

#endif /* ShapeIO_H */
