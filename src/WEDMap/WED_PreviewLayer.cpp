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

#include "WED_PreviewLayer.h"
#include "ILibrarian.h"
#include "WED_ResourceMgr.h"
#include "WED_TexMgr.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_TexMgr.h"
#include "MatrixUtils.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "WED_ToolUtils.h"
#include "WED_MapZoomerNew.h"
#include "WED_DrawUtils.h"
#include "WED_Runway.h"
#include "WED_Taxiway.h"
#include "WED_Sealane.h"
#include "WED_Helipad.h"
#include "WED_ObjPlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_FacadePlacement.h"
#include "TexUtils.h"
#include "ObjDraw.h"
#include "XObjDefs.h"
#include "MathUtils.h"
#include "WED_UIDefs.h"
#include "WED_EnumSystem.h"
#include "GUI_Resources.h"

#if APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/***************************************************************************************************************************************************
 * MISC DRAWING UTILS
 ***************************************************************************************************************************************************/

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


static bool setup_pol_texture(ITexMgr * tman, pol_info_t& pol, double heading, bool no_proj, const Point2& centroid, GUI_GraphState * g, WED_MapZoomerNew * z)
{
	TexRef	ref = tman->LookupTexture(pol.base_tex.c_str(),true, pol.wrap ? tex_Wrap : 0);
	if(ref == NULL) return false;
	int tex_id = tman->GetTexID(ref);

	if (tex_id == 0)
	{
		g->SetState(false,0,false,true,true,false,false);
		glColor3f(0.5,0.5,0.5);
		return true;
	}
	g->SetState(false,1,false,!pol.kill_alpha,!pol.kill_alpha,false,false);
	glColor3f(1,1,1);
	g->BindTex(tex_id,0);

	if(no_proj)
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
	else
	{
		double ppm = z->GetPPM();
		GLdouble	m1[16] = { 1.0,		0.0,		0.0, 					 0.0,
								0.0, 		1.0,	0.0, 					 0.0,
								0.0, 		0.0,		1.0, 					 0.0,
								0.0, 		0.0,		0.0, 					 1.0 };

		double l,b,r,t;

		m1[0] /= (ppm * pol.proj_s);
		m1[5] /= (ppm * pol.proj_t);

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
	}
	return true;
}

struct	Obj_DrawStruct {
	GUI_GraphState *	g;
	int					tex;
	int					drp;
};

static void kill_pol_texture(void)
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}


