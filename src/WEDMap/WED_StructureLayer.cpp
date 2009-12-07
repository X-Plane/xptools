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

#include "WED_StructureLayer.h"
#include "IGIS.h"
#include "GUI_GraphState.h"
#include "WED_Colors.h"
#include "GISUtils.h"
#include "WED_UIDefs.h"
#include "WED_ToolUtils.h"
#include "WED_TextureNode.h"
#include "ITexMgr.h"
#include "TexUtils.h"
#include "WED_EnumSystem.h"
#include "WED_MapZoomerNew.h"
#include "WED_AirportNode.h"
#include "XESConstants.h"
#include "MathUtils.h"
#include "GUI_Resources.h"
#include "WED_Runway.h"
#include "MatrixUtils.h"
#include "WED_Helipad.h"
#include "WED_LightFixture.h"
#include "WED_Taxiway.h"
#include "WED_OverlayImage.h"
#include "WED_Sealane.h"
#include "WED_AirportSign.h"
#include "WED_TowerViewpoint.h"
#include "WED_Airport.h"
#include "WED_UIMeasurements.h"
#include "WED_RampPosition.h"
#include "WED_Windsock.h"
#include "WED_AirportBeacon.h"
#include "WED_DrawUtils.h"
#include "GUI_DrawUtils.h"

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

// This was experimental code to draw lagrange polynomials for taxiway lines.  Turns out that they don't look very good and aren't useful.
// Also, "combo" would need to be rewritten because it tends to overflow...of course we could just use long-long.

inline int factorial(int n)
{
	int t = 1;
	while(n > 1)
		t *= n--;
	return t;
}

inline int combo(int n, int k) { return factorial(n) / (factorial(k) * factorial(n - k)); }

inline double lagrange_coef(int n, int s)
{
	return combo(s-1, n);
}

inline double lagrange_weight(double t, int n, int s)
{
	double it = 1.0 - t;
	double r = 1.0;
	
	int in = s - n;
	while(in-- > 0)
		r *= it;
	while(n-- > 0)
		r *= t;
	return r;
}

