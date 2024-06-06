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

/* Geographic projection motivation

 - improve accuracy of LARGE objects defined in meter-space compare to objects placed by coordinates.
   E.g. when placing a FS2XP object with draped ground markings and the zooming in to a location far away
   (some 10,00 feet / 3km) from the objects origin.
   Currently this results in discrepancies of upto 3-5 feet between the WED and X-Plane depictions 
   at moderate lattitudes.

 - improve accuracy of LONG line segments relative to (small) objects placed at specific corrdinates.
   E.e. a group of individual objects (manually places runway light or polygonal/line based centerline markings) 
   will NOT match the E-W runway at Anchorage by ~1 foot in N/S directions in the center of that runway.

 * Technical reasons of discrepancies

 - change of longitudinal scale with lattitude
   WED up to 2.3 used a mercator projected map, i.e. the map scale was constant for all pixel<->coordinate
   transformations inside as well as outside the current map window and deternined by the center of the
   map window. All parallels and meridians were straight lines.
   Earth radius was constant, i.e. an spherical globe with a radius so 1 deg lattitude is exactly 60.0 nm.

 - lack of projection of 'straight' lines between two vertices
   Lines in OGL are straight between ththeir coordinate based endpoint location, not great circles as in X-Plane.

 - lack of projection of 'objects' defined and drawn in meterspace.
   openGL is limited to linear transformations. A object placed the poles has notable curvature in E-W direction 
   compared to the constant lattitude paralel.

   The new code supports, all optional and modular,
   - arbitrary map projection LLoPixel and PixelToLL, including north heading rotation with longitude
   - GRS80 ellipsoid to model the flattening at the poles
   - local general ground elevation to model change of scale with earth local radius
   - smooth transition from one projection to another while zooming in/out. Currently used to change from
     the 'main' map projection to an generally more "right" looking compromise projection when zooed out 
     very far, i.e. (almost) all of the world map is visible.
*/

// when zoomed out to near world-level - change projection to a nicer looking and better area- and distance 
// preserving Wagner IV. The main use of this is to verify all UI elements use projected transformations.
// https://en.wikipedia.org/wiki/Wagner_VI_projection

#define USE_WAGNER   1
#define THR_WAGNER   0.5

// when zoomed in, change projection from mercator to gnomonic, to
// - allow items defined in meter-space (like objects or lines) to NOT require warping,
//   i.e. make great circles to be EXACTLY straight lines in the map
// - (better) preserve distances from map center for objects at moderate distances from map center
// https://en.wikipedia.org/wiki/Gnomonic_projection

#define USE_GNOMONIC   1
#define FULL_EQUATIONS 0
#define THR_GNOMONIC   0.02


#include "WED_MapZoomerNew.h"
#include "GUI_Messages.h"
#include "XESConstants.h"

#include "CompGeomDefs3.h"
#include "MathUtils.h"
#if APL
  #include <OpenGL/gl.h>
#else
  #include "glew.h"
#endif
#define sinr(x) sin((x) * DEG_TO_RAD)
#define cosr(x) cos((x) * DEG_TO_RAD)

// X-Plane 11- models a spherical world. Map scale is constant everywhere.
// X-Plane 12+ uses a GRS80 ellipsoid, there are 2 radii relevant for map scale.
// https://en.wikipedia.org/wiki/Geographical_distance#Ellipsoidal_Earth_projected_to_a_plane
// https://en.wikipedia.org/wiki/Earth_radius#/media/File:EarthEllipRadii.jpg

#define USE_GRS80 1

// in N-S direction: prime vertical radius N, some 6378 to 6400 km
static double term1(double lat)
{
	double s = sinr(lat);
	return 	1.0 / sqrt(1.0 - EARTH_EPS2 * s * s);
}
static double N_GRS80(double lat)
{
	return EARTH_EQ_RADIUS * term1(lat);
}
// in E-W direction: meridional radius M, some 6335 to 6400 km, multiply by cos(lat) as usual
static double M_GRS80(double lat)
{
	double t = term1(lat);
	return EARTH_EQ_RADIUS * (1.0 - EARTH_EPS2) * t * t * t;
}

