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
#include "WED_MapZoomer.h"

inline	double	rescale(double s1, double s2, double d1, double d2, double v)
{
	return ((v - s1) * (d2 - d1) / (s2 - s1)) + d1;
}

WED_MapZoomer::WED_MapZoomer()
{
	mPixels[0] = 0.0;
	mPixels[1] = 0.0;
	mPixels[2] = 512.0;
	mPixels[3] = 512.0;
	mVisibleBounds[0] = mLogicalBounds[0] = 0.0;
	mVisibleBounds[1] = mLogicalBounds[1] = 0.0;
	mVisibleBounds[2] = mLogicalBounds[2] = 1.0;
	mVisibleBounds[3] = mLogicalBounds[3] = 1.0;
	mAspectRatio = 1.0;
}

double	WED_MapZoomer::XPixelToLon(double x)
{
	return rescale(mPixels[0], mPixels[2], mVisibleBounds[0], mVisibleBounds[2], x);
}

double	WED_MapZoomer::YPixelToLat(double y)
{
	return rescale(mPixels[1], mPixels[3], mVisibleBounds[1], mVisibleBounds[3], y);
}

double	WED_MapZoomer::LonToXPixel(double lon)
{
	return rescale(mVisibleBounds[0], mVisibleBounds[2], mPixels[0], mPixels[2], lon);
}

double	WED_MapZoomer::LatToYPixel(double lat)
{
	return rescale(mVisibleBounds[1], mVisibleBounds[3], mPixels[1], mPixels[3], lat);
}
	
void	WED_MapZoomer::SetPixelBounds(		
					double 	inLeft,			
					double	inBottom,
					double	inRight,
					double	inTop)
{
	mPixels[0] = inLeft;
	mPixels[1] = inBottom;
	mPixels[2] = inRight;
	mPixels[3] = inTop;
}
					
void	WED_MapZoomer::SetMapVisibleBounds(
					double	inWest,
					double	inSouth,		
					double	inEast,
					double	inNorth)
{
	mVisibleBounds[0] = inWest;
	mVisibleBounds[1] = inSouth;
	mVisibleBounds[2] = inEast;
	mVisibleBounds[3] = inNorth;
}					
void	WED_MapZoomer::SetMapLogicalBounds(	
					double	inWest,
					double	inSouth,
					double	inEast,
					double	inNorth)
{
	mLogicalBounds[0] = inWest;
	mLogicalBounds[1] = inSouth;
	mLogicalBounds[2] = inEast;
	mLogicalBounds[3] = inNorth;					
}					


