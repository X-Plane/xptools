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
#include "OE_CubeDeformer.h"
#include "MatrixUtils.h"
#include "XPLMGraphics.h"
#include "GeoUtils.h"
#include "OE_Zoomer3d.h"

const int	kDeformInfo[26][7] = {
	// Coords		
	//	X	Y	Z	
	{	-1,	-1,	-1 },
	{	-1, -1,	 0 },
	{	-1, -1,	 1 },	
	{	-1,	 0,	-1 },
	{	-1,  0,	 0 },
	{	-1,  0,	 1 },
	{	-1,	 1,	-1 },
	{	-1,	 1,	 0 },
	{	-1,	 1,	 1 },	

	{	 0,	-1,	-1 },	
	{	 0, -1,	 0 },	
	{	 0, -1,	 1 },	
	{	 0,	 0,	-1 },	
//	{	 0,  0,	 0 },
	{	 0,  0,	 1 },	
	{	 0,	 1,	-1 },	
	{	 0,	 1,	 0 },	
	{	 0,	 1,	 1 },	

	{ 	 1,	-1,	-1 },
	{	 1, -1,	 0 },
	{	 1, -1,	 1 },
	{	 1,	 0,	-1 },
	{	 1,  0,	 0 },
	{	 1,  0,	 1 },
	{	 1,	 1,	-1 },
	{	 1,	 1,	 0 },
	{	 1,	 1,	 1 },
};

inline	int	NumNotZero(double	x, double y, double z)
{
	int	total = 0;
	if (x != 0.0) total++;
	if (y != 0.0) total++;
	if (z != 0.0) total++;
	return total;
}

inline	double	greatest_abs_3(double a, double b, double c)
{
	double aa = fabs(a);
	double ab = fabs(b);
	double ac = fabs(c);
	if (aa > ab && aa > ac)
		return a;
	return (ab > ac) ? b : c;
}

inline	void	abs_average(double& a, double& b)
{
	double aa = fabs(a);
	double ab = fabs(b);
	bool nega = (a < 0.0);
	bool negb = (b < 0.0);
	double	val = (aa + ab) * 0.5;
	a = b = val;
}



OE_CubeDeformer::OE_CubeDeformer()
{	
	mControlPoint = -1;
	setIdentityMatrix(mRotateMatrix);
	setIdentityMatrix(mTranslateMatrix);
	setIdentityMatrix(mScaleMatrix);
	mScaleMatrix[0] = mScaleMatrix[5] = mScaleMatrix[10] = 10.0;
}

