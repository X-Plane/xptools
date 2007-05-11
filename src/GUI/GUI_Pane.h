#ifndef GUI_PANE_H
#define GUI_PANE_H

/*

	GUI_Pane - THEORY OF OPERATION
	
	MEMORY MANAGEMENT
	
		Each pane must be dynamically allocated - if a pane is deleted, it deletes its children, so
		whole view hierarchies can be deleted at once by deleting the root.
		
		"Behavior" code - that is, derivatives of abstract classes that are "plugged in" to views
		are NOT released.  This allows you to derive from several behaviors and not worry about
		multiple deletes.
	
	LAYOUT MANAGEMENT
	
		Layout management for GUI panes is done in two phase: initial setup and incremental modification.
		
		Initial setup is done manually by client code, whether that means clients calling SetBounds
		explicitly or specific convenience methods, such as the routine that automatically puts a
		scrollable pane into a scroller.
		
		A pane has four 'sticky' flags, indicating that they should move relative to their parents
		corresponding walls.
		
		The ideas is that the initial layout is set once and the stickiness preserves this relationship.

	DRAG & DROP

		allowed - set of all possible operations.
		recommended - best choice based on keys
		tracking result - what will happen
		dropping result (out to the app) - what NEEDS to be done

*/

#if !DEV
doc drag and drop
#endif

#include "GUI_Defs.h"

class	GUI_GraphState;
class	GUI_DragData;

class	GUI_Pane {
public:

						GUI_Pane();
	virtual			   ~GUI_Pane();

	/* GENERAL API - some of these are virtual; don't override.  The use of virtual
	   is meant only to implement the base window class. */

			int			CountChildren(void) const;
			GUI_Pane *	GetNthChild(int n) const;
			GUI_Pane *	GetParent(void) const;
			void		SetParent(GUI_Pane * inParent);

			void		GetBounds(int outBounds[4]);		// Our extend in OGL/win coords
			void		GetVisibleBounds(int outBounds[4]);	// the subset of our extent that is not clipped by parents!
	virtual void		SetBounds(int x1, int y1, int x2, int y2);
	virtual void		SetBounds(int inBounds[4]);
			void		GetSticky(int outSticky[4]);
			void		SetSticky(int inSticky[4]);
			void		SetSticky(int x1, int y1, int x2, int y2);

			int			GetID(void) const;
			void		SetID(int id);
			GUI_Pane *	FindByID(int id);
			GUI_Pane *	FindByPoint(int x, int y);
			void		GetDescriptor(string& outDesc) const;
	virtual	void		SetDescriptor(const string& inDesc);

			bool		IsVisible(void) const;
	virtual bool		IsVisibleNow(void) const;
	virtual void		Show(void);
	virtual void		Hide(void);

			bool		IsEnabled(void) const;
			bool		IsEnabledNow(void) const;
			void		Enable(void);
			void		Disable(void);

	virtual	bool		IsActiveNow(void) const;
			
	virtual void		Refresh(void);		// Schedule an async window redraw.
	
	virtual	void		PopupMenu(GUI_Menu menu, int x, int y);											// Pop up a menu - useful for providing main or fixed menus contextually
	virtual	int			PopupMenuDynamic(const GUI_MenuItem_t items[], int x, int y, int current);		// Pop up dynamic content.  No nesting, built on fly.  For enums, etc.

	bool				IsDragClick(int x, int y);
	GUI_DragOperation	DoDragAndDrop(
							int						x, 
							int						y,
							int						where[4],
							GUI_DragOperation		operations,
							int						type_count, 
							GUI_ClipType			inTypes[], 
							int						sizes[], 
							const void *			ptrs[],
							GUI_GetData_f			get_data_f,
							void *					ref);

			GUI_KeyFlags	GetModifiersNow(void);
	virtual	void			GetMouseLocNow(int * out_x, int * out_y);
			float			GetTimeNow(void);

	/* TEMPLATE METHODS - Override these to customize a pane. */
	virtual	void		Draw(GUI_GraphState * state) { }
	
	virtual	int			MouseDown(int x, int y, int button) { return 0; }
	virtual	void		MouseDrag(int x, int y, int button) { 			}
	virtual	void		MouseUp  (int x, int y, int button) { 			}
	virtual	int			ScrollWheel(int x, int y, int dist, int axis) { return 0; }

	virtual	GUI_DragOperation			DragEnter	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended) { return gui_Drag_None;	}
	virtual	GUI_DragOperation			DragOver	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended) { return gui_Drag_None;	}
	virtual	void						DragLeave	(void) { }
	virtual	GUI_DragOperation			Drop		(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended) { return gui_Drag_None;	}
	

private:

	friend class GUI_Window;
	friend class GUI_Window_DND;		// For Windoze DND
	// Internal dispatch methods

			void		InternalDraw(GUI_GraphState * state);
			GUI_Pane *	InternalMouseDown(int x, int y, int button);
			int			InternalMouseWheel(int x, int y, int dist, int axis);
			void		ParentResized(int inOldBounds[4], int inNewBounds[4]);

			GUI_DragOperation			InternalDragEnter	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);
			GUI_DragOperation			InternalDragOver	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);
			void						InternalDragLeave	(void);
			GUI_DragOperation			InternalDrop		(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);

		int					mBounds[4];
		int					mSticky[4];
		GUI_Pane *			mParent;
		vector<GUI_Pane *>	mChildren;
		int					mID;
		int					mVisible;
		int					mEnabled;
		string				mDesc;

		GUI_Pane *			mDragTarget;

	GUI_Pane(const GUI_Pane&);
	GUI_Pane& operator=(const GUI_Pane&);
		
};


#endif
