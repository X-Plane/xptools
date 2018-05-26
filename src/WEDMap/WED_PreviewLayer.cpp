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
#include "WED_LibraryMgr.h"
#include "WED_TexMgr.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
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
#include "WED_LinePlacement.h"
#include "WED_AirportChain.h"
#include "WED_AirportNode.h"
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
#include "XESConstants.h"
#include "WED_TruckParkingLocation.h"
#include "WED_EnumSystem.h"
#include "GISUtils.h"

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


static bool setup_pol_texture(ITexMgr * tman, pol_info_t& pol, double heading, bool no_proj, const Point2& centroid, GUI_GraphState * g, WED_MapZoomerNew * z, float alpha)
{
	TexRef	ref = tman->LookupTexture(pol.base_tex.c_str(),true, pol.wrap ? (tex_Compress_Ok|tex_Wrap|tex_Always_Pad) : tex_Compress_Ok|tex_Always_Pad);
	if(ref == NULL) return false;
	int tex_id = tman->GetTexID(ref);

	if (tex_id == 0)
	{
		g->SetState(false,0,false,true,true,false,false);
		glColor4f(0.5,0.5,0.5,alpha);
		return true;
	}
	g->SetState(false,1,false,!pol.kill_alpha,!pol.kill_alpha,false,false);
	glColor4f(1,1,1,alpha);
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
	TexRef	ref = tman->LookupTexture(o->texture.c_str() ,true, tex_Wrap|tex_Compress_Ok|tex_Always_Pad);			
	TexRef	ref2 = o->texture_draped.empty() ? ref : tman->LookupTexture(o->texture_draped.c_str() ,true, tex_Wrap|tex_Compress_Ok|tex_Always_Pad);
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
	TexRef	ref = tman->LookupTexture(o->texture.c_str() ,true, tex_Wrap|tex_Compress_Ok|tex_Always_Pad);			
	TexRef	ref2 = o->texture_draped.empty() ? ref : tman->LookupTexture(o->texture_draped.c_str() ,true, tex_Wrap|tex_Compress_Ok|tex_Always_Pad);
	int id1 = ref  ? tman->GetTexID(ref ) : 0;
	int id2 = ref2 ? tman->GetTexID(ref2) : 0;
	g->SetTexUnits(1);
	if(id1)g->BindTex(id1,0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(x,y,z);
	glRotatef(r, 0, -1, 0);
	g->EnableDepth(true,true);
	//glClear(GL_DEPTH_BUFFER_BIT);
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
		g->SetState(false,0,false,false,false,false,false);
		glColor3f(
			interp(0,0.1,1,0.0,fst->GetDensity()),
			interp(0,0.5,1,0.3,fst->GetDensity()),
			interp(0,0.1,1,0.0,fst->GetDensity()));

		if(fst->GetFillMode() == dsf_fill_area)
			preview_polygon::draw_it(zoomer,g,mPavementAlpha);
		else if(fst->GetFillMode() == dsf_fill_line)
		{
			IGISPointSequence * ps = fst->GetOuterRing();
			for(int i = 0; i < ps->GetNumSides(); ++i)
			{
				vector<Point2>	pts;
				SideToPoints(ps,i,zoomer, pts);
				glLineWidth(5);
				glShape2v(GL_LINES/*GL_LINE_STRIP*/, &*pts.begin(), pts.size());
				glLineWidth(1);

			}			
		}
	}
};