void DrawLaGrange(const vector<Point2>& p)
{
	if(p.empty()) return;
	vector<double>	coef(p.size());
	for(int n = 0; n < p.size(); ++n)
		coef[n] = lagrange_coef(n,p.size());
	
	for(double t = 0.0; t <= 1.0; t += 0.00390625)
	{
		Point2	x(0.0,0.0);
		for(int n = 0; n < p.size(); ++n)
		{
			double w = coef[n] * lagrange_weight(t, n, p.size()-1);
			x.x_ += p[n].x_ * w;
			x.y_ += p[n].y_ * w;
		}
		glVertex2(x);	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


WED_StructureLayer::WED_StructureLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(h, zoomer, resolver)
{
	mRealLines = true;
	mVertices = true;
	mPavementAlpha = 0.5;
}

WED_StructureLayer::~WED_StructureLayer()
{
}

static void DrawLineAttrs(GUI_GraphState * state, const Point2 * pts, int count, const set<int>& attrs, WED_Color c)
{
	if (attrs.empty())
	{
		glColor4fv(WED_Color_RGBA(c));
		glShape2v(GL_LINE_STRIP, pts, count);
		return;
	}
	else for(set<int>::const_iterator a = attrs.begin(); a != attrs.end(); ++a)
	switch(*a) {
	// ------------ STANDARD TAXIWAY LINES ------------
	case line_SolidYellow:

		glColor4f(1,1,0,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BrokenYellow:

		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_DoubleSolidYellow:

		glColor4f(1,1,0,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_RunwayHold:

		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);
		break;
	case line_OtherHold:

		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,1.5);
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-1.5);
		break;
	case line_ILSHold:

		glColor4f(1,1,0,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x1111);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_ILSCriticalCenter:

		glColor4f(1,1,0,1);
		glLineWidth(5);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_WideBrokenSingle:

		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_WideBrokenDouble:

		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;

	// ------------ ROADWAY TAXIWAY LINES ------------
	case line_SolidWhite:

		glColor4f(1,1,1,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_Chequered:

		glColor4f(1,1,1,1);
		glLineWidth(6);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0,0,0,1);
		glLineWidth(2);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,1,1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0xCCCC);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BrokenWhite:

		glColor4f(1,1,1,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;

	// ------------ BLACK-BACKED TAXIWAY LINES ------------
	case line_BSolidYellow:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BBrokenYellow:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BDoubleSolidYellow:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BRunwayHold:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(12);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);
		break;
	case line_BOtherHold:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(6);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,1.5);
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-1.5);
		break;
	case line_BILSHold:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(7);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(5);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x1111);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BILSCriticalCenter:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(7);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(5);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BWideBrokenSingle:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BWideBrokenDouble:

		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;

	// ------------ LIGHTS ------------
	case line_TaxiCenter:

		glColor4f(0.3,1,0.3,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_TaxiEdge:

		glColor4f(0,0,1,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-5);
		break;
	case line_HoldLights:

		glColor4f(1,0.5,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7070);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_HoldLightsPulse:

		glColor4f(1,0.5,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(0.3,0.1,0,1);
		glLineStipple(1,0x0070);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_HoldShortCenter:

		glColor4f(0.3,1,0.3,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);
		glColor4f(1,0.5,0,1);
		glLineStipple(1,0x0070);
		glShape2v(GL_LINE_STRIP, pts, count);
		break;
	case line_BoundaryEdge:

		glColor4f(1,0,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShapeOffset2v(GL_LINE_STRIP, pts, count, -5);
		break;
	}

	glLineWidth(1);
	glDisable(GL_LINE_STIPPLE);
}

static bool setup_taxi_texture(int surface_code, double heading, const Point2& centroid, GUI_GraphState * g, WED_MapZoomerNew * z, float alpha)
{
	if (surface_code==surf_Trans) return false;
	if (surface_code==shoulder_None) return false;
	int tex_id = 0;
	switch(surface_code) {
	case shoulder_Asphalt:
	case surf_Asphalt:	tex_id = GUI_GetTextureResource("asphalt.png",tex_Wrap+tex_Linear+tex_Mipmap,NULL);	break;
	case shoulder_Concrete:
	case surf_Concrete:	tex_id = GUI_GetTextureResource("concrete.png",tex_Wrap+tex_Linear+tex_Mipmap,NULL);break;
	case surf_Grass:	tex_id = GUI_GetTextureResource("grass.png",tex_Wrap+tex_Linear+tex_Mipmap,NULL);	break;
	case surf_Dirt:		tex_id = GUI_GetTextureResource("dirt.png",tex_Wrap+tex_Linear+tex_Mipmap,NULL);	break;
	case surf_Gravel:	tex_id = GUI_GetTextureResource("gravel.png",tex_Wrap+tex_Linear+tex_Mipmap,NULL);	break;
	case surf_Lake:		tex_id = GUI_GetTextureResource("lake.png",tex_Wrap+tex_Linear+tex_Mipmap,NULL);	break;
	case surf_Water:	tex_id = GUI_GetTextureResource("water.png",tex_Wrap+tex_Linear+tex_Mipmap,NULL);	break;
	case surf_Snow:		tex_id = GUI_GetTextureResource("snow.png",tex_Wrap+tex_Linear+tex_Mipmap,NULL);	break;
	}
	if (tex_id == 0)
	{
		g->SetState(false,0,false,true,true,false,false);
		glColor4f(0.5,0.5,0.5,alpha);
		return true;
	}
	g->SetState(false,1,false,true,true,false,false);
	glColor4f(1,1,1,alpha);
	g->BindTex(tex_id,0);
	double ppm = z->GetPPM() * 12.5;
	GLdouble	m1[16] = { 1.0,		0.0,		0.0, 					 0.0,
							0.0, 		1.0,	0.0, 					 0.0,
							0.0, 		0.0,		1.0, 					 0.0,
							0.0, 		0.0,		0.0, 					 1.0 };

	double l,b,r,t;

	for (int n = 0; n < 16; ++n)
		m1[n] /= ppm;

	z->GetPixelBounds(l,b,r,t);

	applyRotation(m1, heading, 0, 0, 1);

	applyTranslation(m1, l-centroid.x(),b-centroid.y(),0);

	double	proj_tex_s[4], proj_tex_t[4];

	proj_tex_s[0] = m1[0 ];
	proj_tex_s[1] = m1[4 ];
	proj_tex_s[2] = m1[8 ];
	proj_tex_s[3] = m1[12];
	proj_tex_t[0] = m1[1 ];
	proj_tex_t[1] = m1[5 ];
	proj_tex_t[2] = m1[9 ];
	proj_tex_t[3] = m1[13];

	glEnable(GL_TEXTURE_GEN_S);	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);	glTexGendv(GL_S,GL_OBJECT_PLANE,proj_tex_s);
	glEnable(GL_TEXTURE_GEN_T);	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);	glTexGendv(GL_T,GL_OBJECT_PLANE,proj_tex_t);
	return true;
}

