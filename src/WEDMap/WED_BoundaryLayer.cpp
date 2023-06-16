/*
 * Copyright (c) 2023, Laminar Research.
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

#include "WED_BoundaryLayer.h"

#include "WED_AirportBoundary.h"
#include "WED_TerPlacement.h"

#include "AssertUtils.h"
#include "GISUtils.h"
#include "MathUtils.h"
#include "TexUtils.h"
#include "XESConstants.h"
#include "GUI_DrawUtils.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"
#include "WED_GISUtils.h"
#include "WED_DrawUtils.h"
#include "WED_EnumSystem.h"
#include "WED_HierarchyUtils.h"
#include "WED_MapZoomerNew.h"

#include "WED_ToolUtils.h"
#include "WED_ResourceMgr.h"
#include "WED_LibraryMgr.h"
#include "DEMDefs.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


WED_BoundaryLayer::WED_BoundaryLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(host, zoomer, resolver)
{
}

WED_BoundaryLayer::~WED_BoundaryLayer()
{
}

bool	WED_BoundaryLayer::DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, bool selected, bool locked)
{
	if(entity->GetGISSubtype() == WED_AirportBoundary::sClass)
	{
		auto bdy = dynamic_cast<WED_AirportBoundary*>(entity);
		DebugAssert(bdy);

		IGISPointSequence* ps = bdy->GetOuterRing();
		g->SetState(0, 0, 0, 1, 1, 0, 0);
		glLineWidth(3);

		float color[4];
		const float color_all[4] = { 1, 1 , 1, 1 };

#if HAS_BDY_TYPES
		auto types = bdy->GetType();

		const float color_flatten[4] = { 0,  0,  1, 1 };
		const float color_grass[4]   = { 1, .5,  0, 1 };
		const float color_exclude[4] = { .2, 1, .2, 1 };

		if (types.size() == 2)
		{
			switch (*(types.end()))
			{
				case bdy_flatten:
					glColor4fv(color_flatten);	break;
				case bdy_grass:
					glColor4fv(color_grass); break;
				case bdy_exclude:
					glColor4fv(color_exclude); break;
			}

			for (int i = 0; i < ps->GetNumSides(); ++i)
			{
				vector<Point2>	pts;
				SideToPoints(ps, i, GetZoomer(), pts);
				glShapeOffset2v(GL_LINES, &*pts.begin(), pts.size(), -3);
			}
		}

		if (types.size() == 1 || types.size() == 2)
		{
			switch (*(types.begin()))
			{
			case bdy_flatten:
				for (int i = 0; i < 4; color[i] = color_flatten[i], i++); break;
			case bdy_grass:
				for (int i = 0; i < 4; color[i] = color_grass[i], i++);	break;
			case bdy_exclude:
				for (int i = 0; i < 4; color[i] = color_exclude[i], i++); break;
			}
		}
		else // all 3 type or none set
#endif
		for (int i = 0; i < 4; color[i] = color_all[i], i++);

		glColor4fv(color);
		for (int i = 0; i < ps->GetNumSides(); ++i)
		{
			vector<Point2>	pts;
			SideToPoints(ps, i, GetZoomer(), pts);
			glShapeOffset2v(GL_LINES, &*pts.begin(), pts.size(),3);
		}

		color[3] = 0.6;
		glColor4fv(color);
		for (int i = 0; i < ps->GetNumSides(); ++i)
		{
			vector<Point2>	pts;
			SideToPoints(ps, i, GetZoomer(), pts);
			glShapeOffset2v(GL_LINES, &*pts.begin(), pts.size(), 6);
		}

		color[3] = 0.3;
		glColor4fv(color);
		for (int i = 0; i < ps->GetNumSides(); ++i)
		{
			vector<Point2>	pts;
			SideToPoints(ps, i, GetZoomer(), pts);
			glShapeOffset2v(GL_LINES, &*pts.begin(), pts.size(), 9);
		}

		glLineWidth(1);
	}

	if (selected && entity->GetGISSubtype() == WED_TerPlacement::sClass)
	{
		auto ter = dynamic_cast<WED_TerPlacement*>(entity);
		DebugAssert(ter);

		string rpath;
		const dem_info_t* info;
		ter->GetResource(rpath);
		auto rmgr = WED_GetResourceMgr(GetResolver());
		if (rmgr->GetDem(rpath, info))
		{
			Bbox2 bounds;
			ter->GetBounds(gis_Geo, bounds);

			auto ps = ter->GetOuterRing();
			// get area polygon
			Polygon2 area;
			WED_PolygonForPointSequence(ps, area, COUNTERCLOCKWISE);

			int mesh_dx = ter->GetSamplingFactor();
			int mesh_dy = ter->GetSamplingFactor();

			g->SetState(0, 0, 0, 1, 1, 0, 0);
			glPointSize(4);

			float* color_in = WED_Color_RGBA(locked ? wed_StructureLocked : wed_StructureSelected);
			float* color_out = locked ? nullptr : WED_Color_RGBA(wed_StructureLocked);

			glBegin(GL_POINTS);
			int ymin = info->y_lower(bounds.ymin());
			ymin -= ymin % mesh_dy;
			int xmin = info->x_lower(bounds.xmin());
			xmin -= xmin % mesh_dx;
			for (int y = ymin; y < info->y_upper(bounds.ymax()) + mesh_dy; y += mesh_dy)
				for (int x = xmin; x < info->x_upper(bounds.xmax()) + mesh_dx; x += mesh_dx)
				{
					auto pt = Point2(info->x_to_lon(x), info->y_to_lat(y));
					if(auto color = (area.inside(pt) ? color_in : color_out))
					{
						glColor4fv(color);
						glVertex2(GetZoomer()->LLToPixel(pt));
					}
				}
			glEnd();
		}
	}

	return true;
}

void		WED_BoundaryLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = false;
	cares_about_sel = true;
	draw_ent_s = true;
	wants_clicks = false;
}
