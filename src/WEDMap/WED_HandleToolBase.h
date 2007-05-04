#ifndef WED_HANDLETOOLBASE_H
#define WED_HANDLETOOLBASE_H

#include "WED_MapToolNew.h"
#include "CompGeomDefs2.h"

class	GUI_Pane;
class	WED_MapZoomerNew;
class	IControlHandles;
class	IUnknown;
class	IResolver;
class	IGISEntity;



class	WED_HandleToolBase : public WED_MapToolNew {
public:

						 WED_HandleToolBase(
										const char *			tool_name,
										GUI_Pane *				host,
										WED_MapZoomerNew *		zoomer,
										IResolver *				resolver,
										const char *			root_path,
										const char *			selection_path);
										
	virtual				~WED_HandleToolBase();
	
	// Strategies
			void		SetControlProvider(IControlHandles * provider);
			
	// Mouse handling - from tool
	virtual	int			HandleClickDown			(int inX, int inY, int inButton);
	virtual	void		HandleClickDrag			(int inX, int inY, int inButton);
	virtual	void		HandleClickUp			(int inX, int inY, int inButton);

	// Drawing - from layer
	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g);

protected:

	// When iterating composite GIS structure, how do we handle an entity?
	enum EntityHandling_t { 
		ent_Skip,					// Skip it, don't look at kids.
		ent_Atomic,					// Take or leave it as a whole.
		ent_Container,				// Iterate over kids.
		ent_AtomicOrContainer		// Try whole obj.  If it fails, try kids.
	};

	// Template methods
	
	virtual	EntityHandling_t	TraverseEntity(IGISEntity * ent)=0;
			void				SetCanSelect(int can_select) { mCanSelect = can_select; }

private:

			void		ProcessSelectionRecursive(
									IGISEntity *		entity,
									const Bbox2&		bounds,
									set<IUnknown *>&	result);

	enum	DragType_t {
		drag_None,			// We are not dragging anything
		drag_Handles,		// We are dragging a single control handle
		drag_Links,			// We are dragging a line/link
		drag_Sel,			// We are selecting things
		drag_Move			// We are dragging an entire entity.
	};

		IControlHandles *		mHandles;
		string					mRoot;
		int						mCanSelect;
		
		// Variables for drag tracking
		DragType_t				mDragType;
		int						mDragX;
		int						mDragY;
		int						mSelX;
		int						mSelY;

		int						mHandleEntity;		// Which entity do we drag
		int						mHandleIndex;
		
};

#endif /* WED_HANDLETOOLBASE_H */
