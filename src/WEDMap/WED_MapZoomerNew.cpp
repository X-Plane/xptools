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
#include "WED_MapZoomerNew.h"
#include "GUI_Messages.h"
#include "XESConstants.h"

#include "AssertUtils.h"

#if APL
#include <OpenGL/gl.h>
#else
#include "glew.h"
#endif

WED_MapZoomerNew::WED_MapZoomerNew()
{
	mCacheKey = 0;
	mPixels[0] = 0.0;
	mPixels[1] = 0.0;
	mPixels[2] = 1.0;
	mPixels[3] = 1.0;
	mLogicalBounds[0] = -180.0;
	mLogicalBounds[1] =  -90.0;
	mLogicalBounds[2] =  180.0;
	mLogicalBounds[3] =   90.0;

	mPixel2DegLat = 1.0;
	mLonCenter = mLatCenter = 0.0;
	UpdateProjection();
	RecalcAspectRatio();

}

WED_MapZoomerNew::~WED_MapZoomerNew()
{
}

double	WED_MapZoomerNew::XPixelToLon(double x) const
{
	return mProjection.XToLon(x);
}

double	WED_MapZoomerNew::YPixelToLat(double y) const
{
	return mProjection.YToLat(y);
}

double	WED_MapZoomerNew::LonToXPixel(double lon) const
{
	return mProjection.LonToX(lon);
}

double	WED_MapZoomerNew::LatToYPixel(double lat) const
{
	return mProjection.LatToY(lat);
}

Point2	WED_MapZoomerNew::PixelToLL(const Point2& p) const
{
	return Point2(XPixelToLon(p.x()), YPixelToLat(p.y()));
}

Point2	WED_MapZoomerNew::LLToPixel(const Point2& p) const
{
	return Point2(LonToXPixel(p.x()), LatToYPixel(p.y()));
}

void	WED_MapZoomerNew::PixelToLLv(Point2 * dst, const Point2 * src, int n) const
{
	while(n--)
		*dst++ = PixelToLL(*src++);
}

void	WED_MapZoomerNew::LLToPixelv(Point2 * dst, const Point2 * src, int n) const
{
	while(n--)
		*dst++ = LLToPixel(*src++);
}

double	WED_MapZoomerNew::GetPPM(void) const
{
	return mProjection.XYUnitsPerMeter();
}

double WED_MapZoomerNew::GetClickRadius(double p) const
{
	return fabs(YPixelToLat(p) - YPixelToLat(0));
}


void	WED_MapZoomerNew::SetPixelBounds(
					double 	inLeft,
					double	inBottom,
					double	inRight,
					double	inTop)
{
	++mCacheKey;
	mPixels[0] = inLeft;
	mPixels[1] = inBottom;
	mPixels[2] = inRight;
	mPixels[3] = inTop;
	UpdateProjection();
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}

void	WED_MapZoomerNew::SetMapLogicalBounds(
					double	inWest,
					double	inSouth,
					double	inEast,
					double	inNorth)
{
	++mCacheKey;
	mLogicalBounds[0] = inWest;
	mLogicalBounds[1] = inSouth;
	mLogicalBounds[2] = inEast;
	mLogicalBounds[3] = inNorth;
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}


void	WED_MapZoomerNew::GetPixelBounds(
					double& outLeft,
					double&	outBottom,
					double&	outRight,
					double&	outTop)
{
	outLeft = mPixels[0];
	outBottom = mPixels[1];
	outRight = mPixels[2];
	outTop = mPixels[3];
}

void	WED_MapZoomerNew::GetMapVisibleBounds(
					double&	outWest,
					double&	outSouth,
					double&	outEast,
					double&	outNorth)
{
	outWest = XPixelToLon	(mPixels[0]);
	outSouth = YPixelToLat	(mPixels[1]);
	outEast = XPixelToLon	(mPixels[2]);
	outNorth = YPixelToLat	(mPixels[3]);
}

void	WED_MapZoomerNew::GetMapLogicalBounds(
					double&	outWest,
					double&	outSouth,
					double&	outEast,
					double&	outNorth)
{
	outWest	= 	mLogicalBounds[0];
	outSouth=	mLogicalBounds[1];
	outEast	=	mLogicalBounds[2];
	outNorth=	mLogicalBounds[3];
}


//void	WED_MapZoomerNew::SetAspectRatio(double a)
//{
//	mAspectRatio = a;
//}

