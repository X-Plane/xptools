/*
 * Copyright (c) 2011, mroe .
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
#if WITHNWLINK

#include "WED_NWInfoLayer.h"
#include "WED_MapZoomerNew.h"
#include "WED_Server.h"
#include "WED_Messages.h"

#include "GUI_Fonts.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


WED_NWInfoLayer::WED_NWInfoLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver) : WED_MapLayer(h, zoomer, resolver)
{
	mColor[0]= .5;mColor[1]= .5;mColor[2]= .5;mColor[3]= 1;
	if(IsVisible()) ToggleVisible();
}

WED_NWInfoLayer::~WED_NWInfoLayer()
{

}

void	WED_NWInfoLayer::DrawVisualization (bool inCurrent, GUI_GraphState * g)
{
	int bnds[4];
	GetHost()->GetBounds(bnds);
	GUI_FontDraw(g, font_UI_Basic, mColor, bnds[2] - 90, bnds[3] - 18,"live mode");
}

void	WED_NWInfoLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel)
{
	draw_ent_v = draw_ent_s = cares_about_sel = 0;
}

void	WED_NWInfoLayer::ReceiveMessage(GUI_Broadcaster * inSrc,intptr_t inMsg,intptr_t inParam)
{
	WED_Server * ser = dynamic_cast<WED_Server *>(inSrc);
	if(!ser) return;

	if(inMsg == msg_NetworkStatusInfo )
	{
		switch(inParam)
		{
			case WED_Server::s_newdata :		return;
			case WED_Server::s_started :
				if (!IsVisible())ToggleVisible(); break;
			case WED_Server::s_stopped :
				if (IsVisible())ToggleVisible(); break;
		}
	}

	if (ser->IsReady())
		{mColor[0]= .2 ;mColor[1]=  1;mColor[2]= .2;}
	else
		{mColor[0]=  1 ;mColor[1]= .2;mColor[2]= .2;}

	GetHost()->Refresh();
}
#endif
