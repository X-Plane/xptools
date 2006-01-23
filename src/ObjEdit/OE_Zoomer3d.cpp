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
#include "OE_Zoomer3d.h"
#include "trackball.h"
#include "MatrixUtils.h"
#include "GEoUtils.h"
#include <gl/glu.h>
const	float	kCamDist = 500;
const	float	kNear = 50.0;
const	float	kFar = 2000.0;
const	float	kFOV = 30.0;

static	inline	float	Interp2d(
							float		val,
							float		inMin,
							float		inMax,
							float		outMin,
							float		outMax)
{
	return outMin + ((val - inMin) * (outMax - outMin) / (inMax - inMin));
}					
		
OE_Zoomer3d::OE_Zoomer3d()
{
	float a[3] = { 0.0, 1.0, 0.0 };
	axis_to_quat(a, 0.0, mRotation); 	
	mScale = 1.0;
	mTranslation[0] = 0.0;
	mTranslation[1] = 0.0;
	mTranslation[2] = 0.0;
}


void		OE_Zoomer3d::SetupMatrices(	
							int				inBounds[4])
{
	float	vp[4];

	glPushAttrib(GL_VIEWPORT_BIT);
	glGetFloatv(GL_VIEWPORT, vp);

	glViewport(inBounds[0] + vp[0], inBounds[1] + vp[1], inBounds[2] - inBounds[0], inBounds[3] - inBounds[1]);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

  	double	yMax = kNear * tan(kFOV * M_PI / 360.0);
  	double	yMin = -yMax;
  	double	xMin = yMin * (float)(inBounds[2] - inBounds[0]) / (float)(inBounds[3] - inBounds[1]);
  	double	xMax = yMax * (float)(inBounds[2] - inBounds[0]) / (float)(inBounds[3] - inBounds[1]);

	glFrustum(xMin, xMax, yMin, yMax, kNear, kFar);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	gluLookAt(	0.0, 0.0, -kCamDist, 
				0.0, 0.0, 0.0, 
				0.0, 1.0, 0.0);

	float	m[4][4];

	glScalef(mScale, mScale, mScale);
	
	build_rotmatrix(m, mRotation);
	glMultMatrixf(&m[0][0]);

	glTranslatef(mTranslation[0],mTranslation[1],mTranslation[2]);
}		
		
							
void		OE_Zoomer3d::ResetMatrices(void)
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();		
	glPopAttrib();
}

void		OE_Zoomer3d::ResetToIdentity(void)
{
	float a[3] = { 0.0, 1.0, 0.0 };
	axis_to_quat(a, 0.0, mRotation); 	
	mScale = 1.0;
	mTranslation[0] = 0.0;
	mTranslation[1] = 0.0;
	mTranslation[2] = 0.0;
}


void		OE_Zoomer3d::SetScale(
							float			inScale)
{
	mScale = inScale;
}							


void		OE_Zoomer3d::HandleRotationClick(
						int 			inBounds[4],
						XPLMMouseStatus	inStatus,
						int 			inX,
						int 			inY)
{
	static	float	x_drag, y_drag;
	float	l = inBounds[0];
	float	b = inBounds[1];
	float	r = inBounds[2];
	float	t = inBounds[3];
	float	x = inX;
	float	y = inY;
	
	if (inStatus != xplm_MouseDown)
	{
		float x1 = (float) (x_drag-l) / (float) (r - l);
		float x2 = (float) (x-l) / (float) (r - l);
		float y1 = (float) (y_drag-b) / (float) (t - b);
		float y2 = (float) (y-b) / (float) (t - b);
		
		x1 *= 2.0; x1 -= 1.0;
		x2 *= 2.0; x2 -= 1.0;
		y1 *= 2.0; y1 -= 1.0;
		y2 *= 2.0; y2 -= 1.0;

		float q[4];
		trackball(q, x1, -y1, x2, -y2);				
		add_quats(q, mRotation, mRotation);
	}
	x_drag = x;
	y_drag = y;		
}						
						
void		OE_Zoomer3d::HandleTranslationClick(
						int				inBounds[4],
						XPLMMouseStatus	inStatus,
						int				inX,
						int				inY)
{
	static	int	firstX, firstY;
	if (inStatus != xplm_MouseDown)
	{
		TranslateBy2d(inBounds, firstX, firstY, inX, inY);			
	}
	
	firstX = inX;
	firstY = inY;
}
						
