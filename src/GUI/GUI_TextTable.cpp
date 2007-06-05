#include "GUI_TextTable.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "GUI_Messages.h"
#include "GUI_DrawUtils.h"
#include <math.h>
#include "GUI_TextField.h"
#include "WED_ToolUtils.h"
#include "GUI_Resources.h"
#include "AssertUtils.h"
#define RESIZE_MARGIN 4

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#if BENTODO
	numeric precision control?
#endif

#if OPTIMIZE
	provider to provide content IN BULK?
#endif

GUI_TextTable::GUI_TextTable(GUI_Commander * parent, int indent) : GUI_Commander(parent),
	mContent(NULL),
	mClickCellX(-1),
	mClickCellY(-1),
	mParent(NULL),
	mTextField(NULL),
	mSelStartX(-1),
	mSelStartY(-1),
	mDragX(-1),
	mDragY(-1),
	mDragDest(gui_Table_None),
	mDragPart(drag_WholeCell),
	mCellIndent(indent)
{
	mEditInfo.content_type = gui_Cell_None;
	mColorGridlines[0] = 0.5f;
	mColorGridlines[1] = 0.5f;
	mColorGridlines[2] = 0.5f;
	mColorGridlines[3] = 1.0f;
	
	mColorSelect[0] = 1.0;
	mColorSelect[1] = 1.0;
	mColorSelect[2] = 0.0;
	mColorSelect[3] = 1.0;

	mColorText[0] = 0.8;
	mColorText[1] = 0.8;
	mColorText[2] = 0.8;
	mColorText[3] = 1.0;

	mColorTextSelect[0] = 1.0;
	mColorTextSelect[1] = 1.0;
	mColorTextSelect[2] = 1.0;
	mColorTextSelect[3] = 1.0;	
	
	mColorInsertInto[0] = 1.0;
	mColorInsertInto[1] = 0.0;
	mColorInsertInto[2] = 0.0;
	mColorInsertInto[3] = 1.0;

	mColorInsertBetween[0] = 0.0;
	mColorInsertBetween[1] = 0.5;
	mColorInsertBetween[2] = 1.0;
	mColorInsertBetween[3] = 1.0;
	
	GUI_TexPosition_t	metrics;
	GUI_GetTextureResource("disclose.png", 0, &metrics);
	mDiscloseIndent = metrics.real_width / 2;
}

GUI_TextTable::~GUI_TextTable()
{
}

void	GUI_TextTable::SetColors(
							float		grid_lines[4],
							float		select[4],
							float		text[4],
							float		text_select[4],
							float		insert_between[4],
							float		insert_into[4])
{
	for (int n = 0; n < 4; ++n)
	{
		mColorGridlines		[n]=grid_lines		[n];
		mColorSelect		[n]=select			[n];
		mColorText			[n]=text			[n];
		mColorTextSelect	[n]=text_select		[n];
		mColorInsertBetween	[n]=insert_between	[n];
		mColorInsertInto	[n]=insert_into		[n];
	}
}

void		GUI_TextTable::SetParentTable(GUI_Table * parent)
{
	mParent = parent;
}
	
void		GUI_TextTable::SetProvider(GUI_TextTableProvider * content)
{
	mContent = content;
}


