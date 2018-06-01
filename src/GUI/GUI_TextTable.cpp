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

#include "GUI_TextField.h"
#include "GUI_TextTable.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "GUI_Messages.h"
#include "WED_UIDefs.h"
#include "PlatformUtils.h"
#include "GUI_DrawUtils.h"
#include <math.h>
#include "WED_ToolUtils.h"
#include "GUI_Resources.h"
#include "AssertUtils.h"
#include "WED_Sign_Editor.h"

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

#define CELL_MARGIN 3
#define	MIN_CELL_WIDTH	(CELL_MARGIN*2)

//--------------------------------------------------------------------------------------------------------------------------------------------

class GUI_MouseCatcher : public GUI_Pane, public GUI_Broadcaster {
public:

	GUI_MouseCatcher(intptr_t msg) : mMsg(msg) { }
	virtual ~GUI_MouseCatcher() { }

	virtual	int			MouseDown(int x, int y, int button) { 
		BroadcastMessage(mMsg, 0);
		return 1;
	}
	
private:
	intptr_t		mMsg;
};


//--------------------------------------------------------------------------------------------------------------------------------------------


GUI_TextTable::GUI_TextTable(GUI_Commander * parent, int indent, int live_edit) : GUI_Commander(parent),
	mContent(NULL),
	mClickCellX(-1),
	mClickCellY(-1),
	mParent(NULL),
	mTextField(NULL),
	mSignField(NULL),
	mCatcher(NULL),
	mSelStartX(-1),
	mSelStartY(-1),
	mDragX(-1),
	mDragY(-1),
	mFont(font_UI_Basic),
	mDragDest(gui_Table_None),
	mDragPart(drag_WholeCell),
	mCellIndent(indent),
	mLiveEdit(live_edit),
	mGeometry(NULL)
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

	mTFColorText[0] = 0.0;		mTFColorText[1] = 0.0;		mTFColorText[2] = 0.0;		mTFColorText[3] = 1.0;
	mTFColorHilite[0] = 1.0;	mTFColorHilite[1] = 1.0;	mTFColorHilite[2] = 0.0;	mTFColorHilite[3] = 1.0;
	mTFColorBkgnd[0] = 1.0;		mTFColorBkgnd[1] = 1.0;		mTFColorBkgnd[2] = 1.0;		mTFColorBkgnd[3] = 1.0;
	mTFColorBox[0] = 0.3;		mTFColorBox[1] = 0.5;		mTFColorBox[2] = 1.0;		mTFColorBox[3] = 1.0;

	mDiscloseIndent = GUI_GetImageResourceWidth("disclose.png") / 2;
}

GUI_TextTable::~GUI_TextTable()
{
}

void		GUI_TextTable::SetImage(const char * image, int alternations)
{
	mAlternate = alternations;
	mImage = image;
}

