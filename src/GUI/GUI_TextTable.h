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

#ifndef GUI_TEXTTABLE_H
#define GUI_TEXTTABLE_H

#include "GUI_Commander.h"

class	GUI_Pane;
class	GUI_TextField;

/*

	GUI_TextTable - THEORY OF OPERATION

	The text table classes provide a basic set of behavior for tables and their headers for dealing
	with text data.  They have a further set of plugin behaviors that provide the text content,
	and a series of structs that are used to transfer that content.

*/

#include "GUI_Listener.h"
#include "GUI_Broadcaster.h"

#include "GUI_Table.h"

// Cell content as known by a text table - we have a few different kinds of cell
// displays...see comments for which fields they use.

enum GUI_CellContentType {
								// GET					SET
	gui_Cell_None,
	gui_Cell_Disclose,			// n/a - this is used as an internal symbol for disclosure tris
	gui_Cell_EditText,			// string&string		string
	gui_Cell_FileText,			// string&string		string
	gui_Cell_CheckBox,			// int val				int val
	gui_Cell_Integer,			// string&int val		int val
	gui_Cell_Double,			// string&double val	double val
	gui_Cell_Enum,				// string&int			string&int
	gui_Cell_EnumSet			// string val&int set	int set, int
};

enum GUI_BoolIcon {
	gui_Bool_Check,
	gui_Bool_Lock,
	gui_Bool_Visible
};

struct	GUI_CellContent {
	GUI_CellContentType		content_type;
	int						can_edit;
	int						can_disclose;
	int						can_select;
	int						can_drag;

	int						is_disclosed;
	int						is_selected;
	int						indent_level;

	string					text_val;		// Only one of these is used - which one depends on the cell content type!
	int						int_val;
	double					double_val;
	set<int>				int_set_val;
	GUI_BoolIcon			bool_val;		// for get only - to pick check type!
	int						bool_partial;	// for checks - if we are on but our parent is off...
	int						string_is_resource;

	//Prints a cell's information to the console window, by default it prints only important stuff
	void printCellInfo(
		//Boolean flags to turn on/off drawing
		bool pcontType =true,
		bool pcan_edit =true,
		bool pcan_disclose =true,
		bool pcan_select =true,
		bool pcan_drag =false,

		bool pis_disclosed =true,
		bool pis_selected =true,
		bool pindent_level =true,

		bool ptext_val =true,
		bool pint_val =false,
		bool pdouble_val =false,
		bool pbool_val =false,
		bool pbool_partial =false,
		bool pstring_is_resource = true
		)
	{
	//if(flag is on) printf(stuff new line)
	printf("\n ------------------------------- \n");
	if(pcontType)
	{
		switch(content_type)
		{
			case gui_Cell_None: printf("*content_type: gui_Cell_None \n"); break;
			case gui_Cell_Disclose: printf("*content_type: gui_Cell_Disclose \n"); break;
			case gui_Cell_EditText: printf("*content_type: gui_Cell_EditText \n"); break;
			case gui_Cell_FileText: printf("*content_type: gui_Cell_FileText \n"); break;
			case gui_Cell_CheckBox: printf("*content_type: gui_Cell_CheckBox \n"); break;
			case gui_Cell_Integer: printf("*content_type: gui_Cell_Integer \n"); break;
			case gui_Cell_Double: printf("*content_type: gui_Cell_Double \n"); break;
			case gui_Cell_Enum: printf("*content_type: gui_Cell_Enum \n"); break;
			case gui_Cell_EnumSet: printf("*content_type: gui_Cell_EnumSet \n"); break;
			//for(set<int>::iterator iter=int_set_val.begin();iter != int_set_val.end(); ++iter)
			//{
				
			//}
			default: printf("*content_type: %d \n", content_type); break;
		}
	}	
	if(pcan_edit) printf("*can_edit: %d \n", can_edit);
	if(pcan_disclose) printf("*can_disclose: %d \n", can_disclose);
	if(pcan_select) printf("*can_select: %d \n", can_select);
	if(pcan_drag) printf("can_drag: %d \n", can_drag);

	if(pis_disclosed) printf("*is_disclosed: %d \n", is_disclosed);
	if(pis_selected) printf("*is_selected: %d \n", is_selected);
	if(pindent_level) printf("*indent_level: %d \n", indent_level);

	if(ptext_val) printf("*text_val: %s \n", text_val.c_str());
	if(pint_val) printf("int_val: %d \n", int_val);
	if(pdouble_val) printf("double_val: %lf \n", double_val);
	if(pbool_val) printf("bool_val: %d \n", bool_val);
	if(pbool_partial) printf("bool_partial: %d \n", bool_partial);
	if(pstring_is_resource) printf("string_is_resource: %d \n", string_is_resource);
	}

};