void		GUI_TextTable::CellDraw	 (int cell_bounds[4], int cell_x, int cell_y, GUI_GraphState * inState			  )
{
	inState->SetState(false, false, false,	false, false, false, false);
	glColor4fv(mColorGridlines);
	glBegin(GL_LINE_STRIP);
	glVertex2i(cell_bounds[0]  ,cell_bounds[1]);
	glVertex2i(cell_bounds[2]-1,cell_bounds[1]);
	glVertex2i(cell_bounds[2]-1,cell_bounds[3]);
	glEnd();	
	
	if (!mContent) return;
	GUI_CellContent	c;
	mContent->GetCellContent(cell_x,cell_y,c);

	if (c.is_selected)
	{
		glColor4fv(mColorSelect);
		glBegin(GL_QUADS);
		glVertex2i(cell_bounds[0]  ,cell_bounds[1]+1);
		glVertex2i(cell_bounds[0]  ,cell_bounds[3]  );
		glVertex2i(cell_bounds[2]-1,cell_bounds[3]  );
		glVertex2i(cell_bounds[2]-1,cell_bounds[1]+1);
		glEnd();			
		glColor4fv(mColorGridlines);
	}
	
	cell_bounds[0] += (c.indent_level * mCellIndent);
	
	char buf[50];
	switch(c.content_type) {
	case gui_Cell_Disclose:
		c.text_val.clear();
		break;
	case gui_Cell_CheckBox:
		c.text_val = "";
		break;		
	case gui_Cell_Integer:
		sprintf(buf,"%d",c.int_val);
		c.text_val = buf;
		break;
	case gui_Cell_Double:
		sprintf(buf,"%lf",c.double_val);
		c.text_val = buf;
		break;
	}
	
	if(c.is_disclosed || c.can_disclose)
	{
		int middle = (cell_bounds[1] + cell_bounds[3]) / 2;
		
		int tile[4] = { c.is_disclosed ? 1 : 0, 
			(cell_x == mClickCellX && cell_y == mClickCellY && mEditInfo.content_type == gui_Cell_Disclose && mInBounds) ? 1 : 0,
			2, 2 };
		
		glColor3f(1,1,1);
		GUI_DrawCentered(inState, "disclose.png", cell_bounds, -1, 0, tile, NULL, NULL);
		
		cell_bounds[0] += mDiscloseIndent;
	}
	
	int line_h = (cell_bounds[1] + cell_bounds[3]) / 2 - GUI_GetLineAscent(font_UI_Basic) / 2;
	GUI_FontDraw(inState, font_UI_Basic, 
		c.is_selected ? mColorTextSelect : mColorText,
		cell_bounds[0]+3, line_h, c.text_val.c_str());	

	if (c.content_type == gui_Cell_CheckBox)
	{
		int selector[4] = { (c.int_val != 0) ? 1 : 0, (cell_x == mClickCellX && cell_y == mClickCellY && mEditInfo.content_type == gui_Cell_CheckBox && mInBounds) || c.bool_partial ? 1 : 0, 2, 2 };
		glColor3f(1,1,1);
		switch(c.bool_val) {
		case gui_Bool_Check:		GUI_DrawCentered(inState, "check.png", cell_bounds, 0, 0, selector, NULL, NULL);	break;
		case gui_Bool_Lock:			GUI_DrawCentered(inState, "lock.png", cell_bounds, 0, 0, selector, NULL, NULL);		break;
		case gui_Bool_Visible:		GUI_DrawCentered(inState, "eye.png", cell_bounds, 0, 0, selector, NULL, NULL);		break;
		}		
	}
	
	inState->SetState(false, false, false,	true, true, false, false);
	switch(mDragDest) {
	case gui_Table_Row:
	case gui_Table_Column:
	case gui_Table_Cell:
		if (cell_x == mDragX || mDragDest == gui_Table_Row)
		if (cell_y == mDragY || mDragDest == gui_Table_Column)
		{
			glColor4fv(mColorInsertInto);
			glBegin(GL_QUADS);
			glVertex2i(cell_bounds[0]+1,cell_bounds[1]+1);
			glVertex2i(cell_bounds[0]+1,cell_bounds[3]-1);
			glVertex2i(cell_bounds[2]-1,cell_bounds[3]-1);
			glVertex2i(cell_bounds[2]-1,cell_bounds[1]+1);
			glEnd();
		}
		break;
	
	case gui_Insert_Left:
	case gui_Insert_Right:
	case gui_Insert_Bottom:
	case gui_Insert_Top:
		glColor4fv(mColorInsertBetween);
		glLineWidth(4);
		glBegin(GL_LINES);
		if ((mDragDest == gui_Insert_Left && cell_x == mDragX) || (mDragDest == gui_Insert_Right && cell_x == mDragX+1))
		{
			glVertex2i(cell_bounds[0],cell_bounds[1]);
			glVertex2i(cell_bounds[0],cell_bounds[3]);			
		}			
		if ((mDragDest == gui_Insert_Right && cell_x == mDragX) || (mDragDest == gui_Insert_Left && cell_x == mDragX-1))
		{
			glVertex2i(cell_bounds[2],cell_bounds[1]);
			glVertex2i(cell_bounds[2],cell_bounds[3]);			
		}			
		if ((mDragDest == gui_Insert_Bottom && cell_y == mDragY) || (mDragDest == gui_Insert_Top && cell_y == mDragY+1))
		{
			glVertex2i(cell_bounds[0],cell_bounds[1]);
			glVertex2i(cell_bounds[2],cell_bounds[1]);			
		}			
		if ((mDragDest == gui_Insert_Top && cell_y == mDragY) || (mDragDest == gui_Insert_Bottom && cell_y == mDragY-1))
		{
			glVertex2i(cell_bounds[0],cell_bounds[3]);
			glVertex2i(cell_bounds[2],cell_bounds[3]);			
		}			
		glEnd();
		glLineWidth(1);
		break;
	}
	glColor4fv(mColorGridlines);
	
}

