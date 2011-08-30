/*
 * Copyright (c) 2004, Laminar Research.
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
#include "RF_DrawMap.h"
#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif
#include "ParamDefs.h"
#include "MapAlgs.h"
#include "DEMTables.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "XESConstants.h"
#include "ObjTables.h"
#include "GISUtils.h"

// Set this to 1 to not render faces...we can save a lot of memory this way for a huge XES that won't otherwise fit into RAM.
#define NO_FACE_RENDER 0

#define DRAW_FACES 1
#define DRAW_FEATURES 1
#define DRAW_EDGES 1
#define DRAW_VERTICES 1
#define DRAW_FOOTPRINTS 1

void	IndexPmwx(Pmwx& pmwx, PmwxIndex_t& index)
{
	index.faces.clear();
	vector<pair<Bbox2, Face_handle> >	faces;
	faces.reserve(pmwx.number_of_faces());	
	for(Pmwx::Face_iterator f = pmwx.faces_begin(); f != pmwx.faces_end(); ++f)
	if(!f->is_unbounded())
	{
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = f->outer_ccb();
		Bbox2	bbox(cgal2ben(circ->source()->point()));
		do
		{
			bbox += cgal2ben(circ->source()->point());
		} while(++circ != stop);

		faces.push_back(pair<Bbox2,Face_handle>(bbox, f));
	}	
	index.faces.insert(faces.begin(),faces.end());
	faces.clear();
	trim(faces);
	
	index.halfedges.clear();
	vector<pair<Bbox2, Halfedge_handle> >	halfedges;
	halfedges.reserve(pmwx.number_of_edges());	
	for(Pmwx::Edge_iterator e = pmwx.edges_begin(); e != pmwx.edges_end(); ++e)
	{
		Bbox2	bbox(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()));
		halfedges.push_back(pair<Bbox2,Halfedge_handle>(bbox, e));
	}	
	index.halfedges.insert(halfedges.begin(),halfedges.end());	
	halfedges.clear();
	trim(halfedges);
	
	index.vertices.clear();
	vector<pair<Bbox2, Vertex_handle> > vertices;
	vertices.reserve(pmwx.number_of_vertices());	
	for(Pmwx::Vertex_iterator v = pmwx.vertices_begin(); v != pmwx.vertices_end(); ++v)
	{
		Bbox2	bbox(cgal2ben(v->point()));
		
		vertices.push_back(pair<Bbox2,Vertex_handle>(bbox, v));
	}	
	index.vertices.insert(vertices.begin(),vertices.end());	
	vertices.clear();
	trim(vertices);
	
}




#if APL || LIN
#define STDCALL_MACRO
#elif IBM
#define STDCALL_MACRO __stdcall
#endif




vector<const float*> *	gAccum;
int						gMode;
int						gCount;
const float*			gFirst;
const float*			gPrev;
const float*			gPrevPrev;

void PRECALC_Begin (GLenum mode)
{
	gMode = mode;
	gCount = 0;
}

void PRECALC_Vertex2fv (const GLfloat *v)
{
	if (gMode == GL_TRIANGLES)
	{
		gAccum->push_back(v);
	}
	if (gMode == GL_TRIANGLE_FAN)
	{
		if (gCount == 0) 	{ 	gFirst=v;					}
		if (gCount > 1)		{ 	gAccum->push_back(gFirst);
								gAccum->push_back(gPrev);
								gAccum->push_back(v);		}
		gPrev = v;
	}
	if (gMode == GL_TRIANGLE_STRIP)
	{
		if (gCount > 1 && (gCount % 2)==1) {	gAccum->push_back(gPrev);
												gAccum->push_back(gPrevPrev);
												gAccum->push_back(v); }
		if (gCount > 1 && (gCount % 2)==0) {	gAccum->push_back(gPrevPrev);
												gAccum->push_back(gPrev);
												gAccum->push_back(v); }
		gPrevPrev = gPrev;
		gPrev = v;
	}
	++gCount;
}

void PRECALC_End (void)
{
}



void draw_he(Halfedge_handle e)
{
	Bezier2	b;
	b.p1 = cgal2ben(e->source()->point());
	b.p2 = cgal2ben(e->target()->point());
	b.c1=b.p1;
	b.c2=b.p2;

	if(e->source()->degree() == 2)
	{
		Vector2	vs(b.p1,b.p2);
		Halfedge_handle prev(e->twin()->next());
		Vector2 vp(cgal2ben(prev->source()->point()), cgal2ben(prev->target()->point()));
		if(!CGAL::collinear(prev->target()->point(),e->source()->point(),e->target()->point()))
		if(vs.dot(vp) < 0.0)
		{
			vs.normalize();
			vp.normalize();
			Vector2 n(vs+vp);
			n.normalize();
			vp*=-1.0;
			n*=-1.0;
			if(vp.left_turn(vs))
				n=n.perpendicular_ccw();
			else
				n=n.perpendicular_cw();

			b.c1 = b.p1 + n * (sqrt(Segment2(b.p1,b.p2).squared_length()) / 3.0);
		}
	}
	if(e->target()->degree() == 2)
	{
		Vector2	vs(b.p2,b.p1);
		Halfedge_handle next(e->next());
		Vector2 vn(cgal2ben(next->source()->point()), cgal2ben(next->target()->point()));
		if(!CGAL::collinear(e->source()->point(),e->target()->point(),next->target()->point()))
		if(vs.dot(vn) < 0.0)
		{
			vs.normalize();
			vn.normalize();
			Vector2 n(vs+vn);
			n.normalize();
			vn*=-1.0;
			n*=-1.0;
			if(vn.left_turn(vs))
				n=n.perpendicular_ccw();
			else
				n=n.perpendicular_cw();

			b.c2 = b.p2 + n * (sqrt(Segment2(b.p2,b.p1).squared_length()) / 3.0);
		}

	}

/*
	glVertex2f(b.p1.x_,b.p1.y_);
	glVertex2f(b.p2.x_,b.p2.y_);

	glVertex2f(b.p1.x_,b.p1.y_);
	glVertex2f(b.c1.x_,b.c1.y_);

	glVertex2f(b.p2.x_,b.p2.y_);
	glVertex2f(b.c2.x_,b.c2.y_);

*/
	float c = 10.0;
	if(b.p1 == b.c1 && b.p2 == b.c2)
	c=1.0;

