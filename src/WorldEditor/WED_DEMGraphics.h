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
#ifndef WED_DEMGRAPHICS_H
#define WED_DEMGRAPHICS_H

#include "DEMDefs.h"
#include "BitmapUtils.h"

struct	Point2;
struct	Vector2;

enum {
	dem_Strata = -1000,
	dem_Shaded,
	dem_Enum,
	dem_Normals,
	dem_DDA,
	
	dem_StrataBiomass,
	dem_StrataRainfallYearly,
	dem_StrataTemperature,
	dem_StrataTemperatureRange,
	dem_StrataElevationRange,
	dem_StrataRelativeElevation,
	dem_StrataDrainage
};

int	DEMToBitmap(
				const DEMGeo& 	inDEM,
				ImageInfo&		outImage,
				int				inMode);
				
void ColorForValue(
				int				dem_type,
				float			value,
				unsigned char	rgb[3]);
				
void TensorDDA(
			ImageInfo&	ioImage,
			Vector2 (*	tensor_func)(const Point2& p, void * ref),
			void *		ref);

#endif