void		OE_Zoomer3d::HandleZoomWheel(
							int				inBounds[4],
							int				inDelta,
							int				inX,
							int				inY)
{
	float	zoom = 1.0;
	while (inDelta > 0)
	{
		zoom *= 1.05;
		inDelta--;
	}
	while (inDelta < 0)
	{
		zoom *= 0.95;
		inDelta++;
	}
	ZoomAroundPoint(inBounds, inX, inY, zoom);
}							

bool		OE_Zoomer3d::FindPointOnPlane(
							int				inBounds[4],
							double			plane[4],
							double			inClickX,
							double			inClickY,
							double			where[3],
							bool			doSetup)
{
  	double	yMax = kNear * tan(kFOV * M_PI / 360.0);
  	double	yMin = -yMax;
  	double	xMin = yMin * (float)(inBounds[2] - inBounds[0]) / (float)(inBounds[3] - inBounds[1]);
  	double	xMax = yMax * (float)(inBounds[2] - inBounds[0]) / (float)(inBounds[3] - inBounds[1]);

	if (doSetup)
		SetupMatrices(inBounds);
	bool retval = ::FindPointOnPlane(xMin, xMax, yMin, yMax,
		kNear,
		plane,
		inClickX,
		inClickY,
		where);
	if (doSetup)
		ResetMatrices();
	return retval;
}


bool		OE_Zoomer3d::FindPointOnSphere(
							int				inBounds[4],
							double			sphere[4],
							double			inClickX,
							double			inClickY,
							double			where[3],
							bool			doSetup)
{
  	double	yMax = kNear * tan(kFOV * M_PI / 360.0);
  	double	yMin = -yMax;
  	double	xMin = yMin * (float)(inBounds[2] - inBounds[0]) / (float)(inBounds[3] - inBounds[1]);
  	double	xMax = yMax * (float)(inBounds[2] - inBounds[0]) / (float)(inBounds[3] - inBounds[1]);

	if (doSetup)
		SetupMatrices(inBounds);
	bool retval = ::FindPointOnSphere(xMin, xMax, yMin, yMax,
		kNear,
		sphere,
		inClickX,
		inClickY,
		where);
	if (doSetup)
		ResetMatrices();
	return retval;
}

void		OE_Zoomer3d::TranslateBy2d(
							int				inBounds[4],
							float			inX1,
							float			inY1,
							float			inX2,
							float			inY2)
{
	SetupMatrices(inBounds);
	
	// Reverse x,y,0 to origin pt
	// Reverse center of rect,0 to dest
	
	float	p1[3], p2[3];
	Map2dTo3d(inX1, inY1, p1);
	Map2dTo3d(inX2, inY2, p2);
	
	ResetMatrices();

	mTranslation[0] += (p2[0] - p1[0]);
	mTranslation[1] += (p2[1] - p1[1]);
	mTranslation[2] += (p2[2] - p1[2]);

}							

void	OE_Zoomer3d::ZoomAroundPoint(
						int		inBounds[4],
						int		inZoomX,
						int		inZoomY,
						float	inZoomFactor)
{
	float	ctrX = (inBounds[0] + inBounds[2]) * 0.5;
	float	ctrY = (inBounds[1] + inBounds[3]) * 0.5;
	
	TranslateBy2d(inBounds, inZoomX, inZoomY, ctrX, ctrY);

	mScale *= inZoomFactor;

	TranslateBy2d(inBounds, ctrX, ctrY, inZoomX, inZoomY);
}

void	OE_Zoomer3d::Map2dTo3d(
							float			inX,
							float			inY,
							float			outXYZ[3])
{
	GLdouble	model_view[16], i_model_view[16];
	GLdouble	viewport[4];
	
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	glGetDoublev(GL_VIEWPORT, viewport);
	invertMatrix(i_model_view, model_view);

  	double	yMax = kCamDist * tan(kFOV * M_PI / 360.0);
  	double	yMin = -yMax;
  	double	xMin = yMin * (viewport[2]) / (viewport[3]);
  	double	xMax = yMax * (viewport[2]) / (viewport[3]);
  		
	GLdouble	click[4] = { 
		Interp2d(inX, viewport[0], viewport[0] + viewport[2], xMin, xMax),
		Interp2d(inY, viewport[1], viewport[1] + viewport[3], yMin, yMax),
		-kCamDist,
		1.0 
	};

	GLdouble	pt1[4];
					
	multMatrixVec(pt1, i_model_view, click);
	
	outXYZ[0] = pt1[0];
	outXYZ[1] = pt1[1];
	outXYZ[2] = pt1[2];
}