//	if(XPLMGetModifiers() & xplm_ShiftFlag)
//		c=1.0;

	for(float s = 0.0; s < c; ++s)
	{
		Point2	p(b.midpoint(s/c));
		glVertex2f(p.x_,p.y_);
		p = b.midpoint((s+1.0)/c);
		glVertex2f(p.x_,p.y_);
	}

}

inline void SetColor(GLubyte c[3], float r, float g, float b)
{
	c[0] = r*255.0f; c[1] = g*255.0f; c[2] = b*255.0f;
}

static void	SetColorForHalfedge(Pmwx::Halfedge_const_handle i, GLubyte color[3])
{
	int	terrainChange = !i->face()->data().TerrainMatch(i->twin()->face()->data());
	int	border = i->face()->is_unbounded() || i->twin()->face()->is_unbounded();
	int river = i->data().mParams.find(he_IsRiver) != i->data().mParams.end();
	int dryriver = i->data().mParams.find(he_IsDryRiver) != i->data().mParams.end();

	int	wet = i->face()->data().IsWater() || i->twin()->face()->data().IsWater();

//	bool beach = i->mTransition != 0;

	if (border)
		SetColor(color,1.0, 0.0, 1.0);

	int ft = NO_VALUE, rt = NO_VALUE;
	for( GISNetworkSegmentVector::const_iterator r = i->data().mSegments.begin(); r != i->data().mSegments.end(); ++r)
	{	
		if(rt == NO_VALUE || r->mRepType < rt)
			rt = r->mRepType;
		if(ft == NO_VALUE || r->mFeatType < ft)
			ft = r->mFeatType;
	}
	for( GISNetworkSegmentVector::const_iterator r = i->twin()->data().mSegments.begin(); r != i->twin()->data().mSegments.end(); ++r)
	{
		if(rt == NO_VALUE || r->mRepType < rt)
			rt = r->mRepType;
		if(ft == NO_VALUE || r->mFeatType < ft)
			ft = r->mFeatType;
	}
	
	if(rt != NO_VALUE || ft != NO_VALUE)
	{
		if(gEnumColors.count(rt) && rt != NO_VALUE)
		{
			RGBColor_t& rgbc = gEnumColors[rt];
			SetColor(color,rgbc.rgb[0],rgbc.rgb[1],rgbc.rgb[2]);
		}
		else
		{
			SetColor(color,1.0, 0.0, 1.0);
			if (Road_IsTrain   (ft))	SetColor(color,0.5, 0.3, 0.1);
			if (Road_IsPowerline(ft))	SetColor(color,1.0, 1.0, 0.0);
			if (Road_IsHighway (ft))	SetColor(color,0.0, 1.0, 1.0);
			if (Road_IsMainDrag(ft))	SetColor(color,1.0, 0.0, 0.0);
			if (Road_IsLocal   (ft))	SetColor(color,0.7, 0.4, 0.4);
			if (Road_IsAccess  (ft))	SetColor(color,0.6, 0.6, 0.6);
			if (Road_IsWalkway (ft))	SetColor(color,0.4, 0.2, 0.1);
			if (Road_IsDam	   (ft))	SetColor(color,1.0, 1.0, 1.0);
		}
	} else {
//		if (beach)
//			SetColor(color,0.8, 0.6, 0.2);
		 if (wet && border)
			SetColor(color,0.5, 0.0, 1.0);
		 else if (wet)
			SetColor(color,0.1, 0.3, 1.0);
		else if (terrainChange)
			SetColor(color,0.7, 0.7, 0.7);
		else if (river)
			SetColor(color,0.0, 0.0, 1.0);
		else if (dryriver)
			SetColor(color, 0.6, 0.6, 0.3);
		else if (border)
			SetColor(color,1.0, 0.0, 1.0);
		else
			SetColor(color,0.3, 0.3, 0.3);
	}
}

static	void	SetColorForFace(Pmwx::Face_const_handle f, GLubyte outColor[4])
{
	outColor[0] = outColor[1] = outColor[2] = outColor[3] = 0;
	
	int our_prop = NO_VALUE;
	if (f->data().mTerrainType != terrain_Natural && f->data().mTerrainType != NO_VALUE)
		our_prop = f->data().mTerrainType;

	if (f->data().mAreaFeature.mFeatType != NO_VALUE)
	if(gEnumColors.count(f->data().mAreaFeature.mFeatType))
		our_prop = f->data().mAreaFeature.mFeatType;
		
	if(f->data().GetZoning() != NO_VALUE)	
	if(gEnumColors.count(f->data().GetZoning()))
		our_prop = f->data().GetZoning();

	if(our_prop != NO_VALUE)
	{
		RGBColor_t& rgbc = gEnumColors[our_prop];

		outColor[0] = rgbc.rgb[0]*255.0f;
		outColor[1] = rgbc.rgb[1]*255.0f;
		outColor[2] = rgbc.rgb[2]*255.0f;
		outColor[3] = 128;
	}
	
	outColor[3] -= ((f->data().GetParam(af_Variant,0)) * 24);
	
}

