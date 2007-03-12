#ifndef ICONTROLHANDLES_H
#define ICONTROLHANDLES_H

#include "CompGeomDefs2.h"
#include "IUnknown.h"

class	IControlHandles : public virtual IUnknown {
public:

	// Entities - many entities perhaps?
	virtual		int		CountEntities(void) const=0;
	virtual		int		GetNthEntityID(int n) const=0;

	// Control handles - by number - they can be moved around.
	virtual		int		CountControlHandles(int id						  ) const=0;
	virtual		void	GetNthControlHandle(int id, int n,		 Point2& p) const=0;
	virtual		void	SetNthControlHandle(int id, int n, const Point2& p)		 =0;

	// Links are structural lines between the control handles.  We have 0 or more links
	// and each one has a start and end control handle index number.  This is not editable - 
	// it can only be queried.	
	virtual		int		GetLinks		    (int id) const=0;
	virtual		int		GetNthLinkSource   (int id, int n) const=0;
	virtual		int		GetNthLinkSourceCtl(int id, int n) const=0;	// -1 if no bezier ctl point!
	virtual		int		GetNthLinkTarget   (int id, int n) const=0;
	virtual		int		GetNthLinkTargetCtl(int id, int n) const=0;
	
	// Generic query as to whether a point is on the structure.  Some control-handle-eable entities
	// may have "fill" area other than the structural linkeage.
	// NOTE: control handle interface providers are NOT required to return point-on-structure for linkeage...
	// only for ADDITIONAL areas.
	virtual		bool	PointOnStructure(int id, const Point2& p) const=0;
	
	// Move ALL control handles (that is, the whole entity) by a delta.
	virtual		void	ControlsHandlesBy(int id, int c, const Vector2& delta)=0;			
	virtual		void	ControlsLinksBy	 (int id, int c, const Vector2& delta)=0;			
	virtual		void	ControlsMoveBy	 (int id,        const Vector2& delta)=0;			

};

#endif /* ICONTROLHANDLES_H */