void Obj_SetupPoly(void * ref)
{
	Obj_DrawStruct * d= (Obj_DrawStruct*) ref;
	d->g->SetTexUnits(1);
	glColor3f(1,1,1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
void Obj_SetupLine(void * ref)
{
	Obj_DrawStruct * d= (Obj_DrawStruct*) ref;
	d->g->SetTexUnits(0);
	glColor3f(1,1,1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
void Obj_SetupLight(void * ref)
{
	Obj_DrawStruct * d= (Obj_DrawStruct*) ref;
	d->g->SetTexUnits(0);
	glColor3f(1,1,1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
void Obj_SetupMovie(void * ref)
{
	Obj_DrawStruct * d= (Obj_DrawStruct*) ref;
	d->g->SetTexUnits(0);
	glColor3f(1,1,1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
void Obj_SetupPanel(void * ref)
{
	Obj_DrawStruct * d= (Obj_DrawStruct*) ref;
	d->g->SetTexUnits(0);
	glColor3f(1,1,1);
}

void Obj_TexCoord(const float * st, void * ref)
{
	glTexCoord2fv(st);
}

void Obj_TexCoordPointer(int size, unsigned long type, long stride, const void * pointer, void * ref)
{
	glTexCoordPointer(size, type, stride, pointer);
}

float Obj_GetAnimParam(const char * string, float v1, float v2, void * ref)
{
	return v1;
}

void Obj_SetDraped(void * ref)
{	
	Obj_DrawStruct * d= (Obj_DrawStruct*) ref;
	d->g->BindTex(d->drp,0);
}

void Obj_SetNoDraped(void * ref)
{
	Obj_DrawStruct * d= (Obj_DrawStruct*) ref;
	d->g->BindTex(d->tex,0);
}

static ObjDrawFuncs10_t kFuncs  = { Obj_SetupPoly, Obj_SetupLine, Obj_SetupLight, Obj_SetupMovie, Obj_SetupPanel, Obj_TexCoord, Obj_TexCoordPointer, Obj_GetAnimParam, Obj_SetDraped, Obj_SetNoDraped };

void draw_obj_at_ll(ITexMgr * tman, XObj8 * o, const Point2& loc, float r, GUI_GraphState * g, WED_MapZoomerNew * zoomer)
{
	TexRef	ref = tman->LookupTexture(o->texture.c_str() ,true, tex_Wrap);			
	TexRef	ref2 = o->texture_draped.empty() ? ref : tman->LookupTexture(o->texture_draped.c_str() ,true, tex_Wrap);
	int id1 = ref  ? tman->GetTexID(ref ) : 0;
	int id2 = ref2 ? tman->GetTexID(ref2) : 0;
	g->SetTexUnits(1);
	if(id1)g->BindTex(id1,0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	Point2 l = zoomer->LLToPixel(loc);
	glTranslatef(l.x(),l.y(),0.0);
	float ppm = zoomer->GetPPM();
	glScalef(ppm,ppm,0.001);
	glRotatef(90, 1,0,0);
	glRotatef(r, 0, -1, 0);
//	GLfloat mv[16], pv[16];
//	glGetFloatv(GL_MODELVIEW_MATRIX,mv);
//	glGetFloatv(GL_PROJECTION_MATRIX,pv);
	g->EnableDepth(true,true);
	glClear(GL_DEPTH_BUFFER_BIT);
	Obj_DrawStruct ds = { g, id1, id2 };
	ObjDraw8(*o, 0, &kFuncs, &ds); 
	g->EnableDepth(false,false);
	glPopMatrix();
}

void draw_obj_at_xyz(ITexMgr * tman, XObj8 * o, double x, double y, double z, float r, GUI_GraphState * g)
{
	TexRef	ref = tman->LookupTexture(o->texture.c_str() ,true, tex_Wrap);			
	TexRef	ref2 = o->texture_draped.empty() ? ref : tman->LookupTexture(o->texture_draped.c_str() ,true, tex_Wrap);
	int id1 = ref  ? tman->GetTexID(ref ) : 0;
	int id2 = ref2 ? tman->GetTexID(ref2) : 0;
	g->SetTexUnits(1);
	if(id1)g->BindTex(id1,0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(x,y,z);
	glRotatef(r, 0, -1, 0);
	g->EnableDepth(true,true);
	glClear(GL_DEPTH_BUFFER_BIT);
	Obj_DrawStruct ds = { g, id1, id2 };
	ObjDraw8(*o, 0, &kFuncs, &ds); 
	g->EnableDepth(false,false);
	glPopMatrix();
}



// Given a group name and an offset, this comes up with the total layer number...

const struct { const char * name; int group_lo;  int group_hi; }	kGroupNames[] = {
	"terrain",		group_Terrain,			group_Terrain,
	"beaches",		group_Beaches,			group_Beaches,
	"shoulders",	group_ShouldersBegin,	group_ShouldersEnd,
	"taxiways",		group_TaxiwaysBegin,	group_TaxiwaysEnd,
	"runways",		group_RunwaysBegin,		group_RunwaysEnd,
	"markings",		group_Markings,			group_Markings,
	"airports",		group_AirportsBegin,	group_AirportsEnd,
	"footprints",	group_Footprints,		group_Footprints,
	"roads",		group_Roads,			group_Roads,
	"objects",		group_Objects,			group_Objects,
	"light_objects",group_LightObjects,		group_LightObjects,
	NULL,			0,						0
};


static int layer_group_for_string(const char * s, int o, int def)
{
	int n = 0;
	while(kGroupNames[n].name)
	{
		if(strcasecmp(s,kGroupNames[n].name) == 0)
			return (o < 0) ? (kGroupNames[n].group_lo + o) : (kGroupNames[n].group_hi + o);		
		++n;
	}
	return def;	
}

/***************************************************************************************************************************************************
 * DRAW ITEMS FOR SORT
 ***************************************************************************************************************************************************/


struct sort_item_by_layer {	bool operator()(WED_PreviewItem * lhs, WED_PreviewItem * rhs) const { return lhs->get_layer() < rhs->get_layer(); } };


struct	preview_runway : public WED_PreviewItem {
	WED_Runway * rwy;	
	int			 do_shoulders;
	preview_runway(WED_Runway * r, int l, int is_shoulders) : WED_PreviewItem(l), rwy(r), do_shoulders(is_shoulders) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		Point2 	corners[4], shoulders[8], blas1[4], blas2[4];
		bool	has_shoulders, has_blas1, has_blas2;

		// First, transform our geometry.
						rwy->GetCorners(gis_Geo,corners);			zoomer->LLToPixelv(corners, corners, 4);
		if (has_blas1 = rwy->GetCornersBlas1(blas1))				zoomer->LLToPixelv(blas1, blas1, 4);
		if (has_blas2 = rwy->GetCornersBlas2(blas2))				zoomer->LLToPixelv(blas2, blas2, 4);
		if (has_shoulders = rwy->GetCornersShoulders(shoulders))	zoomer->LLToPixelv(shoulders, shoulders, 8);

		if (mPavementAlpha > 0.0f)
		{
			// "Solid" geometry.
			if(!do_shoulders)
			if (setup_taxi_texture(rwy->GetSurface(),rwy->GetHeading(), zoomer->LLToPixel(rwy->GetCenter()),g,zoomer, mPavementAlpha))
			{
											glShape2v(GL_QUADS, corners, 4);
				if (has_blas1)				glShape2v(GL_QUADS, blas1,4);
				if (has_blas2)				glShape2v(GL_QUADS, blas2,4);
			}
			if(do_shoulders)
			if (setup_taxi_texture(rwy->GetShoulder(),rwy->GetHeading(), zoomer->LLToPixel(rwy->GetCenter()),g,zoomer, mPavementAlpha))
			{
				if (has_shoulders)			glShape2v(GL_QUADS, shoulders, 8);
			}
			kill_taxi_texture();
			g->SetState(false,0,false, true,true, false,false);
		}	
	}
};

struct	preview_helipad : public WED_PreviewItem {
	WED_Helipad * heli;	
	preview_helipad(WED_Helipad * h, int l) : WED_PreviewItem(l), heli(h) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		GLfloat storage[4];
		Point2 corners[4];
		heli->GetCorners(gis_Geo,corners);
		zoomer->LLToPixelv(corners, corners, 4);

		if (mPavementAlpha > 0.0f)
		{
			g->SetState(false,0,false, true,true, false,false);
			glColor4fv(WED_Color_Surface(heli->GetSurface(), mPavementAlpha, storage));
			glShape2v(GL_QUADS, corners, 4);
		}		
	}
};

struct	preview_sealane : public WED_PreviewItem {
	WED_Sealane * sea;	
	preview_sealane(WED_Sealane * s, int l) : WED_PreviewItem(l), sea(s) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		Point2 corners[4];
		sea->GetCorners(gis_Geo,corners);
		zoomer->LLToPixelv(corners, corners, 4);

		if (mPavementAlpha > 0.0f)
		{
			GLfloat storage[4];
			g->SetState(false,0,false, true,true, false,false);
			glColor4fv(WED_Color_RGBA_Alpha(wed_Surface_Water,mPavementAlpha, storage));
			glShape2v(GL_QUADS, corners, 4);
		}
	
	}
};

struct	preview_taxiway : public WED_PreviewItem {
	WED_Taxiway * taxi;	
	preview_taxiway(WED_Taxiway * t, int l) : WED_PreviewItem(l), taxi(t) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		// I tried "LODing" out the solid pavement, but the margin between when the pavement can disappear and when the whole
		// airport can is tiny...most pavement is, while visually insignificant, still sprawling, so a bbox-sizes test is poor.
		// Any other test is too expensive, and for the small pavement squares that would get wiped out, the cost of drawing them
		// is negligable anyway.
		vector<Point2>	pts;
		vector<int>		is_hole_start;

		PointSequenceToVector(taxi->GetOuterRing(), zoomer, pts, false, is_hole_start, 0);
		int n = taxi->GetNumHoles();
		for (int i = 0; i < n; ++i)
			PointSequenceToVector(taxi->GetNthHole(i), zoomer, pts, false, is_hole_start, 1);

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
			if (setup_taxi_texture(taxi->GetSurface(), taxi->GetHeading(), centroid, g, zoomer, mPavementAlpha))
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
};


struct	preview_polygon : public WED_PreviewItem {
	WED_GISPolygon * pol;	
	bool has_uv;
	preview_polygon(WED_GISPolygon * p, int l, bool uv) : WED_PreviewItem(l), pol(p), has_uv(uv) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
			vector<Point2>	pts;
			vector<int>		is_hole_start;

		PointSequenceToVector(pol->GetOuterRing(), zoomer, pts, has_uv, is_hole_start, 0);
		int n = pol->GetNumHoles();
		for (int i = 0; i < n; ++i)
			PointSequenceToVector(pol->GetNthHole(i), zoomer, pts, has_uv, is_hole_start, 1);

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

			glFrontFace(GL_CCW);
			glPolygon2(&*pts.begin(), has_uv, &*is_hole_start.begin(), pts.size() / (has_uv ? 2 : 1));
			glFrontFace(GL_CW);
		}
	}
};

struct	preview_forest : public preview_polygon {
	WED_ForestPlacement * fst;	
	preview_forest(WED_ForestPlacement * f, int l) : preview_polygon(f,l,false), fst(f) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		g->SetState(false,0,false,false, false,false,false);
		glColor3f(
			interp(0,0.1,1,0.0,fst->GetDensity()),
			interp(0,0.5,1,0.3,fst->GetDensity()),
			interp(0,0.1,1,0.0,fst->GetDensity()));

		preview_polygon::draw_it(zoomer,g,mPavementAlpha);
	}
};

struct	preview_facade : public preview_polygon {
	WED_FacadePlacement * fac;	
	preview_facade(WED_FacadePlacement * f, int l) : preview_polygon(f,l,false), fac(f) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		g->SetState(false,0,false,false, false,false,false);
		glColor3f(0.7,0.7,0.7);

		if(fac->GetTopoMode() == WED_FacadePlacement::topo_Area)
			preview_polygon::draw_it(zoomer,g,mPavementAlpha);
		if(fac->GetTopoMode() != WED_FacadePlacement::topo_Chain)
		{
			vector<Point2>	pts;

			SideToPoints(fac->GetOuterRing(), 0, zoomer, pts);

			glColor3f(1,1,1);
			glLineWidth(3);
			glBegin(GL_LINES/*GL_LINE_STRIP*/);
			for(vector<Point2>::iterator p = pts.begin(); p != pts.end(); ++p)
				glVertex2(*p);
			glEnd();
			glLineWidth(1);
		}

		g->SetState(false,0,false,true,true,false,false);
	
		for(int h = -1; h < fac->GetNumHoles(); ++h)
		{
			IGISPointSequence * ps = (h == -1 ? fac->GetOuterRing() : fac->GetNthHole(h));
			for(int i = 0; i < ps->GetNumSides(); ++i)
			{
				vector<Point2>	pts;
				
				SideToPoints(ps,i,zoomer, pts);
				
				int param = 0;
				if(fac->HasCustomWalls())				
				{
					Segment2	sp;
					Bezier2		bp;
					if(ps->GetSide(gis_Param,i,sp,bp))
						param = bp.p1.x();
					else
						param = sp.p1.x();
				}
				
				float colors[24] = {  1, 0, 0, 0.75,
									1, 1, 0, 0.75,
									0, 1, 0, 0.75,
									0, 1, 1, 0.75,
									0, 0, 1, 0.75,
									1, 0, 1, 0.75};
				glColor4fv(colors + (param % 6) * 4);
				glLineWidth(2);
				glShapeOffset2v(GL_LINES/*GL_LINE_STRIP*/, &*pts.begin(), pts.size(), -3);
				glLineWidth(1);

			}			
		}
	}
};

struct	preview_pol : public preview_polygon {
	WED_PolygonPlacement * pol;	
	IResolver * resolver;
	preview_pol(WED_PolygonPlacement * p, int l, IResolver * r) : preview_polygon(p,l,false), pol(p), resolver(r) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
		ITexMgr *	tman = WED_GetTexMgr(resolver);
		string vpath;
		pol_info_t	pol_info;
	
		pol->GetResource(vpath);
		if(rmgr->GetPol(vpath,pol_info))
		{
			Bbox2	bounds;
			pol->GetBounds(gis_Geo,bounds);
			Point2 centroid = Point2(
				(bounds.xmin()+bounds.xmax())*0.5,
				(bounds.ymin()+bounds.ymax())*0.5);
			setup_pol_texture(tman, pol_info, pol->GetHeading(), false, centroid, g, zoomer);
			preview_polygon::draw_it(zoomer,g,mPavementAlpha);
			kill_pol_texture();
		}	

	}
};

struct	preview_ortho : public preview_polygon {
	WED_DrapedOrthophoto * orth;	
	IResolver * resolver;
	preview_ortho(WED_DrapedOrthophoto * o, int l, IResolver * r) : preview_polygon(o,l,true), orth(o), resolver(r) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
		ITexMgr *	tman = WED_GetTexMgr(resolver);
		string vpath;
		pol_info_t	pol_info;
	
		orth->GetResource(vpath);
		if(rmgr->GetPol(vpath,pol_info))
		{
			Bbox2	bounds;
			orth->GetBounds(gis_Geo,bounds);
			Point2 centroid = Point2(
				(bounds.xmin()+bounds.xmax())*0.5,
				(bounds.ymin()+bounds.ymax())*0.5);
			setup_pol_texture(tman, pol_info, 0.0, true, centroid, g, zoomer);
			preview_polygon::draw_it(zoomer,g,mPavementAlpha);
			kill_pol_texture();
		}	
	}
};


struct	preview_object : public WED_PreviewItem {
	WED_ObjPlacement * obj;	
	int	preview_level;
	IResolver * resolver;
	preview_object(WED_ObjPlacement * o, int l, int pl, IResolver * r) : WED_PreviewItem(l), obj(o), resolver(r), preview_level(pl) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
		ITexMgr *	tman = WED_GetTexMgr(resolver);
		ILibrarian * lmgr = WED_GetLibrarian(resolver);
		string vpath;

		obj->GetResource(vpath);
		XObj8 * o;
		#if AIRPORT_ROUTING
		agp_t agp;
		#endif
		if(rmgr->GetObj(vpath,o))
		{
			g->SetState(false,1,false,false,true,false,false);
			glColor3f(1,1,1);
			Point2 loc;
			obj->GetLocation(gis_Geo,loc);
			draw_obj_at_ll(tman, o, loc, obj->GetHeading(), g, zoomer);
		}
		#if AIRPORT_ROUTING
		else if (rmgr->GetAGP(vpath,agp))
		{
			g->SetState(false,1,false,false,true,false,false);
			TexRef	ref = tman->LookupTexture(agp.base_tex.c_str() ,true, tex_Linear|tex_Mipmap);			
			int id1 = ref  ? tman->GetTexID(ref ) : 0;
			if(id1)g->BindTex(id1,0);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			Point2 loc;
			obj->GetLocation(gis_Geo,loc);
			loc = zoomer->LLToPixel(loc);
			float r = obj->GetHeading();
			glTranslatef(loc.x(),loc.y(),0.0);
			float ppm = zoomer->GetPPM();
			glScalef(ppm,ppm,0.001);
			glRotatef(90, 1,0,0);
			glRotatef(r, 0, -1, 0);
			glColor3f(1,1,1);
			if(!agp.tile.empty())
			{
				glDisable(GL_CULL_FACE);
				glBegin(GL_TRIANGLE_FAN);
				for(int n = 0; n < agp.tile.size(); n += 4)
				{
					glTexCoord2f(agp.tile[n+2],agp.tile[n+3]);
					glVertex3f(agp.tile[n],0,-agp.tile[n+1]);
				}
				glEnd();
				glEnable(GL_CULL_FACE);
			}	
			for(vector<agp_t::obj>::iterator o = agp.objs.begin(); o != agp.objs.end(); ++o)
			{
				XObj8 * oo;
				if((o->show_lo+o->show_hi)/2 <= preview_level)
				if(rmgr->GetObjRelative(o->name,vpath,oo))
				{
					draw_obj_at_xyz(tman, oo, o->x,0,-o->y,o->r, g);			
				} 
			}
			
			
//			g->EnableDepth(true,true);
//			glClear(GL_DEPTH_BUFFER_BIT);
//			Obj_DrawStruct ds = { g, id1, id2 };
//			ObjDraw8(*o, 0, &kFuncs, &ds); 
//			g->EnableDepth(false,false);
			glPopMatrix();

		}
		#endif
		else
		{
			Point2 l;
			obj->GetLocation(gis_Geo,l);
			l = zoomer->LLToPixel(l);
			glColor3f(1,0,0);
			GUI_PlotIcon(g,"map_missing_obj.png", l.x(),l.y(),0,1.0);
		}
	}
};


