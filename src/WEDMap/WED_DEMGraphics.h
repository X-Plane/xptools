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

#ifndef WED_DEMGraphics_H
#define WED_DEMGraphics_H

struct DEMGeo;
struct ImageInfo;

// Bakes a DEM into a 3-channel RGB ImageInfo:
//   * elevation -> FAA-sectional color band (matches kTerBands in WED_StructureLayer.cpp)
//   * NW-sun relief shading multiplied in (light dir (-0.5, +0.5, +sqrt(0.5)))
//   * cells with elevation <= clip_m are written as magenta voids
//
// clip_m is in DEM units (meters). outTargetW / outTargetH are the requested bitmap
// dimensions in pixels; callers should cap these at GL_MAX_TEXTURE_SIZE. The DEM is
// point-sampled at the target resolution (a simple inline downsample), so no
// separate ResampleDEM pass is needed.
//
// Returns 0 on success, nonzero on bitmap-allocation failure. Caller owns outImage
// and must DestroyBitmap when done with it.
int DEMToShadedSectionalBitmap(const DEMGeo& inDEM, int outTargetW, int outTargetH,
							   float clip_m, ImageInfo& outImage);

#endif /* WED_DEMGraphics_H */
