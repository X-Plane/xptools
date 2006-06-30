#include "WED_MapPane.h"

WED_MapPane::WED_MapPane()
{
}

WED_MapPane::~WED_MapPane()
{
}

void		WED_MapPane::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1,y1,x2,y2);
	SetPixelBounds(x1,y1,x2,y2);
}

void		WED_MapPane::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	SetPixelBounds(inBounds[0],inBounds[1],inBounds[2],inBounds[3]);
}

void		WED_MapPane::Draw(GUI_GraphState * state)
{
}

int			WED_MapPane::MouseDown(int x, int y, int button)
{
	return 0;
}

void		WED_MapPane::MouseDrag(int x, int y, int button)
{
}

void		WED_MapPane::MouseUp  (int x, int y, int button)
{
}

int			WED_MapPane::ScrollWheel(int x, int y, int dist, int axis)
{
	return 0;
}
