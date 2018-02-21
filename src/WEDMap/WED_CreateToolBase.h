/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef WED_CREATETOOLBASE_H
#define WED_CREATETOOLBASE_H

#include "WED_MapToolNew.h"
#include "CompGeomDefs2.h"
#include <vector>
#include "WED_HandleToolBase.h"
#include "IControlHandles.h"

class	WED_Archive;

using std::vector;

class	WED_CreateToolBase : public WED_HandleToolBase, public virtual IControlHandles {
public:

						 WED_CreateToolBase(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer,
									IResolver *			resolver,
									WED_Archive *		archive,
									int					min_num_pts,
									int					max_num_pts,
									int					can_curve,
									int					must_curve,
									int					can_close,
									int					must_close);
	virtual				~WED_CreateToolBase();

//	virtual	int			HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
//	virtual	void		HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
//	virtual	void		HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers);
//	virtual	void		KillOperation(void);

//	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g);

	virtual		void	BeginEdit(void);
	virtual		void	EndEdit(void);
	virtual		int		CountEntities(void) const;
	virtual		intptr_t	GetNthEntityID(int n) const;
	virtual		int		CountControlHandles(intptr_t id						  ) const;
	virtual		void	GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const;
	virtual		int		GetLinks		    (intptr_t id) const;
	virtual		void	GetNthLinkInfo(intptr_t id, int n, bool * active, LinkType_t * ltype) const;
	virtual		int		GetNthLinkSource   (intptr_t id, int n) const;
	virtual		int		GetNthLinkSourceCtl(intptr_t id, int n) const;	// -1 if no bezier ctl point!
	virtual		int		GetNthLinkTarget   (intptr_t id, int n) const;
	virtual		int		GetNthLinkTargetCtl(intptr_t id, int n) const;
	virtual		bool	PointOnStructure(intptr_t id, const Point2& p) const;
	virtual		void	ControlsHandlesBy(intptr_t id, int c, const Vector2& delta, Point2& io_pt);
	virtual		void	ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_pt);
	virtual		void	ControlsMoveBy	 (intptr_t id,        const Vector2& delta, Point2& io_pt);

	virtual	int					CreationDown(const Point2& start_pt);
	virtual	void				CreationDrag(const Point2& start_pt, const Point2& now_pt);
	virtual	void				CreationUp(const Point2& start_pt, const Point2& now_pt);

	virtual void		KillOperation(bool mouse_is_down);
	virtual	int			HandleToolKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags			  );

protected:

	virtual	void		AcceptPath(
							const vector<Point2>&	pts,
							const vector<Point2>&	dirs_lo,
							const vector<Point2>&	dirs_hi,
							const vector<int>		has_dirs,
							const vector<int>		has_split,
							int						closed)=0;
	virtual	bool		CanCreateNow(void)=0;

		inline WED_Archive * GetArchive(void) { return mArchive; }

//			bool		HasDragNow(
//							Point2&	p,
//							Point2& c);
//			bool		HasPrevNow(
//							Point2& p,
//							Point2& c);

private:

			void		DoEmit(int close_it);

			void		RecalcHeadings(void);

	int					mEditStarted;

	WED_Archive *		mArchive;

	vector<Point2>		mPts;
	vector<Point2>		mControlLo;
	vector<Point2>		mControlHi;
	vector<int>			mHasDirs;
	vector<int>			mIsSplit;

	float	mLastTime;
	Point2	mLastPt;

	int		mCreating;
protected:
	int		mMinPts;
private:
	int		mMaxPts;
	int		mCanClose;
	int		mMustClose;
	int		mCanCurve;
	int		mMustCurve;

};


#endif /* WED_CREATETOOLBASE_H */
