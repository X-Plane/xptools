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
#ifndef OE_TEXEDWINDOW_H
#define OE_TEXEDWINDOW_H

#include "OE_Pane.h"
#include "OE_Notify.h"
#include "OE_Globals.h"

class	OE_Scroller;
class	OE_TexEdPane;

enum	{
	oe_DirectEd,
	oe_PatchEd,
	oe_ProjEd
};

class	OE_TexEdWindow : public OE_Pane {
public:

						OE_TexEdWindow(
									int			inLeft,
									int			inTop,
									int			inRight,
									int			inBottom,
									int			inKind);
						~OE_TexEdWindow();

private:

		OE_Scroller *		mScroller;
		OE_TexEdPane *		mPane;

};


class	OE_TexEdPane : public OE_Pane, OE_Notifiable {
public:

						OE_TexEdPane(
									int			inLeft,
									int			inTop,
									int			inRight,
									int			inBottom,
									OE_Pane *	inParent);
	virtual				~OE_TexEdPane();

	virtual	void		DrawSelf(void);
	virtual	void		HandleNotification(int catagory, int message, void * param);
	virtual	int			HandleClick(XPLMMouseStatus status, int x, int y, int button);
	virtual	int			HandleMouseWheel(int x, int y, int direction);

protected:

			float		GetHandleRadius(bool inSquared);
			float		XCoordToS(float	inX);
			float		YCoordToT(float inY);
			float		SCoordToX(float	inX);
			float		TCoordToY(float inY);

		float		mScale;

private:

			void		RecalcSize(void);


};

class	OE_DirectEdPane: public OE_TexEdPane {
public:

						OE_DirectEdPane(
									int			inLeft,
									int			inTop,
									int			inRight,
									int			inBottom,
									OE_Pane *	inParent);
	virtual				~OE_DirectEdPane();

	virtual	void		DrawSelf(void);
	virtual	int			HandleClick(XPLMMouseStatus status, int x, int y, int button);
	virtual	void		HandleNotification(int catagory, int message, void * param);

private:

		int			mDragCmd;		// The command index for the primitive being dragged
		int			mDragVertex;	// The vertex index in that command
//		float		mDragS;			// The original S&T coordinates when drag started
//		float		mDragT;			// The original  "       "

		XObj		mOriginal;
		int			mMouseX;		// The original mouse coordinates when drag started
		int			mMouseY;
};

class	OE_PatchEdPane : public OE_TexEdPane {
public:

						OE_PatchEdPane(
									int			inLeft,
									int			inTop,
									int			inRight,
									int			inBottom,
									OE_Pane *	inParent,
									bool		inProjection);
	virtual				~OE_PatchEdPane();

	virtual	void		DrawSelf(void);
	virtual	int			HandleClick(XPLMMouseStatus status, int x, int y, int button);
	virtual	void		HandleNotification(int catagory, int message, void * param);

private:

			bool			HasTexture(void);
			OE_Texture_t&	GetTexture(void);

	float	mDragS1;		// Original geometry of the patch on drag start
	float	mDragS2;
	float	mDragT1;
	float	mDragT2;
	float	mMouseX;		// Start of mouse dragging
	float	mMouseY;
	int		mHandle;		// Our current handle or 8 to drag the whole thing
	bool	mProjection;
};

#endif