static void draw_line_preview(const vector<Point2>& pts, const lin_info_t& linfo, int l, double PPM)
{
	double half_width =  (linfo.s2[l]-linfo.s1[l]) / 2.0 * linfo.scale_s * PPM;
	double offset     = ((linfo.s2[l]+linfo.s1[l]) / 2.0 - linfo.sm[l]) * linfo.scale_s * PPM;
	double uv_dt      =  (linfo.s2[l]-linfo.s1[l]) / 2.0 * linfo.scale_s / linfo.scale_t; // correction factor for 'slanted' texture ends
	double uv_t2      = 0.0;                                                              // accumulator for texture t, so each starts where the previous ended

	Vector2	dir2(pts[1],pts[0]);
	dir2.normalize();
	dir2 = dir2.perpendicular_ccw();

	for (int j = 0; j < pts.size()-1; ++j)
	{
		Vector2	dir1(dir2);
		Vector2 dir = Vector2(pts[j+1],pts[j]);
		dir.normalize();
		if(j < pts.size()-2)
		{
			Vector2 dir3(pts[j+2],pts[j+1]);
			dir3.normalize();
			dir2 = (dir + dir3) / (1.0 + dir.dot(dir3));
		}
		else dir2 = dir;
		dir2 = dir2.perpendicular_ccw();

		double uv_t1(uv_t2);
		uv_t2 += sqrt(Vector2(pts[j+1], pts[j]).squared_length()) / PPM / linfo.scale_t;
		double d1 = uv_dt * dir.dot(dir1);
		double d2 = uv_dt * dir.dot(dir2);

		glBegin(GL_QUADS);
			glTexCoord2f(linfo.s1[l],uv_t2 + d2); glVertex2(pts[j+1] + dir2 * (offset - half_width));
			glTexCoord2f(linfo.s1[l],uv_t1 + d1); glVertex2(pts[j]   + dir1 * (offset - half_width));
			glTexCoord2f(linfo.s2[l],uv_t1 - d1); glVertex2(pts[j]   + dir1 * (offset + half_width));
			glTexCoord2f(linfo.s2[l],uv_t2 - d2); glVertex2(pts[j+1] + dir2 * (offset + half_width));
		glEnd();
	}
}

struct	preview_line : WED_PreviewItem {
	WED_LinePlacement * lin;
	IResolver * resolver;
	preview_line(WED_LinePlacement * ln, int l, IResolver * r) : WED_PreviewItem(l), lin(ln), resolver(r) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
		string vpath;
		lin_info_t linfo;
		lin->GetResource(vpath);
		if (!rmgr->GetLin(vpath,linfo)) return;

		ITexMgr *	tman = WED_GetTexMgr(resolver);
		TexRef tref = tman->LookupTexture(linfo.base_tex.c_str(),true,tex_Compress_Ok);
		int tex_id = 0;
		if(tref) tex_id = tman->GetTexID(tref);

		if(tex_id)
		{
			g->SetState(false,1,false,true,true,false,false);
			g->BindTex(tex_id,0);
			glColor3f(1,1,1);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		IGISPointSequence * ps = SAFE_CAST(IGISPointSequence,lin);
		if(ps)
			if(linfo.eff_width * zoomer->GetPPM() < 5.0 || !tex_id)             // cutoff size for real preview
			{
				g->SetState(false,0,false,false,false,false,false);
				
				int locked = 0;
				WED_Entity * thing = dynamic_cast<WED_Entity *>(lin);
				while(thing)
				{
					if(thing->GetLocked())	{ locked=1; break; }
					thing = dynamic_cast<WED_Entity *>(thing->GetParent());
				}
				if (locked)
					glColor3fv(linfo.rgb);
				else                           // do some color correction to account for the green vs grey line
					glColor3f(min(1.0,linfo.rgb[0]+0.2),max(0.0,linfo.rgb[1]-0.0),min(1.0,linfo.rgb[2]+0.2));
					
				for(int i = 0; i < lin->GetNumSides(); ++i)
				{
					vector<Point2>	pts;
					SideToPoints(ps,i,zoomer, pts);
					glLineWidth(3);
					glShape2v(GL_LINES, &*pts.begin(), pts.size());
					glLineWidth(1);
				}
			}
			else
			{
				glFrontFace(GL_CCW);
				for (int l = 0; l < linfo.s1.size(); ++l)
				{
					vector<Point2>	pts;
					vector<int> cont;
					PointSequenceToVector(ps,zoomer,pts,false,cont,0);
					draw_line_preview(pts, linfo, l, zoomer->GetPPM());
				}
				glFrontFace(GL_CW);
			}
	}
};

struct	preview_airportchain : WED_PreviewItem {
	WED_AirportChain * chn;
	map<int,lin_info_t> linfo;
	map<int,int> tex_id;
	
