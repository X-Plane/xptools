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
#include "WED_ToolUtils.h"
#include "WED_MapZoomerNew.h"
#include "WED_DrawUtils.h"
#include "WED_ObjPlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_FacadePlacement.h"
#include "TexUtils.h"
#include "ObjDraw.h"
#include "XObjDefs.h"
#include "MathUtils.h"
#include "WED_UIDefs.h"

#if APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

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

WED_PreviewLayer::WED_PreviewLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) : WED_MapLayer(host, zoomer, resolver)
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
	IGISPolygon * gis_poly = NULL;

	/******************************************************************************************************************************
	 * POLYGON PREVIEW: forests, facades, polygons (ortho and landuse)
	 ******************************************************************************************************************************/

	if (sub_class == WED_PolygonPlacement::sClass)	gis_poly = pol = SAFE_CAST(WED_PolygonPlacement, entity);
	if (sub_class == WED_DrapedOrthophoto::sClass)	gis_poly = orth = SAFE_CAST(WED_DrapedOrthophoto, entity);
	if (sub_class == WED_FacadePlacement::sClass)	gis_poly = fac = SAFE_CAST(WED_FacadePlacement, entity);
	if (sub_class == WED_ForestPlacement::sClass)	gis_poly = forst = SAFE_CAST(WED_ForestPlacement, entity);

	if(pol)
	{
		pol->GetResource(vpath);
		if(!rmgr->GetPol(vpath,pol_info))
			gis_poly = pol = NULL;
	}
	if(orth)
	{
		orth->GetResource(vpath);
		if(!rmgr->GetPol(vpath,pol_info))
			gis_poly = orth = NULL;
	}

	if(gis_poly && gis_poly->GetGISClass() == gis_Polygon)
	{
			vector<Point2>	pts;
			vector<int>		is_hole_start;

		PointSequenceToVector(gis_poly->GetOuterRing(), GetZoomer(), pts, orth != NULL, is_hole_start, 0);
		int n = gis_poly->GetNumHoles();
		for (int i = 0; i < n; ++i)
			PointSequenceToVector(gis_poly->GetNthHole(i), GetZoomer(), pts, orth != NULL, is_hole_start, 1);

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

			// TODO: forest, facade

			if(fac)
			{
				g->SetState(false,0,false,false, false,false,false);
				glColor3f(0.7,0.7,0.7);
			}
			if(forst)
			{
				g->SetState(false,0,false,false,false,false,false);
				glColor3f(
					interp(0,0.1,1,0.0,forst->GetDensity()),
					interp(0,0.5,1,0.3,forst->GetDensity()),
					interp(0,0.1,1,0.0,forst->GetDensity()));
			}

			if(pol || orth)
			{
				setup_pol_texture(tman, pol_info, pol ? pol->GetHeading() : 0.0, orth != NULL, centroid, g, GetZoomer());
			}
			glFrontFace(GL_CCW);
			glPolygon2(&*pts.begin(), orth != NULL, &*is_hole_start.begin(), pts.size() / (orth != NULL ? 2 : 1));
			glFrontFace(GL_CW);
			kill_pol_texture();
		}
	}
	
	if(fac && fac->GetTopoMode() < 2)
	{
		vector<Point2>	pts;
		vector<int>		is_hole_start;

		PointSequenceToVector(gis_poly->GetOuterRing(), GetZoomer(), pts, orth != NULL, is_hole_start, 0);

		glColor3f(1,1,1);
		glLineWidth(3);
		glBegin(GL_LINE_STRIP);
		glVertex2(pts[0]);
		glVertex2(pts[1]);
		glEnd();
		glLineWidth(1);
	}
	if(fac && fac->HasCustomWalls())
	{
		for(int h = -1; h < fac->GetNumHoles(); ++h)
		{
			IGISPointSequence * ps = (h == -1 ? fac->GetOuterRing() : fac->GetNthHole(h));
			for(int i = 0; i < ps->GetNumSides(); ++i)
			{
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
				
				Segment2	sp;
				Bezier2		bp;
				int param;
				if(ps->GetSide(gis_Param,i,sp,bp))
					param = bp.p1.x();
				else
					param = sp.p1.x();
				
				float colors[24] = {  1, 0, 0, 0.75,
									1, 1, 0, 0.75,
									0, 1, 0, 0.75,
									0, 1, 1, 0.75,
									0, 0, 1, 0.75,
									1, 0, 1, 0.75};
				glColor4fv(colors + (param % 6) * 4);
				glLineWidth(2);
				glShapeOffset2v(GL_LINE_STRIP, &*pts.begin(), pts.size(), -3);
				glLineWidth(1);

			}
			
		}
	}

	/******************************************************************************************************************************
	 * OBJECT preview
	 ******************************************************************************************************************************/

	if (sub_class == WED_ObjPlacement::sClass && (obj = SAFE_CAST(WED_ObjPlacement, entity)) != NULL)
	{
		obj->GetResource(vpath);
		XObj8 * o;
		if(rmgr->GetObj(vpath,o))
		{
			TexRef	ref = tman->LookupTexture(o->texture.c_str() ,true, tex_Wrap);			
			TexRef	ref2 = o->texture_draped.empty() ? ref : tman->LookupTexture(o->texture_draped.c_str() ,true, tex_Wrap);
			int id1 = ref  ? tman->GetTexID(ref ) : 0;
			int id2 = ref2 ? tman->GetTexID(ref2) : 0;
			g->SetTexUnits(1);
			if(id1)g->BindTex(id1,0);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			Point2 loc;
			obj->GetLocation(gis_Geo,loc);
			loc = GetZoomer()->LLToPixel(loc);
			float r = obj->GetHeading();
			glTranslatef(loc.x(),loc.y(),0.0);
			float ppm = GetZoomer()->GetPPM();
			glScalef(ppm,ppm,0.0001);
			glRotatef(90, 1,0,0);
			glRotatef(r, 0, -1, 0);
			g->EnableDepth(true,true);
			glClear(GL_DEPTH_BUFFER_BIT);
			Obj_DrawStruct ds = { g, id1, id2 };
			ObjDraw8(*o, 0, &kFuncs, &ds); 
			g->EnableDepth(false,false);
			glPopMatrix();
		}
	}
	return true;
}
