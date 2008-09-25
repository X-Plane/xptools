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
#ifndef UIUTILS_H
#define UIUTILS_H

/************************************************************************
 * DRAG HANDLES
 ************************************************************************/

class	DragHandleManager {
public:

	virtual	double		UIToLogX(double) const=0;
	virtual	double		UIToLogY(double) const=0;
	virtual	double		LogToUIX(double) const=0;
	virtual	double		LogToUIY(double) const=0;

	virtual	double		GetHandleX(int inHandle) const=0;
	virtual	double		GetHandleY(int inHandle) const=0;

	virtual	void		MoveHandleX(int, double)=0;
	virtual	void		MoveHandleY(int, double)=0;

};

struct	DragHandleInfo_t {
	int		x_draw_pos;
	int		y_draw_pos;
	int		drag_x;
	int		drag_y;
};

class	DragHandleSet {
public:

						DragHandleSet(
							int 					inNumHandles,
							const DragHandleInfo_t *inInfos,
							int						inHandleRadius,
							DragHandleManager * 	inMgr);
						~DragHandleSet();

			bool		StartDrag(
							double		inX,
							double		inY);
			void		ContinueDrag(
							double		inX,
							double		inY);
			void		EndDrag(
							double		inX,
							double		inY);

			int			GetCurrentDragHandle(void);

			void		DrawHandle(
							int			inHandle);
			void		ConnectHandle(
							int 		inHandle1,
							int 		inHandle2);

private:

			void		GetHandleRect(
							int			inNum,
							double&		outLeft,
							double&		outBottom,
							double&		outRight,
							double&		outTop);

	vector<DragHandleInfo_t>	mInfo;

	int							mCurHandle;
	double						mLastX;			// Mouse coords
	double						mLastY;
	int							mHandleRadius;
	DragHandleManager *			mMgr;

};

#endif