void		GUI_TextTable::SetFont(int font)
{
	mFont = font;
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

void		GUI_TextTable::SetTextFieldColors(
								float text_color[4],
								float hilite_color[4],
								float bkgnd_color[4],
								float box_color[4])
{
	for (int n = 0; n < 4; ++n)
	{
		mTFColorText[n] = text_color[n];
		mTFColorHilite[n] = hilite_color[n];
		mTFColorBkgnd[n] = bkgnd_color[n];
		mTFColorBox[n] = box_color[n];
	}
}

void	GUI_TextTable::KillEditing(bool save_it)
{
	TerminateEdit(save_it, false, true);
}


void		GUI_TextTable::SetParentTable(GUI_Table * parent)
{
	mParent = parent;
}

void		GUI_TextTable::SetGeometry(GUI_TableGeometry * geometry)
{
	mGeometry = geometry;
}

void		GUI_TextTable::SetProvider(GUI_TextTableProvider * content)
{
	mContent = content;
}


void		GUI_TextTable::CellDraw	 (int cell_bounds[4], int cell_x, int cell_y, GUI_GraphState * inState			  )
{
	//--This section draws?
	if (mImage.empty())
	{
		//Change the instage so it is completely deactivated
		inState->SetState(false, false, false,	false, false, false, false);
		//sets the color
		
		glColor4fv(mColorGridlines);
		//draws the border of where the image should be.
		glBegin(GL_LINE_STRIP);
		GLfloat x1 = cell_bounds[0];
		GLfloat x2 = cell_bounds[2];
		GLfloat y1 = cell_bounds[1];
		GLfloat y2 = cell_bounds[3];
		
		glVertex2f(x1 + 0.5f, y1 + 0.5f);
		glVertex2f(x2 - 0.5f, y1 + 0.5f);
		glVertex2f(x2 - 0.5f, y2 - 0.5f);
		glEnd();
		
	}
	//---------
	
	//Cell Type, intacts with GUI_CellContentType enums
	int cell_type = 0;

	//Cell Content
	GUI_CellContent	c;

	//If there is content
	if (mContent)
	{
		//Get the Cell content bassed on the x,y positions and c
		mContent->GetCellContent(cell_x,cell_y,c);
		//If there is an image
		if (!mImage.empty())
			//And it can be edited
			if (c.can_edit)
				//and the content type is not none, the cell type is 1 (aka Disclose)
				if (c.content_type != gui_Cell_None) cell_type = 1;
	}

	//If there is an image
	if (!mImage.empty())
	{
		//Set the color equal to white
		glColor3f(1,1,1);
		//Set the sprite sheet selector
		int tile[4] = { 0, cell_type, 1, 2 };
		//Draw it
		GUI_DrawHorizontalStretch(inState, mImage.c_str(), cell_bounds, tile);
	}

	//If there is no longer content, exit
	if (!mContent) return;
	
	//--Draw "selected" highlights
	if (c.is_selected)
	{
		//highlight the selected place
		glColor4fv(mColorSelect);
		glBegin(GL_QUADS);
		glVertex2i(cell_bounds[0]  ,cell_bounds[1]+1);
		glVertex2i(cell_bounds[0]  ,cell_bounds[3]  );
		glVertex2i(cell_bounds[2]-1,cell_bounds[3]  );
		glVertex2i(cell_bounds[2]-1,cell_bounds[1]+1);
		glEnd();
		//Reset the color
		glColor4fv(mColorGridlines);
	}
	//-----------------------------------------------------

	//advance the left side of the cell bounds by the indent level * the pixels of an indent
	cell_bounds[0] += (c.indent_level * mCellIndent);

	//Switch on the content type
	switch(c.content_type) {
	//If the Cell is disclosed
	case gui_Cell_Disclose:
		//clear the content's text value
		c.text_val.clear();
		break;
	//If it is a check box, also clear the text value
	case gui_Cell_CheckBox:
		c.text_val = "";
		break;
	}
	
	if(c.can_delete)
	{
					  //x, y, num_x tiles, num_y tiles
		int tile[4] = { 0, 0, 1, 1};

		glColor3f(1,1,1);
		GUI_DrawCentered(inState, "delete.png", cell_bounds, -1,0, tile, NULL, NULL);

		cell_bounds[0] += GUI_GetImageResourceWidth("delete.png");
	}

	if(c.is_disclosed || c.can_disclose)
	{
		int middle = (cell_bounds[1] + cell_bounds[3]) / 2;

		/*-----2x2 grid-
		| A    | B  |
		|  >  |  V  |
		|_____|_____|
		| C   | D   |
		|_____|_____|*/
			
		int tile[4] = { 
			//If discolosed V, else >
			c.is_disclosed ? 1 : 0,
			//If Everything is in its right place 1, else 0 (C or D)
			(cell_x == mClickCellX && cell_y == mClickCellY && mEditInfo.content_type == gui_Cell_Disclose && mInBounds) ? 1 : 0,
			2, 2 };
			
		glColor3f(1,1,1);
		GUI_DrawCentered(inState, "disclose.png", cell_bounds, -1, 0, tile, NULL, NULL);

		cell_bounds[0] += mDiscloseIndent;
	}
	
	float	cell_h = cell_bounds[3] - cell_bounds[1];
	float	line_h = GUI_GetLineHeight(mFont);
	int		descent = GUI_GetLineDescent(mFont);
	float	cell2line = (cell_h - line_h + descent) * 0.5f;

	if (!HasEdit() || mClickCellX != cell_x || mClickCellY != cell_y)
	{
		int trunc_width = cell_bounds[2] - cell_bounds[0] - (CELL_MARGIN*2);
		if (c.content_type == gui_Cell_Enum || c.content_type == gui_Cell_EnumSet)
			trunc_width -= GUI_GetImageResourceWidth("arrows.png");

		if (c.string_is_resource)
		{
			string::size_type s=0, e;
			while(1)
			{
				string res;
				e = c.text_val.find(',',s);
				if (e==c.text_val.npos)
				{
					res = c.text_val.substr(s);
				}
				else
				{
					res = c.text_val.substr(s,e-s);
					s=e+1;
				}

				int tile[4] = { 0, 0, 1, 1 };
				glColor3f(1,1,1);
				GUI_DrawCentered(inState, res.c_str(), cell_bounds, -1, 0, tile, NULL, NULL);

				cell_bounds[0] += 20;

				if (e == c.text_val.npos)
					break;
			}
		}
		else
		{
			if(c.content_type == gui_Cell_TaxiText)
			{
				RenderSign(inState, 
							cell_bounds[0]+CELL_MARGIN, (float) cell_bounds[1] + cell2line,
							c.text_val.c_str(),
							0.5f, mFont,
							(c.is_selected||cell_type) ? mColorTextSelect : mColorText);
			}
			else
			{
				GUI_TruncateText(c.text_val, mFont, trunc_width);
				
				//Draws normal text
				GUI_FontDraw(inState, mFont,
					(c.is_selected||cell_type) ? mColorTextSelect : mColorText,
					cell_bounds[0]+CELL_MARGIN, (float) cell_bounds[1] + cell2line, c.text_val.c_str());
			}
		}
	}

	//--Draws all enums
	if (c.content_type == gui_Cell_Enum || c.content_type == gui_Cell_EnumSet)
	{
		int tile[4] = { 0, 0, 1, 1 };
		glColor4fv((c.is_selected||cell_type) ? mColorTextSelect : mColorText);
		GUI_DrawCentered(inState, "arrows.png", cell_bounds, 1, 0, tile, NULL, NULL);
	}
	//---------------------------------------------------------------------------

	//--This section draws all Checkbox related items
	if (c.content_type == gui_Cell_CheckBox)
	{
		int selector[4] = {
			(c.int_val != 0) ? 1 : (c.bool_partial ? 2 : 0),
			0,
			c.bool_val == gui_Bool_Check ? 2 : 3,
			1 };


		 if (c.bool_val ==  gui_Bool_Check)
		 		glColor4fv((c.is_selected||cell_type) ? mColorTextSelect : mColorText);
		else
				glColor3f(1,1,1);
		switch(c.bool_val) {
		case gui_Bool_Check:		GUI_DrawCentered(inState, "check.png", cell_bounds, 0, 0, selector, NULL, NULL);	break;
		case gui_Bool_Lock:			GUI_DrawCentered(inState, "lock.png", cell_bounds, 0, 0, selector, NULL, NULL);		break;
		case gui_Bool_Visible:		GUI_DrawCentered(inState, "eye.png", cell_bounds, 0, 0, selector, NULL, NULL);		break;
		}
	}
	//---------------------------------------------------------------

	inState->SetState(false, false, false,	true, true, false, false);
	//----This switch draws highlights and effects for a dragging event
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
	//-----------------------------------------------------------------
}

int GUI_TextTable::CreateMenuFromDict(vector<GUI_MenuItem_t>& items, vector<int> enum_vals, GUI_EnumDictionary& dict)
{
	for(GUI_EnumDictionary::iterator di = dict.begin(); di != dict.end(); ++di)
	if(!di->second.second)
		di->second.first.insert(0,";");

	int i = 0;
	int current_sel = -1;
	for (GUI_EnumDictionary::iterator it = dict.begin(); it != dict.end(); ++it, ++i)
	{
		enum_vals[i] = it->first;
		items[i].name = it->second.first.c_str();
		items[i].key = 0;
		items[i].flags = 0;
		items[i].cmd = 0;
		if (mEditInfo.int_val == it->first)
		{
			current_sel = i;
			
			//Saves the selected enum's text value to mEditInfo
			//So the value can be used in whatever calls Accept Edit
			mEditInfo.text_val = it->second.first.c_str();
		}
		items[i].checked = (mEditInfo.int_val == it->first) ? 1 : 0;
	}
	return current_sel;
}

int			GUI_TextTable::CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock)
{
	want_lock = 1;
	mModifiers = flags;
	mSelStartX = mSelStartY = -1;
	mCellResize = -1;

	static int last_cell_x = -1;
	static int last_cell_y = -1;
	static int last_mouse_x = -1;
	static int last_mouse_y = -1;
	static float last_time_now = -1.0;
	float time_now = mParent->GetTimeNow();
	bool did_double = false;
	if (mContent &&
		last_cell_x == cell_x &&
		last_cell_y == cell_y &&
		fabs((float)(last_mouse_x - mouse_x)) < 3.0 &&
		fabs((float)(last_mouse_y - mouse_y)) < 3.0 &&
		time_now - last_time_now < 0.1)
	{
		did_double = mContent->DoubleClickCell(cell_x,cell_y);
	}

	last_cell_x = cell_x;
	last_cell_y = cell_y;
	last_mouse_x = mouse_x;
	last_mouse_y = mouse_y;
	last_time_now = time_now;

	if (did_double) return 1;

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


	if (!mContent)
	{
		mClickCellX = -1;
		mClickCellY = -1;
		mEditInfo.content_type = gui_Cell_None;
		return 1;
	}

	if (!IsFocusedChain())
		TakeFocus();

	if (HasEdit())
	{
		TerminateEdit(true, false, true);	// ben says: mac finder will 'eat' the click that ends edits sometimes, but I find this annoying.
		mClickCellX = -1;
		mClickCellY = -1;
		mEditInfo.content_type = gui_Cell_None;
		return 1;
	}

	//Fill mEditInfo so we can make decisions based on the cell's
	//Abilities, Status, and Content
	mContent->GetCellContent(cell_x,cell_y,mEditInfo);

	mClickCellX = -1;
	mClickCellY = -1;

	cell_bounds[0] += (mEditInfo.indent_level * mCellIndent);

//	if (mouse_x < cell_bounds[0])	{ mEditInfo.content_type = gui_Cell_None; return 1; }

	if (mEditInfo.can_delete == true)
	{
		mTrackLeft = cell_bounds[0];
		mTrackRight = cell_bounds[0] + GUI_GetImageResourceWidth("delete.png");
		if (mouse_x >= mTrackLeft && mouse_x < mTrackRight)
		{
			//Set the TextTable to be thinking about this disclose handle
			mContent->DoDeleteCell(cell_x, cell_y);
			mClickCellX = cell_x;
			mClickCellY = cell_y;
			mInBounds = 1;
			
			return 1;
		}
	}

	if(mEditInfo.is_disclosed || mEditInfo.can_disclose)
	{
		//Regardless of state, if it is able to we're going to toggle
		//the button
		if (mEditInfo.can_disclose)
		{
			//Find the horizontal dimensions
			//(vertical is implied correct by this point)
			mTrackLeft = cell_bounds[0];
			mTrackRight = cell_bounds[0] + mDiscloseIndent;

			//If the mouse is within those dimensions
			if (mouse_x >= mTrackLeft && mouse_x < mTrackRight)
			{
				//Set the TextTable to be thinking about this disclose handle
				//we just clicked on
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
								(mModifiers & gui_ControlFlag) ? 1 : 0);
		mEditInfo.content_type = gui_Cell_None;
		BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);

		if (mEditInfo.can_drag && mParent->IsDragClick(mouse_x,mouse_y,button))
		{
			mContent->SelectionEnd();
			mContent->DoDrag(mParent, mouse_x,mouse_y,button,cell_bounds);
			return 0;
		}

		return 1;
	}

	if (mEditInfo.can_drag && mParent->IsDragClick(mouse_x,mouse_y,button))
	{
		mContent->DoDrag(mParent, mouse_x,mouse_y,button,cell_bounds);
		return 0;
	}

	if (mouse_x < cell_bounds[0])	{ mEditInfo.content_type = gui_Cell_None; return 1; }

	

	if (!mEditInfo.can_edit)	return 1;

	int	all_edit = mParent->GetModifiersNow() & (gui_OptionAltFlag | gui_ControlFlag);

	switch(mEditInfo.content_type) {
	case gui_Cell_FileText:
		{
			mContent->AcceptEdit(cell_x, cell_y, mEditInfo, all_edit);
			mEditInfo.content_type = gui_Cell_None;
		}
		break;
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
	case gui_Cell_TaxiText:
	case gui_Cell_Integer:
	case gui_Cell_Double:
		if (mParent)
		{
			cell_bounds[0] -= mEditInfo.indent_level * mCellIndent;	// clean out bounds...will get changed again later anyway
			CreateEdit(cell_bounds, mEditInfo.text_val, mEditInfo.content_type == gui_Cell_TaxiText);
			mClickCellX = cell_x;
			mClickCellY = cell_y;
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
				int cur = CreateMenuFromDict(items, enum_vals, dict);
				int choice = mParent->PopupMenuDynamic(&*items.begin(), cell_bounds[0],cell_bounds[3],button, cur);
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
				int cur = CreateMenuFromDict(items, enum_vals, dict);
				int choice = mParent->PopupMenuDynamic(&*items.begin(), cell_bounds[0],cell_bounds[3],button, cur);
				if (choice >= 0 && choice < enum_vals.size())
				{
					mEditInfo.int_val=enum_vals[choice];
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
	if (mCellResize >= 0 && mGeometry)
	{
		mouse_x = max(mouse_x, (mLastX - mGeometry->GetCellWidth(mCellResize) + MIN_CELL_WIDTH));
		mGeometry->SetCellWidth(mCellResize,mouse_x - mLastX + mGeometry->GetCellWidth(mCellResize));
		mLastX = mouse_x;
		BroadcastMessage(GUI_TABLE_SHAPE_RESIZED,0);
		return;
	}

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
	if (mCellResize >= 0 && mGeometry)
	{
		mouse_x = max(mouse_x, (mLastX - mGeometry->GetCellWidth(mCellResize) + MIN_CELL_WIDTH));
		mGeometry->SetCellWidth(mCellResize,mouse_x - mLastX + mGeometry->GetCellWidth(mCellResize));
		mCellResize = -1;
		BroadcastMessage(GUI_TABLE_SHAPE_RESIZED,0);
		return;
	}

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

	int	all_edit = mParent->GetModifiersNow() & (gui_OptionAltFlag | gui_ControlFlag);

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
	if (!HasEdit())
	{
		mEditInfo.content_type = gui_Cell_None;
		mClickCellX = -1;
		mClickCellY = -1;
	}
}

int			GUI_TextTable::CellGetCursor(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y)
{
	if (mGeometry && abs(mouse_x - cell_bounds[0]) < RESIZE_MARGIN && cell_x > 0)
	{
		return gui_Cursor_Resize_H;
	}
	if (mGeometry && abs(mouse_x - cell_bounds[2]) < RESIZE_MARGIN && cell_x < mGeometry->GetColCount())
	{
		return gui_Cursor_Resize_H;
	}
	return gui_Cursor_Arrow;
}

int			GUI_TextTable::CellGetHelpTip(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, string& tip								  )
{
	GUI_CellContent	c;
	if (!mContent) return 0;
	mContent->GetCellContent(cell_x,cell_y,c);
	switch(c.content_type) {
	case gui_Cell_EditText:
	case gui_Cell_TaxiText:
	case gui_Cell_FileText:
	case gui_Cell_Integer:
	case gui_Cell_Double:
	case gui_Cell_Enum:
	case gui_Cell_EnumSet:	tip = c.text_val;	return 1;
	default:									return 0;
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



void		GUI_TextTable::CreateEdit(int cell_bounds[4], const string& text, bool is_sign)
{
	if(is_sign)
	{
		if(!mSignField)
		{
			mSignField = new WED_Sign_Editor(this);
			if(mCatcher == NULL) 
			{
				mCatcher = new GUI_MouseCatcher(GUI_MOUSE_OUTSIDE_BOUNDS);
				mCatcher->AddListener(this);
			}
						
			GUI_Pane * parent = mParent;
			while(parent->GetParent())
				parent = parent->GetParent();
			mSignField->SetParent(mCatcher);
			mCatcher->SetParent(parent);
			
			int pb[4];
			parent->GetBounds(pb);
			mCatcher->SetBounds(pb);
			mCatcher->Show();
		}

		int cb[4];
		memcpy(cb,cell_bounds,sizeof(cb));
			
		int wb[4];
		mCatcher->GetBounds(wb);
						
		cb[0] += mEditInfo.indent_level * mCellIndent;
		
		cb[1] = cb[3] - 280;  // desired size for Sign Editor
		cb[2] = cb[0] + 600;
		
		int dx = 0, dy = 0;
		if(cb[2] > wb[2])
			dx = wb[2] - cb[2];
		if(cb[0] < wb[0])
			dx = wb[0] - cb[0];
		cb[0] += dx;
		cb[2] += dx;
		
		if(cb[1] < wb[1])
			dy = wb[1] - cb[1];
		if(cb[3] > wb[3])
			dy = wb[3] - cb[3];
		cb[1] += dy;
		cb[3] += dy;
		
		mSignField->SetBounds(cb);

		if(!mSignField->SetSignText(text))
		{
			is_sign = false;
			mSignField->Hide();
			delete mSignField;
			mSignField = NULL;
			mCatcher->Hide();
		}
		else
		{
			mCatcher->Show();
			mSignField->Show();
			mSignField->TakeFocus();
			mSignField->Refresh();			
		}
	}
	
	if(!is_sign)
	{
		if (!mTextField)
		{
			mTextField = new GUI_TextField(1,this);
			mTextField->SetFont(mFont);
			mTextField->SetKeyMsg(GUI_TEXT_FIELD_TEXT_CHANGED,0);
			mTextField->AddListener(this);
			mTextField->SetParent(mParent);
			mTextField->SetVKAllowed(GUI_VK_RETURN, false);
			mTextField->SetKeyAllowed(GUI_KEY_ESCAPE, false);
			mTextField->SetKeyAllowed(GUI_KEY_TAB, false);
			if (!mImage.empty())
			{
				float transparent[4] = { 0.0, 0.0, 0.0, 0.0 };
				mTextField->SetColors(mColorTextSelect, mColorSelect, transparent, transparent);
			} else {
				mTextField->SetColors(mTFColorText,mTFColorHilite,mTFColorBkgnd,mTFColorBox);
			}
		}

		float	cell_h = cell_bounds[3] - cell_bounds[1];
		float	line_h = GUI_GetLineHeight(mFont);
		int		descent = GUI_GetLineDescent(mFont);
		float	cell2line = (cell_h - line_h + descent) * 0.5f;

		float pad_bottom = cell2line - descent;
		float pad_top = cell_h - line_h - pad_bottom;

		mTextField->SetMargins(CELL_MARGIN,pad_bottom,CELL_MARGIN,pad_top);


		cell_bounds[0] += mEditInfo.indent_level * mCellIndent;
		mTextField->SetBounds(cell_bounds);
		mTextField->SetWidth(max(cell_bounds[2] - cell_bounds[0],2048));
	//	mTextField->SetWidth(1000);
		mTextField->Show();
		mTextField->TakeFocus();

		mParent->TrapFocus();	
		
		mTextField->SetDescriptor(text);
		mTextField->SetSelection(0,text.size());
		mTextField->Refresh();
	}
}

int			GUI_TextTable::TerminateEdit(bool inSave, bool in_all, bool in_close)
{
	GUI_Commander * cmd_field = mTextField ? (GUI_Commander *) mTextField : (GUI_Commander *) mSignField;
	
	if (cmd_field && cmd_field->IsFocused() &&
		(mEditInfo.content_type == gui_Cell_EditText || mEditInfo.content_type == gui_Cell_TaxiText ||  mEditInfo.content_type == gui_Cell_Integer || mEditInfo.content_type == gui_Cell_Double))
	{

		// This is a bit tricky: _if_ we are going to kill off the text field later, memorize the field and mark our member var
		// as null now.  The reason: sometimes the call to our content's AcceptEdit below in the in_save block can cause a message like
		// "size changed" which in turn terminates editing anyway.  If this happens then our text field is removed out from under us and
		// we will later die when trying to kill it off.
		//
		// Sooo...null out the field early - the callback will assume we are not editing, which is more or less true by the time we exit this func.
		// If we are in continuous edit and the field is nuked the in_close block wouldn't be called anyway.
		//
		// This is, at best, hokey...maybe revisit someday?
		GUI_TextField * f = mTextField;
		WED_Sign_Editor * s = mSignField;
		if(in_close) {
			mTextField = NULL;
			mSignField = NULL;
		}
		DebugAssert(f != NULL || s != NULL);	
			
		if (inSave)
		{
			if(f)
				f->GetDescriptor(mEditInfo.text_val);
			else	
				s->GetSignText(mEditInfo.text_val);
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
		if(in_close)
		{
			mTextField = NULL;
			mSignField = NULL;
			this->TakeFocus();
			if(f)
			{
				f->Hide();
				delete f;
			}
			if(s)
			{
				s->Hide();
				delete s;
				mCatcher->Hide();
			}
			mEditInfo.content_type = gui_Cell_None;
			return 1;
		}
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

int			GUI_TextTable::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (!mContent) return 0;

	if ((inFlags & gui_UpFlag) == 0 && inVK == GUI_VK_LEFT)
	if (mContent->SelectDisclose(0, inFlags & gui_OptionAltFlag))	return 1;

	if ((inFlags & gui_UpFlag) == 0 && inVK == GUI_VK_RIGHT)
	if (mContent->SelectDisclose(1, inFlags & gui_OptionAltFlag))	return 1;


	if ((inFlags & gui_UpFlag) == 0 && (inVK == GUI_VK_PRIOR || inVK == GUI_VK_NEXT || inVK == GUI_VK_END || inVK == GUI_VK_HOME))
	{
		float tb[4], vb[4];
		mParent->GetScrollBounds(tb,vb);
		float v = mParent->GetScrollV();
		float d = vb[3] - vb[1];
		float m = tb[3] - tb[1] - d;
		if (inVK == GUI_VK_PRIOR) v += d;	// "page up is positive - y aaxis.
		if (inVK == GUI_VK_NEXT ) v -= d;
		if (inVK == GUI_VK_HOME ) v = m;
		if (inVK == GUI_VK_END ) v = 0;
		if (v < 0) v = 0;
		if (v > m) v = m;
		mParent->ScrollV(v);
		mParent->BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
		mParent->Refresh();
	}

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
					case GUI_KEY_LEFT:		--x1;	if ((inFlags & gui_ShiftFlag) == 0)	x2 = x1;break;
					case GUI_KEY_RIGHT:		++x2;	if ((inFlags & gui_ShiftFlag) == 0)	x1 = x2;break;
					case GUI_KEY_UP:		++y2;	if ((inFlags & gui_ShiftFlag) == 0)	y1 = y2;break;
					case GUI_KEY_DOWN:		--y1;	if ((inFlags & gui_ShiftFlag) == 0)	y2 = y1;break;
					}

					x1 = min(x1, x_max);
					x2 = min(x2, x_max);
					x1 = max(x1, x_min);
					x2 = max(x2, x_min);
					y1 = min(y1, y_max);
					y2 = min(y2, y_max);
					y1 = max(y1, y_min);
					y2 = max(y2, y_min);

					if(mParent)
					switch(inKey) {
					case GUI_KEY_LEFT:		mParent->RevealCol(x1);	break;
					case GUI_KEY_RIGHT:		mParent->RevealCol(x2);	break;
					case GUI_KEY_UP:		mParent->RevealRow(y2);	break;
					case GUI_KEY_DOWN:		mParent->RevealRow(y1);	break;
					}


					mContent->SelectionStart(1);
					mContent->SelectRange(x1,y1,x2,y2, 0);
				}
				mContent->SelectionEnd();
			}
		}
		break;
	}

	if(inKey == GUI_KEY_TAB && HasEdit() && mContent)
	{
		int x = mClickCellX;
		int y = mClickCellY;
		int cell_bounds[4];
		TerminateEdit(true, inFlags & (gui_OptionAltFlag | gui_ControlFlag), true);
		if (mParent)
		{
			if (mContent->TabAdvance(x,y, (inFlags & gui_ShiftFlag) ? -1 : 1, mEditInfo))
			{
				mParent->RevealCell(x,y);
				mParent->CalcCellBounds(x,y,cell_bounds);
				mClickCellX = x;
				mClickCellY = y;
				CreateEdit(cell_bounds, mEditInfo.text_val,mEditInfo.content_type == gui_Cell_TaxiText);
			}
		}
	}

	if(inVK == GUI_VK_RETURN && HasEdit())
	{
		TerminateEdit(true, inFlags & (gui_OptionAltFlag | gui_ControlFlag), true);
		return 1;
	}
	else if (inVK == GUI_VK_RETURN && mContent && mParent)
	{
		int x1, y1, x2, y2;
		if (mContent->SelectGetExtent(x1,y1,x2,y2))
		{
			int cell_bounds[4];
			if (mContent->TabAdvance(x1,y2, 0, mEditInfo))
			{
				mParent->RevealCell(x1,y2);
				mParent->CalcCellBounds(x1,y2,cell_bounds);
				mClickCellX = x1;
				mClickCellY = y2;
				CreateEdit(cell_bounds, mEditInfo.text_val,mEditInfo.content_type == gui_Cell_TaxiText);
			}
		}
	}

	if(inKey == GUI_KEY_ESCAPE && HasEdit())
	{
		TerminateEdit(false, false, true);
		return 1;
	}
	return 0;
}

void	GUI_TextTable::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t    			inMsg,
							intptr_t				inParam)
{
	if(inMsg == GUI_TEXT_FIELD_TEXT_CHANGED && mLiveEdit)
		TerminateEdit(true,false,false);
	if(inMsg == GUI_MOUSE_OUTSIDE_BOUNDS)
	{
		TerminateEdit(true, false, true);
	}
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
	GUI_HeaderContent	c;
	if(mContent)
		mContent->GetHeaderContent(cell_x,c);
	else
		c.is_selected = 0;


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
		int tile[4] = { 0, c.is_selected, 1, 2 };
		GUI_DrawHorizontalStretch(inState, mImage.c_str(), cell_bounds, tile);
	}

	if (!mContent) return;

	int line_h = (cell_bounds[1] + cell_bounds[3]) / 2 - GUI_GetLineAscent(font_UI_Basic) / 2;


	int trunc_width = cell_bounds[2] - cell_bounds[0] - (CELL_MARGIN*2);
	GUI_TruncateText(c.title, font_UI_Basic, trunc_width);
	GUI_FontDraw(inState, font_UI_Basic, mColorText, cell_bounds[0]+CELL_MARGIN, line_h, c.title.c_str());
}

