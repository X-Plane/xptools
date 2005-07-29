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
#ifndef WED_MAPZOOMER_H
#define WED_MAPZOOMER_H

class	WED_MapZoomer {
public:

	WED_MapZoomer();

	// The map zoomer converts lat/lon coordinates to pixel coordinates.
	// This API is called by just about anything that needs to do coordinate
	// conversion.
	double	XPixelToLon(double);
	double	YPixelToLat(double);
	double	LonToXPixel(double);
	double	LatToYPixel(double);
	
	// This API is called by the map class to set up and modify the zoomer
	
	// Overall setup
	void	SetPixelBounds(					// Set the area on the screen the user
					double 	inLeft,			// can see.
					double	inBottom,
					double	inRight,
					double	inTop);
	void	SetMapVisibleBounds(			// Set the amount of the map visible in
					double	inWest,			// this screen area.
					double	inSouth,
					double	inEast,
					double	inNorth);
	void	SetMapLogicalBounds(			// Define the max scrollable map positions.
					double	inWest,
					double	inSouth,
					double	inEast,
					double	inNorth);
	void	SetAspectRatio(double);			// Aspect ratio of logical coords (e.g. 1.3 for Boston)

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

	// Scroll bar operations
	void	GetScrollbarValues(
					double&	hMin,
					double&	hMax,
					double& hPos,
					double& vMin,
					double& vMax,
					double& vPos);
	void	ScrollH(double);
	void	ScrollV(double);
	
private:

	double	mPixels[4];
	double	mVisibleBounds[4];
	double	mLogicalBounds[4];
	double	mAspectRatio;
	
};
	
#endif
