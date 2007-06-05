#include "GUI_ToolBar.h"
#include "GUI_Messages.h"
#include "GUI_DrawUtils.h"
#include "GUI_Resources.h"
#include "MathUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

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
	
	int all_sel[4] = { 0, 0, 2, 1 };
	
	glColor3f(1,1,1);
	
	GUI_DrawStretched(state, mResource.c_str(), bounds, all_sel);
	
	int v = GetValue();
	
	if (v >= 0 && v < (mH * mV))
	{
		int x = v % mH;
		int y = v / mH;
		
		int hilite_sel[4] = { x + mH, y, mH * 2, mV };
		
		int sub_bounds[4] = { 
			interp(0,bounds[0],mH,bounds[2],x  ),
			interp(0,bounds[1],mV,bounds[3],y  ),
			interp(0,bounds[0],mH,bounds[2],x+1),
			interp(0,bounds[1],mV,bounds[3],y+1) 
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

int		GUI_ToolBar::GetHelpTip(int x, int y, int tip_bounds[4], string& tip)
{
	int bounds[6];
	GetBounds(bounds);
	bounds[4] = bounds[2] - bounds[0];
	bounds[5] = bounds[3] - bounds[1];
	bounds[4] /= mH;
	bounds[5] /= mV;
		
	int xt = (x - bounds[0]) / bounds[4];
	int yt = (y - bounds[1]) / bounds[5];
	
	tip_bounds[0] = bounds[0] + (xt  ) * bounds[4];
	tip_bounds[1] = bounds[1] + (yt  ) * bounds[5];
	tip_bounds[2] = bounds[0] + (xt+1) * bounds[4];
	tip_bounds[3] = bounds[1] + (yt+1) * bounds[5];
	char buf[20];
	sprintf(buf,"%d",xt+yt*mH);
	tip = buf;
	return 1;
}


void	GUI_ToolBar::SizeToBitmap(void)
{
	GUI_TexPosition_t	metrics;
	
	GUI_GetTextureResource(mResource.c_str(),0,&metrics);
	int bounds[4];
	GetBounds(bounds);
	bounds[2] = bounds[0] + metrics.real_width / 2;
	bounds[1] = bounds[3] - metrics.real_height;
	SetBounds(bounds);
}
