#ifndef WED_HANDLETOOLBASE_H
#define WED_HANDLETOOLBASE_H

#include "WED_MapToolNew.h"
#include "CompGeomDefs2.h"
#include "GUI_Commander.h"

class	GUI_Pane;
class	WED_MapZoomerNew;
class	IControlHandles;
class	ISelectable;
class	IResolver;
class	IGISEntity;



class	WED_HandleToolBase : public WED_MapToolNew, GUI_Commander_Notifiable {
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
	virtual	void		KillOperation(bool mouse_is_down);

	// GUI_Commander_Notifiable
	virtual	void		PreCommandNotification(GUI_Commander * focus_target, int command);

	// WED_Layer
	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g);
	virtual	void		GetCaps(int& draw_ent_v, int& draw_ent_s, int& cares_about_sel);

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
	
	virtual	EntityHandling_t	TraverseEntity(IGISEntity * ent, int pt_sel)  { return ent_Skip; }
			void				SetCanSelect(int can_select);
			void				SetControlProvider(IControlHandles * provider);

private:

			int					ProcessSelectionRecursive(
									IGISEntity *		entity,
									const Bbox2&		bounds,
									set<IGISEntity *>&	result);

	enum	DragType_t {
		drag_None,			// We are not dragging anything
		drag_Handles,		// Control handles: We are dragging a single control handle
		drag_Links,			// Control handles: We are dragging a line/link
		drag_Ent,			// Control handles: We are dragging an entire entity.
		drag_Sel,			// We are selecting things
		drag_Move,			// we are moving the selection
		drag_Create
	};

		IControlHandles *		mHandles;
		int						mCanSelect;
		
		vector<ISelectable *>	mSelSave;		
		int						mSelToggle;
		
		// Variables for drag tracking
		DragType_t				mDragType;
		int						mDragX;
		int						mDragY;
		int						mSelX;
		int						mSelY;

		int						mHandleEntity;		// Which entity do we drag
		int						mHandleIndex;
		Point2					mTrackPoint;
		
		vector<IGISEntity *>	mSelManip;
		
};

#endif /* WED_HANDLETOOLBASE_H */
