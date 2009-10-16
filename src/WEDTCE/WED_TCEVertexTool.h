/*
 * Copyright (c) 2009, Laminar Research.  All rights reserved.
 *
 */

#ifndef WED_TCEVertexTool_H
#define WED_TCEVertexTool_H

class	ISelection;

#include "WED_HandleToolBase.h"
#include "IControlHandles.h"

class WED_TCEVertexTool : public WED_HandleToolBase, public IControlHandles {
public:

	WED_TCEVertexTool(
							const char *			tool_name,
							GUI_Pane *				host,
							WED_MapZoomerNew *		zoomer,
							IResolver *				resolver);
	virtual				~WED_TCEVertexTool();

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

	virtual	const char *		GetStatusText(void) { return NULL; }

private:

		WED_PropIntEnum		mGrid;

				void	HandleSnap(Point2& io_pt, const Vector2& delta);
				void	SyncRecurse(IGISEntity * g, ISelection * sel) const;
				void	SyncCache(void) const;

	mutable	vector<IGISEntity *>		mCache;

};

#endif /* WED_TCEVertexTool_H */
