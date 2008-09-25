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
#ifndef OE_PROJECTIONMGR_H
#define OE_PROJECTIONMGR_H

#include "XObjDefs.h"
#include "OE_Notify.h"
#include "OE_Globals.h"
#include "XPLMDisplay.h"

class	OE_AbstactTexProjector;
class	OE_CubeDeformer;
class	OE_Zoomer3d;

enum {

	projector_Plane = 0,
	projector_Cylinder = 1,
	projector_Sphere = 2
};

class	OE_ProjectionMgr : public OE_Notifiable {
public:

						OE_ProjectionMgr();
						~OE_ProjectionMgr();

			void		DrawProjectionSetup(void);
			bool		TrackClick(
								OE_Zoomer3d *	zoomer,
								int				bounds[4],
								XPLMMouseStatus status,
								int 			x,
								int 			y,
								int 			button);
			const XObj&		GetPreviewObj(void);
			OE_Texture_t&	GetTexture(void);

			void		ApplyTexture(void);
			void		SetProjector(int type);
			int			GetProjector(void);

	virtual	void		HandleNotification(int catagory, int message, void * param);

private:

			void		ApplyToObj(void);

	XObj						mPreviewObj;

	OE_CubeDeformer *			mDeformer;
	OE_AbstactTexProjector *	mProjector[3];

	OE_Texture_t				mTexture;

	int							mCurProjector;

};

#endif
