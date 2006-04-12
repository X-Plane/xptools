#ifndef GUI_PANE_H
#define GUI_PANE_H

#include "GUI_Defs.h"

class GUI_GraphState;

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

			void		GetBounds(int outBounds[4]);
	virtual void		SetBounds(int x1, int y1, int x2, int y2);
	virtual void		SetBounds(int inBounds[4]);
			void		GetSticky(int outSticky[4]);
			void		SetSticky(int inSticky[4]);
			void		SetSticky(int x1, int y1, int x2, int y2);

			int			GetID(void) const;
			void		SetID(int id);
			GUI_Pane *	FindByID(int id);
			void		GetDescriptor(string& outDesc) const;
	virtual	void		SetDescriptor(const string& inDesc);

			bool		IsVisible(void) const;
	virtual bool		IsVisibleNow(void) const;
	virtual void		Show(void);
	virtual void		Hide(void);
			
	virtual void		Refresh(void);

	/* TEMPLATE METHODS - Override these to customize a pane. */
	virtual	void		Draw(GUI_GraphState * state) { }
	
	virtual	int			MouseDown(int x, int y, int button) { return 0; }
	virtual	void		MouseDrag(int x, int y, int button) { }
	virtual	void		MouseUp(int x, int y, int button) { }
	virtual	int			ScrollWheel(int x, int y, int dist, int axis) { return 0; }

private:

	friend class GUI_Window;

	// Internal dispatch methods

			void		InternalDraw(GUI_GraphState * state);
			GUI_Pane *	InternalMouseDown(int x, int y, int button);
			int			InternalMouseWheel(int x, int y, int dist, int axis);
			void		ParentResized(int inOldBounds[4], int inNewBounds[4]);

		int					mBounds[4];
		int					mSticky[4];
		GUI_Pane *			mParent;
		vector<GUI_Pane *>	mChildren;
		int					mID;
		int					mVisible;
		string				mDesc;

	GUI_Pane(const GUI_Pane&);
	GUI_Pane& operator=(const GUI_Pane&);
		
};


#endif
