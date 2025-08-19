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
#ifndef DEMIO_H
#define DEMIO_H

enum {
	dem_want_Post,	// Use pixel=post sampling
	dem_want_Area,	// Use area-pixel sampling!
	dem_want_File	// Use whatever the file has.
};

struct	DEMSpec {
	// ASSUMED: row major, north-west corner is data start.  Nearly all public DEM formats except for LR's are like htis.
	double			mWest;					// Outer bbox of DEM
	double			mSouth;
	double			mEast;
	double			mNorth;					
	int				mPost;					// Post vs. area pixels
	int				mWidth;					// Image dimensions
	int				mHeight;
	int				mBits;					// Bits per post (8, 16 or 32)
	bool			mBigEndian;				// True if big-endian, false for little endian
	bool			mFloat;					// True if floating point, false for integral types
	float			mNoData;				// No-data value, usually -9999 or -32768
	int				mHeaderBytes;			// Pre-data header size - usually 0, except for oz floats
};

#endif