void	PrecalcOGL(Pmwx&						ioMap, ProgressFunc inFunc)
{
	int total = ioMap.number_of_vertices() + ioMap.number_of_halfedges() + ioMap.number_of_faces();
	int ctr = 0;

	PROGRESS_START(inFunc, 0, 1, "Building preview of vector map...")

	for (Pmwx::Vertex_iterator v = ioMap.vertices_begin(); v != ioMap.vertices_end(); ++v, ++ctr)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Building preview of vector map...", ctr, total, 1000)
		v->data().mGL[0] = CGAL::to_double(v->point().x());
		v->data().mGL[1] = CGAL::to_double(v->point().y());
	}

//	for (Pmwx::Halfedge_iterator h = ioMap.halfedges_begin(); h != ioMap.halfedges_end(); ++h, ++ctr)
//	{
//		PROGRESS_CHECK(inFunc, 0, 1, "Building preview of vector map...", ctr, total, 1000)
//		h->data().mGL[0] = CGAL::to_double(h->source()->point().x());
//		h->data().mGL[1] = CGAL::to_double(h->source()->point().y());
//		h->data().mGL[2] = CGAL::to_double(h->target()->point().x());
//		h->data().mGL[3] = CGAL::to_double(h->target()->point().y());
//	}

	for (Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f, ++ctr)
	if (!f->is_unbounded())
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Building preview of vector map...", ctr, total, 1000)

		f->data().mGLTris.clear();
		#if !NO_FACE_RENDER
		gAccum = &f->data().mGLTris;

		GLUtriangulatorObj *tobj;   /* tessellation object */
		GLdouble v[3];              /* passed to gluTessVertex, prototype used 3d */

		tobj = gluNewTess();
		gluTessCallback(tobj, GLU_BEGIN, (GLvoid (STDCALL_MACRO *)())PRECALC_Begin);
		gluTessCallback(tobj, GLU_VERTEX, (GLvoid (STDCALL_MACRO *)())PRECALC_Vertex2fv);
		gluTessCallback(tobj, GLU_END, (GLvoid (STDCALL_MACRO *)()) PRECALC_End);
		gluBeginPolygon(tobj);
		gluNextContour(tobj, GLU_EXTERIOR);

		Pmwx::Ccb_halfedge_circulator i, stop;
		i = stop = f->outer_ccb();
		do {
			const float * p = i->source()->data().mGL;
			v[0] = CGAL::to_double(i->source()->point().x());
			v[1] = CGAL::to_double(i->source()->point().y());
			v[2] = 0.0;
			gluTessVertex(tobj, v, (GLvoid*) p);
		} while (++i != stop);
		for (Pmwx::Hole_iterator hole = f->holes_begin(); hole != f->holes_end(); ++hole)
		{
			gluNextContour(tobj, GLU_INTERIOR);
			i = stop = *hole;
			do {
				const float * p = i->source()->data().mGL;			
				v[0] = CGAL::to_double(i->source()->point().x());
				v[1] = CGAL::to_double(i->source()->point().y());
			    v[2] = 0.0;
				gluTessVertex(tobj, v, (GLvoid *) p);
			} while (++i != stop);
		}
		gluEndPolygon(tobj);
		gluDeleteTess(tobj);
		#endif
	}

	PROGRESS_DONE(inFunc, 0, 1, "Building preview of vector map...")

	RecalcOGLColors(ioMap, inFunc);
}

void	RecalcOGLColors(Pmwx& ioMap, ProgressFunc inFunc)
{
	int total = ioMap.number_of_halfedges() + ioMap.number_of_faces();
	int ctr = 0;
	PROGRESS_START(inFunc, 0, 1, "Setting colors for vector map...")

	for (Pmwx::Halfedge_iterator h = ioMap.halfedges_begin(); h != ioMap.halfedges_end(); ++h, ++ctr)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Setting colors for vector map...", ctr, total, 1000)
		SetColorForHalfedge(h, h->data().mGLColor);
//		SetColorForHalfedge(h, h->twin()->data().mGLColor);
	}

	for (Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f, ++ctr)
	if (!f->is_unbounded())
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Setting colors for vector map...", ctr, total, 1000)
		SetColorForFace(f, f->data().mGLColor);
	}
	PROGRESS_DONE(inFunc, 0, 1, "Setting colors for vector map...")

}


