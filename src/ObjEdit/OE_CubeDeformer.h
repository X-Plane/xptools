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
#ifndef OE_CUBEDEFORMER_H
#define OE_CUBEDEFORMER_H

#include "XPLMDisplay.h"

class	OE_Zoomer3d;

/*

	The cube deformer provides an interface to apply a series of
	matrix linear transformations (rotate, scale, translate, cumulative)
	to a solid object.  It can draw control points and handle tracking.

	When calling DrawDeformer, make sure the matrix is pre-set up to a unit cube
	surrounding your solid.

	When calling TrackClick, also make sure the matrices are all set up.

	After each call to TrackClick that returns true and isn't a down click,
	call GetTransform to see if a transform has been applied.

*/

class	OE_CubeDeformer {
public:

			OE_CubeDeformer();

	void	DrawDeformer(void);

	bool	TrackClick(
					OE_Zoomer3d *	zoomer,
					int				bounds[4],
					XPLMMouseStatus status,
					int 			x,
					int 			y,
					int 			button);

	void	GetTransform(double		outTransform[16]);
	void	ApplyTransform(void);

private:

	int			mMouseX;
	int			mMouseY;
	int			mControlPoint;
	double		mRotateMatrix[16];
	double		mTranslateMatrix[16];
	double		mScaleMatrix[16];

};


#endif