WED_MapZoomerNew::WED_MapZoomerNew(WED_Camera * c)
	: cam(c),
	  mCacheKey(0),
	  mPixels{0, 0, 1, 1},
	  mLogicalBounds{-180, -90, 180, 90},
	  mLonCenter(0), mLatCenter(0),
	  mCenterX(0.5), mCenterY(0.5)
{
	RecalcAspectRatio();
}

WED_MapZoomerNew::~WED_MapZoomerNew()
{
}

double	WED_MapZoomerNew::XPixelToLon(double x) const
{
	return mLonCenter + ( x - mCenterX) * mScale.Pix2DegLon() / mCenterCOS;
}

double	WED_MapZoomerNew::YPixelToLat(double y) const
{
	return mLatCenter + (y - mCenterY) * mScale.Pix2DegLat();
}

double	WED_MapZoomerNew::LonToXPixel(double lon) const
{
	return mCenterX + (lon - mLonCenter) * mCenterCOS * mScale.Deg2PixLon();
}

double	WED_MapZoomerNew::LatToYPixel(double lat) const
{
	return mCenterY + (lat - mLatCenter) * mScale.Deg2PixLat();
}

double	WED_MapZoomerNew::wagner_proj_mult(double lat) const
{
	float l(lat);
	return sqrtf(1.0f-3.0f*sqr(l/180.0f));
}

Point2	WED_MapZoomerNew::PixelToLL(const Point2& p) const
{
#if USE_GNOMONIC
	if (!cam && mScale.Pix2DegLat() < THR_GNOMONIC)
	{
		Point2 pt((p.x() - mCenterX) * mScale.Pix2DegLon(),
			      (p.y() - mCenterY) * mScale.Pix2DegLat());
		// https://mathworld.wolfram.com/GnomonicProjection.html
		pt.x_ *= DEG_TO_RAD;
		pt.y_ *= DEG_TO_RAD;
#if FULL_EQUATIONS
		double rho = sqrt(pt.x() * pt.x() + pt.y() * pt.y());
		double c = atan(rho);
		double lat = RAD_TO_DEG * asin(cos(c) * sinr(mLatCenter) + pt.y() * sin(c) * cosr(mLatCenter) / rho);
		double lon = mLonCenter + RAD_TO_DEG * atan2(pt.x() * sin(c), rho * cosr(mLatCenter) * cos(c) - pt.y() * sinr(mLatCenter) * sin(c));
#else
		double rho2 = pt.x() * pt.x() + pt.y() * pt.y();
		double ct = 1.0 / sqrt(1.0 + rho2);
		double lat = RAD_TO_DEG * asin(ct * (mLatCenterSIN + pt.y() * mLatCenterCOS)) ;
		double lon = mLonCenter + RAD_TO_DEG * atan2(pt.x(), mLatCenterCOS - pt.y() * mLatCenterSIN);
#endif
		if (mScale.Pix2DegLat() > THR_GNOMONIC * 0.3)
		{
			double blend = min(0.7, (THR_GNOMONIC - mScale.Pix2DegLat()) / THR_GNOMONIC) / 0.7;
			return Point2(XPixelToLon(p.x()) * (1.0 - blend) + lon * blend,
						  YPixelToLat(p.y()) * (1.0 - blend) + lat * blend);
		}
		else
			return Point2(lon, lat);
	}
#endif
#if USE_WAGNER
	if (mMapSize > THR_WAGNER)
	{
		double blend = min(1.0, (mMapSize - THR_WAGNER)/THR_WAGNER);
		Point2 pt(XPixelToLon(p.x()), YPixelToLat(p.y()));
		pt.y_ = min(max(pt.y(), mLogicalBounds[1]), mLogicalBounds[3]);
		return Point2( pt.x() / (1.0 + blend * (wagner_proj_mult(pt.y()) - 1.0)),
		               pt.y() / (1.0 + blend *  0                              ));
	}
#endif
	return Point2(XPixelToLon(p.x()), YPixelToLat(p.y()));
}

