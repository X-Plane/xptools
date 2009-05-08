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
#include "BWImage.h"
//#include "CGALTypes.h"
#include "PolyRasterUtils.h"
#if APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#if APL && DEV && !defined(__MACH__)
#include <agl.h>
#endif

#if !INLINING_BW

BWImage::BWImage() : mData(NULL), mBackup(NULL), mWidth(0), mHeight(0)
{
}

BWImage::BWImage(int width, int height) :
	mWidth(width),
	mHeight(height),
	mXLimit(width),
	mYLimit(height),
	mData((unsigned long *) malloc(width * height / 8)),
	mBackup((unsigned long *) malloc(width * height / 8))
{
	if (mData)
		memset(mData, 0, mWidth * mHeight / 8);
}


BWImage::BWImage(const BWImage& rhs) :
	mWidth(rhs.mWidth),
	mHeight(rhs.mHeight),
	mXLimit(rhs.mXLimit),
	mYLimit(rhs.mYLimit),
	mData((unsigned long *) malloc(rhs.mWidth * rhs.mHeight / 8)),
	mBackup((unsigned long *) malloc(rhs.mWidth * rhs.mHeight / 8))
{
	if (mData && rhs.mData)
		memcpy(mData, rhs.mData, mWidth * mHeight / 8);
	else if (mData)
		memset(mData, 0, mWidth * mHeight / 8);
}

BWImage::~BWImage()
{
	if (mData) free (mData);
	if (mBackup) free (mBackup);
}

BWImage& BWImage::operator=(const BWImage& rhs)
{
	if (mData) free(mData);
	if (mBackup) free(mBackup);
	mData = NULL;

	mWidth = rhs.mWidth;
	mHeight = rhs.mHeight;
	mXLimit = rhs.mXLimit;
	mYLimit = rhs.mYLimit;
	mData = (unsigned long *) malloc(mWidth * mHeight / 8);
	mBackup = (unsigned long *) malloc(mWidth * mHeight / 8);
	if (mData && rhs.mData)
		memcpy(mData, rhs.mData, mWidth * mHeight / 8);
	else if (mData)
		memset(mData, 0, mWidth * mHeight / 8);
	return *this;
}

#endif


#if INLINING_BW || DEV

BWINLINE void	BWImage::ClearBand(int y1, int y2)
{
	if (mData)
	{
		int len = (y2 - y1) * mWidth / 8;
		int off = y1 * mWidth / 8;
		memset(mData + off, 0, len);
	}
}



BWINLINE void			BWImage::RasterizeLocal(
						const vector<Polygon2>& inPolygon)
{
	bool	it = false;
	PolyRasterizer	rasterizer;

	int bottom = mHeight-1;
	int top = 0;

	for (vector<Polygon2>::const_iterator poly = inPolygon.begin(); poly != inPolygon.end(); ++poly)
	for (int i = 0; i < poly->size(); ++i)
	{
		int j = (i+1)%poly->size();
		if ((*poly)[i].y() != (*poly)[j].y())
		{
			if ((*poly)[i].y() < (*poly)[j].y())
				rasterizer.masters.push_back(PolyRasterSeg_t((*poly)[i].x(), (*poly)[i].y(),
												 			 (*poly)[j].x(), (*poly)[j].y()));
			else
				rasterizer.masters.push_back(PolyRasterSeg_t((*poly)[j].x(), (*poly)[j].y(),
												 			 (*poly)[i].x(), (*poly)[i].y()));
		}
		if ((*poly)[i].y() < bottom)
			bottom = (*poly)[i].y();
		if (ceil((*poly)[i].y()) > top)
			top = ceil((*poly)[i].y());
	}

	if (bottom < 0) 	bottom	= 0;
	if (top > mHeight)	top		= mHeight;

	// Optimization - if the user doesn't want us to ever draw and is really just hit testing,
	// there is no need to actually set pixels OR to do any save-restore!
	rasterizer.SortMasters();
	int y = bottom;
	rasterizer.StartScanline(y);

	while (!rasterizer.DoneScan())
	{
		int x1, x2;

		int scan_offset = mWidth * y / 32;
		while (rasterizer.GetRange(x1, x2))
		{
			if (x1 < 0) x1 = 0;
			if (x2 >= mWidth) x2 = mWidth;
			if (x1 < x2)
			{
				// Start word is the first word we will
				// write into...x_start_bit is where in the
				// word to start filling.
				int x_start_word = x1 / 32;
				int x_start_bit = x1 % 32;
				// Stop word is the first word we won't totally
				// fill.  stop_bit is the first non-touched bit
				// in that word (0 indicates the word is not touched at all.
				int x_stop_word = x2 / 32;
				int x_stop_bit = x2 % 32;

				unsigned long start_mask = ~((1 << x_start_bit)-1);
				unsigned long stop_mask = ((1 << x_stop_bit)-1);

				if (x_start_word == x_stop_word)
				{
					if (start_mask & stop_mask)
					{
						mData[scan_offset + x_start_word] |= (start_mask & stop_mask);
					}
				} else {
					if (start_mask)
						mData[scan_offset + x_start_word] |= start_mask;

					for (int i = x_start_word+1;i < x_stop_word; ++i)
					{
						mData[scan_offset + i] = 0xFFFFFFFF;
					}
					if (stop_mask)
					{
						mData[scan_offset + x_stop_word] |= stop_mask;
					}
				}
			}
		}
		++y;
		if (y >= mHeight) break;
		rasterizer.AdvanceScanline(y);
	}

}

