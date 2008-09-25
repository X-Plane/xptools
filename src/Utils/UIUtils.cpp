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
#include "UIUtils.h"
#if APL
#include <OpenGL/gl.h>
#else
#include <gl/gl.h>
#endif

DragHandleSet::DragHandleSet(
							int 					inNumHandles,
							const DragHandleInfo_t *inInfos,
							int						inHandleRadius,
							DragHandleManager * 	inMgr) :
	mCurHandle(-1),
	mLastX(0),
	mLastY(0),
	mHandleRadius(inHandleRadius),
	mMgr(inMgr),
	mInfo(inInfos, inInfos + inNumHandles)
{
}

DragHandleSet::~DragHandleSet()
{
}

bool		DragHandleSet::StartDrag(
				double		inX,
				double		inY)
{
	double	l,r,t,b;
	mCurHandle = -1;
	for (int n = 0; n < mInfo.size(); ++n)
	{
		GetHandleRect(n, l, b, r, t);
		if (inX >= l && inX <= r && inY >= b && inY <= t)
		{
			mCurHandle = n;
			mLastX = inX;
			mLastY = inY;
			return true;
		}
	}
	return false;
}

void		DragHandleSet::ContinueDrag(
				double		inX,
				double		inY)
{
	if (mCurHandle < 0) return;
	if (inX == mLastX && inY == mLastY) return;

	double	x_change = mMgr->UIToLogX(inX) - mMgr->UIToLogX(mLastX);
	double	y_change = mMgr->UIToLogY(inY) - mMgr->UIToLogY(mLastY);
	if (mInfo[mCurHandle].drag_x)
		mMgr->MoveHandleX(mCurHandle, x_change);
	if (mInfo[mCurHandle].drag_y)
		mMgr->MoveHandleY(mCurHandle, y_change);
	mLastX = inX;
	mLastY = inY;
}

void		DragHandleSet::EndDrag(
				double		inX,
				double		inY)
{
	if (mCurHandle < 0) return;
	if (inX == mLastX && inY == mLastY)
	{
		mCurHandle = -1;
		return;
	}

	double	x_change = mMgr->UIToLogX(inX) - mMgr->UIToLogX(mLastX);
	double	y_change = mMgr->UIToLogY(inY) - mMgr->UIToLogY(mLastY);
	if (mInfo[mCurHandle].drag_x)
		mMgr->MoveHandleX(mCurHandle, x_change);
	if (mInfo[mCurHandle].drag_y)
		mMgr->MoveHandleY(mCurHandle, y_change);
	mLastX = inX;
	mLastY = inY;
	mCurHandle = -1;
}

int			DragHandleSet::GetCurrentDragHandle(void)
{
	return mCurHandle;
}

void		DragHandleSet::DrawHandle(
				int			inHandle)
{
	double	l,b,r,t;
	GetHandleRect(inHandle, l, b, r, t);
	glVertex2d(l, b);
	glVertex2d(l, t);
	glVertex2d(r, t);
	glVertex2d(r, b);

}

void		DragHandleSet::ConnectHandle(
				int 		inHandle1,
				int 		inHandle2)
{
	glVertex2d(
			mMgr->LogToUIX(mMgr->GetHandleX(inHandle1)),
			mMgr->LogToUIY(mMgr->GetHandleY(inHandle1)));
	glVertex2d(
			mMgr->LogToUIX(mMgr->GetHandleX(inHandle2)),
			mMgr->LogToUIY(mMgr->GetHandleY(inHandle2)));
}

void		DragHandleSet::GetHandleRect(
				int			inNum,
				double&		outLeft,
				double&		outBottom,
				double&		outRight,
				double&		outTop)
{
	outLeft 	= mMgr->LogToUIX(mMgr->GetHandleX(inNum)) - mHandleRadius;
	outRight 	= mMgr->LogToUIX(mMgr->GetHandleX(inNum)) + mHandleRadius;
	outBottom 	= mMgr->LogToUIY(mMgr->GetHandleY(inNum)) - mHandleRadius;
	outTop 		= mMgr->LogToUIY(mMgr->GetHandleY(inNum)) + mHandleRadius;

	outLeft  	+= (mHandleRadius * mInfo[inNum].x_draw_pos);
	outRight 	+= (mHandleRadius * mInfo[inNum].x_draw_pos);
	outBottom 	+= (mHandleRadius * mInfo[inNum].y_draw_pos);
	outTop 		+= (mHandleRadius * mInfo[inNum].y_draw_pos);
}

