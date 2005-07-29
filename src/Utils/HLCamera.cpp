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
#include "hl_types.h"
#include "HLCamera.h"
#include "MatrixUtils.h"
#include <math.h>
#include <algorithm>

#define LIMIT_CAMERA_FOR_DEBUGGING 0

xcam_class::xcam_class()
{
	setIdentityMatrix(mPerspective);
	setIdentityMatrix(mModelView);	
	mIsOrtho = true;
	mLocationStale = true;
}
	
void		xcam_class::SetupOrtho(
				GLdouble left, GLdouble right, GLdouble bottom, 
				GLdouble top, GLdouble zNear, GLdouble zFar)
{
	mIsOrtho = true;

	buildOrthoMatrix(mPerspective, left, right, bottom, top, zNear, zFar);	
	
#if LIMIT_CAMERA_FOR_DEBUGGING	
	double wid = right - left;
	double hig = top - bottom;
	left += wid * 0.25;
	right -= wid * 0.25;
	bottom += hig * 0.25;
	top -= hig * 0.25;
#endif
	
	mOrthoTop = top;
	mOrthoLeft = left;
	mOrthoRight = right;
	mOrthoBottom = bottom;
	mNear = -zNear;
	mFar = -zFar;
	mFakeFar = mFar;

	mLocationStale = true;	
}

void		xcam_class::SetupFrustum(
				GLdouble left, GLdouble right, GLdouble bottom, 
				GLdouble top, GLdouble zNear, GLdouble zFar)
{
	mIsOrtho = false;
	
	buildFrustumMatrix(mPerspective, left, right, bottom, top, zNear, zFar);

#if LIMIT_CAMERA_FOR_DEBUGGING	
	double wid = right - left;
	double hig = top - bottom;
	left += wid * 0.25;
	right -= wid * 0.25;
	bottom += hig * 0.25;
	top -= hig * 0.25;
#endif

	// For culling, we will want the plane equations of each side of the
	// frustum.  An interesting mathematical thing: for a plane that goes through
	// the origin, the plane in form Ax + By + Cz + D = 0 A,B,C is the normal 
	// vector to the plane and D is 0.  So by calcing normals to the sides of our
	// frustum with a cross product, we can easily get our plane equations.
	// Later we will throw pts into them to figure out if we are in the frustum.
	
	GLdouble v1[3], v2[3], v3[3], v4[3] ;

	vec3_assign ( v1, left , top,    -zNear ) ;
	vec3_assign ( v2, right, top,    -zNear ) ;
	vec3_assign ( v3, left , bottom, -zNear ) ;
	vec3_assign ( v4, right, bottom, -zNear ) ;

	vec3_normalize ( v1 ) ;
	vec3_normalize ( v2 ) ;
	vec3_normalize ( v3 ) ;
	vec3_normalize ( v4 ) ;

	// Note that we always go clockwise around the frustum.  This guarantees that
	// all of our normals will point 'in' to the frustum.
	vec3_cross ( mTop,    v1, v2 ) ;	// Top left and top right make top
	vec3_cross ( mRight,  v2, v4 ) ;	// Top right and bottom right make right
	vec3_cross ( mBottom, v4, v3 ) ;	// Right bottom and left bot make bottom
	vec3_cross ( mLeft,   v3, v1 ) ;	// Left bottom, Left top makes left
	
	// Also remember the near and far clip planes.  Note: near and far planes
	// are always input as distances (and are positive), but the positive Z axis
	// faces to the user, so the near and far clip planes are always _negative_ with
	// zFar < zNear.
	mNear = -zNear;
	mFar = -zFar;
	mFakeFar = mFar;
	
	mLocationStale = true;		
}

