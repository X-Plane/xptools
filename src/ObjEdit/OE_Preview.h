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
#ifndef OE_PREVIEW_H
#define OE_PREVIEW_H

#include "OE_Pane.h"
#include "OE_CubeDeformer.h"
#include "OE_TexProjector.h"

enum {

	oe_PreviewType_Preview = 0,
	oe_PreviewType_Select,
	oe_PreviewType_HiddenSurfaces,
	oe_PreviewType_Projection,
	oe_PreviewType_ProjectionPreview

};

class	OE_Preview : public OE_Pane {
public:

					OE_Preview(
                                   	int                  inLeft,
                                   	int                  inTop,
                                   	int                  inRight,
                                   	int                  inBottom,
									int					 inKind);
	virtual			~OE_Preview();

	virtual	void	DrawSelf(void);
	virtual	int		HandleClick(XPLMMouseStatus status, int x, int y, int button);
	virtual	int		HandleMouseWheel(int x, int y, int direction);

private:

	OE_Pane *	mDisplayPopup[3];

	int		mKind;

	// Mouse tracking state variables
	int		mDragX1;
	int		mDragX2;
	int		mDragY1;
	int		mDragY2;
	bool	mDragging;

	vector<VisibleVector>	mVisible;
	set<int>				mOriginalSelection;
	bool					mMovedOff;

};


#endif
