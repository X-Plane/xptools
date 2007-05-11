#include "GUI_TextTable.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "GUI_Messages.h"
#include <math.h>
#include "GUI_TextField.h"

#define RESIZE_MARGIN 4

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#if !DEV
	todo
	editing: popups, check boxes
	graphical display of check boxes
	numeric precision control?
	provider to provide content IN BULK?
#endif

GUI_TextTable::GUI_TextTable(GUI_Commander * parent) : GUI_Commander(parent),
	mContent(NULL),
	mClickCellX(-1),
	mClickCellY(-1),
	mParent(NULL),
	mTextField(NULL),
	mSelStartX(-1),
	mSelStartY(-1)
{
	mEditInfo.content_type = gui_Cell_None;
}

GUI_TextTable::~GUI_TextTable()
{
}

void		GUI_TextTable::SetParentPanes(GUI_Pane * parent)
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
	glColor3f(0.5,0.5,0.5);
	glBegin(GL_LINE_LOOP);
	glVertex2i(cell_bounds[0],cell_bounds[1]);
	glVertex2i(cell_bounds[2],cell_bounds[1]);
	glVertex2i(cell_bounds[2],cell_bounds[3]);
	glVertex2i(cell_bounds[0],cell_bounds[3]);
	glEnd();	
	
	if (!mContent) return;
	GUI_CellContent	c;
	mContent->GetCellContent(cell_x,cell_y,c);

	if (c.is_selected)
	{
		glColor3f(1,1,0);
		glBegin(GL_QUADS);
		glVertex2i(cell_bounds[0]+1,cell_bounds[1]+1);
		glVertex2i(cell_bounds[0]+1,cell_bounds[3]-1);
		glVertex2i(cell_bounds[2]-1,cell_bounds[3]-1);
		glVertex2i(cell_bounds[2]-1,cell_bounds[1]+1);
		glEnd();			
		glColor3f(0.5,0.5,0.5);
	}
	
	cell_bounds[0] += (c.indent_level * 10);
	
	char buf[50];
	switch(c.content_type) {
	case gui_Cell_Disclose:
		c.text_val.clear();
		break;
	case gui_Cell_CheckBox:
		if (cell_x == mClickCellX && cell_y == mClickCellY && mEditInfo.content_type == gui_Cell_CheckBox && mInBounds)
			sprintf(buf,"%c",c.int_val ? 'O' : 'o');
		else		
			sprintf(buf,"%c",c.int_val ? 'X' : '.');
		c.text_val = buf;
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
		
		if (cell_x == mClickCellX && cell_y == mClickCellY && mEditInfo.content_type == gui_Cell_Disclose && mInBounds)
		glBegin(GL_TRIANGLES);
		else
		glBegin(GL_LINE_LOOP);
		if (c.is_disclosed)
		{
			glVertex2i(cell_bounds[0] + 2, middle + 4); 
			glVertex2i(cell_bounds[0] + 10, middle + 4); 
			glVertex2i(cell_bounds[0] + 6, middle - 4); 
		}
		else
		{
			glVertex2i(cell_bounds[0] + 2, middle + 4); 
			glVertex2i(cell_bounds[0] + 6, middle); 
			glVertex2i(cell_bounds[0] + 2, middle - 4); 
		}
		glEnd();
		cell_bounds[0] += 10;
	}
	
	float col[4] = { 0.0,0.0,0.0,1.0 };
	GUI_FontDraw(inState, font_UI_Basic, col, cell_bounds[0], cell_bounds[1], c.text_val.c_str());	
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
		TerminateEdit(true);	// ben says: mac finder will 'eat' the click that ends edits sometimes, but I find this annoying.		
		mClickCellX = -1;
		mClickCellY = -1;	
		mEditInfo.content_type = gui_Cell_None;
		return 1;
	}

	mContent->GetCellContent(cell_x,cell_y,mEditInfo);

	mClickCellX = -1;
	mClickCellY = -1;	
	
	cell_bounds[0] += (mEditInfo.indent_level * 10);
	
	if (mouse_x < cell_bounds[0])	{ mEditInfo.content_type == gui_Cell_None; return 1; }
	
	if(mEditInfo.is_disclosed || mEditInfo.can_disclose)
	{
		if (mEditInfo.can_disclose)
		{
			mTrackLeft = cell_bounds[0];
			mTrackRight = cell_bounds[0] + 10;

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
		cell_bounds[0] += 10;		
	}
	
	if ((!mEditInfo.is_selected || (mModifiers & (gui_ShiftFlag+gui_ControlFlag))) && mEditInfo.can_select)
	{
		#if !DEV
			drag and drop here?
		#endif
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
		return 1;
	}	
	
	if (!mEditInfo.can_edit)	return 1;
	
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
			if (!mTextField) 
			{
				mTextField = new GUI_TextField(1,this); 
				mTextField->SetParent(mParent); 
				mTextField->SetKeyAllowed(GUI_KEY_RETURN, false); 
				mTextField->SetKeyAllowed(GUI_KEY_ESCAPE, false); 
				mTextField->SetKeyAllowed(GUI_KEY_TAB, false); 
			}
			mTextField->SetBounds(cell_bounds);
			mTextField->Show();
			mTextField->TakeFocus();
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
					mContent->AcceptEdit(cell_x, cell_y, mEditInfo);
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
					mContent->AcceptEdit(cell_x, cell_y, mEditInfo);
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
			
			GUI_Table * p = dynamic_cast<GUI_Table *>(mParent);
			if (p) p->RevealCell(cell_x, cell_y);
		
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

			GUI_Table * p = dynamic_cast<GUI_Table *>(mParent);
			if (p) p->RevealCell(cell_x, cell_y);

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
			mContent->AcceptEdit(cell_x,cell_y, mEditInfo);
			BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
		}	
		break;
	}
	if (!mTextField)
	{
		mEditInfo.content_type == gui_Cell_None;
		mClickCellX = -1;
		mClickCellY = -1;	
	}
}