Point2	WED_MapZoomerNew::LLToPixel(const Point2& p) const
{
#if USE_GNOMONIC
	if (!cam && mScale.Pix2DegLat() < THR_GNOMONIC)
	{
		Point2 pt(p);
		pt.x_ = min(max(mLogicalBounds[0], pt.x()), mLogicalBounds[2]);
		pt.y_ = min(max(mLogicalBounds[1], pt.y()), mLogicalBounds[3]);
#if FULL_EQUATIONS
		// https://mathworld.wolfram.com/GnomonicProjection.html
		double c = sinr(mLatCenter) * sinr(pt.y()) + cosr(mLatCenter) * cosr(pt.y()) * cosr(pt.x() - mLonCenter);
		double x = mCenterX + (cosr(pt.y()) * sinr((pt.x() - mLonCenter)) / c) * RAD_TO_DEG * mPixel2DegLat.inv();
		double y = mCenterY + ((cosr(mLatCenter) * sinr(pt.y()) - sinr(mLatCenter) * cosr(pt.y()) * cosr(pt.x() - mLonCenter)) / c) * RAD_TO_DEG * mPixel2DegLat.inv();
#else
		auto as = sinr(pt.y());               // gcc gets this and uses optimized sincos() function. Yay !
		auto ac = cosr(pt.y());
		auto bs = sinr(pt.x() - mLonCenter);
		auto bc = cosr(pt.x() - mLonCenter);
		double ci = 1.0 / (mLatCenterSIN * as + mLatCenterCOS * ac * bc);

		double x = mCenterX + (ac * bs * ci) * RAD_TO_DEG * mScale.Deg2PixLon();
		double y = mCenterY + ((mLatCenterCOS * as - mLatCenterSIN * ac * bc) * ci) * RAD_TO_DEG * mScale.Deg2PixLat();
#endif
		if (mScale.Pix2DegLat() > THR_GNOMONIC * 0.3)
		{
			double blend = min(0.7, (THR_GNOMONIC - mScale.Pix2DegLat()) / THR_GNOMONIC) / 0.7;
			return Point2(LonToXPixel(p.x()) * (1.0-blend) + x * blend,
						  LatToYPixel(p.y()) * (1.0-blend) + y * blend);
		}
		else
			return Point2(x,y);
	}
#endif
#if USE_WAGNER
	if (mMapSize > THR_WAGNER)
	{
		double blend = min(1.0, (mMapSize - THR_WAGNER) / THR_WAGNER);
		return Point2(LonToXPixel( p.x() * (1.0 + blend * (wagner_proj_mult(p.y()) - 1.0) )),
		              LatToYPixel( p.y() * (1.0 + blend *  0                              )));
	}
#endif
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
	return mScale.ppm();
}

double WED_MapZoomerNew::GetClickRadius(double p) const
{
	return p * mScale.Pix2DegLat();
}

double WED_MapZoomerNew::GetRotation(const Point2& p) const
{
#if USE_GNOMONIC
	if (!cam && mScale.Pix2DegLat() < THR_GNOMONIC)
	{
		Point2 pt(p);
		pt.x_ = min(max(mLogicalBounds[0], pt.x()), mLogicalBounds[2]);
		pt.y_ = min(max(mLogicalBounds[1], pt.y()), mLogicalBounds[3]);

		auto as = sinr(pt.y());                       // todo: create aproximation, this really takes long.
		auto ac = cosr(pt.y());                       // if any kind of precision is needed, zoom is high
		auto as2 = sinr(pt.y() + 1e-3 );               // and rotation for anything visible very small, << 1deg
		auto ac2 = cosr(pt.y() + 1e-3);
		auto bs = sinr(pt.x() - mLonCenter);
		auto bc = cosr(pt.x() - mLonCenter);

		double dx = (ac2 - ac) * bs;
		double dy = (mLatCenterCOS * (as2 -as) - mLatCenterSIN * (ac2 - ac) * bc);

		double rotation = atan2f(dx, dy) * RAD_TO_DEG; // float is enough precision, atan2 takes REALLY long

		if (mScale.Pix2DegLat() > THR_GNOMONIC * 0.3)
		{
			double blend = min(0.7, (THR_GNOMONIC - mScale.Pix2DegLat()) / THR_GNOMONIC) / 0.7;
			return blend * rotation;
		}
		else
			return rotation;
	}
#endif
	// Ignore any object rotation once the maps zooms out to the near the full world.
	// There are no meaningful X-Plane objects big enough to notice rotation that scale.
	return 0.0;
}

