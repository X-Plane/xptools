#include "GUI_ToolBar.h"
#include "GUI_Messages.h"
#include "GUI_DrawUtils.h"

GUI_ToolBar::GUI_ToolBar(int h, int v, const char * in_resource) :
	GUI_Control(),
	mResource(in_resource ? in_resource : string()),
	mH(h),
	mV(v)
{
}
	
GUI_ToolBar::~GUI_ToolBar()
{	
}

void	GUI_ToolBar::SetValue(float inValue)
{
	GUI_Control::SetValue(inValue);
	Refresh();
}

void		GUI_ToolBar::Draw(GUI_GraphState * state)
{
	int bounds[6];
	GetBounds(bounds);
	bounds[4] = bounds[2] - bounds[0];
	bounds[5] = bounds[3] - bounds[1];
	
	int all_sel[4] = { 0, 0, 1, 2 };
	
	GUI_DrawStretched(state, mResource.c_str(), bounds, all_sel);
	
	int v = GetValue();
	
	if (v >= 0 && v < (mH * mV))
	{
		int x = v % mH;
		int y = v / mH;
		
		int hilite_sel[4] = { x, y + mV, mH, mV * 2 };
		
		int sub_bounds[4] = { 
			((float) (hilite_sel[0]  ) * (float) bounds[4] / (float) hilite_sel[2])+bounds[0],
			((float) (hilite_sel[1]  ) * (float) bounds[5] / (float) hilite_sel[3])+bounds[1],
			((float) (hilite_sel[0]+1) * (float) bounds[4] / (float) hilite_sel[2])+bounds[0],
			((float) (hilite_sel[1]+1) * (float) bounds[5] / (float) hilite_sel[3])+bounds[1]
		};
			
		GUI_DrawStretched(state, mResource.c_str(), sub_bounds, hilite_sel);
	}
}


int			GUI_ToolBar::MouseDown(int x, int y, int button) 
{
	int bounds[6];
	GetBounds(bounds);
	bounds[4] = bounds[2] - bounds[0];
	bounds[5] = bounds[3] - bounds[1];
	bounds[4] /= mH;
	bounds[5] /= mV;
		
	int xt = (x - bounds[0]) / bounds[4];
	int yt = (y - bounds[1]) / bounds[5];
	
	if (xt >= 0 && xt < mH && yt >= 0 && yt < mV)
	{
		SetValue(xt + yt * mH);
		BroadcastMessage(GUI_CONTROL_VALUE_CHANGED, 0);
		Refresh();
	}
	return 1;
}
	
void		GUI_ToolBar::MouseDrag(int x, int y, int button) { 			}
void		GUI_ToolBar::MouseUp  (int x, int y, int button) { 			}