	preview_airportchain(WED_AirportChain * c, int l, IResolver * r) : WED_PreviewItem(l), chn(c)
	{ 
	
	// Todo: load only the line types actually needed ?
	
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(r);
		ITexMgr         * tman = WED_GetTexMgr(r);
		WED_LibraryMgr  * lmgr = WED_GetLibraryMgr(r);
		
		for(int i = 1; i < 99; ++i)
		{
			string vpath;
			lin_info_t info;
			if (!lmgr->GetLineVpath(i, vpath)) continue;
			if (!rmgr->GetLin(vpath, info)) continue;
			linfo[i]=info;
			TexRef tref = tman->LookupTexture(info.base_tex.c_str(),true,tex_Compress_Ok);
			if(tref) tex_id[i] = tman->GetTexID(tref);
		}
	}

	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		IGISPointSequence * ps = SAFE_CAST(IGISPointSequence,chn);
		if(ps)
			if(zoomer->GetPPM() > 20.0)             // cutoff size for real preview
			{
				glFrontFace(GL_CCW);
				
				int i = 0;
				while (i < ps->GetNumSides())
				{
					set<int> attrs;
					WED_AirportNode * apt_node = dynamic_cast<WED_AirportNode*>(ps->GetNthPoint(i));
					if (apt_node) apt_node->GetAttributes(attrs);
					
					int t = 0;
					for(set<int>::const_iterator a = attrs.begin(); a != attrs.end(); ++a)
					{
						int n = ENUM_Export(*a);
						if(n < 100)
						{
							t = n;
							break;
						}
					}
					
					if(tex_id.find(t) != tex_id.end())
					{
						vector<Point2> pts;

						g->SetState(false,1,false,true,true,false,false);
						g->BindTex(tex_id[t],0);
						glColor3f(1,1,1);
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

						for ( ; i < ps->GetNumSides(); ++i)
						{
							if (pts.size()) pts.pop_back();
							SideToPoints(ps, i, zoomer, pts);
							
							if(i < ps->GetNumSides()-1) 
							{
								apt_node = dynamic_cast<WED_AirportNode*>(ps->GetNthPoint(i+1));
								if (apt_node) apt_node->GetAttributes(attrs);
								int tn = 0;
								for(set<int>::const_iterator a = attrs.begin(); a != attrs.end(); ++a)
								{
									int n = ENUM_Export(*a);
									if (n < 100)
									{
										tn = n;
										break;
									}
								}
								if (tn != t) { ++i; break; }           // stop, as next segment will need different line type;
							}
						}

						for (int l = 0; l < linfo[t].s1.size(); ++l)
						{
							draw_line_preview(pts, linfo[t], l, zoomer->GetPPM());
						}
					}
					else
						++i; // in case we cang get the attributes, skip to next node. If we dont, we'll loop indefinitely;
				}
				glFrontFace(GL_CW);
			}
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
			glBegin(GL_LINES);
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
					Bezier2		bp;
					ps->GetSide(gis_Param,i,bp);
					param = bp.p1.x();
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
			setup_pol_texture(tman, pol_info, pol->GetHeading(), false, centroid, g, zoomer, mPavementAlpha);
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
		
		//If this ortho is new
		if(orth->IsNew() == true)
		{
			//Set the graph state to default
			g->SetState(false,0,false,false,false,false,false);
			//Get the ring's points
			vector<Point2> pts;
			vector<int> is_hole_start;
			PointSequenceToVector(orth->GetOuterRing(), zoomer, pts, has_uv, is_hole_start, 0);

			//Get the resource string and look it up
			string tempResource = "";
			orth->GetResource(tempResource);
			TexRef ref = tman->LookupTexture(tempResource.c_str(),false,tex_Linear|tex_Mipmap|tex_Rescale);
			
			//If there is no texture exit early
			if(ref == NULL) 
			{
				return;
			}
			
			//Get the tex_id
			int tex_id = tman->GetTexID(ref);

			if (tex_id == 0)
			{
				g->SetState(false,0,false,true,true,false,false);
				glColor3f(0.5,0.5,0.5);
			}
			else
			{
				//Set up the texture
				g->SetState(false,1,false,true,true,false,false);
				glColor3f(1,1,1);
				g->BindTex(tex_id,0);
			}

			if(true)
			{
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
			}
			preview_polygon::draw_it(zoomer,g,mPavementAlpha);
		}
		else
		{
			//Code for use with current polygon tool creation style
			orth->GetResource(vpath);
			if(rmgr->GetPol(vpath,pol_info))
			{
				Bbox2	bounds;
				orth->GetBounds(gis_Geo,bounds);
				Point2 centroid = Point2(
					(bounds.xmin()+bounds.xmax())*0.5,
					(bounds.ymin()+bounds.ymax())*0.5);
				setup_pol_texture(tman, pol_info, 0.0, true, centroid, g, zoomer, mPavementAlpha);
				preview_polygon::draw_it(zoomer,g,mPavementAlpha);
				kill_pol_texture();
			}
		}
	}
};