void nonConvexPolygon( const Polygon2& p, const vector<Polygon2>& pp)
{
	glDisable(GL_CULL_FACE);


	GLUtriangulatorObj *tobj;   /* tessellation object */
	GLdouble v[3];              /* passed to gluTessVertex, prototype used 3d */

	GLenum err = glGetError();

	vector<float>	storage;
	int needed = p.size();
	for (int n = 0; n < pp.size(); ++n)
		needed += pp[n].size();
	storage.resize(needed * 2);
	float * vv = &*storage.begin();

	tobj = gluNewTess();
	gluTessCallback(tobj, GLU_BEGIN, (GLvoid (STDCALL_MACRO *)())glBegin);
	gluTessCallback(tobj, GLU_VERTEX, (GLvoid (STDCALL_MACRO *)())glVertex2fv);
	gluTessCallback(tobj, GLU_END, glEnd);
	gluBeginPolygon(tobj);
	gluNextContour(tobj, GLU_EXTERIOR);

	int ctr = 0;
	for (Polygon2::const_iterator i = p.begin(); i != p.end(); ++i, ++ctr)
	{
		/* send vertex for tessellation, it expects 3d array of double */
		vv[ctr*2] = v[0] = i->x();
		vv[ctr*2+1] = v[1] = i->y();
		v[2] = 0.0;

		gluTessVertex(tobj, v, &vv[ctr*2]);
	}

	for (vector<Polygon2>::const_iterator ii = pp.begin(); ii != pp.end(); ++ii)
	{
		gluNextContour(tobj, GLU_INTERIOR);
		for (Polygon2::const_iterator i = ii->begin(); i != ii->end(); ++i, ++ctr)
		{
			/* send vertex for tessellation, it expects 3d array of double */
			vv[ctr*2] = v[0] = i->x();
			vv[ctr*2+1] = v[1] = i->y();
			v[2] = 0.0;

			gluTessVertex(tobj, v, &vv[ctr*2]);
		}
	}

	gluEndPolygon(tobj);
	gluDeleteTess(tobj);
}

static void	MapMouseToCoord(
				double			mapWest,
				double			mapSouth,
				double			mapEast,
				double			mapNorth,
				double			screenLeft,
				double			screenBottom,
				double			screenRight,
				double			screenTop,
				double			inX,
				double			inY,
				double&			outLon,
				double&			outLat)
{
	double	mapWidth = mapEast - mapWest;
	double	mapHeight = mapNorth - mapSouth;
	double	screenWidth = screenRight - screenLeft;
	double	screenHeight = screenTop - screenBottom;
	outLon = mapWest + (inX - screenLeft) * mapWidth / screenWidth;
	outLat = mapSouth + (inY - screenBottom) * mapHeight / screenHeight;
}

void	RemapPolygonCoords(Polygon2& p,
				double			mapWest,
				double			mapSouth,
				double			mapEast,
				double			mapNorth,
				double			screenLeft,
				double			screenBottom,
				double			screenRight,
				double			screenTop)
{
	for (Polygon2::iterator v = p.begin(); v != p.end(); ++v)
	{
		double	xn, yn;
		MapMouseToCoord(mapWest, mapSouth, mapEast, mapNorth, screenLeft, screenBottom, screenRight, screenTop,
					v->x(), v->y(), xn, yn);
		*v = Point2(xn, yn);
	}
}

void	CirculatorToPoly(Pmwx::Ccb_halfedge_const_circulator circ, Polygon2& poly)
{
	Pmwx::Ccb_halfedge_const_circulator i = circ;
	do {
		poly.push_back(cgal2ben(i->source()->point()));
		++i;
	} while (i != circ);
}