BWINLINE bool			BWImage::RasterizeLocalStopConflicts(
						const vector<Polygon2>& inPolygon)
{
	PolyRasterizer	rasterizer;

	int bottom = mHeight-1;
	int top = 0;

	for (vector<Polygon2>::const_iterator poly = inPolygon.begin(); poly != inPolygon.end(); ++poly)
	for (int i = 0; i < poly->size(); ++i)
	{
		if ((*poly)[i].x() < 0 ||
			(*poly)[i].x() > mXLimit ||
			(*poly)[i].y() < 0 ||
			(*poly)[i].y() > mYLimit)
		{
			return true;
		}

		int j = (i+1)%poly->size();
		if ((*poly)[i].y() != (*poly)[j].y())
		{
			if ((*poly)[i].y() < (*poly)[j].y())
				rasterizer.masters.push_back(PolyRasterSeg_t((*poly)[i].x(), (*poly)[i].y(),
												 			 (*poly)[j].x(), (*poly)[j].y()));
			else
				rasterizer.masters.push_back(PolyRasterSeg_t((*poly)[j].x(), (*poly)[j].y(),
												 			 (*poly)[i].x(), (*poly)[i].y()));
		}
		if ((*poly)[i].y() < bottom)
			bottom = (*poly)[i].y();
		if (ceil((*poly)[i].y()) > top)
			top = ceil((*poly)[i].y());
	}

	if (bottom < 0) 	bottom	= 0;
	if (top > mHeight)	top		= mHeight;

	// Optimization - if the user doesn't want us to ever draw and is really just hit testing,
	// there is no need to actually set pixels OR to do any save-restore!

	StRestoreChunk chunk(
					(char *) mData,
					(char *) mBackup,
					mWidth,
					bottom, top,
					true);

	rasterizer.SortMasters();
	int y = bottom;
	rasterizer.StartScanline(y);

	while (!rasterizer.DoneScan())
	{
		int x1, x2;

		int scan_offset = mWidth * y / 32;
		while (rasterizer.GetRange(x1, x2))
		{
			if (x1 < 0) x1 = 0;
			if (x2 >= mWidth) x2 = mWidth;
			if (x1 < x2)
			{
				// Start word is the first word we will
				// write into...x_start_bit is where in the
				// word to start filling.
				int x_start_word = x1 / 32;
				int x_start_bit = x1 % 32;
				// Stop word is the first word we won't totally
				// fill.  stop_bit is the first non-touched bit
				// in that word (0 indicates the word is not touched at all.
				int x_stop_word = x2 / 32;
				int x_stop_bit = x2 % 32;

				unsigned long start_mask = ~((1 << x_start_bit)-1);
				unsigned long stop_mask = ((1 << x_stop_bit)-1);

				if (x_start_word == x_stop_word)
				{
					if (start_mask & stop_mask)
					{
						if (mData[scan_offset + x_start_word] & (start_mask & stop_mask))
							return true;
						mData[scan_offset + x_start_word] |= (start_mask & stop_mask);
					}
				} else {
					if (start_mask)
					{
						if (mData[scan_offset + x_start_word] & start_mask)
							return true;
						mData[scan_offset + x_start_word] |= start_mask;
					}

					for (int i = x_start_word+1;i < x_stop_word; ++i)
					{
						if (mData[scan_offset + i])
							return true;
						mData[scan_offset + i] = 0xFFFFFFFF;
					}
					if (stop_mask)
					{
						if (mData[scan_offset + x_stop_word] & stop_mask)
							return true;
						mData[scan_offset + x_stop_word] |= stop_mask;
					}
				}
			}
		}
		++y;
		if (y >= mHeight) break;
		rasterizer.AdvanceScanline(y);
	}

	chunk.Commit();
	return false;
}