int			GUI_TextTable::CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock)
{
	want_lock = 1;
	mModifiers = flags;
	mSelStartX = mSelStartY = -1;
	
	if (!mContent) 
	{
		mClickCellX = -1;
		mClickCellY = -1;	
		mEditInfo.content_type = gui_Cell_None;
		return 1;
	}
	
	if (!IsFocusedChain())	
		TakeFocus();
	
	if (mTextField)
	{
		TerminateEdit(true, false);	// ben says: mac finder will 'eat' the click that ends edits sometimes, but I find this annoying.		
		mClickCellX = -1;
		mClickCellY = -1;	
		mEditInfo.content_type = gui_Cell_None;
		return 1;
	}

	mContent->GetCellContent(cell_x,cell_y,mEditInfo);

	mClickCellX = -1;
	mClickCellY = -1;	
	
	cell_bounds[0] += (mEditInfo.indent_level * mCellIndent);
	
//	if (mouse_x < cell_bounds[0])	{ mEditInfo.content_type = gui_Cell_None; return 1; }
	
	if(mEditInfo.is_disclosed || mEditInfo.can_disclose)
	{
		if (mEditInfo.can_disclose)
		{
			mTrackLeft = cell_bounds[0];
			mTrackRight = cell_bounds[0] + mDiscloseIndent;

			if (mouse_x >= mTrackLeft && mouse_x < mTrackRight)
			{
				mEditInfo.content_type = gui_Cell_Disclose;
				mClickCellX = cell_x;
				mClickCellY = cell_y;
				mInBounds = 1;
				BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
				return 1;
			}
		}
		cell_bounds[0] += mDiscloseIndent;		
	}
	
	if ((!mEditInfo.is_selected || (mModifiers & (gui_ShiftFlag+gui_ControlFlag))) && mEditInfo.can_select)
	{
		mSelStartX = cell_x;
		mSelStartY = cell_y;
		want_lock = 0;
		
		int old_x1,old_y1,old_x2,old_y2;
		if ((mModifiers & gui_ShiftFlag) && mContent->SelectGetExtent(old_x1,old_y1,old_x2,old_y2))
		{
			if (cell_x < ((old_x1 + old_x2)/2))		mSelStartX = old_x2;
			else									mSelStartX = old_x1;
			if (cell_y < ((old_y1 + old_y2)/2))		mSelStartY = old_y2;
			else									mSelStartY = old_y1;

			mContent->SelectionStart(1);
		}
		else
			mContent->SelectionStart((mModifiers & (gui_ShiftFlag+gui_ControlFlag)) == 0);
		
		
		mContent->SelectRange(	min(mSelStartX,cell_x), 
								min(mSelStartY,cell_y),
								max(mSelStartX,cell_x), 
								max(mSelStartY,cell_y),		
								mModifiers & gui_ControlFlag);
		mEditInfo.content_type = gui_Cell_None;
		BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
		
		if (mEditInfo.can_drag && mParent->IsDragClick(mouse_x,mouse_y))
		{
			mContent->SelectionEnd();
			mContent->DoDrag(mParent, mouse_x,mouse_y,cell_bounds);
			return 0;
		}
		
		return 1;
	}	

	if (mEditInfo.can_drag && mParent->IsDragClick(mouse_x,mouse_y))
	{
		mContent->DoDrag(mParent, mouse_x,mouse_y,cell_bounds);
		return 0;
	}

	if (mouse_x < cell_bounds[0])	{ mEditInfo.content_type = gui_Cell_None; return 1; }
		
	if (!mEditInfo.can_edit)	return 1;
	
	int	all_edit = mParent->GetModifiersNow() & gui_OptionAltFlag;
	
	char  buf[256];
	switch(mEditInfo.content_type) {
	case gui_Cell_CheckBox:
		{
			mTrackLeft = cell_bounds[0];
			mTrackRight = cell_bounds[2];
			if (mouse_x >= mTrackLeft && mouse_x < mTrackRight)
			{
				mClickCellX = cell_x;
				mClickCellY = cell_y;				
				mInBounds = 1;
				BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);			
				return 1;
			}
		}
		break;
	case gui_Cell_EditText:
	case gui_Cell_Integer:
	case gui_Cell_Double:
		if (mParent)
		{
			switch(mEditInfo.content_type) {
			case gui_Cell_Integer:
				sprintf(buf,"%d",mEditInfo.int_val);
				mEditInfo.text_val = buf;
				break;
			case gui_Cell_Double:
				sprintf(buf,"%lf",mEditInfo.double_val);
				mEditInfo.text_val = buf;
				break;
			}
			CreateEdit(cell_bounds);
			mClickCellX = cell_x;
			mClickCellY = cell_y;
			mTextField->SetDescriptor(mEditInfo.text_val);
			mTextField->SetSelection(0,mEditInfo.text_val.size());
			return 1;
		}
		break;
	case gui_Cell_Enum:
		{
			GUI_EnumDictionary	dict;
			mContent->GetEnumDictionary(cell_x, cell_y,dict);
			if (!dict.empty())
			{
				vector<GUI_MenuItem_t>	items(dict.size()+1);
				vector<int>				enum_vals(dict.size());
				int i = 0;
				int cur = -1;
				for (GUI_EnumDictionary::iterator it = dict.begin(); it != dict.end(); ++it, ++i)
				{
					enum_vals[i] = it->first;
					items[i].name = it->second.c_str();
					items[i].key = 0;
					items[i].flags = 0;
					items[i].cmd = 0;
					if (mEditInfo.int_val == it->first) cur = i;
					items[i].checked = (mEditInfo.int_val == it->first) ? 1 : 0;
				}
				int choice = mParent->PopupMenuDynamic(&*items.begin(), cell_bounds[0],cell_bounds[3],cur);
				if (choice >= 0 && choice < enum_vals.size())
				{
					mEditInfo.int_val = enum_vals[choice];
					mContent->AcceptEdit(cell_x, cell_y, mEditInfo, all_edit);
				}
			}
			mEditInfo.content_type = gui_Cell_None;
		}
		break;
	case gui_Cell_EnumSet:
		{
			GUI_EnumDictionary	dict;
			mContent->GetEnumDictionary(cell_x, cell_y,dict);
			if (!dict.empty())
			{
				vector<GUI_MenuItem_t>	items(dict.size()+1);
				vector<int>				enum_vals(dict.size());
				int i = 0;
				int cur = -1;
				for (GUI_EnumDictionary::iterator it = dict.begin(); it != dict.end(); ++it, ++i)
				{
					enum_vals[i] = it->first;
					items[i].name = it->second.c_str();
					items[i].key = 0;
					items[i].flags = 0;
					items[i].cmd = 0;
					items[i].checked = (mEditInfo.int_set_val.count(it->first) > 0);
				}
				int choice = mParent->PopupMenuDynamic(&*items.begin(), cell_bounds[0],cell_bounds[3],cur);
				if (choice >= 0 && choice < enum_vals.size())
				{
					if(mEditInfo.int_set_val.count(enum_vals[choice]) > 0)
						mEditInfo.int_set_val.erase(enum_vals[choice]);
					else
						mEditInfo.int_set_val.insert(enum_vals[choice]);
					mContent->AcceptEdit(cell_x, cell_y, mEditInfo, all_edit);
				}
			}
			mEditInfo.content_type = gui_Cell_None;
		}
		break;
	}
	return 1;
}