void	WED_MapZoomerNew::SetPixelBounds(
					double 	inLeft,
					double	inBottom,
					double	inRight,
					double	inTop)
{
	++mCacheKey;
	mPixels[0] = inLeft;           // there is some redundancy here ...
	mPixels[1] = inBottom;
	mPixels[2] = inRight;
	mPixels[3] = inTop;
	mCenterX = 0.5 * (inLeft + inRight);
	mCenterY = 0.5 * (inBottom + inTop);
	mMapSize = (mPixels[2] - mPixels[0]) * mScale.Pix2DegLat() / 360.0;

	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}

void	WED_MapZoomerNew::SetPixelCenter(double x, double y)
{
	mCenterX = x;
	mCenterY = y;
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

	mLonCenter = 0.5 * (inWest + inEast);
	mLatCenter = 0.5 * (inNorth + inSouth);
	mLatCenterCOS = cosr(mLatCenter);
	mLatCenterSIN = sinr(mLatCenter);
	mCenterCOS = mLatCenterCOS;

	mScale.set(mScale.ppm(), mLatCenter);

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
	Point2 coords[8];
	coords[0] = PixelToLL(Point2(mPixels[0], mPixels[1]));
	coords[1] = PixelToLL(Point2(mPixels[0], (mPixels[1] + mPixels[3]) * 0.5));
	coords[2] = PixelToLL(Point2(mPixels[0], mPixels[3]));
	coords[3] = PixelToLL(Point2((mPixels[0] + mPixels[2]) * 0.5, mPixels[3]));
	coords[4] = PixelToLL(Point2(mPixels[2], mPixels[3]));
	coords[5] = PixelToLL(Point2(mPixels[2], (mPixels[1] + mPixels[3]) * 0.5));
	coords[6] = PixelToLL(Point2(mPixels[2], mPixels[1]));
	coords[7] = PixelToLL(Point2((mPixels[0] + mPixels[2]) * 0.5, mPixels[1]));

	outWest = fltmin3(coords[0].x(), coords[1].x(), coords[2].x());
	outSouth = fltmin3(coords[6].y(), coords[7].y(), coords[0].y());
	outEast = fltmax3(coords[4].x(), coords[5].x(), coords[6].x());
	outNorth = fltmax3(coords[2].y(), coords[3].y(), coords[4].y());
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

	double required_width_logical  = max(inEast - inWest, 0.00001);
	double required_height_logical = max(inNorth - inSouth, 0.00001);
	double pix_avail_width = mPixels[2] - mPixels[0];
	double pix_avail_height = mPixels[3] - mPixels[1];
	double scale_for_vert = required_height_logical / pix_avail_height;
	double scale_for_horz = required_width_logical / pix_avail_width * cosr(mLatCenter);
	mScale.set(MTR_TO_DEG_LAT / max(scale_for_vert,scale_for_horz), mLatCenter);
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

	RecalcAspectRatio();

	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}

void	WED_MapZoomerNew::ZoomAround(
					double	zoomFactor,
					double	centerXPixel,
					double	centerYPixel)
{
	++mCacheKey;

	if (zoomFactor <= 1.0 || mScale.Pix2DegLat() > 0.001 / DEG_TO_MTR_LAT) // limit manual zoom in to ~1 mm/pixel
	{
		Point2 old_pos = PixelToLL(Point2(centerXPixel, centerYPixel));
		mScale.set(mScale.ppm() * zoomFactor, mLatCenter);

		Point2 new_pos = PixelToLL(Point2(centerXPixel, centerYPixel));
		mLonCenter -= new_pos.x() - old_pos.x();
		mLatCenter -= new_pos.y() - old_pos.y();

		RecalcAspectRatio();

		BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED, 0);
	}
}

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
}

void	WED_MapZoomerNew::ScrollV(float yOffset)
{
	++mCacheKey;

	float log[4], vis[4];
	GetScrollBounds(log, vis);

	float now = vis[1] - log[1];
	yOffset -= now;
	mLatCenter += (yOffset);// * mPixel2DegLat);
	RecalcAspectRatio();
}

