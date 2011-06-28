/* 
 * Copyright (c) 2009, Laminar Research.
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

#ifndef MapRaster_H
#define MapRaster_H

#include "MapDefs.h"
#include "DEMDefs.h"

struct CoordTranslator2;

// This creates a planar map from a DEM - each contiguous (orthogonally connected, not diagonally connected) region of same values becomes a 
// face; DEM samples that have the "null post" value may be in the unbounded face if they aren't fully surrounded. When done, the face
// terrain type is copied from the DEM.
// Note: x1/y1 is the address of the lower left SAMPLE to use, and x2/y2 is the first sample NOT to use - that is, this is [) style range,
// no matter WHAT the DEM format - we are referring to samples, not lat/lon coords.
// So passing 0,0,mWidth,mHeight converts the entire DEM.
void	MapFromDEM(
				const DEMGeo&		in_dem,
				int					x1,
				int					y1,
				int					x2,
				int					y2,
				float				null_post,
				Pmwx&				out_map,
				CoordTranslator2 *	translator);

#endif /* MapRaster_H */