void		GUI_TextTable::CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
	int new_in;
	
	if (mSelStartX != -1 && mSelStartY != -1)
	{
		int x1, y1, x2, y2;
		if (mContent->SelectGetLimits(x1,y1,x2,y2))
		{
			if (cell_x < x1) cell_x = x1;
			if (cell_x > x2) cell_x = x2;
			if (cell_y < y1) cell_y = y1;
			if (cell_y > y2) cell_y = y2;
			
			if (mParent) mParent->RevealCell(cell_x, cell_y);
		
			mContent->SelectRange(min(mSelStartX,cell_x), 
								  min(mSelStartY,cell_y),
								  max(mSelStartX,cell_x), 
								  max(mSelStartY,cell_y),
								  mModifiers & gui_ControlFlag);
			BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
		}
	}
	
	switch(mEditInfo.content_type) {
	case gui_Cell_Disclose:
		new_in = (mouse_x >= mTrackLeft && mouse_x < mTrackRight &&
				  mouse_y >= cell_bounds[1] && mouse_y <= cell_bounds[3]);
		if (new_in != mInBounds)
		{
			mInBounds = new_in;
			BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
		}
		break;
	case gui_Cell_CheckBox:
		new_in = (mouse_x >= mTrackLeft && mouse_x < mTrackRight &&
				  mouse_y >= cell_bounds[1] && mouse_y <= cell_bounds[3]);
		if (new_in != mInBounds)
		{
			mInBounds = new_in;
			BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
		}
		break;
	}
}

void		GUI_TextTable::CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
	int new_in;

	if (mSelStartX != -1 && mSelStartY != -1)
	{
		int x1, y1, x2, y2;
		if (mContent->SelectGetLimits(x1,y1,x2,y2))
		{
			if (cell_x < x1) cell_x = x1;
			if (cell_x > x2) cell_x = x2;
			if (cell_y < y1) cell_y = y1;
			if (cell_y > y2) cell_y = y2;

			if (mParent) mParent->RevealCell(cell_x, cell_y);

			mContent->SelectRange(min(mSelStartX,cell_x), 
								  min(mSelStartY,cell_y),
								  max(mSelStartX,cell_x), 
								  max(mSelStartY,cell_y),
								  mModifiers & gui_ControlFlag);
			mContent->SelectionEnd();
			BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
			mSelStartX = -1;
			mSelStartY = -1;
		}
	}

	int	all_edit = mParent->GetModifiersNow() & gui_OptionAltFlag;
		
	switch(mEditInfo.content_type) {
	case gui_Cell_Disclose:
		mInBounds = (mouse_x >= mTrackLeft && mouse_x < mTrackRight &&
				  mouse_y >= cell_bounds[1] && mouse_y <= cell_bounds[3]);
		if (mInBounds) mContent->ToggleDisclose(cell_x,cell_y);
		BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
		break;
	case gui_Cell_CheckBox:
		mInBounds = (mouse_x >= mTrackLeft && mouse_x < mTrackRight &&
				  mouse_y >= cell_bounds[1] && mouse_y <= cell_bounds[3]);
		if (mInBounds) 
		{
			mEditInfo.int_val = 1 - mEditInfo.int_val;
			mContent->AcceptEdit(cell_x,cell_y, mEditInfo, all_edit);
			BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
		}	
		break;
	}
	if (!mTextField)
	{
		mEditInfo.content_type = gui_Cell_None;
		mClickCellX = -1;
		mClickCellY = -1;	
	}
}


