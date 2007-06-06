#ifndef GUI_TABLE_H
#define GUI_TABLE_H

/*

	NOTES ON THE COORDINATE SYSTEM:
	
	As a scroller, the pane's total bounds are used as the visible rect and the
	logical rect are found relative to it.  Internally the scroll offset deltas
	are maintained and the width is found from the geometry host.
	
	But the geometry provider gets to provide cell boundaries in a 0,0-origin system.
	This is done for simplicity in implementing the geometry provider; the pane classes
	translate between drawing coordinates and these zero-based cell-geometry coordinates.
	
	Drawing and content classes receive cell bounds in drawing coordinates so that they
	can be passed directly to drawing routines.
	
	PARTS OF THE TABLE SYSTEM:
	
	The GUI_Table, GUI_Header, and GUI_Sider make the 3 panes that make a table.
	Tables are scrollercontents and can be embedded in scroller panes.  Headers and
	siders take a table as a link and will internally scroll to stay in lock-step with
	the main table.
	
	Four interfaces give the tables their "brains".  Content plugins do drawing and 
	mouse interaction for the table and its header/sider.  The geometry interface 
	defines the total number of rows and the variable grid layout.

*/

#include "GUI_Broadcaster.h"
#include "GUI_Listener.h"
#include "GUI_Pane.h"
#include "GUI_ScrollerPane.h"

class	GUI_GraphState;

/***********************************************************************************************
 * ABSTRACT TABLE BEHAVIORS
 ***********************************************************************************************/


class	GUI_TableGeometry {
public:

	// Getting geometry
	virtual	int			GetColCount(void)=0;
	virtual	int			GetRowCount(void)=0;
	
	virtual	int			GetCellLeft (int n)=0;
	virtual	int			GetCellRight(int n)=0;
	virtual	int			GetCellWidth(int n)=0;

	virtual	int			GetCellBottom(int n)=0;
	virtual	int			GetCellTop	 (int n)=0;
	virtual	int			GetCellHeight(int n)=0;
	
	// Index
	virtual	int			ColForX(int n)=0;
	virtual	int			RowForY(int n)=0;
	
	// Setting geometry
	virtual	bool		CanSetCellWidth (void) const=0;
	virtual	bool		CanSetCellHeight(void) const=0;
	virtual	void		SetCellWidth (int n, int w)=0;
	virtual	void		SetCellHeight(int n, int h)=0;
};

class	GUI_TableContent {
public:

	virtual	void		CellDraw	 (int cell_bounds[4], int cell_x, int cell_y, GUI_GraphState * inState)=0;

	virtual	int			CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button, GUI_KeyFlags mods, int& wants_lock)=0;
	virtual	void		CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button									  )=0;
	virtual	void		CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button									  )=0;
	virtual	int			CellGetCursor(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y												  )=0;
	virtual	int			CellGetHelpTip(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, string& tip								  )=0;

	virtual	GUI_DragOperation	CellDragEnter	(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)=0;
	virtual	GUI_DragOperation	CellDragWithin	(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)=0;
	virtual	void				CellDragLeave	(int cell_bounds[4], int cell_x, int cell_y)=0;
	virtual	GUI_DragOperation	CellDrop		(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)=0;

	virtual	void		Deactivate(void)=0;

};

class	GUI_TableHeader {
public:

	virtual	void		HeadDraw	 (int cell_bounds[4], int cell_x, GUI_GraphState * inState)=0;
	virtual	int			HeadMouseDown(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button, GUI_KeyFlags mods, int& wants_lock)=0;
	virtual	void		HeadMouseDrag(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button									  )=0;
	virtual	void		HeadMouseUp  (int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button									  )=0;
	virtual	int			HeadGetCursor(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y												  )=0;
	virtual	int			HeadGetHelpTip(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, string& tip								  )=0;

};

class	GUI_TableSide {
public:

