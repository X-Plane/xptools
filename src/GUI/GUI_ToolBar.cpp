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

#include "GUI_ToolBar.h"
#include "GUI_Messages.h"
#include "GUI_DrawUtils.h"
#include "GUI_Resources.h"
#include "MathUtils.h"
#include "GUI_GraphState.h"

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

void	GUI_ToolBar::SetToolTips(const vector<string>& in_tips)
{
	mTips = in_tips;
}

void	GUI_ToolBar::SetValue(float inValue)
{
	GUI_Control::SetValue(inValue);
	Refresh();
}

static void tex_quad(int sx, int sy, int tx, int ty, int dx, int dy, float ttx, float tty)
{
	glTexCoord2f((float) (tx) / ttx, (float) (ty) / tty);
	glVertex2f(sx,sy);

	glTexCoord2f((float) (tx) / ttx, (float) (ty+dy) / tty);
	glVertex2f(sx,sy+dy);

	glTexCoord2f((float) (tx+dx) / ttx, (float) (ty+dy) / tty);
	glVertex2f(sx+dx,sy+dy);

	glTexCoord2f((float) (tx+dx) / ttx, (float) (ty) / tty);
	glVertex2f(sx+dx,sy);

}

void		GUI_ToolBar::Draw(GUI_GraphState * state)
{
	int bounds[6];
	GetBounds(bounds);
	bounds[4] = bounds[2] - bounds[0];
	bounds[5] = bounds[3] - bounds[1];
		
	GUI_TexPosition_t	metrics;

	int tex_id = GUI_GetTextureResource(mResource.c_str(), 0, &metrics);
	
	int cell_h = (metrics.real_width  / 2) / mH;
	int cell_v = (metrics.real_height    ) / mV;
	int extra_h = (metrics.real_width  / 2) % mH;
	int extra_v = (metrics.real_height    ) % mV;
	
	int offset_v = bounds[5] - (cell_v * mV + extra_v);
	
	state->SetState(0, 1, 0, 1, 1, 0, 0);
	state->BindTex(tex_id, 0);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	
	for (int y = 0; y < mV; ++y)
	for (int x = 0; x < mH; ++x)
	{
		int sel = ((x + y * mH) == GetValue());
		
		int tex_x_off = x * cell_h;
		int tex_y_off = y * cell_v;
		if (sel) tex_x_off += (metrics.real_width  / 2);
		
		int scr_x_off = bounds[0] + x * cell_h;
		int scr_y_off = bounds[1] + y * cell_v + offset_v;
		
		tex_quad(scr_x_off,scr_y_off,tex_x_off,tex_y_off,cell_h + extra_h, cell_v + extra_v, metrics.tex_width, metrics.tex_height);
	}
	glEnd();
	
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
	
	int n = xt + yt * mH;
	
	if (n >= 0 && n < mTips.size())
		tip = mTips[n];

	return 1;
}


void	GUI_ToolBar::SizeToBitmap(void)
{
	int	metrics[2];
	
	GUI_GetImageResourceSize(mResource.c_str(),metrics);
	int bounds[4];
	GetBounds(bounds);
	bounds[2] = bounds[0] + metrics[0] / 2;
	bounds[1] = bounds[3] - metrics[1];
	SetBounds(bounds);
}