GUI_DragOperation	GUI_TextTable::CellDragEnter	(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	// MAIN DRAG HANDLER - WHAT IS GOING ON??
	//
	//	First: how do we try to leave the world when we're done?  Well, basically:
	//	1. mDragX, mDragY, and mDragPart contain info about the POTENTIAL drop, no matter whether the drop is legal or not.  
	//	   Note that horizontal vs. vertical reordering is invariant, so we do not store this.
	//  2. mDragDest contains what WOULD happen if there is a drop.  We do not store what kind of operation (move/copy), but we do know how to draw the resulting situation.
	//	3. We save our return value in "mLastOp" as a cache.	
	//
	// To do this we:
	// 1. Find out what legal possibilities we have.
	// 2. Find the part.
	// 3. Check various possible drops in order...
	// 4. Patch-up the result code.
	mDragX = cell_x;
	mDragY = cell_y;
	mDragDest = gui_Table_None;
	
	if (mContent == NULL) return gui_Drag_None;
	
	int	can_insert_col;
	int	can_insert_row;
	int can_into_cell;
	
	mLastOp = gui_Drag_None;
	mDragDest = gui_Table_None;
	int	whole_row = 0, whole_col = 0;
	
	// 1. Find the legal drops we can do.  How we analyze depends on this!
	
	mContent->GetLegalDropOperations(can_insert_col,can_insert_row,can_into_cell);	
	
	// 2. Find the part and
	// 3.  try the drops it suggests.	
	if (can_insert_row)
	{
		mDragPart = GetCellDragPart(cell_bounds,mouse_x,mouse_y, 1);		
		// 3a. We are allowed to do row inserts.
		if (can_into_cell)
		{
			// 3a-1.  We can do a row reorder or a drop-inside.  Evaluate in order by position.
			switch(mDragPart) {
			case drag_LowerOrInto:
			case drag_HigherOrInto:
												if ((mLastOp = mContent->CanDropBetweenRows(cell_y + (mDragPart == drag_HigherOrInto ? 1 : 0), drag, allowed, recommended)) != gui_Drag_None)
													mDragDest = mDragPart == drag_HigherOrInto ? gui_Insert_Top : gui_Insert_Bottom;
				if (mLastOp == gui_Drag_None)	if ((mLastOp = mContent->CanDropIntoCell(cell_x, cell_y, drag, allowed, recommended, whole_col, whole_row)) != gui_Drag_None)
													mDragDest = gui_Table_Cell;
				break;
			case drag_IntoOrLower:
			case drag_IntoOrHigher:
												if ((mLastOp = mContent->CanDropIntoCell(cell_x, cell_y, drag, allowed, recommended, whole_col, whole_row)) != gui_Drag_None)
													mDragDest = gui_Table_Cell;
				if (mLastOp == gui_Drag_None)	if ((mLastOp = mContent->CanDropBetweenRows(cell_y + (mDragPart == drag_IntoOrHigher ? 1 : 0), drag, allowed, recommended)) != gui_Drag_None)
													mDragDest = mDragPart == drag_IntoOrHigher ? gui_Insert_Top : gui_Insert_Bottom;
				break;
			}
		}
		else
		{
			// 3a-2.  We can only reorder.
			if ((mLastOp = mContent->CanDropBetweenRows(cell_y + ((mDragPart == drag_HigherOrInto || mDragPart == drag_IntoOrHigher) ? 1 : 0), drag, allowed, recommended)) != gui_Drag_None)
				mDragDest = (mDragPart == drag_HigherOrInto || mDragPart == drag_IntoOrHigher) ? gui_Insert_Top : gui_Insert_Bottom;
		}
	}
	else if (can_insert_col)
	{
		// 3b.  We are allowed to do column inserts.
		mDragPart = GetCellDragPart(cell_bounds,mouse_x,mouse_y, 0);		
		if (can_into_cell)
		{
			// 3b-1.  Try column reorder or a drop-inside, depending on part.
			switch(mDragPart) {
			case drag_LowerOrInto:
			case drag_HigherOrInto:
												if ((mLastOp = mContent->CanDropBetweenColumns(cell_x + (mDragPart == drag_HigherOrInto ? 1 : 0), drag, allowed, recommended)) != gui_Drag_None)
													mDragDest = mDragPart == drag_HigherOrInto ? gui_Insert_Right : gui_Insert_Left;
				if (mLastOp == gui_Drag_None)	if ((mLastOp = mContent->CanDropIntoCell(cell_x, cell_y, drag, allowed, recommended, whole_col, whole_row)) != gui_Drag_None)
													mDragDest = gui_Table_Cell;
				break;
			case drag_IntoOrLower:
			case drag_IntoOrHigher:
												if ((mLastOp = mContent->CanDropIntoCell(cell_x, cell_y, drag, allowed, recommended, whole_col, whole_row)) != gui_Drag_None)
													mDragDest = gui_Table_Cell;
				if (mLastOp == gui_Drag_None)	if ((mLastOp = mContent->CanDropBetweenColumns(cell_x + (mDragPart == drag_IntoOrHigher ? 1 : 0), drag, allowed, recommended)) != gui_Drag_None)
													mDragDest = mDragPart == drag_IntoOrHigher ? gui_Insert_Right : gui_Insert_Left;
				break;
			}
		}
		else
		{
			// 3b-2.  We can only col-reorder.  Just tr y that.
			if ((mLastOp = mContent->CanDropBetweenColumns(cell_x + ((mDragPart == drag_HigherOrInto || mDragPart == drag_IntoOrHigher) ? 1 : 0), drag, allowed, recommended)) != gui_Drag_None)
				mDragDest = (mDragPart == drag_HigherOrInto || mDragPart == drag_IntoOrHigher) ? gui_Insert_Right : gui_Insert_Left;
		}	
	}
	else if (can_into_cell)
	{
		// 3c. We can only drop into cells.
		mDragPart = drag_WholeCell;
		if ((mLastOp = mContent->CanDropIntoCell(cell_x, cell_y, drag, allowed, recommended, whole_col, whole_row)) != gui_Drag_None)
			mDragDest = gui_Table_Cell;
	} else
		mDragPart = drag_WholeCell;
	
	// 4. Patch up feedback for in-cell drops.
	
	if (mDragDest == gui_Table_Cell && whole_col) mDragDest = gui_Table_Column;
	if (mDragDest == gui_Table_Cell && whole_row) mDragDest = gui_Table_Row;
	
	mParent->Refresh();
	return mLastOp;
}

GUI_DragOperation	GUI_TextTable::CellDragWithin	(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	if (mContent == NULL) return gui_Drag_None;
	
	int				can_insert_col;
	int				can_insert_row;
	int				can_into_cell;
	GUI_DragPart	part;
	mContent->GetLegalDropOperations(can_insert_col,can_insert_row,can_into_cell);	
	
	// 2. Find the part and
	// 3.  try the drops it suggests.	
		 if (can_insert_row)		part = GetCellDragPart(cell_bounds,mouse_x,mouse_y, 1);		
	else if (can_insert_col)		part = GetCellDragPart(cell_bounds,mouse_x,mouse_y, 0);		
	else							part = drag_WholeCell;
	
	// IF something has changed, retry the operation.

	if (mDragPart != part || cell_x != mDragX || cell_y != mDragY)		return this->CellDragEnter(cell_bounds,cell_x,cell_y,mouse_x,mouse_y,drag,allowed,recommended);
																		return mLastOp;
}