void	WED_MapZoomerNew::RecalcAspectRatio(void)
{
	double top_lat = YPixelToLat(mPixels[3]);
	double bot_lat = YPixelToLat(mPixels[1]);

	if (top_lat > 0 && bot_lat < 0)
		mCenterCOS = 1.0;
	else
		mCenterCOS = cos(min(fabs(top_lat),fabs(bot_lat)) * DEG_TO_RAD);

	mLatCenterCOS = cos(mLatCenter * DEG_TO_RAD);
	mLatCenterSIN = sin(mLatCenter * DEG_TO_RAD);
	mMapSize = (mPixels[2] - mPixels[0]) * mScale.Pix2DegLat() / 360.0;

	mScale.set(mScale.ppm(), mLatCenter);
}

void WED_MapZoomerNew::mapScale::set(double PPM, double LatCenterDeg, double AltitudeMSL)
{
	mPPM = PPM;
	double Pixel2DegLat = MTR_TO_DEG_LAT / PPM;
	double altitude_correction = (AltitudeMSL + EARTH_MEAN_RADIUS) / EARTH_MEAN_RADIUS;
	Pixel2DegLat *= altitude_correction;
#if USE_GRS80
	mPixel2DegLat = Pixel2DegLat * EARTH_MEAN_RADIUS / M_GRS80(LatCenterDeg);
	mPixel2DegLon = Pixel2DegLat * EARTH_MEAN_RADIUS / N_GRS80(LatCenterDeg);
#else
	mPixel2DegLat = Pixel2DegLat;
	mPixel2DegLon = Pixel2DegLat;
#endif
	mDeg2PixelLat = 1.0 / mPixel2DegLat;      // division takes a LOT longer than multiplication. So we cache these
	mDeg2PixelLon = 1.0 / mPixel2DegLon;
}

/********** new funcs for 3D preview / prespective projection *********/

double	WED_MapZoomerNew::PixelSize(const Bbox2& bboxLL) const
{
	if (cam)
	{
		Point2 p1 = LLToPixel(bboxLL.p1);
		Point2 p2 = LLToPixel(bboxLL.p2);
		return cam->PixelSize(Bbox3(p1.x(), p1.y(), 0.0, p2.x(), p2.y(), 0.0));
	}
	else
		return max(bboxLL.xspan() * mCenterCOS, bboxLL.yspan()) * mScale.Deg2PixLat();
}

double	WED_MapZoomerNew::PixelSize(const Bbox2& bboxLL, double featureSize) const
{
	if(cam)
	{
		Point2 p1 = LLToPixel(bboxLL.p1);
		Point2 p2 = LLToPixel(bboxLL.p2);
		return cam->PixelSize(Bbox3(p1.x(), p1.y(), 0.0, p2.x(), p2.y(), 0.0), featureSize);
	}
	else
		return featureSize * GetPPM();
}

double	WED_MapZoomerNew::PixelSize(const Point2& positionLL, double diameter) const
{
	if(cam)
	{
		Point2 posPixel = LLToPixel(positionLL);
		return cam->PixelSize({posPixel.x(), posPixel.y(), 0.0}, diameter);
	}
	else
		return diameter * GetPPM();
}

void	WED_MapZoomerNew::PushMatrix(void)
{
	if(cam)
		cam->PushMatrix();
	else
		glPushMatrix();
}

void	WED_MapZoomerNew::Rotatef(float r, float x, float y, float z)
{
	if(cam)
		cam->Rotate(r, Vector3(x, y, z));
	else
		glRotatef(r, x, y, z);
}

void	WED_MapZoomerNew::Translatef(float x, float y, float z)
{
	if(cam)
		cam->Translate(Vector3(x, y, z));
	else
		glTranslatef(x, y, z);
}

void	WED_MapZoomerNew::Scalef(float x, float y, float z)
{
	if(cam)
		cam->Scale(x, y, z);
	else
		glScalef(x, y, z);
}

void	WED_MapZoomerNew::PopMatrix(void)
{
	if(cam)
		cam->PopMatrix();
	else
		glPopMatrix();
}

void	WED_MapZoomerNew::SetPPM(double ppm)
{
	if (ppm > 0.0)
	{
		mScale.set(ppm, mLatCenter);
		mMapSize = (mPixels[2] - mPixels[0]) * mScale.Pix2DegLat() / 360.0;
	}
}
