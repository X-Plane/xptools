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


GUI_Button::GUI_Button(
		const char *		in_res_name,
		GUI_ButtonType		behavior,
		int					off_regular[4],
		int					off_hilite[4],
		int					on_regular[4],
		int					on_hilite[4]) :
	mBehavior(behavior),
	mResource(in_res_name),
	mHilite(0)
{
	for (int n = 0; n < 4; ++n)
	{
		mCellOffReg[n] = off_regular[n];
		mCellOffHilite[n] = off_hilite[n];
		mCellOnReg[n] = on_regular[n];
		mCellOnHilite[n] = on_hilite[n];
	}
}

GUI_Button::~GUI_Button()
{
}

void		GUI_Button::SetValue(float inValue)
{
	GUI_Control::SetValue(inValue);
	Refresh();
}

int		GUI_Button::MouseMove(int x, int y)
{
	if (mBehavior == btn_Web)
	{
		mHilite = 1;
		Refresh();
	}
	return 1;
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
		switch(mBehavior) {
		case btn_Push:
		case btn_Web:
			SetValue(0.0);
			break;
		case btn_Check:
			SetValue(1.0 - GetValue());
			break;
		case btn_Radio:
			SetValue(1.0);
			break;
		}
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
	int * tile_p = GetValue() > 0 ? 
					(mHilite ? mCellOnHilite : mCellOnReg ) :
					(mHilite ? mCellOffHilite : mCellOffReg );
	switch(mBehavior) {
	case btn_Push:
	case btn_Web:
		{
			GUI_DrawStretched(state,mResource.c_str(),bounds,tile_p);
			
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
			GUI_DrawCentered(state, mResource.c_str(), bounds, -1, 0, tile_p, &w, NULL);
			
			if (!desc.empty())
				GUI_FontDraw(state, font_UI_Basic, c, bounds[0] + w, bounds[1], desc.c_str());
		}
		break;
	}
}

void		GUI_Button::SetHilite(int hilite)
{
	if (mHilite != hilite)
	{
		mHilite = hilite;
		Refresh();
	}
}
