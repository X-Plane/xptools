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
#ifndef RF_SELECTIONTOOL_H
#define RF_SELECTIONTOOL_H

#include "RF_MapTool.h"
#include "RF_Selection.h"
#include "MapDefs.h"

class	RF_SelectionTool : public RF_MapTool {
public:
					RF_SelectionTool(RF_MapZoomer * inZoomer);

	virtual	void	DrawFeedbackUnderlay(
							bool				inCurrent);
	virtual	void	DrawFeedbackOverlay(
							bool				inCurrent);
	virtual	bool	HandleClick(
							XPLMMouseStatus		inStatus,
							int 				inX,
							int 				inY,
							int 				inButton);

	virtual int		GetNumProperties(void);
	virtual	void	GetNthPropertyName(int, string&);
	virtual	double	GetNthPropertyValue(int);
	virtual	void	SetNthPropertyValue(int, double);

	virtual	int		GetNumButtons(void);
	virtual	void	GetNthButtonName(int, string&);
	virtual	void	NthButtonPressed(int);

	virtual	char *	GetStatusText(void);

private:
			bool	GetRectMapCoords(double coords[4]);
			void	DoSelectionPreview(void);

	// State we remember for dragging the mouse.
	int							mMouseStartX;
	int							mMouseStartY;
	int							mMouseX;
	int							mMouseY;
	double						mMoveLon;
	double						mMoveLat;
	bool						mIsDrag;
	bool						mIsMoveVertices;
	XPLMKeyFlags				mModifiers;

	// State we remember for the selection.
	set<Pmwx::Face_handle>		mFaceSelection;
	set<Pmwx::Halfedge_handle>	mEdgeSelection;
	set<Pmwx::Vertex_handle>	mVertexSelection;
	set<PointFeatureSelection>	mPointFeatureSelection;
};

#endif