/***************************************************************************************************************************************************
 * DRAWING OBJECT
 ***************************************************************************************************************************************************/

WED_PreviewLayer::WED_PreviewLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) : 
	WED_MapLayer(host, zoomer, resolver), 
	mPavementAlpha(1.0f),
	mObjDensity(6),
	mRunwayLayer(group_RunwaysBegin),
	mTaxiLayer(group_TaxiwaysBegin),
	mShoulderLayer(group_ShouldersBegin)
{
}

WED_PreviewLayer::~WED_PreviewLayer()
{
}

void		WED_PreviewLayer::GetCaps						(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel)
{
	draw_ent_v = true;
	draw_ent_s = false;
	cares_about_sel = false;
}


bool		WED_PreviewLayer::DrawEntityVisualization		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(GetResolver());
		ILibrarian * lmgr = WED_GetLibrarian(GetResolver());
		ITexMgr *	tman = WED_GetTexMgr(GetResolver());
		string vpath;
		pol_info_t	pol_info;

	const char *	sub_class	= entity->GetGISSubtype();
	WED_PolygonPlacement * pol = NULL;
	WED_DrapedOrthophoto * orth = NULL;
	WED_ObjPlacement * obj = NULL;
	WED_ForestPlacement * forst = NULL;
	WED_FacadePlacement * fac = NULL;
	WED_Runway * rwy = NULL;
	WED_Helipad * heli = NULL;
	WED_Taxiway * taxi = NULL;
	WED_Sealane * sea = NULL;
	
	IGISPolygon * gis_poly = NULL;

	/******************************************************************************************************************************
	 * RUNWAYS, HELIPADS, SEALANES, TAIXWAYS, AND OTHER AIRPORT-RELATED GOO
	 ******************************************************************************************************************************/

	if (sub_class == WED_Runway::sClass)			rwy = SAFE_CAST(WED_Runway,entity);
	if (sub_class == WED_Helipad::sClass)			heli = SAFE_CAST(WED_Helipad,entity);
	if (sub_class == WED_Sealane::sClass)			sea = SAFE_CAST(WED_Sealane,entity);
	if (sub_class == WED_Taxiway::sClass)			taxi = SAFE_CAST(WED_Taxiway,entity);

	float							storage[4];

	if(rwy)		mPreviewItems.push_back(new preview_runway(rwy,mRunwayLayer++,0));
	if(rwy)		mPreviewItems.push_back(new preview_runway(rwy,mShoulderLayer++,1));
	if(heli)	mPreviewItems.push_back(new preview_helipad(heli,mRunwayLayer++));
	if(sea)		mPreviewItems.push_back(new preview_sealane(sea,mRunwayLayer++));
	if(taxi)	mPreviewItems.push_back(new preview_taxiway(taxi,mTaxiLayer++));

	/******************************************************************************************************************************
	 * POLYGON PREVIEW: forests, facades, polygons (ortho and landuse)
	 ******************************************************************************************************************************/

	if (sub_class == WED_PolygonPlacement::sClass)	gis_poly = pol = SAFE_CAST(WED_PolygonPlacement, entity);
	if (sub_class == WED_DrapedOrthophoto::sClass)	gis_poly = orth = SAFE_CAST(WED_DrapedOrthophoto, entity);
	if (sub_class == WED_FacadePlacement::sClass)	gis_poly = fac = SAFE_CAST(WED_FacadePlacement, entity);
	if (sub_class == WED_ForestPlacement::sClass)	gis_poly = forst = SAFE_CAST(WED_ForestPlacement, entity);

	int lg = group_TaxiwaysBegin;
	if(pol)	pol->GetResource(vpath);
	if(orth) orth->GetResource(vpath);
	if(!vpath.empty())
	if(rmgr->GetPol(vpath,pol_info))
	if(!pol_info.group.empty())
		lg = layer_group_for_string(pol_info.group.c_str(),pol_info.group_offset, lg);
	
	if(pol)		mPreviewItems.push_back(new preview_pol(pol,lg, GetResolver()));
	if(orth)	mPreviewItems.push_back(new preview_ortho(orth,lg, GetResolver()));
	if(fac)		if(fac->GetShowLevel() <= mObjDensity) mPreviewItems.push_back(new preview_facade(fac,group_Objects));
	if(forst)	
