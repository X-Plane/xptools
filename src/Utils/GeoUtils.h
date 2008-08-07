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
#ifndef GEOUTILS_H
#define GEOUTILS_H

// Intersect a line and a plane, returns true if they intersect
bool	MapLineToPlane(
			double		line_pt[3],		// Line's Pt
			double		line_vec[3],	// Line's vector
			double		plane[4],		// ABCD from plane equation
			double		sect[3]);		// Where they cross

// Intersect a line and a sphere, return trhe the closer point,
// or false if no intersect
bool	MapLineToSphere(
			double		line_pt[3],
			double		line_vec[3],
			double		sphere[4],	// XYZR
			double		sect[3]);


// Given the corners of the near clipping plane in eye coordinates
// and the distance to the near clipping plane (inNear must be positive!!)
// and a plane equation in the form aX + bY + cZ + D = 0 in model-view 
// coordinates, and an X/Y pixel location, this routine returns in 
// model-view coordinates the point on the plane that maps to the pixels X, Y
// given.  Use this to turn a 2-d mouse click into a 3-d mouse click by 
// constraining to an arbitrary plane.  (Warning: if the plane is shear to 
// the viewer (e.g. looks like a line when projected) this will probably
// explode.  The current OGL state is used.
bool	FindPointOnPlane(
			double		left,			// These describe our view
			double		right,			// frustum - can't reverse these 
			double		bottom,			// from the projection matrix!
			double		top,
			double		inNear,
			double		plane[4],		// Our plane in model-view coordinates
			double		inClickX,		// The click, in "screen" coordinates.
			double		inClickY,		// (really OS window coordinates but +Y = up)
			double		where[3]);		// The point on the plane in model-view coordinates

bool	FindPointOnSphere(
			double		left,			// These describe our view
			double		right,			// frustum - can't reverse these 
			double		bottom,			// from the projection matrix!
			double		top,
			double		inNear,
			double		sphere[4],		// Our sphere in model-view coordinates, XYZR
			double		inClickX,		// The click, in "screen" coordinates.
			double		inClickY,		// (really OS window coordinates but +Y = up)
			double		where[3]);		// The point on the plane in model-view coordinates
	

// Given a point in modelview space, find the 2-d screen coords given
// the current OGL state.
void	ModelToScreenPt(
			double		modelPt[3],
			double		screenPt[2],
			double *	outZ = NULL);



#endif