int			GUI_TextTableHeader::HeadMouseDown(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock)
{
	if (!mContent) return 0;

	GUI_HeaderContent	c;
	mContent->GetHeaderContent(cell_x,c);

	want_lock = 1;
	mCellResize = -1;
	if (c.can_resize && mGeometry && abs(mouse_x - cell_bounds[0]) < RESIZE_MARGIN && cell_x > 0)
	{
		mLastX = mouse_x;
		mCellResize = cell_x -1;
		return 1;
	}
	if (c.can_resize && mGeometry && abs(mouse_x - cell_bounds[2]) < RESIZE_MARGIN && cell_x < mGeometry->GetColCount())
	{
		mLastX = mouse_x;
		mCellResize = cell_x;
		return 1;
	}
	
	if(c.can_select)
	{
		mContent->SelectHeaderCell(cell_x);
		return 1;
	}

	return 1;
}

void		GUI_TextTableHeader::HeadMouseDrag(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)
{
	if (mCellResize >= 0 && mGeometry)
	{
		mouse_x = max(mouse_x, (mLastX - mGeometry->GetCellWidth(mCellResize) + MIN_CELL_WIDTH));
		mGeometry->SetCellWidth(mCellResize,mouse_x - mLastX + mGeometry->GetCellWidth(mCellResize));
		mLastX = mouse_x;
		BroadcastMessage(GUI_TABLE_SHAPE_RESIZED,0);
	}
}