static void kill_taxi_texture(void)
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}

bool		WED_StructureLayer::DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
	g->SetState(false,0,false,   false,true,false,false);

	int locked = 0;
	WED_Entity * thing = dynamic_cast<WED_Entity *>(entity);
	while(thing)
	{
		if (thing->GetLocked()) { locked=1;break;}
		thing = dynamic_cast<WED_Entity *>(thing->GetParent());
	}

	WED_Color struct_color = selected ? (locked ? wed_StructureLockedSelected : wed_StructureSelected) :
										(locked ? wed_StructureLocked		 : wed_Structure);

	GISClass_t 		kind		= entity->GetGISClass();
	const char *	sub_class	= entity->GetGISSubtype();
	IGISPoint *						pt;
	IGISPoint_Heading *				pth;
	IGISPoint_WidthLength *			ptwl;
	IGISPointSequence *				ps;
	IGISLine_Width *				lw;
	IGISPolygon *					poly;
	IGISBoundingBox *				box;

	WED_Taxiway *					taxi;
	WED_OverlayImage *				overlay;
	WED_Sealane *					sea;

	WED_Runway *					rwy;
	WED_Helipad *					helipad;

	WED_LightFixture *				lfix;
	WED_AirportSign *				sign;
	WED_RampPosition *				ramp;

	WED_TowerViewpoint *			tower;
	WED_Windsock *					sock;
	WED_AirportBeacon *				beacon;

	WED_Airport *					airport;

	float							storage[4];

	glColor4fv(WED_Color_RGBA(struct_color));

	float icon_scale = GetFurnitureIconScale();

	/******************************************************************************************************************************************************
	 * RUNWAY DRAWING
	 ******************************************************************************************************************************************************/
	if (sub_class == WED_Airport::sClass && (airport = SAFE_CAST(WED_Airport, entity)) != NULL)
	{
		Bbox2	bounds;
		airport->GetBounds(gis_Geo,bounds);
		bounds.p1 = GetZoomer()->LLToPixel(bounds.p1);
		bounds.p2 = GetZoomer()->LLToPixel(bounds.p2);
		if (bounds.xspan() < GetAirportTransWidth() && bounds.yspan() < GetAirportTransWidth())
		{
			float * f1 = WED_Color_RGBA(struct_color);
			float * f2 = f1 + 4;
			Point2 loc = Segment2(bounds.p1,bounds.p2).midpoint();
			switch(airport->GetAirportType()) {
			case type_Airport:		mAirportIconsX.push_back(loc.x());	mAirportIconsY.push_back(loc.y());	mAirportIconsC.insert(mAirportIconsC.end(),f1,f2);		break;
			case type_Seaport:		mSeaportIconsX.push_back(loc.x());	mSeaportIconsY.push_back(loc.y());	mSeaportIconsC.insert(mSeaportIconsC.end(),f1,f2);		break;
			case type_Heliport:		mHeliportIconsX.push_back(loc.x());	mHeliportIconsY.push_back(loc.y());	mHeliportIconsC.insert(mHeliportIconsC.end(),f1,f2);	break;
			}
			return false;
		}
	}
	else if (sub_class == WED_Runway::sClass && (rwy = SAFE_CAST(WED_Runway,entity)) != NULL)
	{
		Point2 	corners[4], shoulders[8], disp1[4], disp2[4], blas1[4], blas2[4];
		bool	has_shoulders, has_disp1, has_disp2, has_blas1, has_blas2;

		// First, transform our geometry.
						rwy->GetCorners(gis_Geo,corners);			GetZoomer()->LLToPixelv(corners, corners, 4);
		if (has_blas1 = rwy->GetCornersBlas1(blas1))				GetZoomer()->LLToPixelv(blas1, blas1, 4);
		if (has_blas2 = rwy->GetCornersBlas2(blas2))				GetZoomer()->LLToPixelv(blas2, blas2, 4);
		if (has_disp1 = rwy->GetCornersDisp1(disp1))				GetZoomer()->LLToPixelv(disp1, disp1, 4);
		if (has_disp2 = rwy->GetCornersDisp2(disp2))				GetZoomer()->LLToPixelv(disp2, disp2, 4);
		if (has_shoulders = rwy->GetCornersShoulders(shoulders))	GetZoomer()->LLToPixelv(shoulders, shoulders, 8);

		if (mPavementAlpha > 0.0f)
		{
			// "Solid" geometry.
			if (setup_taxi_texture(rwy->GetSurface(),rwy->GetHeading(), GetZoomer()->LLToPixel(rwy->GetCenter()),g,GetZoomer(), mPavementAlpha))
			{
											glShape2v(GL_QUADS, corners, 4);
				if (has_blas1)				glShape2v(GL_QUADS, blas1,4);
				if (has_blas2)				glShape2v(GL_QUADS, blas2,4);
			}
			if (setup_taxi_texture(rwy->GetShoulder(),rwy->GetHeading(), GetZoomer()->LLToPixel(rwy->GetCenter()),g,GetZoomer(), mPavementAlpha))
			{
				if (has_shoulders)			glShape2v(GL_QUADS, shoulders, 8);
			}
			kill_taxi_texture();
			g->SetState(false,0,false, true,true, false,false);
		}

		//  "Outline" geometry
		glColor4fv(WED_Color_RGBA(struct_color));
		glShape2v(GL_LINE_LOOP, corners, 4);
		glBegin(GL_LINES);
		glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
		glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
		glEnd();

		if (has_shoulders)
		{
			glColor4fv(WED_Color_RGBA(struct_color));
			glBegin(GL_LINES);
			glVertex2(shoulders[3]);		glVertex2(shoulders[0]);
			glVertex2(shoulders[0]);		glVertex2(shoulders[1]);
			glVertex2(shoulders[1]);		glVertex2(shoulders[2]);
			glVertex2(shoulders[4]);		glVertex2(shoulders[7]);
			glVertex2(shoulders[7]);		glVertex2(shoulders[6]);
			glVertex2(shoulders[6]);		glVertex2(shoulders[5]);
			glEnd();
		}

		if (mRealLines) glColor4f(1,1,0,1);
		if (has_blas1)				glShape2v(GL_LINE_LOOP, blas1,4);
		if (has_blas2)				glShape2v(GL_LINE_LOOP, blas2, 4);

		if (mRealLines) glColor4f(1,1,1,1);
		if (has_disp1)				glShape2v(GL_LINE_LOOP, disp1,4);
		if (has_disp2)				glShape2v(GL_LINE_LOOP, disp2,4);

	}
	else switch(kind) {
	case gis_Point:
	case gis_Point_Bezier:
		/******************************************************************************************************************************************************
		 * NON-DIRECTIONAL POINT ELEMENTS
		 ******************************************************************************************************************************************************/
		{
			Point2			l;
			const char *	icon = NULL;
				 if (sub_class == WED_TowerViewpoint::sClass&& (tower  = SAFE_CAST(WED_TowerViewpoint, entity)) != NULL) pt = tower , icon = "map_towerview.png";
			else if (sub_class == WED_Windsock::sClass		&& (sock   = SAFE_CAST(WED_Windsock		 , entity)) != NULL) pt = sock  , icon = "map_windsock.png" ;
			else if (sub_class == WED_AirportBeacon::sClass && (beacon = SAFE_CAST(WED_AirportBeacon , entity)) != NULL) pt = beacon, icon = "map_beacon.png"   ;
			else pt = SAFE_CAST(IGISPoint,entity);
			if (pt)
			{
				pt->GetLocation(gis_Geo,l);
				l = GetZoomer()->LLToPixel(l);
				if (icon) GUI_PlotIcon(g,icon, l.x(),l.y(),0,icon_scale);
				else {
					glBegin(GL_LINES);
					glVertex2f(l.x(), l.y() - 3);
					glVertex2f(l.x(), l.y() + 3);
					glVertex2f(l.x() - 3, l.y());
					glVertex2f(l.x() + 3, l.y());
					glEnd();
				}
			}
		}
		break;
	case gis_Point_Heading:
		/******************************************************************************************************************************************************
		 * DIRECTIONAL POINT ELEMENTS
		 ******************************************************************************************************************************************************/
		{
			Point2			l;
			Vector2			dir;
			const char *	icon = NULL;

				 if (sub_class == WED_LightFixture::sClass && (lfix = SAFE_CAST(WED_LightFixture,entity)) != NULL)pth = lfix, icon = "map_light.png"	;
			else if (sub_class == WED_AirportSign::sClass  && (sign = SAFE_CAST(WED_AirportSign ,entity)) != NULL)pth = sign, icon = "map_taxisign.png"	;
			else if (sub_class == WED_RampPosition::sClass && (ramp = SAFE_CAST(WED_RampPosition,entity)) != NULL)pth = ramp, icon = "map_rampstart.png";
			else pth = SAFE_CAST(IGISPoint_Heading,entity);
			if (pth)
			{
				pth->GetLocation(gis_Geo,l);
				NorthHeading2VectorMeters(l,l,pth->GetHeading(),dir);
				Vector2 r(dir.perpendicular_cw());
				l = GetZoomer()->LLToPixel(l);
				if (icon)	GUI_PlotIcon(g,icon, l.x(),l.y(),atan2(dir.dx,dir.dy) * RAD_TO_DEG,icon_scale);
				else {
					glBegin(GL_LINES);
					glVertex2(l - dir * 3.0);			glVertex2(l + dir * 6.0);
					glVertex2(l - r   * 3.0);			glVertex2(l + r   * 3.0);
					glEnd();
				}
			}
		}
		break;
	case gis_Point_HeadingWidthLength:
		/******************************************************************************************************************************************************
		 * HELIPADS AND OTHER GIS-SIMILAR
		 ******************************************************************************************************************************************************/
		{
			if (sub_class == WED_Helipad::sClass && (helipad = SAFE_CAST(WED_Helipad		 , entity)))	ptwl = helipad;
			else									 ptwl    = SAFE_CAST(IGISPoint_WidthLength,entity)					  ;

			if (ptwl)
			{
				Point2 corners[4];
				ptwl->GetCorners(gis_Geo,corners);
				GetZoomer()->LLToPixelv(corners, corners, 4);

				if (helipad && mPavementAlpha > 0.0f)
				{
					glColor4fv(WED_Color_Surface(helipad->GetSurface(), mPavementAlpha, storage));
					glShape2v(GL_QUADS, corners, 4);
					glColor4fv(WED_Color_RGBA(struct_color));
				}

				glShape2v(GL_LINE_LOOP, corners, 4);

				glBegin(GL_LINES);
				glVertex2(Segment2(corners[0],corners[1]).midpoint(0.5));
				glVertex2(Segment2(corners[2],corners[3]).midpoint(0.5));
				glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
				glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
				glEnd();

				if (helipad)
				{
					Point2	p;
					helipad->GetLocation(gis_Geo,p);
					p = GetZoomer()->LLToPixel(p);
					GUI_PlotIcon(g, "map_helipad.png", p.x(), p.y(), ptwl->GetHeading(),icon_scale);
				}
			}
		}
		break;
	case gis_PointSequence:
	case gis_Line:
	case gis_Ring:
	case gis_Edge:
	case gis_Chain:
		/******************************************************************************************************************************************************
		 * CHAINS
		 ******************************************************************************************************************************************************/
		if ((ps = SAFE_CAST(IGISPointSequence,entity)) != NULL)
		{
			int i, n = ps->GetNumSides();
			for (i = 0; i < n; ++i)
			{
				set<int>		attrs;
				if (mRealLines)
				{
					WED_AirportNode * apt_node = dynamic_cast<WED_AirportNode*>(ps->GetNthPoint(i));
					if (apt_node) apt_node->GetAttributes(attrs);
				}
				vector<Point2>	pts;

				Segment2	s;
				Bezier2		b;
				if (ps->GetSide(gis_Geo,i,s,b))
				{
					s.p1 = b.p1;
					s.p2 = b.p2;

					b.p1 = GetZoomer()->LLToPixel(b.p1);
					b.p2 = GetZoomer()->LLToPixel(b.p2);
					b.c1 = GetZoomer()->LLToPixel(b.c1);
					b.c2 = GetZoomer()->LLToPixel(b.c2);


					int pixels_approx = sqrt(Vector2(b.p1,b.c1).squared_length()) +
										sqrt(Vector2(b.c1,b.c2).squared_length()) +
										sqrt(Vector2(b.c2,b.p2).squared_length());
					int point_count = intlim(pixels_approx / BEZ_PIX_PER_SEG, BEZ_MIN_SEGS, BEZ_MAX_SEGS);
					pts.reserve(point_count+1);
					for (int n = 0; n <= point_count; ++n)
						pts.push_back(b.midpoint((float) n / (float) point_count));

				}
				else
				{
					pts.push_back(GetZoomer()->LLToPixel(s.p1));
					pts.push_back(GetZoomer()->LLToPixel(s.p2));
				}
				DrawLineAttrs(g, &*pts.begin(), pts.size(), attrs, struct_color);
			}

			if (mVertices)
			{
				n = ps->GetNumPoints();
				glPointSize(5);
				glColor4fv(WED_Color_RGBA(struct_color));
				glBegin(GL_POINTS);

				for (i = 0; i < n; ++i)
				{
					IGISPoint * pt = ps->GetNthPoint(i);
					WED_Entity * e = dynamic_cast<WED_Entity *>(pt);
					if(!e || !e->GetHidden())
					{
						Point2 p;
						pt->GetLocation(gis_Geo,p);
						glVertex2(GetZoomer()->LLToPixel(p));
					}
				}
				glEnd();
				glPointSize(1);
			}
		}
		break;

	case gis_Line_Width:
		/******************************************************************************************************************************************************
		 * NON-RUNWAY LINES
		 ******************************************************************************************************************************************************/
		if (sub_class == WED_Sealane::sClass && (sea = SAFE_CAST(WED_Sealane, entity)) != NULL) lw = sea;
		else									 lw  = SAFE_CAST(IGISLine_Width,entity);
		if (lw)
		{
			Point2 corners[4];
			lw->GetCorners(gis_Geo,corners);
			GetZoomer()->LLToPixelv(corners, corners, 4);

			if (sea && mPavementAlpha > 0.0f)
			{
				glColor4fv(WED_Color_RGBA_Alpha(wed_Surface_Water,mPavementAlpha, storage));
				glShape2v(GL_QUADS, corners, 4);
				glColor4fv(WED_Color_RGBA(struct_color));
			}

			glShape2v(GL_LINE_LOOP, corners, 4);

			glBegin(GL_LINES);
			glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
			glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
			glEnd();
		}
		break;

	case gis_BoundingBox:
		/******************************************************************************************************************************************************
		 * BOUNDING BOXES
		 ******************************************************************************************************************************************************/
		box = SAFE_CAST(IGISBoundingBox, entity);
		if(box)
		{
			Point2	pts[2];
			box->GetMin()->GetLocation(gis_Geo,pts[0]);
			box->GetMax()->GetLocation(gis_Geo,pts[1]);
			GetZoomer()->LLToPixelv(pts,pts,2);
			glColor4fv(WED_Color_RGBA_Alpha(wed_Link, mPavementAlpha, storage));
			glBegin(GL_LINE_LOOP);
			glVertex2f(pts[0].x(), pts[0].y());
			glVertex2f(pts[0].x(), pts[1].y());
			glVertex2f(pts[1].x(), pts[1].y());
			glVertex2f(pts[1].x(), pts[0].y());
			glEnd();
		}
		break;

	case gis_Polygon:
		/******************************************************************************************************************************************************
		 * POLYGONS (TAXIWAYAS, ETC.)
		 ******************************************************************************************************************************************************/
		taxi = NULL;
		overlay = NULL;
		if (sub_class == WED_Taxiway::sClass && (taxi = SAFE_CAST(WED_Taxiway, entity)) != NULL) poly = taxi;
		else if (sub_class == WED_OverlayImage::sClass && (overlay = SAFE_CAST(WED_OverlayImage, entity)) != NULL) poly = overlay;
		else								     poly = SAFE_CAST(IGISPolygon,entity);

		if (poly)
		{
			if (taxi && mPavementAlpha > 0.0f && taxi->GetSurface() != surf_Trans)
			{
				// I tried "LODing" out the solid pavement, but the margin between when the pavement can disappear and when the whole
				// airport can is tiny...most pavement is, while visually insignificant, still sprawling, so a bbox-sizes test is poor.
				// Any other test is too expensive, and for the small pavement squares that would get wiped out, the cost of drawing them
				// is negligable anyway.
				vector<Point2>	pts;
				vector<int>		is_hole_start;

				PointSequenceToVector(poly->GetOuterRing(), GetZoomer(), pts, false, is_hole_start, 0);
				int n = poly->GetNumHoles();
				for (int i = 0; i < n; ++i)
					PointSequenceToVector(poly->GetNthHole(i), GetZoomer(), pts, false, is_hole_start, 1);

				if (!pts.empty())
				{
					Point2 centroid(0,0);
					for (int i = 0; i < pts.size(); ++i)
					{
						centroid.x_ += pts[i].x();
						centroid.y_ += pts[i].y();
					}
					centroid.x_ /= (double) pts.size();
					centroid.y_ /= (double) pts.size();

//					glColor4fv(WED_Color_Surface(taxi->GetSurface(), mPavementAlpha, storage));
					if (setup_taxi_texture(taxi->GetSurface(), taxi->GetHeading(), centroid, g, GetZoomer(), mPavementAlpha))
					{
//						glDisable(GL_CULL_FACE);
						glFrontFace(GL_CCW);
						glPolygon2(&*pts.begin(),false,  &*is_hole_start.begin(), pts.size());
						glFrontFace(GL_CW);
//						glEnable(GL_CULL_FACE);
					}
					kill_taxi_texture();
				}
			}

			this->DrawEntityStructure(inCurrent,poly->GetOuterRing(),g,selected);
			int n = poly->GetNumHoles();
			for (int c = 0; c < n; ++c)
				this->DrawEntityStructure(inCurrent,poly->GetNthHole(c),g,selected);
		}
		break;
	}
	return true;
}

