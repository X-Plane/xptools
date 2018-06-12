/* 
 * Copyright (c) 2009, Laminar Research.
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

#ifndef WED_TCEMarqueeTool_H
#define WED_TCEMarqueeTool_H

#include "WED_HandleToolBase.h"
#include "IControlHandles.h"

enum tce_marquee_mode_t {
	tmm_None=0,			// No edit going on
	tmm_Drag,			// Drag any corner
	tmm_Rotate,			// Rotate around center
	tmm_Center,			// Drag, keep center in center
	tmm_Prop,			// Drag, maintain aspect ratio
	tmm_Prop_Center		// DRag, maintain aspect ratio AND keep center in center
};

class	WED_TCEMarqueeTool : public WED_HandleToolBase, public virtual IControlHandles {
public:

						 WED_TCEMarqueeTool(
										const char *			tool_name,
										GUI_Pane *				host,
										WED_MapZoomerNew *		zoomer,
										IResolver *				resolver);
	virtual				~WED_TCEMarqueeTool();


	// CONTROL HANDLE INTERFACE:
	virtual		void	BeginEdit(void);
	virtual		void	EndEdit(void);

	virtual		int				CountEntities(void) const;
	virtual		intptr_t		GetNthEntityID(int n) const;

	virtual		int				CountControlHandles(intptr_t id						  ) const;
	virtual		void			GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const;

	virtual		int				GetLinks		    (intptr_t id) const;
	virtual		void			GetNthLinkInfo		(intptr_t id, int n, bool * active, LinkType_t * ltype) const;
	virtual		int				GetNthLinkSource   (intptr_t id, int n) const;
	virtual		int				GetNthLinkSourceCtl(intptr_t id, int n) const;	// -1 if no bezier ctl point!
	virtual		int				GetNthLinkTarget   (intptr_t id, int n) const;
	virtual		int				GetNthLinkTargetCtl(intptr_t id, int n) const;

	virtual		bool	PointOnStructure(intptr_t id, const Point2& p) const;
	virtual bool 				WantSticky() {	return false; }

	virtual		void	ControlsMoveBy(intptr_t id, const Vector2& delta, Point2& io_pt);
	virtual		void	ControlsHandlesBy(intptr_t id, int c, const Vector2& delta, Point2& io_pt);
	virtual		void	ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_pt);

	virtual	const char *		GetStatusText(void) { return NULL; }

private:

	virtual	EntityHandling_t	TraverseEntity(IGISEntity * ent, int pt_sel) { return ent_AtomicOrContainer; }

				bool	GetTotalBounds(void) const;
				void	ApplyRescale(const Bbox2& old_bounds, const Bbox2& new_bounds);
				void	ApplyRotate(const Point2& ctr, double angle);

	tce_marquee_mode_t	mEditMode;
	Point2				mRotateCtr;
	Point2				mRotatePt;
	WED_PropBoolText	mSnap;

	mutable	Bbox2		mCacheBounds;
	mutable long long	mCacheKeyArchive;
	mutable bool		mCacheIconic;

};


#endif /* WED_TCEMarqueeTool_H */
