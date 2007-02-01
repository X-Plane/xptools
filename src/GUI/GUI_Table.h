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

*/

#include "GUI_Broadcaster.h"
#include "GUI_Listener.h"
#include "GUI_Pane.h"
#include "GUI_ScrollerPane.h"

class	GUI_GraphState;

/***********************************************************************************************
 * ABSTRACT TABLE BEHAVIORS
 ***********************************************************************************************/


class	GUI_TableGeometry : public GUI_Broadcaster {
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
	virtual	void		SetCellWidth (int n, int w)=0;
	virtual	void		SetCellHeight(int n, int h)=0;
};

class	GUI_TableContent : public GUI_Broadcaster {
public:

	virtual	void		CellDraw	 (int cell_bounds[4], int cell_x, int cell_y, GUI_GraphState * inState			  )=0;
	virtual	int			CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)=0;
	virtual	void		CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)=0;
	virtual	void		CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)=0;
	virtual	void		Deactivate(void)=0;

};

class	GUI_TableHeader : public GUI_Broadcaster {
public:

	virtual	void		HeadDraw	 (int cell_bounds[4], int cell_x, GUI_GraphState * inState			  )=0;
	virtual	int			HeadMouseDown(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)=0;
	virtual	void		HeadMouseDrag(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)=0;
	virtual	void		HeadMouseUp  (int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)=0;

};

class	GUI_TableSide : public GUI_Broadcaster {
public:

	virtual	void		SideDraw	 (int cell_bounds[4], int cell_y, GUI_GraphState * inState			  )=0;
	virtual	int			SideMouseDown(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button)=0;
	virtual	void		SideMouseDrag(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button)=0;
	virtual	void		SideMouseUp  (int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button)=0;

};

/***********************************************************************************************
 * TABLE PANES
 ***********************************************************************************************/


class	GUI_Table : public GUI_Pane, public GUI_Listener, public GUI_ScrollerPaneContent {
public:

						 GUI_Table();
	virtual				~GUI_Table();

			void		SetGeometry(GUI_TableGeometry * inContent);
			void		SetContent(GUI_TableContent * inContent);
			int			GetScrollH(void) { return mScrollH; }
			int			GetScrollV(void) { return mScrollV; }

	virtual	void		Draw(GUI_GraphState * state);	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

	virtual	void	GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4]);	
	virtual	void	ScrollH(float xOffset);
	virtual	void	ScrollV(float yOffset);

private:
			int		MouseToCellX(int x);
			int		MouseToCellY(int y);
			int		CalcVisibleCells(int bounds[4]);
			int		CalcCellBounds(int x, int y, int bounds[4]);

			GUI_TableGeometry *		mGeometry;
			GUI_TableContent *		mContent;
			int						mScrollH;
			int						mScrollV;
			int						mClickCellX;
			int						mClickCellY;

};

class	GUI_Header : public GUI_Pane, public GUI_Listener {
public:

						 GUI_Header();
	virtual				~GUI_Header();

			void		SetGeometry(GUI_TableGeometry * inContent);
			void		SetHeader(GUI_TableHeader * inHeader);
			void		SetTable(GUI_Table * inTable);

	virtual	void		Draw(GUI_GraphState * state);	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);

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

};


#endif /* GUI_TABLE_H */