bool		WED_StructureLayer::DrawEntityVisualization		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
	g->SetState(false,0,false,   false,true,false,false);

	int locked = 0;
	WED_Entity * thing = dynamic_cast<WED_Entity *>(entity);
	while(thing)
	{
		if (thing->GetLocked()) { locked=1;break;}
		thing = dynamic_cast<WED_Entity *>(thing->GetParent());
	}

	WED_Color struct_color = selected ? (locked ? wed_StructureLockedSelected : wed_StructureSelected) :
										(locked ? wed_StructureLocked		 : wed_Structure);

	GISClass_t 		kind		= entity->GetGISClass();
	const char *	sub_class	= entity->GetGISSubtype();
	IGISPoint *						pt;
	IGISPoint_Heading *				pth;
	IGISPoint_WidthLength *			ptwl;
	IGISPointSequence *				ps;
	IGISLine_Width *				lw;
	IGISPolygon *					poly;
	IGISBoundingBox *				box;

	WED_Taxiway *					taxi;
	WED_OverlayImage *				overlay;
	WED_Sealane *					sea;

	WED_Runway *					rwy;
	WED_Helipad *					helipad;

	WED_LightFixture *				lfix;
	WED_AirportSign *				sign;
	WED_RampPosition *				ramp;

	WED_TowerViewpoint *			tower;
	WED_Windsock *					sock;
	WED_AirportBeacon *				beacon;

	WED_Airport *					airport;

	float							storage[4];

	glColor4fv(WED_Color_RGBA(struct_color));

	float icon_scale = GetFurnitureIconScale();

	/******************************************************************************************************************************************************
	 * RUNWAY DRAWING
	 ******************************************************************************************************************************************************/
	switch(kind) {
	case gis_Polygon:
		/******************************************************************************************************************************************************
		 * POLYGONS (TAXIWAYAS, ETC.)
		 ******************************************************************************************************************************************************/
		taxi = NULL;
		overlay = NULL;
		if (sub_class == WED_Taxiway::sClass && (taxi = SAFE_CAST(WED_Taxiway, entity)) != NULL) poly = taxi;
		else if (sub_class == WED_OverlayImage::sClass && (overlay = SAFE_CAST(WED_OverlayImage, entity)) != NULL) poly = overlay;
		else								     poly = SAFE_CAST(IGISPolygon,entity);

		if (poly)
		{
/*		
			vector<Point2>	errs;			
			if(IsBezierSequenceScrewed(poly->GetOuterRing(), NULL &errs))
			{
				g->SetState(false,0,false,false, false,false,false);
				glColor3f(1,0,0);
				Bbox2	bounds;
				poly->GetBounds(gis_Geo,bounds);
				
				glPointSize(5);
				glBegin(GL_POINTS);
//				for(int n = 0; n < errs.size(); ++n)
				{
//					glVertex2(GetZoomer()->LLToPixel(errs[n]));
					glVertex2(GetZoomer()->LLToPixel(bounds.p1));
					glVertex2(GetZoomer()->LLToPixel(bounds.p2));
				}
				glEnd();
			}
*/			
		
		
			if (overlay)
			{
				IGISPointSequence * oring = overlay->GetOuterRing();
				WED_TextureNode * tn1 = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(0));
				WED_TextureNode * tn2 = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(1));
				WED_TextureNode * tn3 = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(2));
				WED_TextureNode * tn4 = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(3));
				Point2 st1,st2,st3,st4, v1,v2,v3,v4;
				tn1->GetLocation(gis_UV,st1);	tn1->GetLocation(gis_Geo,v1);
				tn2->GetLocation(gis_UV,st2);	tn2->GetLocation(gis_Geo,v2);
				tn3->GetLocation(gis_UV,st3);	tn3->GetLocation(gis_Geo,v3);
				tn4->GetLocation(gis_UV,st4);	tn4->GetLocation(gis_Geo,v4);


				string img_file;
				overlay->GetImage(img_file);

				ITexMgr * mgr = WED_GetTexMgr(GetResolver());
				TexRef ref = mgr->LookupTexture(img_file.c_str(),false,0);
				g->SetState(0,ref ? 1 : 0,0, 1, 1, 0, 0);
				if (ref) { g->BindTex(mgr->GetTexID(ref),0);

				int vis_x, vis_y, tot_x, tot_y;
				mgr->GetTexInfo(ref,&vis_x,&vis_y,&tot_x,&tot_y, NULL, NULL);
				double sx = (double) vis_x / (double) tot_x;
				double sy = (double) vis_y / (double) tot_y;
				st1.x_ *= sx; st1.y_ *= sy;
				st2.x_ *= sx; st2.y_ *= sy;
				st3.x_ *= sx; st3.y_ *= sy;
				st4.x_ *= sx; st4.y_ *= sy;
				}
				glDisable(GL_CULL_FACE);
				glColor3f(1,1,1);
				glBegin(GL_QUADS);
				glTexCoord2(st4);	glVertex2(GetZoomer()->LLToPixel(v4));
				glTexCoord2(st3);	glVertex2(GetZoomer()->LLToPixel(v3));
				glTexCoord2(st2);	glVertex2(GetZoomer()->LLToPixel(v2));
				glTexCoord2(st1);	glVertex2(GetZoomer()->LLToPixel(v1));
				glEnd();
				glEnable(GL_CULL_FACE);

			}
		}
		break;
	}
	return true;
}