void		GUI_TextTable::Deactivate(void)
{
}

int			GUI_TextTable::TerminateEdit(bool inSave)
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
			mContent->AcceptEdit(mClickCellX, mClickCellY, mEditInfo);	
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
				GUI_Table * p = dynamic_cast<GUI_Table *>(mParent);
									
			
				if (!mContent->SelectGetExtent(x1,y1,x2,y2))
				{
					mContent->SelectionStart(0);
					switch(inKey) {
					case GUI_KEY_LEFT:		mContent->SelectRange(x_max,y_max,x_max,y_max,0);	if(p)p->RevealCol(x_max);	break;
					case GUI_KEY_RIGHT:		mContent->SelectRange(0,0,0,0,0);					if(p)p->RevealCol(0);		break;
					case GUI_KEY_UP:		mContent->SelectRange(x_max,y_max,x_max,y_max,0);	if(p)p->RevealRow(y_max);	break;
					case GUI_KEY_DOWN:		mContent->SelectRange(0,0,0,0,0);					if(p)p->RevealRow(0);		break;
					}

				} else {
				
					switch(inKey) {
					case GUI_KEY_LEFT:		--x1;	if ((inFlags & gui_ShiftFlag) == 0)	x2 = x1;	if(p)p->RevealCol(x1);	break;
					case GUI_KEY_RIGHT:		++x2;	if ((inFlags & gui_ShiftFlag) == 0)	x1 = x2;	if(p)p->RevealCol(x2);	break;
					case GUI_KEY_UP:		++y2;	if ((inFlags & gui_ShiftFlag) == 0)	y1 = y2;	if(p)p->RevealRow(y2);	break;
					case GUI_KEY_DOWN:		--y1;	if ((inFlags & gui_ShiftFlag) == 0)	y2 = y1;	if(p)p->RevealRow(y1);	break;
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
		TerminateEdit(true);
		if (mParent)
		{
			#if !DEV
				this is an enormous @#$@#ing hack.  The problem is that table content gets its geometry fed in
				and cannot calc this itself.
			#endif
			GUI_Table * p = dynamic_cast<GUI_Table *>(mParent);
			if (p)
			if (mContent->TabAdvance(x,y, inFlags & gui_ShiftFlag, mEditInfo))	
			{
				p->RevealCell(x,y);
				p->CalcCellBounds(x,y,cell_bounds);
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
				if (!mTextField) 
				{
					mTextField = new GUI_TextField(1,this); 
					mTextField->SetParent(mParent); 
					mTextField->SetKeyAllowed(GUI_KEY_RETURN, false); 
					mTextField->SetKeyAllowed(GUI_KEY_ESCAPE, false); 
					mTextField->SetKeyAllowed(GUI_KEY_TAB, false); 
				}
				mTextField->SetBounds(cell_bounds);
				mTextField->Show();
				mTextField->TakeFocus();
				mTextField->SetDescriptor(mEditInfo.text_val);
				mTextField->SetSelection(0,mEditInfo.text_val.size());
				mTextField->Refresh();
			}
		}
	}

	if(inKey == GUI_KEY_RETURN && mTextField)
	{
		TerminateEdit(true);
		return 1;
	}

	if(inKey == GUI_KEY_ESCAPE && mTextField)
	{
		TerminateEdit(false);
		return 1;
	}
}



