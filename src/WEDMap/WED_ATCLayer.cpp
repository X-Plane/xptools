//
//  WED_ATCLayer.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 1/25/16.
//
//

#include "XDefs.h"
#include "WED_ATCLayer.h"
#include "WED_TaxiRoute.h"
#include "AssertUtils.h"
#include "WED_EnumSystem.h"
#include "GISUtils.h"
#include "GUI_GraphState.h"
#include "WED_MapZoomerNew.h"
#include "WED_DrawUtils.h"
#include "WED_RampPosition.h"
#include "GUI_Resources.h"
#include "TexUtils.h"

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif


WED_ATCLayer::WED_ATCLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(host, zoomer, resolver)
{
}

WED_ATCLayer::~WED_ATCLayer()
{
}

static void make_arrow_line(Point2 p[5])
{
	// incoming:
	// 0------------1
	// S            D
	// 3------------2
	
	// outgoing:
	// 0-----------1
	// S            2
	// 4-----------3
	
	p[4] = p[3];
	p[3] = p[2];
	
	Vector2 v1to3(p[1],p[3]);
	
	v1to3 *= 0.5;
	p[2] = p[1] + v1to3;
	
	v1to3 = v1to3.perpendicular_cw();
	p[1] += v1to3;
	p[3] += v1to3;
}

bool		WED_ATCLayer::DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
	if(entity->GetGISSubtype() == WED_RampPosition::sClass)
	{
		WED_RampPosition * pos = dynamic_cast<WED_RampPosition *>(entity);
		DebugAssert(pos);
		
		Point2 nose_wheel;
		int icao_width = pos->GetWidth();
		float mtr = 5;
		float offset = 0;
		
		int id = 0;
		
		pos->GetLocation(gis_Geo, nose_wheel);
		switch(icao_width) {
		case width_A:	mtr = 15.0;	offset = 1.85f;	id = GUI_GetTextureResource("ClassA.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_B:	mtr = 27.0;	offset = 2.75f; id = GUI_GetTextureResource("ClassB.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_C:	mtr = 41.0;	offset = 4.70f; id = GUI_GetTextureResource("ClassC.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_D:	mtr = 56.0;	offset = 9.50f; id = GUI_GetTextureResource("ClassD.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_E:	mtr = 72.0;	offset = 8.20f; id = GUI_GetTextureResource("ClassE.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_F:	mtr = 80.0;	offset = 8.80f; id = GUI_GetTextureResource("ClassF.png",tex_Linear|tex_Mipmap,NULL);	break;
		}
		
		Point2	c[4];
		Quad_1to4(nose_wheel, pos->GetHeading(), mtr, mtr, c);
		
		g->SetState(0, id ? 1 : 0, 0, 0, 1, 0, 0);
		
		glColor4f(1,1,0,0.5);
		if(id)
			g->BindTex(id, 0);
		
		GetZoomer()->LLToPixelv(c,c,4);
		
		Vector2	v_left (c[1],c[0]);
		Vector2 v_right(c[2],c[3]);
		
		float relative_slip = (mtr * 0.5f - offset) / mtr;
		
		c[0] += v_left * relative_slip;
		c[1] += v_left * relative_slip;
		c[2] += v_right* relative_slip;
		c[3] += v_right* relative_slip;
		
		glBegin(GL_QUADS);
		glTexCoord2f(0,0);	glVertex2(c[0]);
		glTexCoord2f(0,1);	glVertex2(c[1]);
		glTexCoord2f(1,1);	glVertex2(c[2]);
		glTexCoord2f(1,0);	glVertex2(c[3]);
		glEnd();
			
	}
	if(entity->GetGISSubtype() == WED_TaxiRoute::sClass)
	{
		WED_TaxiRoute * seg = dynamic_cast<WED_TaxiRoute *>(entity);
		DebugAssert(seg);
		
		Point2 ends[2];
		seg->GetNthPoint(0)->GetLocation(gis_Geo, ends[0]);
		seg->GetNthPoint(1)->GetLocation(gis_Geo, ends[1]);
		
		int icao_width = seg->GetWidth();
		bool hot = seg->HasHotArrival() || seg->HasHotDepart();
		bool ils = seg->HasHotILS();
		bool rwy = seg->IsRunway();
		bool road = seg->AllowTrucks() && !seg->AllowAircraft();
		bool one_way = seg->IsOneway();
		
		int mtr1 = 5, mtr2 = 10;
		switch(icao_width) {
		case width_A:	mtr1 = 4.5;		mtr2 = 15.0;	break;
		case width_B:	mtr1 = 6.0;		mtr2 = 24.0;	break;
		case width_C:	mtr1 = 9.0;		mtr2 = 36.0;	break;
		case width_D:	mtr1 = 14.0;	mtr2 = 52.0;	break;
		case width_E:	mtr1 = 14.0;	mtr2 = 65.0;	break;
		case width_F:	mtr1 = 16.0;	mtr2 = 80.0;	break;
		}
		
		if(road) { mtr1 = one_way ? 4.0 : 8.0; mtr2 = 0.0; }
		
		Point2	c[5], d[5];
		Quad_2to4(ends, mtr1, c);
		Quad_2to4(ends, mtr2, d);
		
		g->SetState(0, 0, 0, 0, 1, 0, 0);
		if(rwy && hot)
			glColor4f(0.9,0.1,0.7,0.4);
		else if(hot)
			glColor4f(1,0,0,0.4);
		else if(ils)
			glColor4f(0.8,0.5,0,0.4);
		else if(road) //Warning! Because a ground route can also have IsRunway() == true, this check must come first
			glColor4f(1, 1, 1, 0.4);
		else if(rwy)
			glColor4f(0.0,0.2,0.6,0.4);
		else
			glColor4f(1,1,0,0.4);
		
		GetZoomer()->LLToPixelv(c,c,4);

		GetZoomer()->LLToPixelv(d,d,4);
		
		int np = 4;
		if(one_way)
		{
			make_arrow_line(c);
			make_arrow_line(d);
			np = 5;
		}
		
		glBegin(GL_TRIANGLE_FAN);
		glVertex2v(c,np);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glVertex2v(d,np);
		glEnd();
		
	}

	return true;
}

void		WED_ATCLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = false;
	cares_about_sel = false;
	draw_ent_s = true;
	wants_clicks = false;
}
