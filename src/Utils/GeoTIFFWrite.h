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
#ifndef GEOTIFF_WRITE_H
#define GEOTIFF_WRITE_H

#include <stdint.h>

// Geographic bounding box, WGS84 degrees. Rows are stored top-to-bottom
// (north -> south), columns left-to-right (west -> east).
struct GeoTIFFBounds {
    double west;
    double south;
    double east;
    double north;
};

// Write an 8-bit-per-channel raster (1, 3, or 4 channels) as a WGS84-geographic
// GeoTIFF. Photometric = MINISBLACK for 1-channel, RGB for 3+. Returns true
// on success.
bool WriteGeoTIFF_UInt8(const char* path, int width, int height, int channels,
                        const GeoTIFFBounds& bb, const uint8_t* data);

// Convenience: 3- or 4-channel RGB(A) at 8 bpc. Identical semantics to
// WriteGeoTIFF_UInt8 with channels in {3,4} and PHOTOMETRIC_RGB.
bool WriteGeoTIFF_RGBA(const char* path, int width, int height, int channels,
                       const GeoTIFFBounds& bb, const uint8_t* data);

// Single-band signed 16-bit elevation raster, MINISBLACK, with a nodata tag.
bool WriteGeoTIFF_Int16(const char* path, int width, int height,
                        const GeoTIFFBounds& bb, const int16_t* data,
                        int16_t nodata);

#endif
