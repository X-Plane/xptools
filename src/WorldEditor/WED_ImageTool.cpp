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
#include "WED_ImageTool.h"
#include "XPLMGraphics.h"
#include "WED_MapZoomer.h"
#include "PlatformUtils.h"
#include "TexUtils.h"
#include "GISUtils.h"
#include "Terraserver.h"
#include "BitmapUtils.h"
#include "WED_Progress.h"
#if IBM
#include <windows.h>
#endif
#include "PCSBSocket.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

static const DragHandleInfo_t kHandleInfos[4] = {
{ /* BL */	-1, -1,		1,	1		},
{ /* BR */	 1,	-1,		1,	1		},
{ /* TL */	-1,  1,		1,	1		},
{ /* TR */	 1,	 1,		1,	1		} };

static const char * kFieldNames[] = { "SWLon", "SWLat", "SELon", "SELat", "NWLon", "NWLat", "NELon", "NELat" };
static const char * kBtnNames[] = { "Open", "Show/Hide", "Clear", "All", "Visible", "Fetch" };

inline double	Interp2(double frac, double sml, double big)
{
	return sml + frac * (big - sml);
}

static void	MapQuads(	double old_bounds[4][2], double old_sub[4][2],
					double new_bounds[4][2], double new_sub[4][2])
{
	double A = old_bounds[1][0] - old_bounds[0][0];
	double B = old_bounds[3][0] - old_bounds[0][0];
	double C = old_bounds[1][1] - old_bounds[0][1];
	double D = old_bounds[3][1] - old_bounds[0][1];

	for (int n = 0; n < 4; ++n)
	{
		double i = (D * (old_sub[n][0] - old_bounds[0][0]) - B * (old_sub[n][1] - old_bounds[0][1])) / (A * D - B * C);
		double j = (C * (old_sub[n][0] - old_bounds[0][0]) - A * (old_sub[n][1] - old_bounds[0][1])) / (B * C - D * A);

		new_sub[n][0] = Interp2(j,
							Interp2(i, new_bounds[0][0],new_bounds[1][0]),
							Interp2(i, new_bounds[3][0],new_bounds[2][0]));
		new_sub[n][1] = Interp2(i,
							Interp2(j, new_bounds[0][1],new_bounds[3][1]),
							Interp2(j, new_bounds[1][1],new_bounds[2][1]));
	}
}

WED_ImageTool::WED_ImageTool(WED_MapZoomer * inZoomer) :
	WED_MapTool(inZoomer),
	mHandles(4, kHandleInfos, 4, this),
	mVisible(false),
	mBits(false)
{
	XPLMGenerateTextureNumbers(&mTexID, 1);
}

WED_ImageTool::~WED_ImageTool()
{
	if (mBits)
		glDeleteTextures(1, (GLuint *) &mTexID);
}

void	WED_ImageTool::DrawFeedbackUnderlay(
		bool				inCurrent)
{
}

void	WED_ImageTool::DrawFeedbackOverlay(
		bool				inCurrent)
{
	if (mVisible && mBits)
	{
		XPLMSetGraphicsState(0, 1, 0,    0, 1,  0, 0);
		glColor4f(1.0, 1.0, 1.0, 0.5);
		XPLMBindTexture2d(mTexID, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[0]),
					GetZoomer()->LatToYPixel(mCoords[1]));
		glTexCoord2f(0.0, 1.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[4]),
					GetZoomer()->LatToYPixel(mCoords[5]));
		glTexCoord2f(1.0, 1.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[6]),
					GetZoomer()->LatToYPixel(mCoords[7]));
		glTexCoord2f(1.0, 0.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[2]),
					GetZoomer()->LatToYPixel(mCoords[3]));
		glEnd();
	}

	if (inCurrent && mVisible)
	{
		XPLMSetGraphicsState(0, 0, 0,    0, 0,  0, 0);
		glColor3f(0.0, 1.0, 0.3);
		glBegin(GL_QUADS);
		for (int n = 0; n < 4; ++n)
			mHandles.DrawHandle(n);
		glEnd();
		glBegin(GL_LINES);
		mHandles.ConnectHandle(0, 1);	// bottom
		mHandles.ConnectHandle(2, 3);	// top
		mHandles.ConnectHandle(0, 2);	// left
		mHandles.ConnectHandle(1, 3);	// right
		glEnd();
	}
}

bool	WED_ImageTool::HandleClick(
		XPLMMouseStatus		inStatus,
		int 				inX,
		int 				inY,
		int 				inButton)
{
	if (!mVisible) return false;
	if (inButton != 0) return false;
	switch(inStatus) {
	case xplm_MouseDown:
		return mHandles.StartDrag(inX, inY);
	case xplm_MouseDrag:
		mHandles.ContinueDrag(inX, inY);
		return true;
	case xplm_MouseUp:
		mHandles.EndDrag(inX, inY);
		return true;
	}
	return false;
}

