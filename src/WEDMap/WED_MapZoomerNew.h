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
#ifndef WED_MAPZOOMERNEW_H
#define WED_MAPZOOMERNEW_H

#include "GUI_ScrollerPane.h"
#include "WED_Camera.h"
#include "WED_MapProjection.h"

/*

	The map zoomer maintains a relationship between logical and screen coordinates
	AND maintains a "windowing" system.  Thus it does scrolling and viewing at the same time.

	It does this with two rectangles:

	- Visible bounds - what we can see.
	- Total bounds - the entire image.

	It does this with two coordinate systems:

	- Pixels - screen drawing units.
	- Logical - whatever units our map is in (degrees lat/lon for WED).

	Thus we have the pixel visible bounds, logical visible bounds, and logical total bounds.

*/

class	WED_MapZoomerNew : public GUI_ScrollerPaneContent, public WED_Camera {
public:

					 WED_MapZoomerNew();
	virtual			~WED_MapZoomerNew();
	// The map zoomer converts lat/lon coordinates to pixel coordinates.
	// This API is called by just about anything that needs to do coordinate
	// conversion.

			double	XPixelToLon(double) const;
			double	YPixelToLat(double) const;
			double	LonToXPixel(double) const;
			double	LatToYPixel(double) const;

			Point2	PixelToLL(const Point2& p) const;
			Point2	LLToPixel(const Point2& p) const;

			void	PixelToLLv(Point2 * dst, const Point2 * src, int n) const;
			void	LLToPixelv(Point2 * dst, const Point2 * src, int n) const;

			double	GetPPM(void) const;
			double	GetClickRadius(double pixels) const;
			long long	CacheKey(void) const { return mCacheKey; }

			const WED_MapProjection& Projection() const { return mProjection; }

	// This API is called by the map class to set up and modify the zoomer

	// Overall setup

			void	SetMapLogicalBounds(			// Define the max scrollable map positions.
							double	inWest,
							double	inSouth,
							double	inEast,
							double	inNorth);

			void	GetPixelBounds(					// Get the area on the screen the user
							double& outLeft,			// can see.
							double&	outBottom,
							double&	outRight,
							double&	outTop);
			void	GetMapVisibleBounds(			// Get the amount of the map visible in
							double&	outWest,		// this screen area.
							double&	outSouth,
							double&	outEast,
							double&	outNorth);
			void	GetMapLogicalBounds(			// Defoute the max scrollable map positions.
							double&	outWest,
							double&	outSouth,
							double&	outEast,
							double&	outNorth);


	// Scrolling operations
			void	ZoomShowAll(void);				// Zoom out to reveal the whole map
			void	ZoomShowArea(
							double	inWest,
							double	inSouth,
							double	inEast,
							double	inNorth);
			void	PanPixels(						// Pan so that the logical pixel under p1
							double	x1,				// is now visible under p2
							double	y1,
							double	x2,
							double	y2);
			void	ZoomAround(						// Zoom in and out keeping one pixel constant
							double	zoomFactor,
							double	centerXPixel,
							double	centerYPixel);
			void	ScrollReveal(
							double	inLon,
							double	inLat);
			void	ScrollReveal(
							double	inWest,
							double	inSouth,
							double	inEast,
							double	inNorth);


	virtual	void	GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4]);
	virtual	void	ScrollH(float xOffset);
	virtual	void	ScrollV(float yOffset);

	bool PointVisible(const Point3& point) const override;
	bool BboxVisible(const Bbox3& bbox) const override;
	double PointDistance(const Point3& point) const override
	{
		return 0.0;
	}
	double PixelSize(double zCamera, double featureSize) const override;
	double PixelSize(const Bbox3& bbox) const override;
	double PixelSize(const Bbox3& bbox, double featureSize) const override;
	double PixelSize(const Point3& position, double diameter) const override;
	void PushMatrix() override;
	void PopMatrix() override;
	void Translate(const Vector3& v) override;
	void Scale(double sx, double sy, double sz) override;
	void Rotate(double deg, const Vector3& axis) override;

protected:
			void	SetPixelBounds(					// Set the area on the screen the user
							double 	inLeft,			// can see.
							double	inBottom,
							double	inRight,
							double	inTop);


private:

			void	RecalcAspectRatio(void);
			void	UpdateProjection();

	double	mPixels[4];
	double	mLogicalBounds[4];
	double	mLatCenter;
	double	mLonCenter;
	double	mLonCenterCOS;
	long long mCacheKey;
	WED_MapProjection mProjection;
protected:
	double	mPixel2DegLat;

};

#endif