//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

GUI_TextTableHeader::GUI_TextTableHeader() : mContent(NULL), mGeometry(NULL)
{	
}

GUI_TextTableHeader::~GUI_TextTableHeader()
{
}
	
void		GUI_TextTableHeader::SetProvider(GUI_TextTableHeaderProvider * content)
{
	mContent = content;
}

void		GUI_TextTableHeader::SetGeometry(GUI_TableGeometry * geometry)
{
	mGeometry = geometry;
}

void		GUI_TextTableHeader::HeadDraw	 (int cell_bounds[4], int cell_x, GUI_GraphState * inState			  )
{
	inState->SetState(false, false, false,	false, false, false, false);
	glColor3f(0.5,0.5,0.5);
	glBegin(GL_LINE_LOOP);
	glVertex2i(cell_bounds[0],cell_bounds[1]);
	glVertex2i(cell_bounds[2],cell_bounds[1]);
	glVertex2i(cell_bounds[2],cell_bounds[3]);
	glVertex2i(cell_bounds[0],cell_bounds[3]);
	glEnd();
	
	if (!mContent) return;
	GUI_HeaderContent	c;
	mContent->GetHeaderContent(cell_x,c);
	
	float col[4] = { 0.0,0.0,0.0,1.0 };
	GUI_FontDraw(inState, font_UI_Basic, col, cell_bounds[0], cell_bounds[1], c.title.c_str());	
}

int			GUI_TextTableHeader::HeadMouseDown(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock)
{	
	want_lock = 1;
	mCellResize = -1;
	if (mGeometry && fabs(mouse_x - cell_bounds[0]) < RESIZE_MARGIN && cell_x > 0)
	{
		mLastX = mouse_x;
		mCellResize = cell_x -1;
		return 1;
	}
	if (mGeometry && fabs(mouse_x - cell_bounds[2]) < RESIZE_MARGIN && cell_x < mGeometry->GetColCount())
	{
		mLastX = mouse_x;
		mCellResize = cell_x;
		return 1;
	}
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

void		GUI_TextTableSide::SideDraw	 (int cell_bounds[4], int cell_y, GUI_GraphState * inState			  )
{
	inState->SetState(false, false, false,	false, false, false, false);
	glColor3f(0.5,0.5,0.5);
	glBegin(GL_LINE_LOOP);
	glVertex2i(cell_bounds[0],cell_bounds[1]);
	glVertex2i(cell_bounds[2],cell_bounds[1]);
	glVertex2i(cell_bounds[2],cell_bounds[3]);
	glVertex2i(cell_bounds[0],cell_bounds[3]);
	glEnd();
	
	if (!mContent) return;
	GUI_HeaderContent	c;
	mContent->GetHeaderContent(cell_y,c);
	
	float col[4] = { 0.0,0.0,0.0,1.0 };
	GUI_FontDraw(inState, font_UI_Basic, col, cell_bounds[0], cell_bounds[1], c.title.c_str());	
}

int			GUI_TextTableSide::SideMouseDown(int cell_bounds[4], int cell_y, int mouse_x, int mouse_y, int button, GUI_KeyFlags flags, int& want_lock)
{
	want_lock = 1;
	mCellResize = -1;
	if (mGeometry && fabs(mouse_y - cell_bounds[1]) < RESIZE_MARGIN && cell_y > 0)
	{
		mLastY = mouse_y;
		mCellResize = cell_y -1;
		return 1;
	}
	if (mGeometry && fabs(mouse_y - cell_bounds[2]) < RESIZE_MARGIN && cell_y < mGeometry->GetRowCount())
	{
		mLastY = mouse_y;
		mCellResize = cell_y;
		return 1;
	}
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

