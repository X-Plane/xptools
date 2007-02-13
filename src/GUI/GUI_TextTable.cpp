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
	all editing!
	graphical display of check boxes
	numeric precision control?
	provider to provide content IN BULK?
#endif

GUI_TextTable::GUI_TextTable(GUI_Commander * parent) : GUI_Commander(parent),
	mContent(NULL),
	mClickCellX(-1),
	mClickCellY(-1),
	mParent(NULL),
	mTextField(NULL)
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
	if (mContent)		mContent->RemoveListener(this);
						mContent = content;
	if (mContent)		mContent->AddListener(this);	
}

void		GUI_TextTable::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	BroadcastMessage(inMsg, inParam);
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
		glColor3f(1,0,0);
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

int			GUI_TextTable::CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
	if (!mContent) return 0;
	mContent->GetCellContent(cell_x,cell_y,mEditInfo);
	if (mTextField) TerminateEdit();	// ben says: mac finder will 'eat' the click that ends edits sometimes, but I find this annoying.
	
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
				mInBounds = 0;
				BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
				return 1;
			}
		}
		cell_bounds[0] += 10;		
	}
	
	if (!mEditInfo.is_selected && mEditInfo.can_select)
	{
		#if !DEV
			drag and drop here?
		#endif
		mContent->SelectCell(cell_x, cell_y);
		mEditInfo.content_type = gui_Cell_None;
		return 1;
	}	
	
	char  buf[256];
	switch(mEditInfo.content_type) {
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
			if (!mTextField) { mTextField = new GUI_TextField(1,this); mTextField->SetParent(mParent); }
			mTextField->SetBounds(cell_bounds);
			mTextField->Show();
			mTextField->TakeFocus();
			mClickCellX = cell_x;
			mClickCellY = cell_y;
			mTextField->SetDescriptor(mEditInfo.text_val);
			return 1;
		}
		break;
	}
	return 0;
}

void		GUI_TextTable::CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
	int new_in;
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
	}
}

void		GUI_TextTable::CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
	int new_in;
	switch(mEditInfo.content_type) {
	case gui_Cell_Disclose:
		mInBounds = (mouse_x >= mTrackLeft && mouse_x < mTrackRight &&
				  mouse_y >= cell_bounds[1] && mouse_y <= cell_bounds[3]);
		if (mInBounds) mContent->ToggleDisclose(cell_x,cell_y);
//		BroadcastMessage(GUI_TABLE_CONTENT_CHANGED, 0);
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

int			GUI_TextTable::TerminateEdit(void)
{
	if (mTextField && mTextField->IsFocused() && 
		(mEditInfo.content_type == gui_Cell_EditText ||  mEditInfo.content_type == gui_Cell_Integer || mEditInfo.content_type == gui_Cell_Double))
	{
		mTextField->GetDescriptor(mEditInfo.text_val);
		switch(mEditInfo.content_type) {
		case gui_Cell_Integer:
			mEditInfo.int_val = atoi(mEditInfo.text_val.c_str());
			break;
		case gui_Cell_Double:
			mEditInfo.int_val = atof(mEditInfo.text_val.c_str());
			break;
		}
		mContent->AcceptEdit(mClickCellX, mClickCellY, mEditInfo);	
		this->TakeFocus();
		mTextField->Hide();
		delete mTextField;
		mTextField = NULL;
		mEditInfo.content_type = gui_Cell_None;
		return 1;				
	}
	return 0;
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
	if (mContent)	mContent->RemoveListener(this);
	mContent = content;
	if (mContent)	mContent->AddListener(this);
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

int			GUI_TextTableHeader::HeadMouseDown(int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)
{	
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
	}
}

void		GUI_TextTableHeader::HeadMouseUp  (int cell_bounds[4], int cell_x, int mouse_x, int mouse_y, int button)
{
	if (mCellResize >= 0 && mGeometry)
	{
		mGeometry->SetCellWidth(mCellResize,mouse_x - mLastX + mGeometry->GetCellWidth(mCellResize));
		mCellResize = -1;
	}
}

void		GUI_TextTableHeader::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	BroadcastMessage(inMsg, inParam);
}

