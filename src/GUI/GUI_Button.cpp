#include "GUI_Button.h"
#include "GUI_GraphState.h"
#include "GUI_Messages.h"
#include "GUI_DrawUtils.h"
#include "GUI_Fonts.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif


GUI_Button::GUI_Button() : mStyle(btn_Push), mHilite(0)
{
}

GUI_Button::~GUI_Button()
{
}

void		GUI_Button::SetStyle(int style)
{
	mStyle = style; 
	Refresh();
}

void		GUI_Button::SetValue(float inValue)
{
	GUI_Control::SetValue(inValue);
	Refresh();
}

int			GUI_Button::MouseDown(int x, int y, int button)
{
	mHilite = 1;
	Refresh();
	return 1;
}

void		GUI_Button::MouseDrag(int x, int y, int button)
{
	int b[4];
	GetBounds(b);
	int new_hilite = (x >= b[0] && x <= b[2] && y >= b[1] && y <= b[3]);
	if (new_hilite != mHilite)
	{
		mHilite = new_hilite;
		Refresh();
	}
}

void		GUI_Button::MouseUp  (int x, int y, int button)
{
	int b[4];
	GetBounds(b);
	int new_hilite = (x >= b[0] && x <= b[2] && y >= b[1] && y <= b[3]);
	if (new_hilite)
	{
		switch(mStyle) {
		case btn_Push:
			SetValue(0.0);
			break;
		case btn_Check:
			SetValue(1.0 - GetValue());
			break;
		case btn_Radio:
			SetValue(1.0);
			break;
		}
		BroadcastMessage(GUI_CONTROL_VALUE_CHANGED, 0);
	}
	
	mHilite = 0;
	Refresh();
}

void		GUI_Button::Draw(GUI_GraphState * state)
{
	float c[4] = { 0,0,0,1 };
	int w;
	int bounds[4];
	string desc;
	glColor3f(1,1,1);
	GetDescriptor(desc);
	GetBounds(bounds);
	switch(mStyle) {
	case btn_Push:
		{
			int tile_p[4] = { 0, mHilite ? 0 : 1, 1, 2 };
			GUI_DrawStretched(state,"pushbutton.png",bounds,tile_p);
			
			if (!desc.empty())
			{
				w = GUI_MeasureRange(font_UI_Basic, &*desc.begin(), &*desc.end());
				w = (bounds[2] - bounds[0] - w) / 2;
				GUI_FontDraw(state, font_UI_Basic, c, bounds[0] + w, bounds[1], desc.c_str());
			}
		}
		break;
	case btn_Check:
	case btn_Radio:
		{
			int tile_c[4] = { mHilite ? 1 : 0, GetValue() > 0 ? 1 : 0, 2, 2 };
			GUI_DrawCentered(state,(mStyle == btn_Check) ? "checkbutton.png" : "radiobutton.png", bounds, -1, 0, tile_c, &w, NULL);
			
			if (!desc.empty())
				GUI_FontDraw(state, font_UI_Basic, c, bounds[0] + w, bounds[1], desc.c_str());
		}
		break;
	}
}
