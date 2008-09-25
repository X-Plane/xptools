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
#ifndef HL_CAMERA_H
#define HL_CAMERA_H

/*

	HLCamera - THEORY OF OPERATION

	HLCamera is an abstraction of a bunch of camera-related code inside X-Plane.  The idea of this class
	is to keep all camera functions together so that we can manipulate the camera as we want, and then
	do culling operations based on the end results.

	FAKE FAR CLIPPING PLANES

	One problem with X-Camera is that the far visibility as set by the camera may be much much larger
	than the visibility we might expect.  Consider the case where X-Plane is going into orbital mode.
	In this case, the far clipping plane goes a loooong way away to accomidate outer space.  But we still
	only want to render the nearby scenery based on atmospheric visibility.

	We still want to be able to use our single camera-based cull to wipe out most geometry...if we can't
	do that, we end up with way too many buckets, and also nothing is as precise as the sphere test.  So
	we have the 'fake' far clipping plane.  This is a fake far clipping plane that can be set to any
	point and is used only for SphereInView tests.  It can be closer or farther than the real OGL frustum,
	but nearer is the only real usage.  Pass in positive numbers, in eye-coord units from the camera.

	NOTE: the fake far clipping plane is reset every time the camera frustum is reset.
*/

#if LIN
#  include <GL/gl.h>
#endif
#if APL || IBM
#  include <gl.h>
#endif

class	xcam_class {
private:

	bool		mIsOrtho;			// Are we orthographic or perspective projection?
	GLdouble	mNear;				// The near and far clipping plane, in Z coordinates.
	GLdouble	mFar;				// (These are always negative!)
	GLdouble	mFakeFar;			// See comments on fake far clipping planes

	GLdouble	mTop[3];			// For perspective cameras only:
	GLdouble	mLeft[3];			// Plane equations for the four slanted sides of the view
	GLdouble	mRight[3];			// frustum (in eye coordinates).  These are planes that
	GLdouble	mBottom[3];			// go through the point 0,0,0 in the form Ax + By + Cz = 0.

	GLdouble	mOrthoTop;			// For orthographic cameras only:
	GLdouble	mOrthoLeft;			// The dimensions of the sides of the camera (in eye coordinates).
	GLdouble	mOrthoRight;
	GLdouble	mOrthoBottom;

	GLdouble	mLocation[4];		// For all cameras - the camera's location.  Need 4th variable ("W" param) for proper OGL transforms!
	bool		mLocationStale;

public:

	GLdouble	mPerspective[16];	// Projection matrix for this camera.
	GLdouble	mModelView[16];		// Model-view matrix for this camera.

	xcam_class();

	/******************************************************************************************************
	 * CAMERA SETUP AND MOVEMENT ROUTINES
	 ******************************************************************************************************/

	// These routines control the perspective transformation the camera uses.  Control the field of view and near/far
	// clip planes (which controls how big things look).  Pass in FOV or calc it yourself.
	// IMPORTANT: these routes do not update OpenGL; you must use the "Load" routines to update OpenGL.

