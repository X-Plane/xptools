#include "GUI_TextTable.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"

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

GUI_TextTable::GUI_TextTable() :
	mContent(NULL)
{
}

GUI_TextTable::~GUI_TextTable()
{
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
	
	char buf[50];
	switch(c.content_type) {
	case gui_Cell_CheckBox:
	case gui_Cell_Integer:
		sprintf(buf,"%d",c.int_val);
		c.text_val = buf;
		break;
	case gui_Cell_Double:
		sprintf(buf,"%lf",c.double_val);
		c.text_val = buf;
		break;
	}
	
	float col[4] = { 0.0,0.0,0.0,1.0 };
	GUI_FontDraw(inState, font_UI_Basic, col, cell_bounds[0], cell_bounds[1], c.text_val.c_str());	
}

int			GUI_TextTable::CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
	return 0;
}

void		GUI_TextTable::CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
}

void		GUI_TextTable::CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
}

void		GUI_TextTable::Deactivate(void)
{
}

