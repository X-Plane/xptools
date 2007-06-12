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

#include "WED_WorldMapLayer.h"
#include "GUI_Pane.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"
#include "WED_MapZoomerNew.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

WED_WorldMapLayer::WED_WorldMapLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(host,zoomer,resolver),
	mVisible(true)
{
}

WED_WorldMapLayer::~WED_WorldMapLayer()
{
}

void		WED_WorldMapLayer::ToggleVisible(void)
{ 
	mVisible = !mVisible;
	GetHost()->Refresh();
}


void		WED_WorldMapLayer::DrawVisualization		(int inCurrent, GUI_GraphState * g)
{
	if (mVisible)		
	{
		for (int y = 0; y < 2; ++y)
		for (int x = 0; x < 4; ++x)
		{
			char	fname[30];
			sprintf(fname,"earth_%d%d.jpg",x+1,y+1);
			int	tex_id = GUI_GetTextureResource(fname, 0, NULL);
			if (tex_id)
			{

				g->SetState(0, 1, 0,    0, 1,  0, 0);
				glColor4f(1.0, 1.0, 1.0, 1.0);
				g->BindTex(tex_id, 0);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);
				glVertex2f( GetZoomer()->LonToXPixel(-180 + x * 90),
							GetZoomer()->LatToYPixel( -90 + y * 90));
				glTexCoord2f(0.0, 1.0);
				glVertex2f( GetZoomer()->LonToXPixel(-180 + x * 90),
							GetZoomer()->LatToYPixel(       y * 90));
				glTexCoord2f(1.0, 1.0);
				glVertex2f( GetZoomer()->LonToXPixel( -90 + x * 90),
							GetZoomer()->LatToYPixel(       y * 90));
				glTexCoord2f(1.0, 0.0);
				glVertex2f( GetZoomer()->LonToXPixel( -90 + x * 90),
							GetZoomer()->LatToYPixel( -90 + y * 90));
				glEnd();
			}
		}
	}

}

void		WED_WorldMapLayer::GetCaps(int& draw_ent_v, int& draw_ent_s, int& cares_about_sel)
{
	draw_ent_v = draw_ent_s = cares_about_sel = 0;
}
