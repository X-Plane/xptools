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
										IResolver *				resolver);
										
	virtual				~WED_HandleToolBase();
				
	// WED_MapToolNew
	virtual	int			HandleClickDown			(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	virtual	void		HandleClickDrag			(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	virtual	void		HandleClickUp			(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	virtual	int			HandleKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags);
	virtual	void		KillOperation(void);

	// WED_Layer
	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g);

protected:

	// When iterating composite GIS structure, how do we handle an entity?
	enum EntityHandling_t { 
		ent_Skip,					// Skip it, don't look at kids.
		ent_Atomic,					// Take or leave it as a whole.
		ent_Container,				// Iterate over kids.
		ent_AtomicOrContainer		// Try whole obj.  If it fails, try kids.
	};

	// Template method for subclass	
	virtual	int					CreationDown(const Point2& start_pt) { return 0; }
	virtual	void				CreationDrag(const Point2& start_pt, const Point2& now_pt) { }
	virtual	void				CreationUp(const Point2& start_pt, const Point2& now_pt) { }
	
	virtual	EntityHandling_t	TraverseEntity(IGISEntity * ent)  { return ent_Skip; }
			void				SetCanSelect(int can_select);
			void				SetControlProvider(IControlHandles * provider);

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
		drag_Move,			// We are dragging an entire entity.
		drag_Create
	};

		IControlHandles *		mHandles;
		int						mCanSelect;
		
		vector<IUnknown *>			mSelSave;		
		int							mSelToggle;
		
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