struct GUI_HeaderContent {
	string					title;
	int						is_selected;

	int						can_resize;
	int						can_select;
};

typedef	map<int, pair<string, bool> >	GUI_EnumDictionary;	// For each enum: what it is, can we pick it?

class	GUI_TextTableProvider {
public:

	virtual void	GetCellContent(
						int							cell_x,
						int							cell_y,
						GUI_CellContent&			the_content)=0;
	virtual	void	GetEnumDictionary(
						int							cell_x,
						int							cell_y,
						GUI_EnumDictionary&			out_dictionary)=0;
	virtual	void	AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content,
						int							apply_all)=0;
	virtual	void	ToggleDisclose(
						int							cell_x,
						int							cell_y)=0;
	virtual	void	DoDrag(
						GUI_Pane *					drag_emitter,
						int							mouse_x,
						int							mouse_y,
						int							button,
						int							bounds[4])=0;

	virtual void	SelectionStart(
						int							clear)=0;
	virtual	int		SelectGetExtent(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y)=0;
	virtual	int		SelectGetLimits(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y)=0;
	virtual	void	SelectRange(
						int							start_x,
						int							start_y,
						int							end_x,
						int							end_y,
						int							is_toggle)=0;
	virtual	void	SelectionEnd(void)=0;
	virtual	int		SelectDisclose(
						int							open_it,
						int							all)=0;		// return true if you support this op.

	virtual	int		TabAdvance(
						int&						io_x,
						int&						io_y,
						int							dir,
						GUI_CellContent&			the_content)=0;
	virtual	int		DoubleClickCell(
						int							cell_x,
						int							cell_y)=0;

	virtual	void					GetLegalDropOperations(
											int&						allow_between_col,
											int&						allow_between_row,
											int&						allow_into_cell)=0;
	virtual	GUI_DragOperation		CanDropIntoCell(
											int							cell_x,
											int							cell_y,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended,
											int&						whole_col,
											int&						whole_row)=0;
	virtual	GUI_DragOperation		CanDropBetweenColumns(
											int							cell_x,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended)=0;
	virtual	GUI_DragOperation		CanDropBetweenRows(
											int							cell_y,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended)=0;


	virtual	GUI_DragOperation		DoDropIntoCell(
											int							cell_x,
											int							cell_y,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended)=0;
	virtual	GUI_DragOperation		DoDropBetweenColumns(
											int							cell_x,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended)=0;
	virtual	GUI_DragOperation		DoDropBetweenRows(
											int							cell_y,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended)=0;


};

class	GUI_TextTableHeaderProvider {
public:

	virtual	void	GetHeaderContent(
						int							cell_x,
						GUI_HeaderContent&			the_content)=0;
						
	virtual	void	SelectHeaderCell(
						int							cell_x)=0;

};

// A text table is table content - that is, the drawing brains of a table.
// It in turn queries the "provider" for the actual content.  It allows you to specify a table as strings (easy)
// instead of drawing calls (PITA)

class	GUI_TextTable : public GUI_TableContent, public GUI_Broadcaster, public GUI_Commander, public GUI_Listener {
public:

						 GUI_TextTable(GUI_Commander * parent, int indent, int live_edit);
	virtual				~GUI_TextTable();

			void		SetProvider(GUI_TextTableProvider * content);
			void		SetGeometry(GUI_TableGeometry * geometry);
			void		SetParentTable(GUI_Table * parent);

			void		SetImage(const char * image, int alternations);
			void		SetFont(int font);
			void		SetColors(
							float		grid_lines[4],
							float		select[4],
							float		text[4],
							float		text_select[4],
							float		insert_between[4],
							float		insert_into[4]);
			void		SetTextFieldColors(
								float text_color[4],
								float hilite_color[4],
								float bkgnd_color[4],
								float box_color[4]);

	//Cell Drawing Method, takes the bounds of the cell, the x and y positions of the cell and the graph state
	virtual	void		CellDraw	 (int cell_bounds[4], int cell_x, int cell_y, GUI_GraphState * inState);
	virtual	int			CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock);
	virtual	void		CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button									  );
	virtual	void		CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button									  );
	virtual	int			CellGetCursor(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y												  );
	virtual	int			CellGetHelpTip(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, string& tip								  );
	virtual	GUI_DragOperation	CellDragEnter	(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);
	virtual	GUI_DragOperation	CellDragWithin	(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);
	virtual	void				CellDragLeave	(int cell_bounds[4], int cell_x, int cell_y);
	virtual	GUI_DragOperation	CellDrop		(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended);
	virtual	void		KillEditing(bool save_it);

	virtual	int			HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);
	virtual	int			AcceptTakeFocus(void) 	{ return 1; }

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam);