void		xcam_class::SetupPerspective(
				GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
  GLdouble xmin, xmax, ymin, ymax;

  ymax = zNear * tan(fovy * M_PI / 360.0);
  ymin = -ymax;

  xmin = ymin * aspect;
  xmax = ymax * aspect;

  SetupFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void		xcam_class::SetFakeFarClippingPlane(GLdouble fake_far)
{
	// Fake far should be in meters, and all clipping planes are along the negative Z axis.
	mFakeFar = -fake_far;
}

void		xcam_class::ResetFakeFarClippingPlane(void)
{
	mFakeFar = mFar;
}


#pragma mark -

void		xcam_class::LookAtPtFromPt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
                   GLdouble centerx, GLdouble centery, GLdouble centerz,
                   GLdouble upx, GLdouble upy, GLdouble upz)
{
	buildLookAtMatrix(mModelView, eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);

	mLocationStale = true;	
}

void		xcam_class::LookAtPtFromDir(GLdouble centerx, GLdouble centery, GLdouble centerz,
					GLdouble heading, GLdouble tilt, GLdouble distance)
{
	tilt *= DEG2RAD;
	heading *= DEG2RAD;
	GLdouble ground_dist = distance * cos(tilt);
	GLdouble loc[3];
	loc[1] = distance * sin(tilt);
	loc[0] = ground_dist * cos(heading);
	loc[2] = ground_dist * sin(heading);
	
	buildLookAtMatrix(mModelView,
				loc[0] + centerx, loc[1] + centery, loc[2] + centerz,
				centerx, centery, centerz,
				0.0, 1.0, 0.0);
				
	mLocationStale = true;					
}					

void		xcam_class::Identity(void)
{
	setIdentityMatrix(mModelView);
	
	mLocationStale = true;		
}

void		xcam_class::Translate(GLdouble x, GLdouble y, GLdouble z)
{
	applyTranslation(mModelView, x, y, z);
	mLocationStale = true;	
}

void		xcam_class::Rotate(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	GLdouble	m[16], temp[16];
	buildRotation(m, angle, x, y, z);
	multMatrices(temp, mModelView,m );
	copyMatrix(mModelView, temp);
	mLocationStale = true;	
}

	
#pragma mark -

	


void	xcam_class::CalcLocation(GLdouble	location[3])
{
	if (mLocationStale)
	{
		GLdouble	m[16];
		mLocation[0] = mLocation[1] = mLocation[2] = 0.0;
		mLocation[3] = 1.0;
		if (invertMatrix(m, mModelView))
		{			
			applyMatrixVec(mLocation, m);
		} 
	}
	mLocationStale = false;
	location[0] = mLocation[0];	
	location[1] = mLocation[1];	
	location[2] = mLocation[2];		
}