#if AIRPORT_ROUTING
	if(forst->GetGISClass() == gis_Polygon)
#endif
		mPreviewItems.push_back(new preview_forest(forst,group_Objects));
	

	/******************************************************************************************************************************
	 * OBJECT preview
	 ******************************************************************************************************************************/

	if (sub_class == WED_ObjPlacement::sClass && (obj = SAFE_CAST(WED_ObjPlacement, entity)) != NULL)
	if(obj->GetShowLevel() <= mObjDensity) 	
	mPreviewItems.push_back(new preview_object(obj,group_Objects, mObjDensity, GetResolver()));

	return true;
}

void		WED_PreviewLayer::DrawVisualization			(bool inCurent, GUI_GraphState * g)
{
	// This is called after per-entity visualization; we have one preview item for everything we need.
	// sort, draw, nuke 'em.

	sort(mPreviewItems.begin(),mPreviewItems.end(),sort_item_by_layer());
	for(vector<WED_PreviewItem *>::iterator i = mPreviewItems.begin(); i != mPreviewItems.end(); ++i)
	{
		(*i)->draw_it(GetZoomer(), g, mPavementAlpha);
		delete *i;
	}
	mPreviewItems.clear();
	mRunwayLayer=	group_RunwaysBegin;
	mTaxiLayer=		group_TaxiwaysBegin;
	mShoulderLayer=	group_ShouldersBegin;
	
}

void		WED_PreviewLayer::SetPavementTransparency(float alpha)
{
	mPavementAlpha = alpha;
	GetHost()->Refresh();
}

float		WED_PreviewLayer::GetPavementTransparency(void) const
{
	return mPavementAlpha;
}

void		WED_PreviewLayer::SetObjDensity(int d)
{
	mObjDensity = d;
	GetHost()->Refresh();
}

int			WED_PreviewLayer::GetObjDensity(void) const
{
	return mObjDensity;
}
	