#pragma mark -

int		WED_ImageTool::GetNumProperties(void)
{
	return 8;
}

void	WED_ImageTool::GetNthPropertyName(int n, string& s)
{
	s = kFieldNames[n];
}

double	WED_ImageTool::GetNthPropertyValue(int n)
{
	return mCoords[n];
}

void	WED_ImageTool::SetNthPropertyValue(int n, double v)
{
	mCoords[n] = v;
}

int		WED_ImageTool::GetNumButtons(void)
{
	return 6;
}

void	WED_ImageTool::GetNthButtonName(int n, string& s)
{
	s = kBtnNames[n];
}

void	WED_ImageTool::NthButtonPressed(int n)
{
	char	buf[1024];
	switch (n) {
	case 0:	/* OPEN */
		if (GetFilePathFromUser(getFile_Open, "Pick a bitmap", "Open", 1, buf, sizeof(buf)))
		{
			XPLMBindTexture2d(mTexID, 0);
			if (LoadTextureFromFile(buf, mTexID, tex_Rescale + tex_Linear + tex_Mipmap, NULL, NULL, NULL, NULL))
			{
				mVisible = true;
				mBits = true;
				mFile = buf;
				if (!FetchTIFFCorners(buf, mCoords))
				{
					GetZoomer()->GetMapLogicalBounds(mCoords[0], mCoords[1], mCoords[6], mCoords[7]);
					GetZoomer()->GetMapLogicalBounds(mCoords[4], mCoords[3], mCoords[2], mCoords[5]);
				}
			}
		}
		break;
	case 1: /* SHOW/HIDE */
		mVisible = !mVisible;
		break;
	case 2: /* CLEAR */
		if (mBits)
		{
			glDeleteTextures(1, (GLuint *) &mTexID);
			mBits = false;
		}
		break;
	case 3: /* ALL */
		GetZoomer()->GetMapLogicalBounds(mCoords[0], mCoords[1], mCoords[6], mCoords[7]);
		GetZoomer()->GetMapLogicalBounds(mCoords[4], mCoords[3], mCoords[2], mCoords[5]);
		break;
	case 4: /* VISIBLE */
		GetZoomer()->GetMapVisibleBounds(mCoords[0], mCoords[1], mCoords[6], mCoords[7]);
		GetZoomer()->GetMapVisibleBounds(mCoords[4], mCoords[3], mCoords[2], mCoords[5]);
		break;
	case 5: /* FETCH */
		GetOrthoPhotos();
		mVisible = true;
		mBits = true;
		break;
	}
}

char *	WED_ImageTool::GetStatusText(void)
{
	return NULL;
}

#pragma mark -

double		WED_ImageTool::UIToLogX(double v) const
{
	return GetZoomer()->XPixelToLon(v);
}

double		WED_ImageTool::UIToLogY(double v) const
{
	return GetZoomer()->YPixelToLat(v);
}

double		WED_ImageTool::LogToUIX(double v) const
{
	return GetZoomer()->LonToXPixel(v);
}

double		WED_ImageTool::LogToUIY(double v) const
{
	return GetZoomer()->LatToYPixel(v);
}

double		WED_ImageTool::GetHandleX(int inHandle) const
{
	return mCoords[inHandle * 2];
}

double		WED_ImageTool::GetHandleY(int inHandle) const
{
	return mCoords[inHandle * 2 + 1];
}

void		WED_ImageTool::MoveHandleX(int h, double v)
{
	mCoords[h * 2] += v;
}

void		WED_ImageTool::MoveHandleY(int h, double v)
{
	mCoords[h * 2 + 1] += v;
}

#pragma mark -