void	OE_CubeDeformer::DrawDeformer(void)
{
	XPLMSetGraphicsState(0,0,0,   0, 0,  1,1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	ApplyTransform();
	glPointSize(5);
	glBegin(GL_POINTS);
	for (int n = 0; n < 26; ++n)
	{
		if (mControlPoint == n)
			glColor3f(1.0, 0.0, 1.0);
		else
			glColor3f(1.0, 1.0, 0.0);
		glVertex3f(kDeformInfo[n][0],kDeformInfo[n][1],kDeformInfo[n][2]);
	}
	glEnd();
	glPointSize(1);
	glPopMatrix();
}
	
bool	OE_CubeDeformer::TrackClick(
					OE_Zoomer3d *	zoomer,
					int				bounds[4],
					XPLMMouseStatus status, 
					int 			x, 
					int 			y, 
					int 			button)
{
		double	pt1[3], pt2[3], diff[3], cross[3];

	if (status == xplm_MouseDown) 
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		ApplyTransform();

		mControlPoint = -1;
		double	bestZ = 0.0;
		for (int n = 0; n < 28; ++n)
		{
			double	screenPt[2];
			double	z;
			double	modPt[3];
			
			modPt[0] = kDeformInfo[n][0];
			modPt[1] = kDeformInfo[n][1];
			modPt[2] = kDeformInfo[n][2];
			ModelToScreenPt(modPt, screenPt, &z);
			
			double	distSq = (x - screenPt[0]) * (x - screenPt[0]) + 
							(y - screenPt[1]) * (y - screenPt[1]);
			if (distSq < 9.0)
			{
				if (mControlPoint == -1 || bestZ < z)
				{
					mControlPoint = n;
					bestZ = z;
				}
			}
		}

		glPopMatrix();

		mMouseX = x;
		mMouseY = y;
		
		return (mControlPoint != -1);
		
	} else {
		if (mControlPoint == -1)	return false;

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		ApplyTransform();

		double	dx = kDeformInfo[mControlPoint][0];
		double	dy = kDeformInfo[mControlPoint][1];
		double	dz = kDeformInfo[mControlPoint][2];
		
		double	plane[4] = { 0.0, 0.0, 0.0, 0.0 };
		
		int numNotZero = NumNotZero(dx,dy,dz);
		
		if (numNotZero == 3)
		{
			/* CORNER - TRACKBALL ROTATE AROUND POINT */
			double	sphere[4] = { 0.0, 0.0, 0.0, 1.8 };
			if (zoomer->FindPointOnSphere(bounds, sphere, mMouseX, mMouseY, pt1, false))
			if (zoomer->FindPointOnSphere(bounds, sphere, x, y, pt2, false))
			{
				vec3_normalize(pt1);
				vec3_normalize(pt2);
				vec3_cross(cross, pt1, pt2);

				double	theta = asin(vec3_length(cross)) * 180.0 / M_PI;
				vec3_normalize(cross);
				
				double	newMatrix[16], temp[16];
				buildRotation(newMatrix, theta, cross[0],cross[1],cross[2]);
				multMatrices(temp, mRotateMatrix, newMatrix);
				copyMatrix(mRotateMatrix, temp);
			}
		} 
		else if (numNotZero == 1)
		{
			/* CENTER - 1-AXIS SCALE/DRAG/MOVE */
			
			// We need to pick our plane carefully here...we're only going to 
			// care about one axis.  The other's going to be bogus.  So we need
			// to pick a plane that is as flat against the monitor as possible.
			// That's pretty straight forward: we know we want a line embedded
			// in the screen to be in the plane.  We also want our axis to be in
			// the plane.  So....
			
			// Transform the line vector into eye coordinates.
			// Takes its X and Y and form a line in eye coordinates that
			// goes perpendicular in the screen plane in x and Y (Z delta = 0)
			// Transform this line back to model-view coords.
			// Build the plane off of their cross-product
			
			double	majorAxisMV[4] = { dx, dy, dz, 1.0 };
			double	majorAxisE[4];
			double	minorAxisMV[4];
			double	minorAxisE[4] = { 0.0, 0.0, 0.0, 1.0 };
			
			double	mvMatrix[16], mvMatrix_i[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, mvMatrix);
			mvMatrix[12] = mvMatrix[13] = mvMatrix[14] = 0.0;
			invertMatrix(mvMatrix_i, mvMatrix);
			multMatrixVec(majorAxisE, mvMatrix, majorAxisMV);
			minorAxisE[0] = majorAxisE[1];
			minorAxisE[1] = majorAxisE[0];
			vec3_normalize(minorAxisE);
			multMatrixVec(minorAxisMV, mvMatrix_i, minorAxisE);
			vec3_normalize(minorAxisMV);
			
			vec3_cross(plane, majorAxisMV, minorAxisMV);
			vec3_normalize(plane);
					
			zoomer->FindPointOnPlane(bounds, plane, mMouseX, mMouseY, pt1, false);
			zoomer->FindPointOnPlane(bounds, plane, x, y, pt2, false);
			diff[0] = pt2[0] - pt1[0];
			diff[1] = pt2[1] - pt1[1];
			diff[2] = pt2[2] - pt1[2];

			XPLMKeyFlags	flags = XPLMGetModifiers(); 			

			if (dx == 0.0)	diff[0] = 0.0;
			if (dy == 0.0)	diff[1] = 0.0;
			if (dz == 0.0)	diff[2] = 0.0;

			if ((flags & (xplm_ShiftFlag + xplm_OptionAltFlag)) == (xplm_ShiftFlag + xplm_OptionAltFlag))
			{
				diff[0] = diff[1] = diff[2] = greatest_abs_3(diff[0], diff[1], diff[2]);
				dx = dy = dz = 1.0;
			}
			
			double	newScale[16], newTransform[16], temp[16];
			setIdentityMatrix(newScale);
			setIdentityMatrix(newTransform);

			if (flags & xplm_ControlFlag)
			{
				newTransform[12] = diff[0] * 1.0;
				newTransform[13] = diff[1] * 1.0;
				newTransform[14] = diff[2] * 1.0;
			} else if (flags & xplm_ShiftFlag) {
				newScale[0 ] = 1.0 + diff[0] * 1.0 * dx;
				newScale[5 ] = 1.0 + diff[1] * 1.0 * dy;
				newScale[10] = 1.0 + diff[2] * 1.0 * dz;
			} else {			
				newTransform[12] = diff[0] * 0.5;
				newTransform[13] = diff[1] * 0.5;
				newTransform[14] = diff[2] * 0.5;
				newScale[0 ] = 1.0 + diff[0] * 0.5 * dx;
				newScale[5 ] = 1.0 + diff[1] * 0.5 * dy;
				newScale[10] = 1.0 + diff[2] * 0.5 * dz;
			}
			
			multMatrices(temp, mScaleMatrix, newTransform);
			copyMatrix(newTransform, temp);
			multMatrices(temp, mRotateMatrix, newTransform);
			copyMatrix(newTransform, temp);
			newTransform[0] = newTransform[5] = newTransform[10] = 1.0;
			newTransform[1] = newTransform[2] = newTransform[3] =
			newTransform[4] = newTransform[6] = newTransform[7] =
			newTransform[8] = newTransform[9] = newTransform[11] = 0.0;

			multMatrices(temp, mTranslateMatrix, newTransform);
			copyMatrix(mTranslateMatrix, temp);

			multMatrices(temp, mScaleMatrix, newScale);
			copyMatrix(mScaleMatrix, temp);		
		} 
		else if (numNotZero == 2 && button == 1)
		{
			/* EDGE RIGHT BUTTON - ROTATE AROUND AXIS */
			if (dx == 0.0)	plane[0] = 1.0;
			if (dy == 0.0)	plane[1] = 1.0;
			if (dz == 0.0)	plane[2] = 1.0;
			
			zoomer->FindPointOnPlane(bounds, plane, mMouseX, mMouseY, pt1, false);
			zoomer->FindPointOnPlane(bounds, plane, x, y, pt2, false);
			
			vec3_normalize(pt1);
			vec3_normalize(pt2);			
			vec3_cross(cross, pt1, pt2);
			
			double	theta = asin(vec3_length(cross)) * 180.0 / M_PI;
			vec3_normalize(cross);
			
			double	newMatrix[16], temp[16];
			buildRotation(newMatrix, theta, cross[0],cross[1],cross[2]);
			multMatrices(temp, mRotateMatrix, newMatrix);
			copyMatrix(mRotateMatrix, temp);
		}
		else if (numNotZero == 2 && button == 0)
		{
			/* EDGE LEFT BUTTON - 2 AXIS MOVE/ROTATE/SCALE */
			
			if (dx == 0.0)	plane[0] = 1.0;
			if (dy == 0.0)	plane[1] = 1.0;
			if (dz == 0.0)	plane[2] = 1.0;
			
			zoomer->FindPointOnPlane(bounds, plane, mMouseX, mMouseY, pt1, false);
			zoomer->FindPointOnPlane(bounds, plane, x, y, pt2, false);
			diff[0] = pt2[0] - pt1[0];
			diff[1] = pt2[1] - pt1[1];
			diff[2] = pt2[2] - pt1[2];

			XPLMKeyFlags	flags = XPLMGetModifiers(); 			
			if ((flags & (xplm_ShiftFlag + xplm_OptionAltFlag)) == (xplm_ShiftFlag + xplm_OptionAltFlag))
			{
				if (dx == 0.0)	diff[1] = diff[2] = 0.5 * (diff[1] * dy + diff[2] * dz);
				if (dy == 0.0)	diff[0] = diff[2] = 0.5 * (diff[0] * dx + diff[2] * dz);
				if (dz == 0.0)	diff[0] = diff[1] = 0.5 * (diff[0] * dx + diff[1] * dy);
				diff[0] *= dx;
				diff[1] *= dy;
				diff[2] *= dz;

			}

			if (dx == 0.0)	diff[0] = 0.0;
			if (dy == 0.0)	diff[1] = 0.0;
			if (dz == 0.0)	diff[2] = 0.0;
			
			double	newScale[16], newTransform[16], temp[16];
			setIdentityMatrix(newScale);
			setIdentityMatrix(newTransform);
			if (flags & xplm_ControlFlag)
			{
				newTransform[12] = diff[0] * 1.0;
				newTransform[13] = diff[1] * 1.0;
				newTransform[14] = diff[2] * 1.0;
			} else if (flags & xplm_ShiftFlag) {
				newScale[0 ] = 1.0 + diff[0] * 1.0 * dx;
				newScale[5 ] = 1.0 + diff[1] * 1.0 * dy;
				newScale[10] = 1.0 + diff[2] * 1.0 * dz;
			} else {			
				newTransform[12] = diff[0] * 0.5;
				newTransform[13] = diff[1] * 0.5;
				newTransform[14] = diff[2] * 0.5;
				newScale[0 ] = 1.0 + diff[0] * 0.5 * dx;
				newScale[5 ] = 1.0 + diff[1] * 0.5 * dy;
				newScale[10] = 1.0 + diff[2] * 0.5 * dz;
			}
			
			multMatrices(temp, mScaleMatrix, newTransform);
			copyMatrix(newTransform, temp);
			multMatrices(temp, mRotateMatrix, newTransform);
			copyMatrix(newTransform, temp);
			newTransform[0] = newTransform[5] = newTransform[10] = 1.0;
			newTransform[1] = newTransform[2] = newTransform[3] =
			newTransform[4] = newTransform[6] = newTransform[7] =
			newTransform[8] = newTransform[9] = newTransform[11] = 0.0;

			multMatrices(temp, mTranslateMatrix, newTransform);
			copyMatrix(mTranslateMatrix, temp);

			multMatrices(temp, mScaleMatrix, newScale);
			copyMatrix(mScaleMatrix, temp);		
		}
		
		mMouseX = x;
		mMouseY = y;
		
		glPopMatrix();
		return true;
	}
	
	

	return false;		
}						

void	OE_CubeDeformer::GetTransform(double	outTransform[16])
{
	double temp[16];
	multMatrices(temp, mTranslateMatrix, mRotateMatrix);
	multMatrices(outTransform, temp, mScaleMatrix);
}

void	OE_CubeDeformer::ApplyTransform(void)
{
	glMatrixMode(GL_MODELVIEW);
	double	matrix[16];
	GetTransform(matrix);
	glMultMatrixd(matrix);
}