void	FaceToScaledPoly(Pmwx::Face_const_handle	f, Polygon2& p, vector<Polygon2>& pp,
				double			mapWest,
				double			mapSouth,
				double			mapEast,
				double			mapNorth,
				double			screenLeft,
				double			screenBottom,
				double			screenRight,
				double			screenTop)
{
	if (f->is_unbounded()) return;
	pp.clear();
	CirculatorToPoly(f->outer_ccb(), p);
	for (Pmwx::Hole_const_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
	{
		Polygon2 hole;
		CirculatorToPoly(Pmwx::Ccb_halfedge_const_circulator(*h), hole);
		pp.push_back(hole);
	}
	RemapPolygonCoords(p, mapWest, mapSouth, mapEast, mapNorth, screenLeft, screenBottom, screenRight, screenTop);
	for (vector<Polygon2>::iterator iter = pp.begin(); iter != pp.end(); ++iter)
	{
		RemapPolygonCoords(*iter, mapWest, mapSouth, mapEast, mapNorth, screenLeft, screenBottom, screenRight, screenTop);
	}
}


void	DrawMapBucketed(
				GUI_GraphState *state,
				Pmwx&			inMap,
				PmwxIndex_t&	inIndex,
				double			mapWest,
				double			mapSouth,
				double			mapEast,
				double			mapNorth,
//				double			screenLeft,
//				double			screenBottom,
//				double			screenRight,
//				double			screenTop,
				const set<Pmwx::Vertex_handle>&		vertexSel,
				const set<Pmwx::Halfedge_handle>&	edgeSel,
				const set<Pmwx::Face_handle>&		faceSel,
				const set<PointFeatureSelection>&	pointFeatureSel)
{
	GLint		vp[4];
	GLdouble	mv[16];
	GLdouble	pr[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,mv);
	glGetDoublev(GL_PROJECTION_MATRIX,pr);
	glGetIntegerv(GL_VIEWPORT, vp);
	vector<double>		label_pts;
	vector<string>		label_strings;

	#define ACCUM_LABEL(x,y,s) \
		label_pts.push_back(x); \
		label_pts.push_back(y); \
		label_strings.push_back(s);
							

	state->SetState(0, 0, 0,   0, 1,   0, 0);

	double	mapWidth = mapEast - mapWest;
	double	mapHeight = mapNorth - mapSouth;
//	double	screenWidth = screenRight - screenLeft;
//	double	screenHeight = screenTop - screenBottom;

	vector<PmwxIndex_t::FaceTree::item_type>		faces;
	vector<PmwxIndex_t::HalfedgeTree::item_type>	halfedges;
	vector<PmwxIndex_t::VertexTree::item_type>		vertices;
//	FindFaceTouchesRectFast(inMap,Point2(mapWest, mapSouth), Point2(mapEast, mapNorth), faces);
//	FindHalfedgeTouchesRectFast(inMap,Point2(mapWest, mapSouth), Point2(mapEast, mapNorth), halfedges);
//	FindVerticesTouchesRect(inMap,Point2(mapWest, mapSouth), Point2(mapEast, mapNorth), vertices);

	Bbox2	box(Point2(mapWest,mapSouth),Point2(mapEast,mapNorth));

	faces.reserve(inMap.number_of_faces());
	inIndex.faces.query<back_insert_iterator<vector<PmwxIndex_t::FaceTree::item_type> > > (box,back_inserter(faces));
	halfedges.reserve(inMap.number_of_edges());
	inIndex.halfedges.query(box, back_inserter(halfedges));
	vertices.reserve(inMap.number_of_vertices());
	inIndex.vertices.query(box, back_inserter(vertices));
	
//	for (Pmwx::Face_iterator f = inMap.faces_begin(); f != inMap.faces_end(); ++f) faces.push_back(f);
//	for (Pmwx::Halfedge_iterator e = inMap.halfedges_begin(); e != inMap.halfedges_end(); ++e) halfedges.push_back(e);
//	for (Pmwx::Vertex_iterator v = inMap.vertices_begin(); v != inMap.vertices_end(); ++v) vertices.push_back(v);

//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glTranslated(screenLeft, screenBottom, 0.0);
//	glScaled(screenWidth / mapWidth, screenHeight / mapHeight, 1.0);
//	glTranslated(-mapWest, -mapSouth, 0.0);

	/******************************************************************************************
	 * DRAW FACES - SELECTED AND CONTAINING ZONING INFO
	 ******************************************************************************************/

#if DRAW_FACES
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);

	for (vector<PmwxIndex_t::FaceTree::item_type>::iterator fi = faces.begin(); fi != faces.end(); ++fi)
	{
		Pmwx::Face_handle f = fi->second;
		if (!f->is_unbounded())
		{
			bool	sel = faceSel.count(f) > 0;
			bool	draw = sel || f->data().mGLColor[3] != 0.0;

			if (draw)
			{
				if (sel)
					glColor4ub(f->data().mGLColor[0] / 2 + 127, f->data().mGLColor[1]/2, f->data().mGLColor[2]/2,200);
				else
					glColor4ubv(f->data().mGLColor);

				for(vector<const float *>::const_iterator v = f->data().mGLTris.begin(); v != f->data().mGLTris.end(); ++v)
					glVertex2fv(*v);
			}
		}
	}
	glEnd();
#endif

	/******************************************************************************************
	 * DRAW HALFEDGES - SELECTED AND ONES WITH TRANSPORTATION
	 ******************************************************************************************/

#if DRAW_EDGES
	glBegin(GL_LINES);
	int	width = 1;
	for (vector<PmwxIndex_t::HalfedgeTree::item_type>::iterator he = halfedges.begin(); he != halfedges.end(); ++he)
	for(int n = 0; n < 2; ++n)
	{
		Pmwx::Halfedge_handle e = n ? he->second->twin() : he->second;
		{
			glColor3ubv(e->data().mGLColor);
			int wantWidth = (edgeSel.find(e) != edgeSel.end() || edgeSel.find(e->twin()) != edgeSel.end()) ? 2 : 1;
			if (width != wantWidth)
			{
				glEnd();
				glLineWidth(wantWidth);
				width = wantWidth;
				glBegin(GL_LINES);
			}

			glVertex2fv(e->source()->data().mGL);
			glVertex2fv(e->target()->data().mGL);

//			draw_he(e);
		}
	}
	glEnd();
	glLineWidth(1);
#endif

	/******************************************************************************************
	 * DRAW POLYGON AND POINT OBJECTS INSIDE FACES
	 ******************************************************************************************/
#if DRAW_FEATURES
	for (vector<PmwxIndex_t::FaceTree::item_type>::iterator fi = faces.begin(); fi != faces.end(); ++fi)
	{
		Pmwx::Face_handle f = fi->second;
		if (!f->is_unbounded())
		{
			bool	draw = faceSel.count(f);

			if (draw)
			{
				// THIS CODE DRAWS PT OBJECTS IN CYAN INSIDE ALL FACES
				glPointSize(3);
				glBegin(GL_POINTS);
				for (GISObjPlacementVector::const_iterator obj = f->data().mObjs.begin(); obj != f->data().mObjs.end(); ++obj)
				{
					double	x1 = CGAL::to_double(obj->mLocation.x());
					double	y1 = CGAL::to_double(obj->mLocation.y());
					glVertex2f(x1, y1);
				}
				glEnd();
				glPointSize(1);
			}
		}
	}
#endif

	/******************************************************************************************
	 * DRAW VERTICES
	 ******************************************************************************************/

#if DRAW_VERTICES
	if (!vertices.empty())
	{
		glPointSize(3);
		glColor3f(1.0, 1.0, 0.0);
		glBegin(GL_POINTS);
		for (vector<PmwxIndex_t::VertexTree::item_type>::iterator v = vertices.begin(); v != vertices.end(); ++v)
		{
			if (vertexSel.find(v->second) == vertexSel.end())
			{
				if(v->second->data().mNeighborBurned)
					glColor3f(1,0,0);
				else if(v->second->data().mNeighborNotBurned)
					glColor3f(1,1,0);
				else
					continue;
				glVertex2fv(v->second->data().mGL);				
				continue;
			}

//			double	x1 = CGAL::to_double(v->second->point().x());
//			double	y1 = CGAL::to_double(v->second->point().y());
//			x1 = screenLeft + ((x1 - mapWest) * screenWidth / mapWidth);
//			y1 = screenBottom + ((y1 - mapSouth) * screenHeight / mapHeight);
			glVertex2fv(v->second->data().mGL);
			
			if(!v->second->is_isolated())
			{
				Pmwx::Halfedge_around_vertex_circulator circ,stop;
				GISNetworkSegmentVector::iterator r;
				circ=stop=v->second->incident_halfedges();
				char str[25];
				do {
					for(r=circ->data().mSegments.begin(); r != circ->data().mSegments.end(); ++r)
					{
						sprintf(str,"%d",(int) r->mTargetHeight);
						ACCUM_LABEL(
							v->second->data().mGL[0] + 0.25 * (circ->source()->data().mGL[0] - circ->target()->data().mGL[0]),
							v->second->data().mGL[1] + 0.25 * (circ->source()->data().mGL[1] - circ->target()->data().mGL[1]),
							str);
					}
					for(r=circ->twin()->data().mSegments.begin(); r != circ->twin()->data().mSegments.end(); ++r)
					{
						sprintf(str,"%d",(int) r->mSourceHeight);
						ACCUM_LABEL(
							v->second->data().mGL[0] + 0.25 * (circ->source()->data().mGL[0] - circ->target()->data().mGL[0]),
							v->second->data().mGL[1] + 0.25 * (circ->source()->data().mGL[1] - circ->target()->data().mGL[1]),
							str);
					}
				} while (++circ != stop);
			}
			
		}
		glEnd();
		glPointSize(1);
		
		glBegin(GL_LINES);
		
		for (vector<PmwxIndex_t::VertexTree::item_type>::iterator v = vertices.begin(); v != vertices.end(); ++v)
		if(v->second->degree() == 2)
		{
			if (vertexSel.find(v->second) == vertexSel.end())
				continue;
				
			continue;
				
			Point2	p1(cgal2ben(v->second->incident_halfedges()->next()->target()->point()));
			Point2	p2(cgal2ben(v->second->point()));
			Point2	p3(cgal2ben(v->second->incident_halfedges()->source()->point()));
			
			Vector2	v1(p2,p1);
			Vector2 v2(p2,p3);
			double r = cos(p2.y() * DEG_TO_RAD);
			v1.dx *= r;
			v2.dx *= r;
			double l1 = sqrt(v1.squared_length());
			double l2 = sqrt(v2.squared_length());
			v1.normalize();
			v2.normalize();
			double d = v1.dot(v2);
			Vector2 bisector;
			if(d > 0.7 || d < -0.7)
			{
				bisector = v1+v2;
				bisector.normalize();
			} 
			else
			{
				if(right_turn(p1,p2,p3))
					bisector = v1.perpendicular_ccw() + v2.perpendicular_cw();
				else
					bisector = v1.perpendicular_cw() + v2.perpendicular_ccw();
				bisector.normalize();
			}
			
			double arc = (l1+l2) * 0.5;
			double lrat = (l1 < l2) ? l2 / l1 : l1 / l2;
			if(lrat > 2.0)
			{
				glColor3f(1,1,1);
				arc = min(l1,l2);
			}
			else
				glColor3f(1,1,0);
			double radius = arc / acos(-d);
			bisector *= radius;
			
			bisector.dx / r;
			
			glVertex2f(p2.x(),p2.y());
			glVertex2f(p2.x() + bisector.dx,p2.y() + bisector.dy);			
		}
		glEnd();
	}
#endif

	/******************************************************************************************
	 * DRAW BUILDING FOOTPRINTS ON SELECTED FACES
	 ******************************************************************************************/

#if DRAW_FOOTPRINTS
	glMatrixMode(GL_MODELVIEW);
	for (vector<PmwxIndex_t::FaceTree::item_type>::iterator fi = faces.begin(); fi != faces.end(); ++fi)
	{
		int n = 0;
		bool	 fsel = faceSel.find(fi->second) != faceSel.end();
		if (!fsel) continue;
		for (int j = 0; j < fi->second->data().mObjs.size(); ++j)
		{
			float shade = (float) (n % 10) / 20.0 + 0.1;
			++n;
			glColor4f(shade, shade, fi->second->data().mObjs[j].mDerived ? 1.0 : 0.0, 1.0);

			double	x1 = CGAL::to_double(fi->second->data().mObjs[j].mLocation.x());
			double	y1 = CGAL::to_double(fi->second->data().mObjs[j].mLocation.y());
			double r = fi->second->data().mObjs[j].mHeading;

			double	w = 0.5 * gRepTable[gRepFeatureIndex[fi->second->data().mObjs[j].mRepType]].width_min;
			double	h = 0.5 * gRepTable[gRepFeatureIndex[fi->second->data().mObjs[j].mRepType]].depth_min;

			Point2	corners[4];
			Quad_1to4(Point2(x1,y1), r, 2.0 * h, 2.0 * w, corners);

			float x_scale = /*(screenWidth / mapWidth) */ 1.0 /  (DEG_TO_MTR_LAT * cos (y1 * DEG_TO_RAD));
			float y_scale = /*(screenHeight / mapHeight) */ 1.0 /  (DEG_TO_MTR_LAT   );

//			x1 = screenLeft + ((x1 - mapWest) * screenWidth / mapWidth);
//			y1 = screenBottom + ((y1 - mapSouth) * screenHeight / mapHeight);

			glPushMatrix();
			glTranslatef(x1, y1, 0.0);
			glScalef(x_scale, y_scale, 1.0);
			glRotatef(r, 0, 0, -1);

			glBegin(GL_QUADS);
			glVertex2f(-w, -h);
			glVertex2f(-w,  h);
			glVertex2f( w,  h);
			glVertex2f( w, -h);
			glEnd();
			glPopMatrix();
			
			glBegin(GL_LINE_LOOP);
			glColor3f(1,1,1);
			glVertex2f(corners[0].x(),corners[0].y());
			glColor3f(1,1,0);
			glVertex2f(corners[1].x(),corners[1].y());
			glColor3f(0,1,0);
			glVertex2f(corners[2].x(),corners[2].y());
			glColor3f(0,1,1);
			glVertex2f(corners[3].x(),corners[3].y());
			glEnd();
			
		}

		for (GISPolyObjPlacementVector::iterator j = fi->second->data().mPolyObjs.begin(); j != fi->second->data().mPolyObjs.end(); ++j)
		{
			int is_string = strstr(FetchTokenString(j->mRepType),".ags") != NULL;		
			
			float shade = (float) (n % 10) / 20.0 + 0.1;
			++n;
			glColor4f(shade, shade, j->mDerived ? 1.0 : 0.0, 1.0);

			for(vector<Polygon2>::const_iterator r = j->mShape.begin(); r != j->mShape.end(); ++r)
			{
				int is_hot = (r - j->mShape.begin()) < j->mParam;
				glLineWidth(is_string ? (is_hot ? 3 : 1) : 1);
				glBegin(is_string ? GL_LINE_STRIP : GL_LINE_LOOP);
				for(int l = 0; l < r->size(); ++l)
					glVertex2f(
						((*r)[l].x()),
						((*r)[l].y()));
				glEnd();
			}
			glLineWidth(1);
			
			glPointSize(2);
			glColor3f(1,1,1);
			glBegin(GL_POINTS);
			for(vector<Polygon2>::const_iterator r = j->mShape.begin(); r != j->mShape.end(); ++r)
			for(int l = 0; l < r->size(); ++l)
				glVertex2f(
					((*r)[l].x()),
					((*r)[l].y()));
			glEnd();
			
		}
	}
#endif

	/******************************************************************************************
	 * DRAW POINT FEATURES ON SELECTED FACES AS WELL AS INDIVIDUAL SELECTED FEATURES
	 ******************************************************************************************/

#if DRAW_FEATURES
	glPointSize(3);
	glColor4f(0.0, 1.0, 1.0, 1.0);
	glBegin(GL_POINTS);
	int osz = 3;
	for (vector<PmwxIndex_t::FaceTree::item_type>::iterator fi = faces.begin();
		fi != faces.end(); ++fi)
	{
		bool	 fsel = faceSel.find(fi->second) != faceSel.end();
		for (int j = 0; j < fi->second->data().mPointFeatures.size(); ++j)
		{
			bool	isel = pointFeatureSel.find(PointFeatureSelection(fi->second, j)) != pointFeatureSel.end();
			bool	has_height = fi->second->data().mPointFeatures[j].mParams.count(pf_Height) != 0;
			
			int sz = has_height ? 4 : 2;
			
			if(sz != osz)
			{
				glEnd();
				glPointSize(sz);
				glBegin(GL_POINTS);
				osz = sz;
			}

			if (isel || fsel)
				glColor4f(1.0, 1.0, 1.0, 1.0);
			else if(gEnumColors.count(fi->second->data().mPointFeatures[j].mFeatType))
			{
				glColor3fv(gEnumColors[fi->second->data().mPointFeatures[j].mFeatType].rgb);
			}
			else
				glColor4f(0.0, 0.6, 0.6, 1.0);

			double	x1 = CGAL::to_double(fi->second->data().mPointFeatures[j].mLocation.x());
			double	y1 = CGAL::to_double(fi->second->data().mPointFeatures[j].mLocation.y());
//			x1 = screenLeft + ((x1 - mapWest) * screenWidth / mapWidth);
//			y1 = screenBottom + ((y1 - mapSouth) * screenHeight / mapHeight);
			glVertex2f(x1, y1);
		}
	}
	glEnd();
	glPointSize(1);
#endif

//	glPopMatrix();

	if(!label_strings.empty())
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(vp[0],vp[0]+vp[2],vp[1],vp[1]+vp[3],-1.0,1.0);

		float c[4] = { 1,1,1,1 };
		for(int n = 0; n < label_strings.size(); ++n)
		{
			double xyz[3];
			gluProject(label_pts[n*2],label_pts[n*2+1],0,mv,pr,vp,xyz,xyz+1,xyz+2);
//			xyz[0] *= (double) vp[2];
//			xyz[1] *= (double) vp[3];
//			xyz[0] += vp[0];
//			xyz[1] += vp[1];
			GUI_FontDraw(state, font_UI_Small,c,xyz[0],xyz[1],label_strings[n].c_str());
		}

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}






	/******************************************************************************************
	 * EXTRA JUNK CODe
	 ******************************************************************************************/



