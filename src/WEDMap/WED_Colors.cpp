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

#include "WED_Colors.h"
#include "GUI_Resources.h"
#include "AssertUtils.h"
#include "BitmapUtils.h"
#include "WED_EnumSystem.h"

vector<float>	colors;

#define	SWATCH_HEIGHT	10

float *		WED_Color_RGBA(WED_Color c)
{
	if (colors.empty())
	{
		struct ImageInfo	im;
		int result =  GUI_GetImageResource("colors.png", &im);
		if (result != 0)	AssertPrintf("Unable to open colors.png resource.");

		if (im.channels != 4) AssertPrintf("Error, expected alpha, but we have %d channels.", im.channels);

		colors.resize(wed_Last*4);
		int row_bytes = im.width * im.channels + im.pad;
		for (int n = 0; n < wed_Last; ++n)
		{
			unsigned char * ptr = im.data + row_bytes * (n*SWATCH_HEIGHT+SWATCH_HEIGHT/2) + im.channels * (SWATCH_HEIGHT/2);

			colors[n*4  ] = (float) ptr[2] / 255.0;
			colors[n*4+1] = (float) ptr[1] / 255.0;
			colors[n*4+2] = (float) ptr[0] / 255.0;
			colors[n*4+3] = (float) ptr[3] / 255.0;

		}

		DestroyBitmap(&im);
	}

	return &*colors.begin() + c * 4;
}

float *		WED_Color_RGBA_Alpha(WED_Color c, float alpha, float storage[4])
{
	float * raw = WED_Color_RGBA(c);
	if (alpha == 1.0 || storage == NULL) return raw;
	storage[0] = raw[0];
	storage[1] = raw[1];
	storage[2] = raw[2];
	storage[3] = raw[3] * alpha;
	return storage;
}

float *		WED_Color_Surface	(int surface, float alpha, float storage[4])
{
	switch(surface) {
	case surf_Asphalt:			return WED_Color_RGBA_Alpha(wed_Surface_Asphalt, alpha, storage);
	case surf_Concrete:			return WED_Color_RGBA_Alpha(wed_Surface_Concrete, alpha, storage);
	case surf_Grass:			return WED_Color_RGBA_Alpha(wed_Surface_Grass, alpha, storage);
	case surf_Dirt:				return WED_Color_RGBA_Alpha(wed_Surface_Dirt, alpha, storage);
	case surf_Gravel:			return WED_Color_RGBA_Alpha(wed_Surface_Gravel, alpha, storage);
	case surf_Lake:				return WED_Color_RGBA_Alpha(wed_Surface_DryLake, alpha, storage);
	case surf_Water:			return WED_Color_RGBA_Alpha(wed_Surface_Water, alpha, storage);
	case surf_Snow:				return WED_Color_RGBA_Alpha(wed_Surface_Snow, alpha, storage);
	case surf_Trans:			return WED_Color_RGBA_Alpha(wed_Surface_Transparent, alpha, storage);
	case shoulder_Asphalt:		return WED_Color_RGBA_Alpha(wed_Surface_Asphalt, alpha, storage);
	case shoulder_Concrete:		return WED_Color_RGBA_Alpha(wed_Surface_Concrete, alpha, storage);
	case shoulder_None:			return WED_Color_RGBA_Alpha(wed_Surface_Transparent, alpha, storage);
	default:
		AssertPrintf("Unknown surface %d\n", surface); return NULL;
	}
}
