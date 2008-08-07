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
#include "GeoUtils.h"
#include "MatrixUtils.h"
#include <math.h>
#if APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

static	inline	float	Interp2d(
							float		val,
							float		inMin,
							float		inMax,
							float		outMin,
							float		outMax)
{
	return outMin + ((val - inMin) * (outMax - outMin) / (inMax - inMin));
}							

bool	MapLineToPlane(
			double		line_pt[3],		// Line's Pt
			double		line_vec[3],	// Line's vector
			double		plane[4],		// ABCD from plane equation
			double		sect[3])		// Where they cross
{
/*
	Parametric line eq = 
	x = x0 + t * x1
	y = y0 + t * y1
	z = z0 + t * z1
	
	Ax + By + Cz + D = 0
	
	plug them in and we get
	
	A(x0 + t * x1) + B(y0 + t * y1) + C(z0 + t * z1) + D = 0
	
	A * x0 + A * t * x1 + B * y0 + t B * t * y1 + C * z0 + C * t * z1 + D= 0
	t (A * x1 + B * y1 + C * z1) = -(A * x0 + B * y0 + c * z0 + D)
	t = -(A * x0 + B * y0 + c * z0 + D) / (A * x1 + B * y1 + C * z1)
	
	Plug in for T and we're done
	
	Try to understand this using vectors:
	P = P0 + t * Vl
	
	Vnormal ¥ P + D = 0	(where D = 3rd term in the plane eq)
	
	Therefore 
	Vnormal ¥ (P0 + t * Vl) + D = 0
	Vnormal ¥ P0 + t * Vnormal ¥ Vl + D = 0
	t * Vnormal ¥ÊVl = -(Vnormal ¥ P0 + D)
	t = -(Vnormal ¥ÊP0 + D) / (Vnormal ¥ Vl)
	
	
*/

	double	num = -(plane[0] * line_pt[0] + plane[1] * line_pt[1] + plane[2] * line_pt[2] + plane[3]);
	double	denom = (plane[0] * line_vec[0] + plane[1] * line_vec[1] + plane[2] * line_vec[2]);
	
	if (denom == 0.0) return false;
	
	double	t = num / denom;
	
	sect[0] = line_pt[0] + line_vec[0] * t;
	sect[1] = line_pt[1] + line_vec[1] * t;
	sect[2] = line_pt[2] + line_vec[2] * t;
	return true;
}

bool	MapLineToSphere(
			double		line_pt[3],
			double		line_vec[3],
			double		sphere[4],	// XYZR
			double		sect[3])
{
/*
	From Foley Van Dam:
	Line is x = x0 + t * dx
			y = y0 + t * dy
			z = z0 + t * dz
	Sphere is (x - px)^2 + (y - py)^2 + (z - pz)^2 = r^2
	
	Substitude:
	
	(x0 + t * dx - px)^2 + (y0 + t * dy - py)^2 + (z0 + t * dz - pz)^2 = r^2
	
	When we massage this to group t^2, t and constant terms, we get: (straight out of Van Dam p 703)
	
	(dx^2  + dy^2 + dz^2) t^2 + 2t (dx(x0 - px) + dy(y0 - py) + dz (z0 - pz) ) +  (x0 - px)^2 + (y0 - py)^2 + (z0 - pz)^2 - r^2 = 0

	We're looking for At^2 + Bt + C = 0, solving this mess we get:
	
	A = (dx^2  + dy^2 + dz^2)
	B = 2(dx(x0 - px) + dy(y0 - py) + dz (z0 - pz) )
	C = (x0 - px)^2 + (y0 - py)^2 + (z0 - pz)^2 - r^2
	
	Quadratic formula is:
	-b +- sqrt(b^2 - 4ac)
	---------------------
			2a	
	
	If b^2 - 4ac < 0, we have no intersection.  if a == 0, no intersection.  Otherwise, solve this mess, lesser value = intersection?!?
*/

	// A = dx^2 + dy^2 + dz^2
	double A = line_vec[0] * line_vec[0] + line_vec[1] * line_vec[1] + line_vec[2] * line_vec[2];
	// B = 2.0 ( dx(x0 - px) + dy(y0 - py) + dz(z0-pz))
	double B = 2.0 * (line_vec[0] * (line_pt[0] - sphere[0]) + line_vec[1] * (line_pt[1] - sphere[1]) + line_vec[2] * (line_pt[2] - sphere[2]));
	// C = (x0 - px)^2 + (y0 - py)^2 + (z0 - pz)^2 - r^2
	double C = (line_pt[0] - sphere[0]) * (line_pt[0] - sphere[0]) + 
			   (line_pt[1] - sphere[1]) * (line_pt[1] - sphere[1]) + 
			   (line_pt[2] - sphere[2]) * (line_pt[2] - sphere[2]) - sphere[3] * sphere[3];

	if ((B * B - 4 * A * C) < 0.0)	return false;
	if (A == 0.0) return false;
	
	double	T = (-B - sqrt(B * B - 4 * A * C)) / (2.0 * A);
	sect[0] = line_pt[0] + T * line_vec[0];
	sect[1] = line_pt[1] + T * line_vec[1];
	sect[2] = line_pt[2] + T * line_vec[2];
	
	return true;
}			


