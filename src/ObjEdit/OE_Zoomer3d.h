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
#ifndef OE_ZOOMER3D_H
#define OE_ZOOMER3D_H

//#include "XPLMDisplay.h"

enum {
     xplm_MouseDown                           = 1,
     xplm_MouseDrag                           = 2,
     xplm_MouseUp                             = 3,

     xplm_MouseWheel						  = 4
};
typedef int XPLMMouseStatus;

/*

	OE_Zoomer3d

	This class manages camera control for a 3-d model.  It implements the gestures
	for dragging the model around.

	This code has to be in one place because you need to know about camera placement
	to implement 3-d manipulation given 2-d input.

*/


class	OE_Zoomer3d {
public:
				OE_Zoomer3d();

	/* Setting up the camera. */

	void		SetupMatrices(
							int				inBounds[4]);
	void		ResetMatrices(void);

	/* Direct access */

	void		ResetToIdentity(void);

	void		SetScale(
							float			inScale);

	/* Mouse Gestures */

	void		HandleRotationClick(
							int 			inBounds[4],
							XPLMMouseStatus	inStatus,
							int 			inX,
							int 			inY);
	void		HandleTranslationClick(
							int				inBounds[4],
							XPLMMouseStatus	inStatus,
							int				inX,
							int				inY);
	void		HandleZoomWheel(
							int				inBounds[4],
							int				inDelta,
							int				inX,
							int				inY);

	/* Since we are the camera, we can do various selection-like things. */

	bool		FindPointOnPlane(
							int				inBounds[4],
							double			plane[4],
							double			inClickX,
							double			inClickY,
							double			where[3],
							bool			doSetup = true);

	bool		FindPointOnSphere(
							int				inBounds[4],
							double			sphere[4],
							double			inClickX,
							double			inClickY,
							double			where[3],
							bool			doSetup = true);

private:

	void		TranslateBy2d(
							int				inBounds[4],
							float			inX1,
							float			inY1,
							float			inX2,
							float			inY2);

	void		ZoomAroundPoint(
							int				inBounds[4],
							int				inZoomX,
							int				inZoomY,
							float			inZoomFactor);

	void		Map2dTo3d(
							float			inX,
							float			inY,
							float			outXYZ[3]);

	float				mRotation[4];
	float				mScale;
	float				mTranslation[3];

};

#endif