BWINLINE bool			BWImage::RasterizeLocalCheck(
						const vector<Polygon2>& inPolygon)
{
	PolyRasterizer	rasterizer;

	int bottom = mHeight-1;
	int top = 0;

	for (vector<Polygon2>::const_iterator poly = inPolygon.begin(); poly != inPolygon.end(); ++poly)
	for (int i = 0; i < poly->size(); ++i)
	{
		if ((*poly)[i].x() < 0 ||
			(*poly)[i].x() > mXLimit ||
			(*poly)[i].y() < 0 ||
			(*poly)[i].y() > mYLimit)
		{
			return true;
		}

		int j = (i+1)%poly->size();
		if ((*poly)[i].y() != (*poly)[j].y())
		{
			if ((*poly)[i].y() < (*poly)[j].y())
				rasterizer.masters.push_back(PolyRasterSeg_t((*poly)[i].x(), (*poly)[i].y(),
												 			 (*poly)[j].x(), (*poly)[j].y()));
			else
				rasterizer.masters.push_back(PolyRasterSeg_t((*poly)[j].x(), (*poly)[j].y(),
												 			 (*poly)[i].x(), (*poly)[i].y()));
		}
		if ((*poly)[i].y() < bottom)
			bottom = (*poly)[i].y();
		if (ceil((*poly)[i].y()) > top)
			top = ceil((*poly)[i].y());
	}

	if (bottom < 0) 	bottom	= 0;
	if (top > mHeight)	top		= mHeight;

	rasterizer.SortMasters();
	int y = bottom;
	rasterizer.StartScanline(y);

	while (!rasterizer.DoneScan())
	{
		int x1, x2;

		int scan_offset = mWidth * y / 32;
		while (rasterizer.GetRange(x1, x2))
		{
			if (x1 < 0) x1 = 0;
			if (x2 >= mWidth) x2 = mWidth;
			if (x1 < x2)
			{
				// Start word is the first word we will
				// write into...x_start_bit is where in the
				// word to start filling.
				int x_start_word = x1 / 32;
				int x_start_bit = x1 % 32;
				// Stop word is the first word we won't totally
				// fill.  stop_bit is the first non-touched bit
				// in that word (0 indicates the word is not touched at all.
				int x_stop_word = x2 / 32;
				int x_stop_bit = x2 % 32;

				unsigned long start_mask = ~((1 << x_start_bit)-1);
				unsigned long stop_mask = ((1 << x_stop_bit)-1);

				if (x_start_word == x_stop_word)
				{
					if (start_mask & stop_mask)
					{
						if (mData[scan_offset + x_start_word] & (start_mask & stop_mask))
							return true;
					}
				} else {
					if (start_mask)
					{
						if (mData[scan_offset + x_start_word] & start_mask)
							return true;
					}

					for (int i = x_start_word+1;i < x_stop_word; ++i)
					{
						if (mData[scan_offset + i])
							return true;
					}
					if (stop_mask)
					{
						if (mData[scan_offset + x_stop_word] & stop_mask)
							return true;
					}
				}
			}
		}
		++y;
		if (y >= mHeight) break;
		rasterizer.AdvanceScanline(y);
	}
	return false;
}

#endif

#if !INLINING_BW

#if APL && DEV && !defined(__MACH__)
void BWImage::Debug()
{
	#if BIG
	for (int i = 0; i < (mWidth * mHeight / 32); ++i)
	{
		mData[i] = Endian32_Swap(mData[i]);
	}
	#endif
	XPLMSetGraphicsState(0, 0, 0,   0, 0,  0, 0);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	glVertex2i(0,0);
	glVertex2i(0,mHeight);
	glVertex2i(mWidth,mHeight);
	glVertex2i(mWidth,0);
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2i(0, 0);
	glPixelStorei(GL_UNPACK_LSB_FIRST, 1);
	glBitmap(mWidth, mHeight, 0,0, 0, 0, (unsigned char *) mData);
	aglSwapBuffers(aglGetCurrentContext());
	while (!Button()) { }
	while (Button()) { }
	glPixelStorei(GL_UNPACK_LSB_FIRST, 0);
	#if BIG
	for (int i = 0; i < (mWidth * mHeight / 32); ++i)
	{
		mData[i] = Endian32_Swap(mData[i]);
	}
	#endif
}
#endif

#endif