static bool cull_tile(WED_MapZoomerNew * zoomer, double x1, double z1, double x2, double z2, const Point2& ll, double heading)
{
	double cs = cos(heading * DEG_TO_RAD);
	double ss = sin(heading * DEG_TO_RAD);
	
	double p[8] = {
		x1,	z1,
		x1,	z2,
		x2,	z2,
		x2,	z1 };

	double y_scale = zoomer->GetPPM();
	double x_scale = zoomer->GetPPM();
	
	for(int i = 0; i < 8; i += 2)
	{
		double x = p[i  ];
		double z = p[i+1];
		
		p[i  ] = (x * cs + z * ss) *  x_scale;
		p[i+1] = (z * cs - x * ss) *  y_scale;		// invert Y here...Z = down in obj space frmo above...
		
	}
	
	double x_min = min(min(p[0],p[2]),min(p[4],p[6]));
	double x_max = max(max(p[0],p[2]),max(p[4],p[6]));
	double z_min = min(min(p[1],p[3]),min(p[5],p[7]));
	double z_max = max(max(p[1],p[3]),max(p[5],p[7]));

	if(x_max - x_min < 5 && z_max - z_min < 5)
		return true;

	double bounds_pix[4];
	zoomer->GetPixelBounds(bounds_pix[0],bounds_pix[1],bounds_pix[2],bounds_pix[3]);
	Point2 xy = zoomer->LLToPixel(ll);
	x_min += xy.x();
	x_max += xy.x();
	z_min += xy.y();
	z_max += xy.y();
	return 
		x_max < bounds_pix[0] ||
		z_max < bounds_pix[1] ||
		x_min > bounds_pix[2] ||
		z_min > bounds_pix[3];
}

bool cull_obj(WED_MapZoomerNew * zoomer, XObj8 * obj, const Point2& ll, double heading)
{
	return cull_tile(
		zoomer,
		obj->xyz_min[0],-obj->xyz_max[2],
		obj->xyz_max[0],-obj->xyz_min[2],
		ll,heading);
}

