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
#include "RF_CropTool.h"

#include "RF_MapTool.h"
#include "RF_MapZoomer.h"
#include "RF_Notify.h"
#include "RF_Msgs.h"
#include "RF_Globals.h"
#include "RF_Progress.h"
#include "MapAlgs.h"
#include "MapTopology.h"
#include "UIUtils.h"
#include "AssertUtils.h"
#include "RF_DrawMap.h"
#include "XPLMGraphics.h"
#include "GISTool_Globals.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif


static const DragHandleInfo_t kHandleInfos[8] = {
{ /* BL */	-1, -1,		1,	1		},
{ /* B  */	 0,	-1,		0,	1		},
{ /* BR */	 1,	-1,		1,	1		},

{ /*  L */	-1,  0,		1,	0		},
{ /*  R */	 1,	 0,		1,	0		},

{ /* TL */	-1,  1,		1,	1		},
{ /* T  */	 0,	 1,		0,	1		},
{ /* TR */	 1,	 1,		1,	1		} };

static const	int	kHandleToBoundsX[8] = { 0, 4, 2, 0, 2, 0, 4, 2 };
static const	int	kHandleToBoundsY[8] = { 1, 1, 1, 5, 5, 3, 3, 3
 };

static const char * kFieldNames[] = { "West", "South", "East", "North" };
static const char * kBtnNames[] = { "All", "Screen", "Clear", "Crop", "Delete", "Remove Oceans" };

RF_CropTool::RF_CropTool(RF_MapZoomer * inZoomer) :
	RF_MapTool(inZoomer),
	mHandles(8, kHandleInfos, 4, this),
	mCrop(false)
{
	mBounds[0] = mBounds[1] = mBounds[2] = mBounds[3] = 0.0;
}

RF_CropTool::~RF_CropTool()
{
}

void	RF_CropTool::DrawFeedbackUnderlay(
							bool				inCurrent)
{
}

void	RF_CropTool::DrawFeedbackOverlay(
							bool				inCurrent)
{
	if (inCurrent && mCrop)
	{
		XPLMSetGraphicsState(0, 0, 0,    0, 0,  0, 0);
		glColor3f(0.0, 1.0, 0.3);
		glBegin(GL_QUADS);
		for (int n = 0; n < 8; ++n)
			mHandles.DrawHandle(n);
		glEnd();
		glBegin(GL_LINES);
		mHandles.ConnectHandle(0, 2);	// bottom
		mHandles.ConnectHandle(5, 7);	// top
		mHandles.ConnectHandle(0, 5);	// left
		mHandles.ConnectHandle(2, 7);	// right
		glEnd();
	}
}

bool	RF_CropTool::HandleClick(
							XPLMMouseStatus		inStatus,
							int 				inX,
							int 				inY,
							int 				inButton)
{
	if (!mCrop) return false;
	if (inButton != 0) return false;
	switch(inStatus) {
	case xplm_MouseDown:
		return mHandles.StartDrag(inX, inY);
	case xplm_MouseDrag:
		mHandles.ContinueDrag(inX, inY);
		return true;
	case xplm_MouseUp:
		mHandles.EndDrag(inX, inY);
		return true;
	}
	return false;
}

#pragma mark -

int		RF_CropTool::GetNumProperties(void)
{
	return 4;
}

void	RF_CropTool::GetNthPropertyName(int n, string& s)
{
	s = kFieldNames[n];
}

double	RF_CropTool::GetNthPropertyValue(int n)
{
	return mBounds[n];
}

void	RF_CropTool::SetNthPropertyValue(int n, double v)
{
	mBounds[n] = v;
}

#pragma mark -

int		RF_CropTool::GetNumButtons(void)
{
	return 6;
}

void	RF_CropTool::GetNthButtonName(int n, string& s)
{
	s = kBtnNames[n];
}

void	RF_CropTool::NthButtonPressed(int n)
{
	switch(n) {
	case 0: /* ALL */
		GetZoomer()->GetMapLogicalBounds(mBounds[0], mBounds[1], mBounds[2], mBounds[3]);
		mCrop = true;
		break;
	case 1: /* VISIBLE */
		GetZoomer()->GetMapVisibleBounds(mBounds[0], mBounds[1], mBounds[2], mBounds[3]);
		mCrop = true;
		break;
	case 2: /* CLEAR */
		mCrop = false;
		break;
	case 3: /* CROP */
		CropMap(gMap, mBounds[0], mBounds[1], mBounds[2], mBounds[3], false, RF_ProgressFunc);
		DebugAssert(gMap.is_valid());
		DebugAssert(ValidateMapDominance(gMap));
		RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
		break;
	case 4: /* DELETE */
		CropMap(gMap, mBounds[0], mBounds[1], mBounds[2], mBounds[3], true, RF_ProgressFunc);
		DebugAssert(gMap.is_valid());
		DebugAssert(ValidateMapDominance(gMap));
		RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
		break;
	case 5: /* REMOVE OCEANS */
		RemoveUnboundedWater(gMap);
		DebugAssert(gMap.is_valid());
		DebugAssert(ValidateMapDominance(gMap));
		RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
		break;
	}
}

char *	RF_CropTool::GetStatusText(void)
{
	return NULL;
}

#pragma mark -

double		RF_CropTool::UIToLogX(double v) const
{
	return GetZoomer()->XPixelToLon(v);
}

double		RF_CropTool::UIToLogY(double v) const
{
	return GetZoomer()->YPixelToLat(v);
}

double		RF_CropTool::LogToUIX(double v) const
{
	return GetZoomer()->LonToXPixel(v);
}

double		RF_CropTool::LogToUIY(double v) const
{
	return GetZoomer()->LatToYPixel(v);
}

double		RF_CropTool::GetHandleX(int inHandle) const
{
	int	x_ind = kHandleToBoundsX[inHandle];
	if (x_ind == 4) return 0.5 * (mBounds[0] + mBounds[2]);
	return mBounds[x_ind];
}

double		RF_CropTool::GetHandleY(int inHandle) const
{
	int	y_ind = kHandleToBoundsY[inHandle];
	if (y_ind == 5) return 0.5 * (mBounds[1] + mBounds[3]);
	return mBounds[y_ind];
}

void		RF_CropTool::MoveHandleX(int handle, double deltaX)
{
	int	x_ind = kHandleToBoundsX[handle];
	if (x_ind < 4)
		mBounds[x_ind] += deltaX;
}

void		RF_CropTool::MoveHandleY(int handle, double deltaY)
{
	int	y_ind = kHandleToBoundsY[handle];
	if (y_ind < 4)
		mBounds[y_ind] += deltaY;
}