#if 0
			THIS CODE DRAWS THE SELECTED POLYGONS CCB IN CYAN AND HOLES IN GREEN, ALLOWING FOR TOPOLOGY DIAGNOSTICS!!

			glColor3f(0.0, 1.0, 1.0);
//			for (GISPolyObjPlacementVector::const_iterator poly = f->data().mPolyObjs.begin(); poly != f->data().mPolyObjs.end(); ++poly)
			if (faceSel.find(f) != faceSel.end())	// HACK
			{
				glLineWidth(3);	// HACK
//				p = poly->where;
//				RemapPolygonCoords(p, screenLeft, screenBottom, screenRight, screenTop, mapWest, mapSouth, mapEast, mapNorth);
				glBegin(GL_LINE_LOOP);
				for (Polygon2::Vertex_iterator v = p.begin(); v != p.end(); ++v)
				{
					glVertex2f(v->x), v->y));
				}
				glEnd();
				glLineWidth(1);	// HACK
			}

// COPY OF HACK

			glColor3f(0.0, 1.0, 0.3);
//			for (GISPolyObjPlacementVector::const_iterator poly = f->data().mPolyObjs.begin(); poly != f->data().mPolyObjs.end(); ++poly)
			for (vector<Polygon2>::iterator iter = pp.begin(); iter != pp.end(); ++iter)
			if (faceSel.find(f) != faceSel.end())	// HACK
			{
				glLineWidth(5);	// HACK
				p = *iter;
//				p = poly->where;
//				RemapPolygonCoords(p, screenLeft, screenBottom, screenRight, screenTop, mapWest, mapSouth, mapEast, mapNorth);
				glBegin(GL_LINE_LOOP);
				for (Polygon2::Vertex_iterator v = p.begin(); v != p.end(); ++v)
				{
					glVertex2f(v->x), v->y));
				}
				glEnd();
				glLineWidth(1);	// HACK
			}