	void		SetupOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	void		SetupFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	void		SetupPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);

	// Set or remove a fake far clipping plane, see above.
	void		SetFakeFarClippingPlane(GLdouble fake_far);
	void		ResetFakeFarClippingPlane(void);

	// These routines control the model-view matrix.  Use this to move the camera.  You can absolutely position the camera
	// or apply a series of transformations.
	// NOTE: these routines also do not update OpenGL; call the "load" routines to update OpenGL.

	void		LookAtPtFromPt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
                       GLdouble centerx, GLdouble centery, GLdouble centerz,
                       GLdouble upx, GLdouble upy, GLdouble upz);
	void		LookAtPtFromDir(GLdouble centerx, GLdouble centery, GLdouble centerz,
						GLdouble heading, GLdouble tilt, GLdouble distance);

	void		Identity(void);
	void		Translate(GLdouble x, GLdouble y, GLdouble z);
	void		Rotate(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);

	// These routines copy parts of the camera into the OpenGL matrix.  Provided separately so you can avoid reloading
	// two matrices when only one must be loaded.  These routines change the current matrix.

	/******************************************************************************************************
	 * OPEN GL ROUTINES
	 ******************************************************************************************************/

	void		LoadPerspective(void) 	{ glMatrixMode(GL_PROJECTION); glLoadMatrixd(mPerspective); }
	void		LoadModelView(void)		{ glMatrixMode(GL_MODELVIEW ); glLoadMatrixd(mModelView  ); }

	/******************************************************************************************************
	 * CULLING ROUTINES
	 ******************************************************************************************************/

	// Given a sphere's center and radius, this routine tells whether the camera is at least partly within the view.

	inline bool		SphereInView(GLdouble x, GLdouble y, GLdouble z, GLdouble radius);
	void		CalcLocation(GLdouble	location[3]);

	/******************************************************************************************************
	 * SPECIAL OPENGL ROUTINES
	 ******************************************************************************************************/

	// Given a camera and a direction to look, this routine sets up this camera to look along the direction at
	// the other camera at the given distance.
	// Why would you ever want to do this!??!  When rendering the shadow texture for hardware-shadowing, this
	// routine lets you set up the "looking at the world from the Sun's perspective" camera, given the regular
	// camera.  The end result is that this camera is always orthographic.  (Since in practice the sun is
	// virtually infinitely far away, the sun's rays are parallel, so an orthographic camera models this.)
	// The distance you pass in controls how far away the camera is...make sure that it is far enough away that
	// nothing in the world is 'behind' the sun camera.  Look_dir must be a normalized vector (length 1.0).

	void		LookAtCamera(xcam_class * inCamera, GLdouble look_dir[3], GLdouble look_dist);

	// Call this routine to receive the min and max xyz coordinates (in world coordinates) that can be seen
	// by this camera.
	void		GetFrustumBounds(GLdouble	outMin[3], GLdouble outMax[3]);


	// This routine converts a 3-d point to a window coordinate.
	void		mtr_to_pix(GLdouble		inXYZ[3], GLdouble outPixel[2]);

};

inline bool		xcam_class::SphereInView(GLdouble x, GLdouble y, GLdouble z, GLdouble radius)
{
	// CULLING!  It is NOT a coincidence that you cannot _scale_ the model view matrix.
	// This is because we do culling by (1) transforming the center of the sphere by
	// the model view, and then (2) seeing if it is within "radius" of the frustum
	// edges.

	// First transform our Z coordinate.  Don't even do X and Y because there's a good chance
	// we might not care.

	register GLdouble	pt0, pt1, pt2;

	pt2 = x * mModelView[2] + y * mModelView[6] + z * mModelView[10] + mModelView[14];

	// Test against near and far clipping.  This eliminates a LOT of geometry, especially near (cause
	// half the world is prolly behind us.
	if ((pt2 - radius > mNear) ||
		(pt2 + radius < mFakeFar))
	{
		return false;
	}

	// Ok, we're in bounds, calc x and y.
	pt0 = x * mModelView[0] + y * mModelView[4] + z * mModelView[8 ] + mModelView[12];
	pt1 = x * mModelView[1] + y * mModelView[5] + z * mModelView[9 ] + mModelView[13];

	// Now figure out how close the pt is to our boundaries.

	// If we're orthogrpahic (e.g. culling for the sun), we can go faster, so try this first!
	if (mIsOrtho)
	{
		// Simply see if we're outside our bounds by more than radius.
		if (pt0 + radius < mOrthoLeft ||
			pt0 - radius > mOrthoRight ||
			pt1 + radius < mOrthoBottom ||
			pt1 - radius > mOrthoTop)
		{
			return false;
		}
		return true;
	}

	// Ok we really have to do our tests against the sides.  For speed, we check out bottom plane first.
	// During a climb this will wipe out nearly everything.  We do the top last because there isn't a lot
	// that gets culled up in the sky.
	GLdouble	bottom_dis = mBottom[0] * pt0 + mBottom[1] * pt1 + mBottom[2] * pt2; if (-bottom_dis >= radius) return false;
	GLdouble	left_dis   = mLeft  [0] * pt0 + mLeft  [1] * pt1 + mLeft  [2] * pt2; if (-left_dis   >= radius) return false;
	GLdouble	right_dis  = mRight [0] * pt0 + mRight [1] * pt1 + mRight [2] * pt2; if (-right_dis  >= radius) return false;
	GLdouble	top_dis    = mTop   [0] * pt0 + mTop   [1] * pt1 + mTop   [2] * pt2; if (-top_dis    >= radius) return false;

	return true;
}

#endif