void	xcam_class::LookAtCamera(xcam_class * inCamera, GLdouble look_dir[3], GLdouble look_dist)
{
	// Set up this camera to point to the center of the other camera based on the angle and
	// distance.  The _hard_ part is that we have to set our culling bounds to include 
	// any object that could cast a shadow that would be seen in the other camera.  Crazy!

	// First we create "inverse" matrices for the other camera.  This lets us take things as
	// seen by the other camera and get them into our coordinates, by first implying the inverse
	// matrices from the other camera and then ours.
	GLdouble	i_perp[16], i_mod[16];
		
	invertMatrix(i_perp, inCamera->mPerspective);
	invertMatrix(i_mod, inCamera->mModelView);

	// In "eye-coordinates", the center of a camera is always 0,0,0.  By inverse-applying
	// the other camera's location to the point 0,0,0, we get the center of the other camera
	// in world coordinates.  When done, we have to patch the 3rd coordinate to be 1.0 for
	// mathematical reasons that I won't get into.

	GLdouble	cntr[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLdouble	loc[4];
	
	applyMatrixVec(cntr, i_perp);
	applyMatrixVec(cntr, i_mod);
	cntr[3] = 1.0;
	
	// Now figure out where our camera is.  That's easy...just go "backward" from the center
	// of the camera along our look direction by the distance.  Note that look_dir MUST be
	// normalized.
	
	loc[0] = cntr[0] - (look_dir[0] * look_dist);
	loc[1] = cntr[1] - (look_dir[1] * look_dist);
	loc[2] = cntr[2] - (look_dir[2] * look_dist);
	loc[3] = 1.0;
	
	// Now just look from our camera to the other camera.  This sets up our model view matrix.
	
	LookAtPtFromPt(loc[0], loc[1], loc[2],
				cntr[0], cntr[1], cntr[2], 
				0.0, 1.0, 0.0);

	// Here's where things get really weird.  We need to set up our culling so that we include
	// everything that shadows the other camera's space within our camera.  Remember, a shadow-
	// casting objects might be off screen from the main camera, but they MUST be seen by the sun
	// camera to cast shadows.

	// Get the near and far planes from the old camera.

	GLdouble	zNear = inCamera->mNear;
	GLdouble	zFar = inCamera->mFar;	// NOTE: it is expected that the sun cam does NOT have a fake far plane!

	// We want to build a cube that represents the visible extent of the old camera.  
	// What are the locations of that cube?  Well...after "perspective division" the 
	// x and y coordinates must be -1 or 1.  And the Z coordinates are all zNear or zFar.
	// So...it turns out that all coordinates must be built in terms of zNear and zFar 
	// to build a unit-sized cube.

	GLdouble	pts[8][4] = {
		{ -zNear, -zNear, -zNear, zNear },	// -1 for a normalized Z is the near clipping plane,
		{ -zFar , -zFar ,  zFar , zFar  },	// so Z must = -zNear.  Given Z we know W, and we
		{ -zNear,  zNear, -zNear, zNear },	// can fill in X and Y.  Similarly, use far for the
		{ -zFar ,  zFar ,  zFar , zFar  },	// Z(n) = 1 coordinate.
		{  zNear, -zNear, -zNear, zNear },
		{  zFar , -zFar ,  zFar , zFar  },
		{  zNear,  zNear, -zNear, zNear },
		{  zFar ,  zFar ,  zFar , zFar  } };

	// Now we transform the cube by the inverse perspective, inverse model view, and our perspective
	// matrix.  This gives us the bounding cube of the old camera in our eye coordinates.  Of course,
	// it isn't a cube along the X, Y and Z axes anymore, it goes every which way.

	for (int n = 0; n < 8; ++n)
	{
		applyMatrixVec(pts[n], i_perp);
		applyMatrixVec(pts[n], i_mod);
		applyMatrixVec(pts[n], mModelView);
	}
	
	// Now find the min and max coordinates along each axis.  This will form the limits of what
	// area our camera must include.
	
	GLdouble	minX = pts[0][0];
	GLdouble	maxX = pts[0][0];
	GLdouble	minY = pts[0][1];
	GLdouble	maxY = pts[0][1];
	GLdouble	minZ = pts[0][2];
	GLdouble	maxZ = pts[0][2];
	
	for (int n = 0; n < 8; ++n)
	{
		minX = std::min(minX, pts[n][0]);
		maxX = std::max(maxX, pts[n][0]);

		minY = std::min(minY, pts[n][1]);
		maxY = std::max(maxY, pts[n][1]);

		minZ = std::min(minZ, pts[n][2]);
		maxZ = std::max(maxZ, pts[n][2]);
	}
	
	// These form the limits of our camera...use these to do orthographic projection
	// with those bounds as the limits.
	
	SetupOrtho(minX, maxX, minY, maxY, minZ, maxZ);	
}

void		xcam_class::GetFrustumBounds(GLdouble	outMin[3], GLdouble outMax[3])
{
	GLdouble	i_perp[16], i_mod[16];
		
	invertMatrix(i_perp, mPerspective);
	invertMatrix(i_mod, mModelView);

	if (mIsOrtho)
	{
		GLdouble	pts[8][4] = {
			{ mOrthoLeft , mOrthoTop   , -mNear, 1.0 },
			{ mOrthoLeft , mOrthoTop   , -mFar , 1.0 },
			{ mOrthoRight, mOrthoTop   , -mNear, 1.0 },
			{ mOrthoRight, mOrthoTop   , -mFar , 1.0 },
			{ mOrthoLeft , mOrthoBottom, -mNear, 1.0 },
			{ mOrthoLeft , mOrthoBottom, -mFar , 1.0 },
			{ mOrthoRight, mOrthoBottom, -mNear, 1.0 },
			{ mOrthoRight, mOrthoBottom, -mFar , 1.0 } };

		for (int n = 0; n < 8; ++n)
		{
			applyMatrixVec(pts[n], i_mod);
		}
		
		GLdouble	minX = pts[0][0];
		GLdouble	maxX = pts[0][0];
		GLdouble	minY = pts[0][1];
		GLdouble	maxY = pts[0][1];
		GLdouble	minZ = pts[0][2];
		GLdouble	maxZ = pts[0][2];
		
		for (int n = 0; n < 8; ++n)
		{
			minX = std::min(minX, pts[n][0]);
			maxX = std::max(maxX, pts[n][0]);

			minY = std::min(minY, pts[n][1]);
			maxY = std::max(maxY, pts[n][1]);

			minZ = std::min(minZ, pts[n][2]);
			maxZ = std::max(maxZ, pts[n][2]);
		}

		outMin[0] = minX;
		outMin[1] = minY;
		outMin[2] = minZ;
		outMax[0] = maxX;
		outMax[1] = maxY;
		outMax[2] = maxZ;
	
	} else {

		GLdouble	zNear = mNear;
		GLdouble	zFar = mFakeFar;
		GLdouble	pts[8][4] = {
			{ -zNear, -zNear, -zNear, zNear },	// -1 for a normalized Z is the near clipping plane,
			{ -zFar , -zFar ,  zFar , zFar  },	// so Z must = -zNear.  Given Z we know W, and we
			{ -zNear,  zNear, -zNear, zNear },	// can fill in X and Y.  Similarly, use far for the
			{ -zFar ,  zFar ,  zFar , zFar  },	// Z(n) = 1 coordinate.
			{  zNear, -zNear, -zNear, zNear },
			{  zFar , -zFar ,  zFar , zFar  },
			{  zNear,  zNear, -zNear, zNear },
			{  zFar ,  zFar ,  zFar , zFar  } };

		for (int n = 0; n < 8; ++n)
		{
			applyMatrixVec(pts[n], i_perp);
			applyMatrixVec(pts[n], i_mod);
		}
		
		GLdouble	minX = pts[0][0];
		GLdouble	maxX = pts[0][0];
		GLdouble	minY = pts[0][1];
		GLdouble	maxY = pts[0][1];
		GLdouble	minZ = pts[0][2];
		GLdouble	maxZ = pts[0][2];
		
		for (int n = 0; n < 8; ++n)
		{
			minX = std::min(minX, pts[n][0]);
			maxX = std::max(maxX, pts[n][0]);

			minY = std::min(minY, pts[n][1]);
			maxY = std::max(maxY, pts[n][1]);

			minZ = std::min(minZ, pts[n][2]);
			maxZ = std::max(maxZ, pts[n][2]);
		}

		outMin[0] = minX;
		outMin[1] = minY;
		outMin[2] = minZ;
		outMax[0] = maxX;
		outMax[1] = maxY;
		outMax[2] = maxZ;
	}
}

#if SIM

void xcam_class::mtr_to_pix(GLdouble inXYZ[3],GLdouble outPixel[2])
{
	GLdouble vec[4]={inXYZ[0],inXYZ[1],inXYZ[2],1.0};
	applyMatrixVec(vec,mModelView	);	
	applyMatrixVec(vec,mPerspective	);	
	if(	vec[3]){
		vec[0]/=vec[3];
		vec[1]/=vec[3];}

	GLfloat	viewport[4];
	glGetFloatv(GL_VIEWPORT, viewport);
	vec[0]		+=1.0;
	vec[1]		+=1.0;
	vec[0]		*=0.5;
	vec[1]		*=0.5;
	outPixel[0]	 =(vec[0]*viewport[2]+viewport[0]-mnw.x_off	)/mnw.xy_scale;	// xoff is on the left, we need to offset by it
	outPixel[1]	 =(vec[1]*viewport[3]+viewport[1]			)/mnw.xy_scale;	// yoff is on the top, we don't need to offset by it
}

#endif