#endif





void		FindFaceTouchesPt(Pmwx& inMap, PmwxIndex_t& index, const Point2& p, vector<Face_handle>& outIDs)
{
	vector<Face_handle>	faces;
	outIDs.clear();
	index.faces.query_value(Bbox2(p), back_inserter(faces));
	for (vector<Face_handle>::iterator ff = faces.begin(); ff != faces.end(); ++ff)
	{
		Face_handle f = *ff;
//		printf("Scanning poly...\n");
		Polygon2	pol;
		Pmwx::Ccb_halfedge_circulator circ = f->outer_ccb(), stop = f->outer_ccb();
		do {
			pol.push_back(cgal2ben(circ->target()->point()));
//			printf("  pt %lf,%lf\n",CGAL::to_double(circ->target()->point().x()),CGAL::to_double(circ->target()->point().y()));
		} while(++circ != stop);

		if(pol.inside(p))
		{
			bool in_hole = false;
			for(Pmwx::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
			{
				Polygon2 polh;
				Pmwx::Ccb_halfedge_circulator circ = *h, stop = *h;
				do {
					polh.push_back(cgal2ben(circ->target()->point()));
		//			printf("  pt %lf,%lf\n",CGAL::to_double(circ->target()->point().x()),CGAL::to_double(circ->target()->point().y()));
				} while(--circ != stop);
				if(polh.inside(p))
				{
					in_hole = true;
						break;
				}
			}
			if(!in_hole)
				outIDs.push_back(f);
		}
	}
}

void		FindFaceTouchesRectFast(Pmwx& inMap, PmwxIndex_t& index, const Point2& p1, const Point2& p2, vector<Face_handle>& outIDs)
{
	Bbox2	sel(p1, p2);
	outIDs.clear();
	index.faces.query_value(sel, back_inserter(outIDs));
}

void		FindFaceFullyInRect(Pmwx& inMap, PmwxIndex_t& index, const Point2& p1, const Point2& p2, vector<Face_handle>& outIDs)
{
	outIDs.clear();
	Bbox2	sel(p1,p2);
	vector<PmwxIndex_t::FaceTree::item_type>	faces;
	outIDs.clear();
	index.faces.query(sel, back_inserter(faces));
	for (vector<PmwxIndex_t::FaceTree::item_type>::iterator ff = faces.begin(); ff != faces.end(); ++ff)
	if(sel.contains(ff->first))
		outIDs.push_back(ff->second);
}

void		FindHalfedgeTouchesRectFast(Pmwx& inMap, PmwxIndex_t& index, const Point2& p1, const Point2& p2, vector<Halfedge_handle>& outIDs)
{
	outIDs.clear();
	Bbox2 sel(p1,p2);
	index.halfedges.query_value(sel,back_inserter(outIDs));
}

void		FindHalfedgeFullyInRect(Pmwx& inMap, PmwxIndex_t& index, const Point2& p1, const Point2& p2, vector<Halfedge_handle>& outIDs)
{
	outIDs.clear();
	Bbox2 sel(p1,p2);
	vector<PmwxIndex_t::HalfedgeTree::item_type>	edges;
	index.halfedges.query(sel,back_inserter(edges));
	for(vector<PmwxIndex_t::HalfedgeTree::item_type>::iterator e = edges.begin(); e != edges.end(); ++e)
	if(sel.contains(e->first))
		outIDs.push_back(e->second);
}


void		FindVerticesTouchesPt(Pmwx& inMap, PmwxIndex_t& index, const Point2& p, vector<Vertex_handle>& outIDs)
{
	outIDs.clear();
	index.vertices.query_value(Bbox2(p), back_inserter(outIDs));

}

void		FindVerticesTouchesRect(Pmwx& inMap, PmwxIndex_t& index, const Point2& p1, const Point2& p2, vector<Vertex_handle>& outIDs)
{
	Bbox2 sel(p1,p2);
	outIDs.clear();
	index.vertices.query_value(sel, back_inserter(outIDs));
}