void	GUI_TextTable::CellDragLeave	(int cell_bounds[4], int cell_x, int cell_y)
{
	mDragDest = gui_Table_None;
	mDragPart = drag_WholeCell;
	mDragX = mDragY = -1;
	mLastOp = gui_Drag_None;
	mParent->Refresh();
}

GUI_DragOperation	GUI_TextTable::CellDrop		(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	if (mContent == NULL) return gui_Drag_None;

	// Same logic as moved.  Basically if XY has changed, re-evaluate.  This shouldn't be needed but...what the heck.
	
	int				can_insert_col;
	int				can_insert_row;
	int				can_into_cell;
	GUI_DragPart	part;
	mContent->GetLegalDropOperations(can_insert_col,can_insert_row,can_into_cell);	
	
	// 2. Find the part and
	// 3.  try the drops it suggests.	
		 if (can_insert_row)		part = GetCellDragPart(cell_bounds,mouse_x,mouse_y, 1);		
	else if (can_insert_col)		part = GetCellDragPart(cell_bounds,mouse_x,mouse_y, 0);		
	else							part = drag_WholeCell;
	
	// IF something has changed, retry the operation.

	if (mDragPart != part || cell_x != mDragX || cell_y != mDragY)
	{
		DebugAssert(!"Strange.  Drop not on location we last dragged to.");
		mLastOp = this->CellDragEnter(cell_bounds,cell_x,cell_y,mouse_x,mouse_y,drag,allowed,recommended);
	}
	
	if (mLastOp != gui_Drag_None)
	switch(mDragDest) {
	case	gui_Table_Row:
	case 	gui_Table_Column:
	case 	gui_Table_Cell:
		mLastOp = mContent->DoDropIntoCell(mDragX, mDragY, drag, allowed, recommended);
		break;
	case 	gui_Insert_Left:
	case 	gui_Insert_Right:
		mLastOp = mContent->DoDropBetweenColumns(mDragX + (mDragDest == gui_Insert_Right ? 1 : 0), drag, allowed, recommended);
		break;
	case 	gui_Insert_Bottom:
	case 	gui_Insert_Top:
		mLastOp = mContent->DoDropBetweenRows(mDragY + (mDragDest == gui_Insert_Top ? 1 : 0), drag, allowed, recommended);
		break;
	default:
		mLastOp = gui_Drag_None;
		break;
	}

	return mLastOp;	
}



void		GUI_TextTable::Deactivate(void)
{
}

void		GUI_TextTable::CreateEdit(int cell_bounds[4])
{
	if (!mTextField) 
	{
		mTextField = new GUI_TextField(1,this); 
		mTextField->SetParent(mParent); 
		mTextField->SetVKAllowed(GUI_VK_RETURN, false); 
		mTextField->SetKeyAllowed(GUI_KEY_ESCAPE, false); 
		mTextField->SetKeyAllowed(GUI_KEY_TAB, false); 
	}
	mTextField->SetBounds(cell_bounds);
	mTextField->Show();
	mTextField->TakeFocus();
}

int			GUI_TextTable::TerminateEdit(bool inSave, bool in_all)
{
	if (mTextField && mTextField->IsFocused() && 
		(mEditInfo.content_type == gui_Cell_EditText ||  mEditInfo.content_type == gui_Cell_Integer || mEditInfo.content_type == gui_Cell_Double))
	{
		if (inSave)
		{
			mTextField->GetDescriptor(mEditInfo.text_val);
			switch(mEditInfo.content_type) {
			case gui_Cell_Integer:
				mEditInfo.int_val = atoi(mEditInfo.text_val.c_str());
				break;
			case gui_Cell_Double:
				mEditInfo.double_val = atof(mEditInfo.text_val.c_str());
				break;
			}
			mContent->AcceptEdit(mClickCellX, mClickCellY, mEditInfo, in_all);	
		}
		this->TakeFocus();
		mTextField->Hide();
		delete mTextField;
		mTextField = NULL;
		mEditInfo.content_type = gui_Cell_None;
		return 1;				
	}
	return 0;
}

GUI_TextTable::GUI_DragPart	GUI_TextTable::GetCellDragPart(int cell_bounds[4], int x, int y, int vertical)
{
	float span = vertical ? cell_bounds[3] - cell_bounds[1] : cell_bounds[2] - cell_bounds[0];
	float delta = vertical ? y - cell_bounds[1] : x - cell_bounds[0];
	if (span == 0.0) return drag_LowerOrInto;
	float ratio = delta / span;
	
	if (ratio < 0.25) return drag_LowerOrInto;
	if (ratio < 0.50) return drag_IntoOrLower;
	if (ratio < 0.75) return drag_IntoOrHigher;
					  return drag_HigherOrInto;

}

