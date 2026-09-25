/*
 * Copyright (c) 2026, Laminar Research.
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

#include "WED_DEMGraphics.h"

#include "DEMDefs.h"
#include "BitmapUtils.h"
#include "MathUtils.h"

#include <math.h>

// FAA sectional-style elevation tint table (mirror of kTerBands in
// WED_StructureLayer.cpp). Kept locally so this file does not depend on
// the structure layer. Elevation is in feet.
namespace {
struct ter_band_t { float ft; float r, g, b; };
const ter_band_t kBands[] = {
	{     0.f, 0.55f, 0.72f, 0.45f },
	{   500.f, 0.70f, 0.80f, 0.48f },
	{  1000.f, 0.83f, 0.85f, 0.55f },
	{  2000.f, 0.92f, 0.86f, 0.62f },
	{  3000.f, 0.92f, 0.78f, 0.55f },
	{  5000.f, 0.88f, 0.66f, 0.45f },
	{  7000.f, 0.78f, 0.52f, 0.35f },
	{  9000.f, 0.65f, 0.42f, 0.28f },
	{ 12000.f, 0.50f, 0.32f, 0.22f },
	{ 14000.f, 0.42f, 0.30f, 0.32f },
};
const int kNBands = sizeof(kBands) / sizeof(kBands[0]);

inline void sectional_color(float elev_ft, float& r, float& g, float& b)
{
	if (elev_ft <= kBands[0].ft)         { r = kBands[0].r;         g = kBands[0].g;         b = kBands[0].b;         return; }
	if (elev_ft >= kBands[kNBands-1].ft) { r = kBands[kNBands-1].r; g = kBands[kNBands-1].g; b = kBands[kNBands-1].b; return; }
	for (int i = 1; i < kNBands; ++i)
		if (elev_ft <= kBands[i].ft)
		{
			float t = (elev_ft - kBands[i-1].ft) / (kBands[i].ft - kBands[i-1].ft);
			r = kBands[i-1].r + t * (kBands[i].r - kBands[i-1].r);
			g = kBands[i-1].g + t * (kBands[i].g - kBands[i-1].g);
			b = kBands[i-1].b + t * (kBands[i].b - kBands[i-1].b);
			return;
		}
}

} // anonymous namespace

int DEMToShadedSectionalBitmap(const DEMGeo& inDEM, int outTargetW, int outTargetH,
							   float clip_m, ImageInfo& outImage)
{
	if (outTargetW < 1) outTargetW = 1;
	if (outTargetH < 1) outTargetH = 1;
	if (outTargetW > inDEM.mWidth)  outTargetW = inDEM.mWidth;
	if (outTargetH > inDEM.mHeight) outTargetH = inDEM.mHeight;

	int err = CreateNewBitmap(outTargetW, outTargetH, 3, &outImage);
	if (err) return err;
	outImage.pad = 0;

	// Metric scale of one DEM cell, used so the shading is independent of
	// downsample factor (gradient_x/y of the source DEM are per-source-cell).
	const float x_m_per_src = (float) inDEM.x_dist_to_m(1);
	const float y_m_per_src = (float) inDEM.y_dist_to_m(1);

	// NW sun, alt 45 deg, in (+x=east, +y=north, +z=up): toward-light vector.
	const float Lx = -0.5f, Ly = 0.5f, Lz = 0.7071068f;

	// Source-cell indices for output pixel (ox, oy). Center-of-pixel sampling.
	const double sx_scale = (double) inDEM.mWidth  / (double) outTargetW;
	const double sy_scale = (double) inDEM.mHeight / (double) outTargetH;

	for (int oy = 0; oy < outTargetH; ++oy)
	{
		int sy = (int) ((oy + 0.5) * sy_scale);
		if (sy < 0) sy = 0; else if (sy >= inDEM.mHeight) sy = inDEM.mHeight - 1;

		unsigned char * row = outImage.data + oy * outImage.width * 3;
		for (int ox = 0; ox < outTargetW; ++ox)
		{
			int sx = (int) ((ox + 0.5) * sx_scale);
			if (sx < 0) sx = 0; else if (sx >= inDEM.mWidth) sx = inDEM.mWidth - 1;

			float elev = inDEM.get(sx, sy);
			unsigned char * px = row + ox * 3;

			// ImageInfo data is BGR-ordered (codebase convention; see
			// LoadTextureFromImage which uploads via GL_BGR/GL_BGRA).
			if (elev == DEM_NO_DATA)
			{
				// Magenta marker only for actual missing data. clip_m is honored
				// by the Phase 2 point overlay, not by the texture bake -- a sea-
				// level DEM should still render as sectional green here.
				px[0] = 255; px[1] = 0; px[2] = 255;
				continue;
			}
			(void) clip_m;

			float r, g, b;
			sectional_color(elev * 3.28084f, r, g, b);

			// Relief: surface normal from source-DEM-cell forward differences.
			float h  = elev;
			float ha = inDEM.get(sx, sy + 1);
			float hr = inDEM.get(sx + 1, sy);
			if (ha != DEM_NO_DATA && hr != DEM_NO_DATA)
			{
				float dh_dx = (hr - h) / x_m_per_src;
				float dh_dy = (ha - h) / y_m_per_src;
				float il = 1.f / sqrtf(dh_dx*dh_dx + dh_dy*dh_dy + 1.f);
				float nx = -dh_dx * il, ny = -dh_dy * il, nz = il;
				float dot = nx*Lx + ny*Ly + nz*Lz;
				float shade = fltlim(dot, 0.7f, 1.f);
				r *= shade; g *= shade; b *= shade;
			}

			px[0] = (unsigned char) fltlim(b * 255.f, 0.f, 255.f);
			px[1] = (unsigned char) fltlim(g * 255.f, 0.f, 255.f);
			px[2] = (unsigned char) fltlim(r * 255.f, 0.f, 255.f);
		}
	}

	return 0;
}
