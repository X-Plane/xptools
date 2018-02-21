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

#ifndef WED_VERTEXTOOL_H
#define WED_VERTEXTOOL_H

#include "WED_HandleToolBase.h"
#include "IControlHandles.h"
#include "IOperation.h"

class	IGISEntity;
class	IGISPoint;
class	ISelection;

class	WED_VertexTool : public WED_HandleToolBase, public virtual IControlHandles {
public:

						 WED_VertexTool(
										const char *			tool_name,
										GUI_Pane *				host,
										WED_MapZoomerNew *		zoomer,
										IResolver *				resolver,
										int						sel_verts);
	virtual				~WED_VertexTool();

	// CONTROL HANDLE INTERFACE:
	virtual		void	BeginEdit(void);
	virtual		void	EndEdit(void);

	virtual		int		CountEntities(void) const;
	virtual		intptr_t	GetNthEntityID(int n) const;

	virtual		int		CountControlHandles(intptr_t id						  ) const;
	virtual		void	GetNthControlHandle(intptr_t id, int n, bool * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const;

	virtual		int		GetLinks		    (intptr_t id) const;
	virtual		void	GetNthLinkInfo		(intptr_t id, int n, bool * active, LinkType_t * ltype) const;
	virtual		int		GetNthLinkSource   (intptr_t id, int n) const;
	virtual		int		GetNthLinkSourceCtl(intptr_t id, int n) const;	// -1 if no bezier ctl point!
	virtual		int		GetNthLinkTarget   (intptr_t id, int n) const;
	virtual		int		GetNthLinkTargetCtl(intptr_t id, int n) const;

	virtual		bool	PointOnStructure(intptr_t id, const Point2& p) const;

	virtual		void	ControlsMoveBy(intptr_t id, const Vector2& delta, Point2& io_handle);
	virtual		void	ControlsHandlesBy(intptr_t id, int c, const Vector2& delta, Point2& io_pt);
	virtual		void	ControlsLinksBy	 (intptr_t id, int c, const Vector2& delta, Point2& io_pt);

//	virtual	int			FindProperty(const char * in_prop) { return -1; }
//	virtual int			CountProperties(void) { return 0; }
//	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info) {}
//	virtual	void		GetNthPropertyDict(int n, PropertyDict_t& dict) { }
//	virtual	void		GetNthPropertyDictItem(int n, int e, string& item) { }

//	virtual void		GetNthProperty(int n, PropertyVal_t& val) { }
//	virtual void		SetNthProperty(int n, const PropertyVal_t& val) { }

	virtual	const char *		GetStatusText(void) { return NULL; }

	virtual	void		DrawSelected			(bool inCurrent, GUI_GraphState * g);


//	virtual void *		QueryInterface(const char * class_id);

private:

	virtual	EntityHandling_t	TraverseEntity(IGISEntity * ent,int pt_sel);

			void		GetEntityInternal(void) const;
			void		AddEntityRecursive(IGISEntity * e, const Bbox2& bounds) const;
			void		AddSnapPointRecursive(IGISEntity * e, const Bbox2& bounds, ISelection * sel) const;
			bool		SnapMovePoint(const Point2& ideal_track_pt, Point2& io_thing_pt, IGISEntity * who);

		int						mInEdit;
		int						mIsRotate;
		int						mIsScale;
		int						mIsSymetric;
		int						mIsTaxiSpin;
		mutable Point2			mRotateCtr;
		mutable Point2			mTaxiDest;
		mutable double			mRotateOffset;
		mutable	int				mRotateIndex;
		mutable	Vector2			mPointOffset1;
		mutable	Vector2			mPointOffset2;

		IGISPoint *				mNewSplitPoint;		// When we option-click to get a split point...this is the newly born point.
		
		WED_PropBoolText		mSnapToGrid;

		mutable vector<IGISEntity *>	mEntityCache;
		mutable long long				mEntityCacheKeyArchive;
		mutable long long				mEntityCacheKeyZoomer;

		mutable vector<pair<Point2,IGISEntity *> >		mSnapCache;
		mutable long long								mSnapCacheKeyArchive;
		mutable long long								mSnapCacheKeyZoomer;

};


#endif /* WED_VERTEXTOOL_H */