bool	FindPointOnPlane(
			double		left,
			double		right,
			double		bottom,
			double		top,
			double		inNear,
			double		plane[4],
			double		inClickX,
			double		inClickY,
			double		where[3])
{
	// First we have to fetch a bunch of OGL crap.
	GLdouble	model_view[16], i_model_view[16];
	GLdouble	viewport[4];
	
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	glGetDoublev(GL_VIEWPORT, viewport);
	invertMatrix(i_model_view, model_view);
	
	GLdouble	origin[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLdouble	click[4] = { 
		Interp2d(inClickX, viewport[0], viewport[0] + viewport[2], left, right),
		Interp2d(inClickY, viewport[1], viewport[1] + viewport[3], bottom, top),
		-inNear,
		1.0 };

	GLdouble	pt1[4], pt2[4], vec[4];
					
	multMatrixVec(pt1, i_model_view, origin);
	multMatrixVec(pt2, i_model_view, click);
	vec[0] = pt2[0] - pt1[0];
	vec[1] = pt2[1] - pt1[1];
	vec[2] = pt2[2] - pt1[2];

	return MapLineToPlane(pt1, vec, plane, where);
}

bool	FindPointOnSphere(
			double		left,
			double		right,
			double		bottom,
			double		top,
			double		inNear,
			double		sphere[4],
			double		inClickX,
			double		inClickY,
			double		where[3])
{
	// First we have to fetch a bunch of OGL crap.
	GLdouble	model_view[16], i_model_view[16];
	GLdouble	viewport[4];
	
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	glGetDoublev(GL_VIEWPORT, viewport);
	invertMatrix(i_model_view, model_view);
	
	GLdouble	origin[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLdouble	click[4] = { 
		Interp2d(inClickX, viewport[0], viewport[0] + viewport[2], left, right),
		Interp2d(inClickY, viewport[1], viewport[1] + viewport[3], bottom, top),
		-inNear,
		1.0 };

	GLdouble	pt1[4], pt2[4], vec[4];
					
	multMatrixVec(pt1, i_model_view, origin);
	multMatrixVec(pt2, i_model_view, click);
	vec[0] = pt2[0] - pt1[0];
	vec[1] = pt2[1] - pt1[1];
	vec[2] = pt2[2] - pt1[2];

	return MapLineToSphere(pt1, vec, sphere, where);
}

void	ModelToScreenPt(
			double		modelPt[3],
			double		screenPt[2],
			double *	outZ)
{
	GLdouble	model_view[16];
	GLdouble	proj[16];
	GLdouble	viewport[4];
	
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_VIEWPORT, viewport);
	
	double	pt[4] = { modelPt[0], modelPt[1], modelPt[2], 1.0 };
	double	ept[4];
	double	npt[4];
	multMatrixVec(ept, model_view, pt);
	multMatrixVec(npt, proj, ept);
	
	npt[0] /= npt[3];
	npt[1] /= npt[3];
	npt[2] /= npt[3];
	
	screenPt[0] = Interp2d(npt[0], -1.0, 1.0, viewport[0], viewport[0] + viewport[2]);
	screenPt[1] = Interp2d(npt[1], -1.0, 1.0, viewport[1], viewport[1] + viewport[3]);
	
	if (outZ)	*outZ = npt[2];
}			