void		GUI_TextTableHeader::HeadMouseUp  (int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)
{
	if (mCellResize >= 0 && mGeometry)
	{
		mouse_x = max(mouse_x, (mLastX - mGeometry->GetCellWidth(mCellResize) + MIN_CELL_WIDTH));
		mGeometry->SetCellWidth(mCellResize,mouse_x - mLastX + mGeometry->GetCellWidth(mCellResize));
		mCellResize = -1;
		BroadcastMessage(GUI_TABLE_SHAPE_RESIZED,0);
	}
}

int			GUI_TextTableHeader::HeadGetCursor(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y)
{
	if (mGeometry && abs(mouse_x - cell_bounds[0]) < RESIZE_MARGIN && cell_x > 0)
	{
		return gui_Cursor_Resize_H;
	}
	if (mGeometry && abs(mouse_x - cell_bounds[2]) < RESIZE_MARGIN && cell_x < mGeometry->GetColCount())
	{
		return gui_Cursor_Resize_H;
	}
	return gui_Cursor_Arrow;
}

int			GUI_TextTableHeader::HeadGetHelpTip(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, string& tip)
{
	if (!mContent) return 0;
	GUI_HeaderContent	c;
	mContent->GetHeaderContent(cell_x,c);
	tip = c.title;
	return 1;
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

void		GUI_TextTableSide::SetImage(const char * image)
{
	mImage = image;
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
		GUI_DrawHorizontalStretch(inState, mImage.c_str(), cell_bounds, tile);
	}

	if (!mContent) return;
	GUI_HeaderContent	c;
	mContent->GetHeaderContent(cell_y,c);

	int line_h = (cell_bounds[1] + cell_bounds[3]) / 2 - GUI_GetLineAscent(font_UI_Basic) / 2;
	int trunc_width = cell_bounds[2] - cell_bounds[0] - (CELL_MARGIN*2);
	GUI_TruncateText(c.title, font_UI_Basic, trunc_width);
	GUI_FontDraw(inState, font_UI_Basic, mColorText, cell_bounds[0]+CELL_MARGIN, line_h, c.title.c_str());
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

int			GUI_TextTableSide::SideGetCursor(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y)
{
	return gui_Cursor_Arrow;
}

int			GUI_TextTableSide::SideGetHelpTip(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, string& tip								  )
{
	if (!mContent) return 0;
	GUI_HeaderContent	c;
	mContent->GetHeaderContent(cell_y,c);
	tip = c.title;
	return 1;
}