bool		WED_StructureLayer::GetRealLinesShowing(void) const
{
	return mRealLines;
}

void		WED_StructureLayer::SetRealLinesShowing(bool show)
{
	mRealLines = show;
	GetHost()->Refresh();
}

void		WED_StructureLayer::SetPavementTransparency(float alpha)
{
	mPavementAlpha = alpha;
	GetHost()->Refresh();
}

float		WED_StructureLayer::GetPavementTransparency(void) const
{
	return mPavementAlpha;
}

bool		WED_StructureLayer::GetVerticesShowing(void) const
{
	return mVertices;
}

void		WED_StructureLayer::SetVerticesShowing(bool show)
{
	mVertices = show;
	GetHost()->Refresh();
}

void		WED_StructureLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel)
{
	draw_ent_v = 1;
	draw_ent_s = 1;
	cares_about_sel = 1;
}

void		WED_StructureLayer::DrawStructure(bool inCurrent, GUI_GraphState * g)
{
	// Drawing each icon during iteration is slow because:
	// - We have a ton of OGL state thrash.
	// - We have to make individual calls to PlotIcon, which does overhead work per icon.
	// So instead, we accumulate all of the icons into vectors and then blit them out all
	// at once.  This gives us one bind, one set state and one glBegin.  We're still transforming
	// vertices per frame, but that's okay -- the OGL state was the single really big cost.
	//
	// Note that we clear the vectors to keep them from building up forever, but their memory
	// is not dealloated, so this works up a high-water-mark of icons.  This is good, as it means
	// that in the long term our memory usage will stabilize.
	float scale = GetAirportIconScale();
	if (!mAirportIconsX.empty())
	{
		GUI_PlotIconBulk(g,"map_airport.png", mAirportIconsX.size(), &*mAirportIconsX.begin(), &*mAirportIconsY.begin(), &*mAirportIconsC.begin(), scale);
		mAirportIconsX.clear();
		mAirportIconsY.clear();
		mAirportIconsC.clear();
	}
	if (!mSeaportIconsX.empty())
	{
		GUI_PlotIconBulk(g,"map_seaport.png", mSeaportIconsX.size(), &*mSeaportIconsX.begin(), &*mSeaportIconsY.begin(), &*mSeaportIconsC.begin(), scale);
		mSeaportIconsX.clear();
		mSeaportIconsY.clear();
		mSeaportIconsC.clear();
	}
	if (!mHeliportIconsX.empty())
	{
		GUI_PlotIconBulk(g,"map_heliport.png", mHeliportIconsX.size(), &*mHeliportIconsX.begin(), &*mHeliportIconsY.begin(), &*mHeliportIconsC.begin(), scale);
		mHeliportIconsX.clear();
		mHeliportIconsY.clear();
		mHeliportIconsC.clear();
	}
}

