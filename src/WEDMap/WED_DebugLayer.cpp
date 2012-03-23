/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "WED_DebugLayer.h"
#include "GUI_GraphState.h"
#include "WED_Globals.h"
#include "WED_DrawUtils.h"
#include "WED_MapZoomerNew.h"

WED_DebugLayer::WED_DebugLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) : 
	WED_MapLayer(host, zoomer, resolver)
{
}

WED_DebugLayer::~WED_DebugLayer()
{
}

void	WED_DebugLayer::DrawStructure			(bool inCurrent, GUI_GraphState * g)
{
	WED_MapZoomerNew * z = GetZoomer();
	#if DEV
		g->SetState(false, 0, false,    false, false,   false, false);

		for(int n = 0; n < gMeshPolygons.size(); ++n)
		{
			glColor4f(gMeshPolygons[n].second.x,gMeshPolygons[n].second.y,gMeshPolygons[n].second.z, 0.3);			
			glPolygon2(&*gMeshPolygons[n].first.begin(), false, NULL, gMeshPolygons[n].first.size());
		}
		

		glBegin(GL_LINES);
		for(int n = 0; n < gMeshLines.size(); ++n)
		{
			glColor3f(gMeshLines[n].second.x,gMeshLines[n].second.y,gMeshLines[n].second.z);
			glVertex2(z->LLToPixel(gMeshLines[n].first));
		}
		glEnd();
		glPointSize(5);
		glBegin(GL_POINTS);
		for(int n = 0; n < gMeshPoints.size(); ++n)
		{
			glColor3f(gMeshPoints[n].second.x,gMeshPoints[n].second.y,gMeshPoints[n].second.z);
			glVertex2(z->LLToPixel(gMeshPoints[n].first));
		}
		glEnd();
		
		glPointSize(1);

	#endif
}

void	WED_DebugLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = false;
	draw_ent_s = false;
	cares_about_sel = false;
	wants_clicks = false;
}