void	WED_MapZoomerNew::ZoomShowAll(void)
{
	ZoomShowArea(mLogicalBounds[0],mLogicalBounds[1],mLogicalBounds[2],mLogicalBounds[3]);
}

void	WED_MapZoomerNew::ZoomShowArea(
							double	inWest,
							double	inSouth,
							double	inEast,
							double	inNorth)
{
	++mCacheKey;
	mLonCenter = (inWest + inEast) * 0.5;
	mLatCenter = (inSouth + inNorth) * 0.5;

	double required_width_logical = inEast - inWest;
	double required_height_logical = inNorth - inSouth;
	
	if(required_width_logical == 0)
		required_width_logical = 0.00001;
	
	if(required_height_logical == 0)
		required_height_logical = 0.00001;
		
	double pix_avail_width = mPixels[2] - mPixels[0];
	double pix_avail_height = mPixels[3] - mPixels[1];

	double scale_for_vert = required_height_logical / pix_avail_height;
	double scale_for_horz = required_width_logical / pix_avail_width * mLonCenterCOS;

	mPixel2DegLat = max(scale_for_vert,scale_for_horz);
	UpdateProjection();
	RecalcAspectRatio();

	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}


void	WED_MapZoomerNew::PanPixels(
					double	x1,
					double	y1,
					double	x2,
					double	y2)
{
	++mCacheKey;

	// This is straight-forward: calculate the difference in lat/lon numbers between
	// the two pixels and change.  Positive numbers mean the user dragged up/right
	// therefore the map's bounds are decreasing!  Whacky, eh?
	double delta_lon = XPixelToLon(x2) - XPixelToLon(x1);
	double delta_lat = YPixelToLat(y2) - YPixelToLat(y1);

	mLatCenter -= delta_lat;
	mLonCenter -= delta_lon;
	UpdateProjection();
	RecalcAspectRatio();

	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}

void	WED_MapZoomerNew::ZoomAround(
					double	zoomFactor,
					double	centerXPixel,
					double	centerYPixel)
{
	++mCacheKey;

	// Zoom the map around a point.  We do this in three steps because I am lazy:
	// 1. Scroll the map so that we are zooming around the lower left corner.
	// 2. Zoom the map by adjusting only the top and right logical bounds, not
	//    the lower left.
	// 3. Scroll the map back.

	double px = (mPixels[0]+mPixels[2]) * 0.5;
	double py = (mPixels[1]+mPixels[3]) * 0.5;

	PanPixels(centerXPixel, centerYPixel, px,py);

	if (zoomFactor <= 1.0 || mPixel2DegLat > 1e-8) // limit manual zoom in to 1 mm/pixel (108,900 meter / deg lat)
		mPixel2DegLat /= zoomFactor;
	UpdateProjection();
	RecalcAspectRatio();

	PanPixels(px,py, centerXPixel, centerYPixel);
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}

/*
void	WED_MapZoomerNew::ScrollReveal(
				double	inLon,
				double	inLat)
{
	double	delta_lon = inLon - ((mVisibleBounds[0] + mVisibleBounds[2]) * 0.5);
	double	delta_lat = inLat - ((mVisibleBounds[1] + mVisibleBounds[3]) * 0.5);
	mVisibleBounds[0] += delta_lon;
	mVisibleBounds[1] += delta_lat;
	mVisibleBounds[2] += delta_lon;
	mVisibleBounds[3] += delta_lat;
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);

}


void	WED_MapZoomerNew::ScrollReveal(
				double	inWest,
				double	inSouth,
				double	inEast,
				double	inNorth)
{
	if (inWest == inEast || inNorth == inSouth)
	{
		ScrollReveal((inWest + inEast) * 0.5, (inNorth + inSouth) * 0.5);
		return;
	}
	double	width = (inEast - inWest) * 0.5;
	double	height = (inNorth - inSouth) * 0.5;
	double	aspect = height / width;
	double	x = (inEast + inWest) * 0.5;
	double	y =	(inNorth + inSouth) * 0.5;

	double	viewWidth = mPixels[2] - mPixels[0];
	double	viewHeight = mPixels[3] - mPixels[1];
	double	visAspectPixels = viewHeight / viewWidth;
	double	visAspectLogical = visAspectPixels / mAspectRatio;


	if (aspect > visAspectLogical)
	{
		mVisibleBounds[0] = x - height / visAspectLogical;
		mVisibleBounds[1] = y - height;
		mVisibleBounds[2] = x + height / visAspectLogical;
		mVisibleBounds[3] = y + height;

	} else {

		mVisibleBounds[0] = x - width;
		mVisibleBounds[1] = y - width * visAspectLogical;
		mVisibleBounds[2] = x + width;
		mVisibleBounds[3] = y + width * visAspectLogical;
	}
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}
*/