int			GUI_TextTable::KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (!mContent) return 0;
	
	if ((inFlags & gui_UpFlag) == 0)
	switch(inKey) {
	case GUI_KEY_LEFT:		
	case GUI_KEY_RIGHT:
	case GUI_KEY_UP:
	case GUI_KEY_DOWN:
		{
			int x1, x2, y1, y2;
			int x_min, x_max, y_min, y_max;
			if (mContent->SelectGetLimits(x_min,y_min,x_max,y_max))
			{
				if (!mContent->SelectGetExtent(x1,y1,x2,y2))
				{
					mContent->SelectionStart(0);
					switch(inKey) {
					case GUI_KEY_LEFT:		mContent->SelectRange(x_max,y_max,x_max,y_max,0);	if(mParent)mParent->RevealCol(x_max);	break;
					case GUI_KEY_RIGHT:		mContent->SelectRange(0,0,0,0,0);					if(mParent)mParent->RevealCol(0);		break;
					case GUI_KEY_UP:		mContent->SelectRange(x_max,y_max,x_max,y_max,0);	if(mParent)mParent->RevealRow(y_max);	break;
					case GUI_KEY_DOWN:		mContent->SelectRange(0,0,0,0,0);					if(mParent)mParent->RevealRow(0);		break;
					}

				} else {
				
					switch(inKey) {
					case GUI_KEY_LEFT:		--x1;	if ((inFlags & gui_ShiftFlag) == 0)	x2 = x1;	if(mParent)mParent->RevealCol(x1);	break;
					case GUI_KEY_RIGHT:		++x2;	if ((inFlags & gui_ShiftFlag) == 0)	x1 = x2;	if(mParent)mParent->RevealCol(x2);	break;
					case GUI_KEY_UP:		++y2;	if ((inFlags & gui_ShiftFlag) == 0)	y1 = y2;	if(mParent)mParent->RevealRow(y2);	break;
					case GUI_KEY_DOWN:		--y1;	if ((inFlags & gui_ShiftFlag) == 0)	y2 = y1;	if(mParent)mParent->RevealRow(y1);	break;
					}
					
					x1 = min(x1, x_max);
					x2 = min(x2, x_max);
					x1 = max(x1, x_min);
					x2 = max(x2, x_min);
					y1 = min(y1, y_max);
					y2 = min(y2, y_max);
					y1 = max(y1, y_min);
					y2 = max(y2, y_min);
					
					mContent->SelectionStart(1);
					mContent->SelectRange(x1,y1,x2,y2, 0);
				}
				mContent->SelectionEnd();			
			}
		}
		break;
	}

	if(inKey == GUI_KEY_TAB && mTextField && mContent)
	{
		char buf[50];
		int x = mClickCellX;
		int y = mClickCellY;
		int cell_bounds[4];
		TerminateEdit(true, inFlags & inFlags & gui_OptionAltFlag);
		if (mParent)
		{
			if (mContent->TabAdvance(x,y, inFlags & gui_ShiftFlag, mEditInfo))	
			{
				mParent->RevealCell(x,y);
				mParent->CalcCellBounds(x,y,cell_bounds);
				mClickCellX = x;
				mClickCellY = y;
				switch(mEditInfo.content_type) {
				case gui_Cell_Integer:
					sprintf(buf,"%d",mEditInfo.int_val);
					mEditInfo.text_val = buf;
					break;
				case gui_Cell_Double:
					sprintf(buf,"%lf",mEditInfo.double_val);
					mEditInfo.text_val = buf;
					break;
				}
				CreateEdit(cell_bounds);
				mTextField->SetDescriptor(mEditInfo.text_val);
				mTextField->SetSelection(0,mEditInfo.text_val.size());
				mTextField->Refresh();
			}
		}
	}

	if(inVK == GUI_VK_RETURN && mTextField)
	{
		TerminateEdit(true, inFlags & gui_OptionAltFlag);
		return 1;
	}

	if(inKey == GUI_KEY_ESCAPE && mTextField)
	{
		TerminateEdit(false, false);
		return 1;
	}
	return 0;
}



//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

GUI_TextTableHeader::GUI_TextTableHeader() : mContent(NULL), mGeometry(NULL)
{	
	mColorGridlines[0] = 0.5f;
	mColorGridlines[1] = 0.5f;
	mColorGridlines[2] = 0.5f;
	mColorGridlines[3] = 1.0f;

	mColorText[0] = 0.8;
	mColorText[1] = 0.8;
	mColorText[2] = 0.8;
	mColorText[3] = 1.0;
}

GUI_TextTableHeader::~GUI_TextTableHeader()
{
}
	
void		GUI_TextTableHeader::SetProvider(GUI_TextTableHeaderProvider * content)
{
	mContent = content;
}

void		GUI_TextTableHeader::SetImage(const char * image)
{
	mImage = image;
}


void		GUI_TextTableHeader::SetColors(
							float		grid_lines[4],
							float		text[4])
{
	for (int n = 0; n < 4; ++n)
	{
		mColorGridlines		[n]=grid_lines		[n];
		mColorText			[n]=text			[n];
	}
}

void		GUI_TextTableHeader::SetGeometry(GUI_TableGeometry * geometry)
{
	mGeometry = geometry;
}