	virtual	void		SideDraw	 (int cell_bounds[4], int cell_y, GUI_GraphState * inState)=0;
	virtual	int			SideMouseDown(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button, GUI_KeyFlags mods, int& wants_lock)=0;
	virtual	void		SideMouseDrag(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button									  )=0;
	virtual	void		SideMouseUp  (int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button									  )=0;
	virtual	int			SideGetCursor(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y												  )=0;
	virtual	int			SideGetHelpTip(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, string& tip								  )=0;

};

/***********************************************************************************************
 * TABLE PANES
 ***********************************************************************************************/


class	GUI_Table : public GUI_Pane, public GUI_Listener, public GUI_ScrollerPaneContent {
public:

						 GUI_Table(int fill_right);
	virtual				~GUI_Table();

			void		SetGeometry(GUI_TableGeometry * inContent);
			void		SetContent(GUI_TableContent * inContent);
			int			GetScrollH(void) { return mScrollH; }
			int			GetScrollV(void) { return mScrollV; }
			void		SizeShowAll(void);

	virtual	void		Draw(GUI_GraphState * state);	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	int			GetCursor(int x, int y);
	virtual	int			GetHelpTip(int x, int y, int tip_bounds[4], string& tip);
	
	virtual	GUI_DragOperation			DragEnter	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);
	virtual	GUI_DragOperation			DragOver	(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);
	virtual	void						DragScroll	(int x, int y);
	virtual	void						DragLeave	(void);
	virtual	GUI_DragOperation			Drop		(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);

	virtual void		SetBounds(int x1, int y1, int x2, int y2);
	virtual void		SetBounds(int inBounds[4]);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

	virtual	void	GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4]);	
	virtual	void	ScrollH(float xOffset);
	virtual	void	ScrollV(float yOffset);

			int		CalcCellBounds(int x, int y, int bounds[4]);
			void	RevealCol(int x);
			void	RevealRow(int y);
			void	RevealCell(int x, int y);


private:
			int		MouseToCellX(int x);
			int		MouseToCellY(int y);
			int		CalcVisibleCells(int bounds[4]);
			
			void	AlignContents(void);

			GUI_TableGeometry *		mGeometry;
			GUI_TableContent *		mContent;
			int						mScrollH;
			int						mScrollV;
			int						mClickCellX;
			int						mClickCellY;
			int						mLocked;
		int					mDragX;
		int					mDragY;
		
		int					mExtendSide;
		
		
};

class	GUI_Header : public GUI_Pane, public GUI_Listener {
public:

						 GUI_Header(int fill_right);
	virtual				~GUI_Header();

			void		SetGeometry(GUI_TableGeometry * inContent);
			void		SetHeader(GUI_TableHeader * inHeader);
			void		SetTable(GUI_Table * inTable);

	virtual	void		Draw(GUI_GraphState * state);	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	int			GetCursor(int x, int y);
	virtual	int			GetHelpTip(int x, int y, int tip_bounds[4], string& tip);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:
			int		MouseToCellX(int x);
			int		CalcVisibleCells(int bounds[2]);
			int		CalcCellBounds(int x, int bounds[4]);
	
			GUI_TableGeometry *		mGeometry;
			GUI_TableHeader *		mHeader;
			GUI_Table *				mTable;
			int						mClickCellX;
			int						mLocked;
			int						mExtendSide;

};

class	GUI_Side : public GUI_Pane, public GUI_Listener {
public:

						 GUI_Side();
	virtual				~GUI_Side();

			void		SetGeometry(GUI_TableGeometry * inContent);
			void		SetSide(GUI_TableSide * inSide);
			void		SetTable(GUI_Table * inTable);

	virtual	void		Draw(GUI_GraphState * state);	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	int			GetCursor(int x, int y);
	virtual	int			GetHelpTip(int x, int y, int tip_bounds[4], string& tip);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:
			int		MouseToCellY(int y);
			int		CalcVisibleCells(int bounds[2]);
			int		CalcCellBounds(int y, int bounds[4]);
			GUI_TableGeometry *		mGeometry;
			GUI_TableSide *			mSide;
			GUI_Table *				mTable;
			int						mClickCellY;
			int						mLocked;

};


#endif /* GUI_TABLE_H */