void		WED_ImageTool::GetOrthoPhotos(void)
{
	int texSize = 1024;
	const char * scale = "16m";
	const char * theme = "1";	// 1 = photo, 4 = urban
	int good = 0, fail = 0;

	int n;
	double lonwest = mCoords[0], loneast = mCoords[0];
	double latsouth = mCoords[1], latnorth = mCoords[1];

	for (n = 1; n < 4; ++n)
	{
		lonwest = min(lonwest, mCoords[n*2]);
		loneast = max(loneast, mCoords[n*2]);
		latsouth = min(latsouth, mCoords[n*2+1]);
		latnorth = max(latnorth, mCoords[n*2+1]);
	}

#if ALLOW_RESCALING
	bool	rescale = false;
	if (latsouth >  90.0 ||
		latsouth < -90.0 ||
		latnorth >  90.0 ||
		latnorth < -90.0 ||
		loneast >  180.0 ||
		loneast < -180.0 ||
		lonwest >  180.0 ||
		lonwest < -180.0)
	{
		rescale = true;
		latsouth /= 1000000.0;
		latnorth /= 1000000.0;
		loneast /= 1000000.0;
		lonwest /= 1000000.0;
	}
#endif

	double	maxDimDegs = max(latnorth - latsouth, loneast - lonwest);
	double	lenPixel = maxDimDegs * 114000.0 / 2048.0;
	if (lenPixel < 4.0)
		scale = "4m";
	if (lenPixel < 1.0)
		scale = "1m";

	int tiles[4][3];

	if (GetTilesForArea(scale, theme, latsouth, lonwest, latnorth, loneast, tiles) == 0)
	{
		int domain = tiles[0][2];
		int	max_x = max(tiles[1][0], tiles[2][0]);
		int min_x = min(tiles[0][0], tiles[3][0]);
		int max_y = max(tiles[0][1], tiles[1][1]);
		int min_y = min(tiles[2][1], tiles[3][1]);

		int	x_size = (max_x - min_x + 1) * 200;
		int y_size = (max_y - min_y + 1) * 200;

		ImageInfo	image;
		CreateNewBitmap(x_size, y_size, 3, &image);

			double	coords[4][2];	// Location of any one tile
			double	bounds[4][2];	// Location of our entire bitmap
			double	env[4][2];		// Location of the subsection we want to resample
			double 	src[4][2];		// Location of the whole bitmap as pixels
			double	pixels[4][2];	// Location of the subsection as pixels

		bool	exists = true;
		if (FetchTilePositioning(scale, theme, domain, min_x, max_y, coords) != 0) exists = false;
		bounds[0][0] = coords[0][0];	bounds[0][1] = coords[0][1];
		if (FetchTilePositioning(scale, theme, domain, max_x, max_y, coords) != 0) exists = false;
		bounds[1][0] = coords[1][0];	bounds[1][1] = coords[1][1];
		if (FetchTilePositioning(scale, theme, domain, max_x, min_y, coords) != 0) exists = false;
		bounds[2][0] = coords[2][0];	bounds[2][1] = coords[2][1];
		if (FetchTilePositioning(scale, theme, domain, min_x, min_y, coords) != 0) exists = false;
		bounds[3][0] = coords[3][0];	bounds[3][1] = coords[3][1];

		env[0][0] = mCoords[5];
		env[0][1] = mCoords[4];
		env[1][0] = mCoords[7];
		env[1][1] = mCoords[6];
		env[2][0] = mCoords[3];
		env[2][1] = mCoords[2];
		env[3][0] = mCoords[1];
		env[3][1] = mCoords[0];

#if ALLOW_RESCALING
		if (rescale)
		for (n = 0; n < 4; ++n)
		{
			env[n][0] /= 1000000.0;
			env[n][1] /= 1000000.0;
		}
#endif

		src[0][1] = 0.0;			src[0][0] = image.height;
		src[1][1] = image.width;	src[1][0] = image.height;
		src[2][1] = image.width;	src[2][0] = 0;
		src[3][1] = 0.0;			src[3][0] = 0;

		if (exists)
		{
			char	buf[512];
			sprintf(buf, "Fetching %d x %d pixels at %s ppm", x_size, y_size, scale);
			for (int x = min_x; x <= max_x; ++x)
			for (int y = min_y; y <= max_y; ++y)
			{
				double so_far = (y - min_y) + (x - min_x) * (max_y - min_y + 1);
				double total = (max_x - min_x + 1) * (max_y - min_y + 1);
				if(total > 0)
					WED_ProgressFunc(0, 1, buf, so_far / total);

				for (int n = 0; n < 4; ++n)
				{
					if (FetchTile(scale, theme, domain, x, y, &image, (x - min_x) * 200, (y - min_y) * 200)==0)
						break;
					fail++;
				}
				good++;
			}
			WED_ProgressFunc(0, 1, buf, 1.0);

			MapQuads(bounds, env, src, pixels);

	//		HackShowRects(bounds, env, src, pixels);

			ImageInfo	final;
			CreateNewBitmap(texSize, texSize, 3, &final);

			CopyBitmapSectionWarped(&image, &final,
				pixels[3][1], pixels[3][0],
				pixels[2][1], pixels[2][0],
				pixels[1][1], pixels[1][0],
				pixels[0][1], pixels[0][0],
				0, 0, texSize, texSize);

			XPLMBindTexture2d(mTexID, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
					texSize, texSize, 0,
					GL_RGB,
					GL_UNSIGNED_BYTE,
					final.data);

			DestroyBitmap(&final);
		}

		DestroyBitmap(&image);

	}
}