private:

	enum GUI_DragPart {			// DRAG PARTS - divide the cell into 4 zones, for the uppe rand lower half, and closer to the center or edges.
		drag_LowerOrInto,		// If the cell doesn't support reordering, we use "whole cell".
		drag_IntoOrLower,
		drag_IntoOrHigher,
		drag_HigherOrInto,
		drag_WholeCell
	};

	enum GUI_DragTableDest {	// Where we expect the row to go:
		gui_Table_None,					// No drag
		gui_Table_Row,					// Entire row at once - INTO cell
		gui_Table_Column,				// Entire col at once - INTO cell
		gui_Table_Cell,					// Just the cell! - INTO cell
		gui_Insert_Left,				// INsertions - BETWEEN cells, based on this position relative to the insert cell.
		gui_Insert_Right,
		gui_Insert_Bottom,
		gui_Insert_Top
	};

			void			CreateEdit(int cell_bounds[4]);
			int				TerminateEdit(bool inSave, bool inAll, bool inDone);
			GUI_DragPart	GetCellDragPart(int cell_bounds[4], int x, int y, int vertical);

	GUI_TextTableProvider * mContent;
	int						mClickCellX;
	int						mClickCellY;
	GUI_CellContent			mEditInfo;
	int						mInBounds;
	int						mTrackLeft;
	int						mTrackRight;
	GUI_Table *				mParent;
	GUI_TextField *			mTextField;
	GUI_TableGeometry *		mGeometry;

	int						mCellResize;
	int						mLastX;

	GUI_KeyFlags			mModifiers;
	int						mSelStartX;
	int						mSelStartY;


	GUI_DragTableDest		mDragDest;
	int						mDragX;
	int						mDragY;
	GUI_DragPart			mDragPart;
	GUI_DragOperation		mLastOp;

	int						mCellIndent;
	int						mLiveEdit;
	int						mDiscloseIndent;

	float					mColorGridlines[4];
	float					mColorSelect[4];
	float					mColorText[4];
	float					mColorTextSelect[4];
	float					mColorInsertInto[4];
	float					mColorInsertBetween[4];

	float					mTFColorText[4];
	float					mTFColorHilite[4];
	float					mTFColorBkgnd[4];
	float					mTFColorBox[4];

	int						mFont;

	string					mImage;
	int						mAlternate;

};



class	GUI_TextTableHeader : public GUI_TableHeader, public GUI_Broadcaster {
public:
						 GUI_TextTableHeader();
	virtual				~GUI_TextTableHeader();

			void		SetProvider(GUI_TextTableHeaderProvider * content);
			void		SetGeometry(GUI_TableGeometry * geometry);

			void		SetImage(const char * image);
			void		SetColors(
							float		grid_lines[4],
							float		text[4]);

	virtual	void		HeadDraw	 (int cell_bounds[4], int cell_x, GUI_GraphState * inState);
	virtual	int			HeadMouseDown(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock);
	virtual	void		HeadMouseDrag(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button									  );
	virtual	void		HeadMouseUp  (int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button									  );
	virtual	int			HeadGetCursor(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y												  );
	virtual	int			HeadGetHelpTip(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, string& tip								  );

private:
	GUI_TextTableHeaderProvider *	mContent;
	GUI_TableGeometry *				mGeometry;
	int								mCellResize;
	int								mLastX;

	string							mImage;

	float					mColorGridlines[4];
	float					mColorText[4];

};

class	GUI_TextTableSide : public GUI_TableSide, public GUI_Broadcaster {
public:
						 GUI_TextTableSide();
	virtual				~GUI_TextTableSide();

			void		SetProvider(GUI_TextTableHeaderProvider * content);
			void		SetGeometry(GUI_TableGeometry * geometry);

			void		SetImage(const char * image);
			void		SetColors(
							float		grid_lines[4],
							float		text[4]);

	virtual	void		SideDraw	 (int cell_bounds[4], int cell_y, GUI_GraphState * inState);
	virtual	int			SideMouseDown(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock);
	virtual	void		SideMouseDrag(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button									   );
	virtual	void		SideMouseUp  (int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button									   );
	virtual	int			SideGetCursor(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y												   );
	virtual	int			SideGetHelpTip(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, string& tip								   );

private:
	GUI_TextTableHeaderProvider *	mContent;
	GUI_TableGeometry *				mGeometry;
	int								mCellResize;
	int								mLastY;

	string							mImage;

	float					mColorGridlines[4];
	float					mColorText[4];

};



#endif /* GUI_TEXTTABLE_H */