void		GUI_TextTableHeader::HeadDraw	 (int cell_bounds[4], int cell_x, GUI_GraphState * inState			  )
{
	if (mImage.empty())
	{
		inState->SetState(false, false, false,	false, false, false, false);
		glColor4fv(mColorGridlines);
		glBegin(GL_LINE_STRIP);
		glVertex2i(cell_bounds[0]  ,cell_bounds[1]);
		glVertex2i(cell_bounds[2]-1,cell_bounds[1]);
		glVertex2i(cell_bounds[2]-1,cell_bounds[3]);
		glEnd();
	}
	else
	{
		glColor3f(1,1,1);
		int tile[4] = { 0, 0, 1, 1 };
		GUI_DrawHorizontalStretch(inState, "header.png", cell_bounds, tile);
	}

	
	if (!mContent) return;
	GUI_HeaderContent	c;
	mContent->GetHeaderContent(cell_x,c);
	
	int line_h = (cell_bounds[1] + cell_bounds[3]) / 2 - GUI_GetLineAscent(font_UI_Basic) / 2;
	GUI_FontDraw(inState, font_UI_Basic, mColorText, cell_bounds[0]+3, line_h, c.title.c_str());	
}

int			GUI_TextTableHeader::HeadMouseDown(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock)
{	
	want_lock = 1;
	mCellResize = -1;
	if (mGeometry && abs(mouse_x - cell_bounds[0]) < RESIZE_MARGIN && cell_x > 0)
	{
		mLastX = mouse_x;
		mCellResize = cell_x -1;
		return 1;
	}
	if (mGeometry && abs(mouse_x - cell_bounds[2]) < RESIZE_MARGIN && cell_x < mGeometry->GetColCount())
	{
		mLastX = mouse_x;
		mCellResize = cell_x;
		return 1;
	}
	return 1;
}

void		GUI_TextTableHeader::HeadMouseDrag(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)
{
	if (mCellResize >= 0 && mGeometry)
	{
		mGeometry->SetCellWidth(mCellResize,mouse_x - mLastX + mGeometry->GetCellWidth(mCellResize));
		mLastX = mouse_x;
		BroadcastMessage(GUI_TABLE_SHAPE_RESIZED,0);
	}
}

void		GUI_TextTableHeader::HeadMouseUp  (int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)
{
	if (mCellResize >= 0 && mGeometry)
	{
		mGeometry->SetCellWidth(mCellResize,mouse_x - mLastX + mGeometry->GetCellWidth(mCellResize));
		mCellResize = -1;
		BroadcastMessage(GUI_TABLE_SHAPE_RESIZED,0);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

GUI_TextTableSide::GUI_TextTableSide() : mContent(NULL), mGeometry(NULL)
{	
	mColorGridlines[0] = 0.5f;
	mColorGridlines[1] = 0.5f;
	mColorGridlines[2] = 0.5f;
	mColorGridlines[3] = 1.0f;

	mColorText[0] = 0.8;
	mColorText[1] = 0.8;
	mColorText[2] = 0.8;
	mColorText[3] = 1.0;
}

GUI_TextTableSide::~GUI_TextTableSide()
{
}
	
void		GUI_TextTableSide::SetProvider(GUI_TextTableHeaderProvider * content)
{
	mContent = content;
}

void		GUI_TextTableSide::SetGeometry(GUI_TableGeometry * geometry)
{
	mGeometry = geometry;
}

void		GUI_TextTableSide::SetColors(
							float		grid_lines[4],
							float		text[4])
{
	for (int n = 0; n < 4; ++n)
	{
		mColorGridlines		[n]=grid_lines		[n];
		mColorText			[n]=text			[n];
	}
}

void		GUI_TextTableSide::SideDraw	 (int cell_bounds[4], int cell_y, GUI_GraphState * inState			  )
{
	inState->SetState(false, false, false,	false, false, false, false);
	glColor4fv(mColorGridlines);
	glBegin(GL_LINE_STRIP);
	glVertex2i(cell_bounds[0]  ,cell_bounds[1]);
	glVertex2i(cell_bounds[2]-1,cell_bounds[1]);
	glVertex2i(cell_bounds[2]-1,cell_bounds[3]);
	glEnd();

	if (!mContent) return;
	GUI_HeaderContent	c;
	mContent->GetHeaderContent(cell_y,c);
	
	int line_h = (cell_bounds[1] + cell_bounds[3]) / 2 - GUI_GetLineAscent(font_UI_Basic) / 2;
	GUI_FontDraw(inState, font_UI_Basic, mColorText, cell_bounds[0]+3, line_h, c.title.c_str());	
}

int			GUI_TextTableSide::SideMouseDown(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock)
{
	want_lock = 1;
	mCellResize = -1;
	if (mGeometry && abs(mouse_y - cell_bounds[1]) < RESIZE_MARGIN && cell_y > 0)
	{
		mLastY = mouse_y;
		mCellResize = cell_y -1;
		return 1;
	}
	if (mGeometry && abs(mouse_y - cell_bounds[2]) < RESIZE_MARGIN && cell_y < mGeometry->GetRowCount())
	{
		mLastY = mouse_y;
		mCellResize = cell_y;
		return 1;
	}
	return 1;
}

void		GUI_TextTableSide::SideMouseDrag(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button)
{
	if (mCellResize >= 0 && mGeometry)
	{
		mGeometry->SetCellHeight(mCellResize,mouse_y - mLastY + mGeometry->GetCellHeight(mCellResize));
		mLastY = mouse_y;
		BroadcastMessage(GUI_TABLE_SHAPE_RESIZED,0);
	}
}

void		GUI_TextTableSide::SideMouseUp  (int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button)
{
	if (mCellResize >= 0 && mGeometry)
	{
		mGeometry->SetCellHeight(mCellResize,mouse_y - mLastY + mGeometry->GetCellWidth(mCellResize));
		mCellResize = -1;
		BroadcastMessage(GUI_TABLE_SHAPE_RESIZED,0);
	}
}

