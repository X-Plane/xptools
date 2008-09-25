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
#ifndef BWIMAGE_H
#define BWIMAGE_H

#include "CompGeomDefs2.h"
#include "CompGeomUtils.h"

struct	StRestoreChunk {
	char *			mDst;
	char *			mSrc;
	int				mLength;
	bool			mRestore;

	StRestoreChunk(char * inSrc, char * inDst, int width, int bot, int top, bool save)
	{
		mRestore = true;
		int rowb = width / 8;
		mLength = (top - bot) * rowb;
		mSrc = inSrc + rowb * bot;
		mDst = inDst + rowb * bot;
		if (save)
			memcpy(mDst, mSrc, mLength);
		else
			mRestore = false;
	}

	void	Commit(void) { mRestore = false; }
	~StRestoreChunk()
	{
		if (mDst && mSrc && mRestore && mLength)
		{
			memcpy(mSrc, mDst, mLength);
		}
	}
};

#if DEV
#define BWINLINE
#else
#define BWINLINE inline
#endif

struct	BWImage {

	int				mWidth;
	int				mHeight;

	int				mXLimit;
	int				mYLimit;

	unsigned long *	mData;
	unsigned long * mBackup;

	BWImage();
	BWImage(int width, int height);
	BWImage(const BWImage&);
	~BWImage();
	BWImage& operator=(const BWImage&);

	BWINLINE void		ClearBand(int y1, int y2);
	// This routine always blasts in the polygon, period.
	BWINLINE void			RasterizeLocal(
						const vector<Polygon2>& inPolygon);
	// This routine draws the polygon, but aborts if it won't fit.
	BWINLINE bool		RasterizeLocalStopConflicts(
						const vector<Polygon2>& inPolygon);
	// This routine checks to see if it will fit but makes no changes.
	BWINLINE bool		RasterizeLocalCheck(
						const vector<Polygon2>& inPolygon);

#if APL && DEV
	void Debug();
#endif
};

#if !DEV
#define INLINING_BW 1
#include "BWImage.cpp"
#undef INLINING_BW
#endif

#endif