void	WED_MapZoomer::GetPixelBounds(		
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
					
void	WED_MapZoomer::GetMapVisibleBounds(
					double&	outWest,
					double&	outSouth,		
					double&	outEast,
					double&	outNorth)
{
	outWest = mVisibleBounds[0];
	outSouth = mVisibleBounds[1];
	outEast = mVisibleBounds[2];
	outNorth = mVisibleBounds[3];
}					
void	WED_MapZoomer::GetMapLogicalBounds(	
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


void	WED_MapZoomer::SetAspectRatio(double a)
{
	mAspectRatio = a;
}

void	WED_MapZoomer::ZoomShowAll(void)
{
	double	mapWidth = mLogicalBounds[2] - mLogicalBounds[0];
	double	mapHeight = mLogicalBounds[3] - mLogicalBounds[1];
	double	mapCenterX = 0.5 * (mLogicalBounds[2] + mLogicalBounds[0]);
	double	mapCenterY = 0.5 * (mLogicalBounds[3] + mLogicalBounds[1]);
	double	viewWidth = mPixels[2] - mPixels[0];
	double	viewHeight = mPixels[3] - mPixels[1];
	
	// Our official aspect ratio is the shape of a 1x1 logical unit box.
	// So we have to bias our logical zoomout to take in extra dead area to
	// preserve this aspect ratio!!  Here's how we do it:
	
	// Our visible aspect ratio is the aspect ratio of the actual window...
	// probably varies depending on the user's monitor and other weird stuff.
	double	visAspectPixels = viewHeight / viewWidth;
	// Map aspect ratio is the difference in logical units of the shape of our map.
	double	mapAspect = mapHeight / mapWidth;
	
	// Now we have to transform the map's aspect ratio into logical units, e.g. 
	// how much of the map _should_ be visible.
	double	visAspectLogical = visAspectPixels / mAspectRatio;
	
	// This number needs to be the same as mapAspect...if it's not we have to make
	// an adjustment.
	if (visAspectLogical > mapAspect)
	{
		// The area we have to display is taller than our map.  Pad the map on the top
		// and bottom.
		mapHeight = mapWidth * visAspectLogical;
	} else {
		// The area we have to display is wider than our map.  PAd the map on the left and
		// right.
		mapWidth = mapHeight / visAspectLogical;
	}
	
	// Now set the map bounds.
	mVisibleBounds[0] = mapCenterX - 0.5 * mapWidth;
	mVisibleBounds[1] = mapCenterY - 0.5 * mapHeight;
	mVisibleBounds[2] = mapCenterX + 0.5 * mapWidth;
	mVisibleBounds[3] = mapCenterY + 0.5 * mapHeight;
}

void	WED_MapZoomer::PanPixels(
					double	x1,
					double	y1,
					double	x2,
					double	y2)
{
	// This is straight-forward: calculate the difference in lat/lon numbers between
	// the two pixels and change.  Positive numbers mean the user dragged up/right
	// therefore the map's bounds are decreasing!  Whacky, eh?
	double delta_lon = XPixelToLon(x2) - XPixelToLon(x1);
	double delta_lat = YPixelToLat(y2) - YPixelToLat(y1);
	
	// Move the map in the opposite direction.
	mVisibleBounds[0] -= delta_lon;
	mVisibleBounds[1] -= delta_lat;
	mVisibleBounds[2] -= delta_lon;
	mVisibleBounds[3] -= delta_lat;
}

void	WED_MapZoomer::ZoomAround(
					double	zoomFactor,
					double	centerXPixel,
					double	centerYPixel)
{
	// Zoom the map around a point.  We do this in three steps because I am lazy:
	// 1. Scroll the map so that we are zooming around the lower left corner.
	// 2. Zoom the map by adjusting only the top and right logical bounds, not
	//    the lower left.
	// 3. Scroll the map back.
	
	PanPixels(centerXPixel, centerYPixel, mPixels[0], mPixels[1]);
	
	double	width = mVisibleBounds[2] - mVisibleBounds[0];
	double	height = mVisibleBounds[3] - mVisibleBounds[1];
	width /= zoomFactor;
	height /= zoomFactor;
	mVisibleBounds[2] = mVisibleBounds[0] + width;
	mVisibleBounds[3] = mVisibleBounds[1] + height;

	PanPixels(mPixels[0], mPixels[1], centerXPixel, centerYPixel);
}					

void	WED_MapZoomer::ScrollReveal(
				double	inLon,
				double	inLat)
{
	double	delta_lon = inLon - ((mVisibleBounds[0] + mVisibleBounds[2]) * 0.5);
	double	delta_lat = inLat - ((mVisibleBounds[1] + mVisibleBounds[3]) * 0.5);
	mVisibleBounds[0] += delta_lon;
	mVisibleBounds[1] += delta_lat;
	mVisibleBounds[2] += delta_lon;
	mVisibleBounds[3] += delta_lat;
	
}

void	WED_MapZoomer::ScrollReveal(
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
}

void	WED_MapZoomer::GetScrollbarValues(
				double&	hMin,
				double&	hMax,
				double& hPos,
				double& vMin,
				double& vMax,
				double& vPos)
{
	double	hVis = mVisibleBounds[2] - mVisibleBounds[0];
	double	vVis = mVisibleBounds[3] - mVisibleBounds[1];
	double	hTot = mLogicalBounds[2] - mLogicalBounds[0];
	double	vTot = mLogicalBounds[3] - mLogicalBounds[1];
	
	hMin = 0;
	vMin = 0;
	hMax = hTot - hVis;
	vMax = vTot - vVis;
	if (hMax < 0.0) hMax = 0.0;
	if (vMax < 0.0) vMax = 0.0;
	hPos = mVisibleBounds[0] - mLogicalBounds[0];
	vPos = mVisibleBounds[1] - mLogicalBounds[1];	
}
				
void	WED_MapZoomer::ScrollH(double lon)
{
	double	width = mVisibleBounds[2] - mVisibleBounds[0];
	mVisibleBounds[0] = mLogicalBounds[0] + lon;
	mVisibleBounds[2] = mVisibleBounds[0] + width;
}

void	WED_MapZoomer::ScrollV(double lat)
{
	double	height = mVisibleBounds[3] - mVisibleBounds[1];
	mVisibleBounds[1] = mLogicalBounds[1] + lat;
	mVisibleBounds[3] = mVisibleBounds[1] + height;
}