#if AIRPORT_ROUTING
bool cull_agp(WED_MapZoomerNew * zoomer, agp_t * agp, const Point2& ll, double heading)
{
	double x_min, x_max, z_min, z_max;
	x_min = x_max = agp->tile[0];
	z_min = z_max = agp->tile[1];
	for(int n = 4; n < agp->tile.size(); n += 4)
	{
		x_min = min(x_min,agp->tile[n]);
		x_max = max(x_max,agp->tile[n]);
		z_min = min(z_min,agp->tile[n+1]);
		z_max = max(z_max,agp->tile[n+1]);
	}

	return cull_tile(
		zoomer,
		x_min,z_min,
		x_max,z_max,
		ll,heading);

}
#endif

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
			if(!cull_obj(zoomer,o, loc, obj->GetHeading()))
			{
				draw_obj_at_ll(tman, o, loc, obj->GetHeading(), g, zoomer);
			}
		}
		#if AIRPORT_ROUTING
		else if (rmgr->GetAGP(vpath,agp))
		{
			Point2 loc;
			obj->GetLocation(gis_Geo,loc);
			if(!cull_agp(zoomer, &agp, loc, obj->GetHeading()))
			{
				g->SetState(false,1,false,true,true,false,false);
				TexRef	ref = tman->LookupTexture(agp.base_tex.c_str() ,true, tex_Linear|tex_Mipmap|tex_Compress_Ok|tex_Always_Pad);			
				int id1 = ref  ? tman->GetTexID(ref ) : 0;
				if(id1)g->BindTex(id1,0);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				loc = zoomer->LLToPixel(loc);
				float r = obj->GetHeading();
				glTranslatef(loc.x(),loc.y(),0.0);
				float ppm = zoomer->GetPPM();
				glScalef(ppm,ppm,0.001);
				glRotatef(90, 1,0,0);
				glRotatef(r, 0, -1, 0);
				glColor3f(1,1,1);
				g->EnableDepth(true,true);
				glClear(GL_DEPTH_BUFFER_BIT);
				if(!agp.tile.empty() && !agp.hide_tiles)
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

struct	preview_truck : public WED_PreviewItem {
	WED_TruckParkingLocation * trk;
	int	preview_level;
	IResolver * resolver;
	preview_truck(WED_TruckParkingLocation * o, int l, int pl, IResolver * r) : WED_PreviewItem(l), trk(o), resolver(r), preview_level(pl) { }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float mPavementAlpha)
	{
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
		ITexMgr *	tman = WED_GetTexMgr(resolver);
		ILibrarian * lmgr = WED_GetLibrarian(resolver);
		string vpath1, vpath2;

		switch(trk->GetTruckType()) {
		case atc_ServiceTruck_Baggage_Loader:		vpath1 = "lib/airport/vehicles/baggage_handling/belt_loader.obj";break;
		case atc_ServiceTruck_Baggage_Train:		vpath1 = "lib/airport/vehicles/baggage_handling/tractor.obj";
													vpath2 = "lib/airport/vehicles/baggage_handling/bag_cart.obj";	break;
		case atc_ServiceTruck_Crew_Limo:
		case atc_ServiceTruck_Crew_Car:				vpath1 = "lib/airport/vehicles/servicing/crew_car.obj";			break;
		case atc_ServiceTruck_Crew_Ferrari:			vpath1 = "lib/airport/vehicles/servicing/crew_ferrari.obj";		break;
		case atc_ServiceTruck_Food:					vpath1 = "lib/airport/vehicles/servicing/catering_truck.obj";	break;
		case atc_ServiceTruck_FuelTruck_Jet:		vpath1 = "lib/airport/vehicles/servicing/fuel_truck_large.obj";	break;
		case atc_ServiceTruck_FuelTruck_Liner:		vpath1 = "lib/airport/vehicles/fuel/hyd_disp_truck.obj";		break;
		case atc_ServiceTruck_FuelTruck_Prop:		vpath1 = "lib/airport/vehicles/servicing/fuel_truck_small.obj";	break;
		case atc_ServiceTruck_Ground_Power_Unit:	vpath1 = "lib/airport/vehicles/baggage_handling/tractor.obj";
													vpath2 = "lib/airport/vehicles/servicing/GPU.obj";				break;
		case atc_ServiceTruck_Pushback:				vpath1 = "lib/airport/vehicles/pushback/tug.obj";				break;
		}

		XObj8 * o1 = NULL, * o2 = NULL;
		#if AIRPORT_ROUTING
		agp_t agp;
		#endif
		if(!vpath1.empty() && rmgr->GetObj(vpath1,o1))
		{
			g->SetState(false,1,false,false,true,false,false);
			glColor3f(1,1,1);
			Point2 loc;
			trk->GetLocation(gis_Geo,loc);
			double trk_heading = trk->GetHeading();
			if(!cull_obj(zoomer,o1, loc, trk_heading))
			{
				draw_obj_at_ll(tman, o1, loc, trk_heading, g, zoomer);
			}

			if(trk->GetTruckType() == atc_ServiceTruck_Baggage_Train)
			{
				rmgr->GetObj(vpath2,o2);
				if(o2)
				{
					double gap = 3.899;
					Vector2 dirv(sin(trk_heading * DEG_TO_RAD),
								 cos(trk_heading * DEG_TO_RAD));
					
					Vector2 llv = VectorMetersToLL(loc, dirv);
				
					
					for(int c = 0; c < trk->GetNumberOfCars(); ++c)
					{
						loc -= (llv * gap);
						draw_obj_at_ll(tman, o2, loc, trk_heading, g, zoomer);
						gap = 3.598;
					}
				}
			}
			if(trk->GetTruckType() == atc_ServiceTruck_Ground_Power_Unit)
			{
				rmgr->GetObj(vpath2,o2);
				if(o2)
				{
					double gap = 4.247;
					Vector2 dirv(sin(trk_heading * DEG_TO_RAD),
								 cos(trk_heading * DEG_TO_RAD));
					
					Vector2 llv = VectorMetersToLL(loc, dirv);
				
					
					loc -= (llv * gap);
					draw_obj_at_ll(tman, o2, loc, trk_heading, g, zoomer);
				}
			}
		}
		else
		{
			Point2 l;
			trk->GetLocation(gis_Geo,l);
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

void		WED_PreviewLayer::GetCaps						(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = true;
	draw_ent_s = false;
	cares_about_sel = false;
	wants_clicks = false;
}


bool		WED_PreviewLayer::DrawEntityVisualization		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(GetResolver());
		ILibrarian * lmgr = WED_GetLibrarian(GetResolver());
		string vpath;
		pol_info_t	pol_info;

	const char *	sub_class	= entity->GetGISSubtype();
	WED_PolygonPlacement * pol = NULL;
	WED_DrapedOrthophoto * orth = NULL;
	WED_ObjPlacement * obj = NULL;
	WED_TruckParkingLocation * trk = NULL;
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

	if(rwy)		mPreviewItems.push_back(new preview_runway(rwy,mRunwayLayer++,0));
	if(rwy)		mPreviewItems.push_back(new preview_runway(rwy,mShoulderLayer++,1));
	if(heli)	mPreviewItems.push_back(new preview_helipad(heli,mRunwayLayer++));
	if(sea)		mPreviewItems.push_back(new preview_sealane(sea,mRunwayLayer++));
	if(taxi)	mPreviewItems.push_back(new preview_taxiway(taxi,mTaxiLayer++));

	/******************************************************************************************************************************
	 * POLYGON & LINE PREVIEW: forests, facades, polygons (ortho and landuse)
	 ******************************************************************************************************************************/

	if (sub_class == WED_PolygonPlacement::sClass)	gis_poly = pol = SAFE_CAST(WED_PolygonPlacement, entity);
	if (sub_class == WED_DrapedOrthophoto::sClass)	gis_poly = orth = SAFE_CAST(WED_DrapedOrthophoto, entity);
	if (sub_class == WED_FacadePlacement::sClass)	gis_poly = fac = SAFE_CAST(WED_FacadePlacement, entity);
	if (sub_class == WED_ForestPlacement::sClass)	gis_poly = forst = SAFE_CAST(WED_ForestPlacement, entity);

	int lg = group_TaxiwaysBegin;
	if(pol)	pol->GetResource(vpath);
	if(orth) orth->GetResource(vpath);
	
	if(!vpath.empty() && rmgr->GetPol(vpath,pol_info) && !pol_info.group.empty())
			lg = layer_group_for_string(pol_info.group.c_str(),pol_info.group_offset, lg);
	if(pol)
		mPreviewItems.push_back(new preview_pol(pol,lg, GetResolver()));
	if(orth)
		mPreviewItems.push_back(new preview_ortho(orth,lg, GetResolver()));
	if(fac && fac->GetShowLevel() <= mObjDensity)
		mPreviewItems.push_back(new preview_facade(fac,group_Objects));
	if(forst)
		mPreviewItems.push_back(new preview_forest(forst, group_Objects));

	if(sub_class == WED_LinePlacement::sClass)
	{
		WED_LinePlacement * line = SAFE_CAST(WED_LinePlacement, entity);
		if(line)
			mPreviewItems.push_back(new preview_line(line, group_Markings, GetResolver()));
	}
	else if(sub_class == WED_AirportChain::sClass)
	{
		WED_AirportChain * chn = SAFE_CAST(WED_AirportChain, entity);
		if(chn)
			mPreviewItems.push_back(new preview_airportchain(chn, group_Markings, GetResolver()));
	}

	/******************************************************************************************************************************
	 * OBJECT preview
	 ******************************************************************************************************************************/

	if (sub_class == WED_ObjPlacement::sClass && (obj = SAFE_CAST(WED_ObjPlacement, entity)) != NULL)
		if(obj->GetShowLevel() <= mObjDensity) 	
			mPreviewItems.push_back(new preview_object(obj,group_Objects, mObjDensity, GetResolver()));

	if (sub_class == WED_TruckParkingLocation::sClass && (trk = SAFE_CAST(WED_TruckParkingLocation, entity)) != NULL)
		mPreviewItems.push_back(new preview_truck(trk, group_Objects, mObjDensity, GetResolver()));

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
	