void	WED_MapZoomerNew::GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4])
{
	double vbounds[4];
	GetMapVisibleBounds(vbounds[0],vbounds[1],vbounds[2],vbounds[3]);

	for (int n = 0; n < 4; ++n)
	{
		outTotalBounds[n] = mLogicalBounds[n];
		outVisibleBounds[n] = vbounds[n];
	}
}

void	WED_MapZoomerNew::ScrollH(float xOffset)
{
	++mCacheKey;

	float log[4], vis[4];
	GetScrollBounds(log, vis);

	float now = vis[0] - log[0];
	xOffset -= now;
	mLonCenter += xOffset;// * mPixel2DegLat / mLonCenterCOS);
	UpdateProjection();
}

void	WED_MapZoomerNew::ScrollV(float yOffset)
{
	++mCacheKey;

	float log[4], vis[4];
	GetScrollBounds(log, vis);

	float now = vis[1] - log[1];
	yOffset -= now;
	mLatCenter += (yOffset);// * mPixel2DegLat);
	UpdateProjection();
	RecalcAspectRatio();
}

bool WED_MapZoomerNew::PointVisible(const Point3& point) const
{
	return
		point.x >= mPixels[0] && point.x <= mPixels[2] &&
		point.y >= mPixels[1] && point.y <= mPixels[3];
}

bool WED_MapZoomerNew::BboxVisible(const Bbox3& bbox) const
{
	return
		bbox.xmax() >= mPixels[0] && bbox.xmin() <= mPixels[2] &&
		bbox.ymax() >= mPixels[1] && bbox.ymin() <= mPixels[3];
}

double WED_MapZoomerNew::PixelSize(double zCamera, double featureSize) const
{
	return featureSize * GetPPM();
}

double WED_MapZoomerNew::PixelSize(const Bbox3& bbox) const
{
	return max(bbox.xmax() - bbox.xmin(), bbox.ymax() - bbox.ymin()) * GetPPM();
}

double WED_MapZoomerNew::PixelSize(const Bbox3& bbox, double featureSize) const
{
	return featureSize * GetPPM();
}

double WED_MapZoomerNew::PixelSize(const Point3& position, double diameter) const
{
	return diameter * GetPPM();
}

void	WED_MapZoomerNew::RecalcAspectRatio(void)
{
	++mCacheKey;

	double top_lat = YPixelToLat(mPixels[3]);
	double bot_lat = YPixelToLat(mPixels[1]);

	if (top_lat > 0 && bot_lat < 0)
	{
		mLonCenterCOS = 1.0;
		mProjection.SetStandardParallel(0.0);
	}
	else
	{
		mLonCenterCOS = cos(min(fabs(top_lat), fabs(bot_lat)) * DEG_TO_RAD);
		mProjection.SetStandardParallel(min(fabs(top_lat), fabs(bot_lat)));
	}

	UpdateProjection();
}

void	WED_MapZoomerNew::PushMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

void	WED_MapZoomerNew::PopMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void	WED_MapZoomerNew::Translate(const Vector3& v)
{
	glMatrixMode(GL_MODELVIEW);
	glTranslated(v.dx, v.dy, v.dz);
}

void	WED_MapZoomerNew::Scale(double sx, double sy, double sz)
{
	glMatrixMode(GL_MODELVIEW);
	glScaled(sx, sy, sz);
}

void	WED_MapZoomerNew::Rotate(double deg, const Vector3& axis)
{
	glMatrixMode(GL_MODELVIEW);
	glRotated(deg, axis.dx, axis.dy, axis.dz);
}

void	WED_MapZoomerNew::UpdateProjection()
{
	// Need to call this whenever mLonCenter, mLatCenter, mPixels,
	// mPixel2DegLat or mLonCenterCOS changes.

	mProjection.SetOriginLL({
		mLonCenter - (mPixels[2] + mPixels[0])*0.5 * mPixel2DegLat / mLonCenterCOS,
		mLatCenter - (mPixels[3] + mPixels[1])*0.5 * mPixel2DegLat});

	// Or should we do this directly where we update mPixel2DegLat?
	mProjection.SetXYUnitsPerDegLat(1.0 / mPixel2DegLat);
}