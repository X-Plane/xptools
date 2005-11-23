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
#include "WED_SelectionTool.h"

#include "WED_MapZoomer.h"
#include "WED_Globals.h"
#include "WED_Document.h"
#include "WED_Selection.h"
#include "WED_Progress.h"

#include "WED_Notify.h"
#include "WED_Msgs.h"
#include "ParamDefs.h"
#include "AptIO.h"
#include "XESIO.h"

#include "GISUtils.h"
#include "MapAlgs.h"
#include "Skeleton.h"
#include "AssertUtils.h"

#include "WED_DrawMap.h"
#include "PlatformUtils.h"

#include "XPLMGraphics.h"
#include "MemFileUtils.h"
#include "Hydro.h"

#include "ObjPlacement.h"

const	int kPointClickSlop = 4;

template <typename __InputIterator, typename  __Ref>
void ApplyRange(__InputIterator begin, __InputIterator end, void (* func)(typename __InputIterator::value_type, __Ref), __Ref ref)
{
	for (__InputIterator i = begin; i != end; ++i)
		func(*i, ref);
}


WED_SelectionTool::WED_SelectionTool(WED_MapZoomer * inZoomer) : WED_MapTool(inZoomer)
{
	mIsDrag = false;
}

void	WED_SelectionTool::DrawFeedbackUnderlay(
				bool				inCurrent)
{
}
				
void	WED_SelectionTool::DrawFeedbackOverlay(
				bool				inCurrent)
{
	if (mIsDrag && (mMouseX != mMouseStartX || mMouseY != mMouseStartY))
	{
		int xMin = min(mMouseX, mMouseStartX);
		int yMin = min(mMouseY, mMouseStartY);
		int xMax = max(mMouseX, mMouseStartX);
		int yMax = max(mMouseY, mMouseStartY);
		
		XPLMSetGraphicsState(0, 0, 0,    0, 1,   0, 0);
		glColor4f(0.0, 0.5, 0.0, 0.3);
		glBegin(GL_QUADS);
		glVertex2i(xMin, yMin);
		glVertex2i(xMin, yMax);
		glVertex2i(xMax, yMax);
		glVertex2i(xMax, yMin);
		glEnd();

		glColor4f(1.0, 1.0, 1.0, 0.3);
		glBegin(GL_LINE_LOOP);
		glVertex2i(xMin, yMin);
		glVertex2i(xMin, yMax);
		glVertex2i(xMax, yMax);
		glVertex2i(xMax, yMin);
		glEnd();
	}
}
				
bool	WED_SelectionTool::HandleClick(
				XPLMMouseStatus		inStatus,
				int 				inX, 
				int 				inY, 
				int 				inButton)
{
	if (inButton != 0) return 0;
	switch(inStatus) {
	case xplm_MouseDown:
		mModifiers = XPLMGetModifiers();
		mMouseStartX = mMouseX = inX;
		mMouseStartY = mMouseY = inY;
		mIsDrag = true;
		mIsMoveVertices = false;
		mFaceSelection = gFaceSelection;
		mEdgeSelection = gEdgeSelection;
		mVertexSelection = gVertexSelection;
		mPointFeatureSelection = gPointFeatureSelection;
		DoSelectionPreview();
		if (gSelectionMode == wed_Select_Vertex)
		if ((mModifiers & (xplm_ShiftFlag + xplm_ControlFlag)) == 0)
		if (!gVertexSelection.empty() && mVertexSelection.find(*gVertexSelection.begin()) != mVertexSelection.end())
		{
			gVertexSelection = mVertexSelection;
			mIsDrag = false;
			mIsMoveVertices = true;
			mMoveLon = GetZoomer()->XPixelToLon(inX);
			mMoveLat = GetZoomer()->YPixelToLat(inY);
		}
		return 1;
	case xplm_MouseDrag:
		if (mIsDrag && (inX != mMouseX || inY != mMouseY))	// Prevent thrash!!
		{
			mMouseX = inX;
			mMouseY = inY;
			gFaceSelection = mFaceSelection;
			gEdgeSelection = mEdgeSelection;
			gVertexSelection = mVertexSelection;
			gPointFeatureSelection = mPointFeatureSelection;
			DoSelectionPreview();
		}
		if (mIsMoveVertices && (inX != mMouseX || inY != mMouseY))
		{
			double	lonNow = GetZoomer()->XPixelToLon(inX);
			double	latNow = GetZoomer()->YPixelToLat(inY);
			Vector2	delta(lonNow - mMoveLon, latNow - mMoveLat);
			mMoveLon = lonNow;
			mMoveLat = latNow;
			for (set<Pmwx::Vertex_handle>::iterator v = gVertexSelection.begin(); v != gVertexSelection.end(); ++v)
			{
				Point2 p = (*v)->point();
				(*v)->point() = p + delta;
			}
		}
		return 1;
	case xplm_MouseUp:
		if (mIsDrag)
		{
			mMouseX = inX;
			mMouseY = inY;
			gFaceSelection = mFaceSelection;
			gEdgeSelection = mEdgeSelection;
			gVertexSelection = mVertexSelection;
			gPointFeatureSelection = mPointFeatureSelection;
			DoSelectionPreview();
			mFaceSelection.clear();
			mEdgeSelection.clear();
			mVertexSelection.clear();
			mPointFeatureSelection.clear();
			mIsDrag = false;
//			if (gFaceSelection.size() == 1)
//				WED_PlaceBuildings(*gFaceSelection.begin());
		}
		if (mIsMoveVertices && (inX != mMouseX || inY != mMouseY))
		{
			double	lonNow = GetZoomer()->XPixelToLon(inX);
			double	latNow = GetZoomer()->YPixelToLat(inY);
			Vector2	delta(lonNow - mMoveLon, latNow - mMoveLat);
			mMoveLon = lonNow;
			mMoveLat = latNow;
			for (set<Pmwx::Vertex_handle>::iterator v = gVertexSelection.begin(); v != gVertexSelection.end(); ++v)
			{
				Point2 p = (*v)->point();
				(*v)->point() = p + delta;				
			}
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);
			mIsMoveVertices = false;
		}
		
		return 1;
	default:	
		return 0;
	}		
}
		
static int gStopPt = -1;
				
int		WED_SelectionTool::GetNumProperties(void)
{
	return 1;
}

void	WED_SelectionTool::GetNthPropertyName(int n, string& s)
{
	s = "Stop:";
}

double	WED_SelectionTool::GetNthPropertyValue(int n)
{
	return gStopPt;
}

void	WED_SelectionTool::SetNthPropertyValue(int, double v)
{
	gStopPt = v;
}
	
int		WED_SelectionTool::GetNumButtons(void)
{
	return 10;
}

void	WED_SelectionTool::GetNthButtonName(int n, string& s)
{
	switch(n) {
	case 0: s = "Delete"; break;
	case 1: s = "Clean"; break;
	case 2: s=  "Inset"; break;
	case 3: s=  "Clear"; break;
	case 4: s=  "Next"; break;
	case 5: s=  "Simplify Water"; break;
	case 6: s=  "Simplify pmwx"; break;
	case 7: s=	"Gut Area"; break;
	case 8: s=	"Insert Map"; break;
	case 9: s=  "Make Faces Wet"; break;
	}
}

void	WED_SelectionTool::NthButtonPressed(int n)
{
	GISFace * d = NULL;
	int ctr = 0;
	switch(n) {
	case 0:
		{
			set<GISHalfedge *>	nuke;
			for (set<GISHalfedge *>::iterator sel = gEdgeSelection.begin(); sel != gEdgeSelection.end(); ++sel)
				if ((*sel)->mDominant)
					nuke.insert(*sel);
				else
					nuke.insert((*sel)->twin());
			for (set<GISHalfedge *>::iterator nukeme = nuke.begin(); nukeme != nuke.end(); ++nukeme)
			{
				gDocument->gMap.remove_edge(*nukeme);
				DebugAssert(gDocument->gMap.is_valid());
			}
		}
		break;
	case 1:
		for (set<GISFace *>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel)
		{
			CleanFace(gDocument->gMap, *fsel);
		}
		break;
	case 2:
	case 4:
		 {
			if (n == 4)
				++gStopPt;
			
			set<GISFace *>	fail;
				
			for (set<GISFace *>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel, ++ctr)
//			for (Pmwx::Face_iterator the_face = gMap.faces_begin(); the_face != gMap.faces_end(); ++the_face, ++ctr)
			{
				GISFace * the_face = *fsel;
				if (gStopPt == ctr)
				{
					gFaceSelection.clear();
					gFaceSelection.insert(the_face);
					return;
				}
				if (the_face->is_unbounded()) continue;
				if (the_face->mTerrainType == terrain_Water) continue;
				
				ComplexPolygon2			orig;
				ComplexPolygonWeight 	insets;
				vector<ComplexPolygon2>	inset;
				
				FaceToComplexPolygon(the_face, orig, &insets, GetInsetForEdgeDegs, NULL);

				try {
					int result = SK_InsetPolygon(orig, insets, inset, gStopPt);
					if (result != skeleton_OK)
						fail.insert(the_face);
				} catch(...) {
					fail.insert(the_face);
				}

				for (vector<ComplexPolygon2>::iterator inner = inset.begin(); inner != inset.end(); ++inner)
				{
					GISPolyObjPlacement_t	obj;
					obj.mRepType = NO_VALUE;
					obj.mDerived = true;
					obj.mHeight = 1.0;

					obj.mShape = *inner;
					obj.mLocation = obj.mShape[0].centroid();
					(the_face)->mPolyObjs.push_back(obj);
				}
			}
			
			if (!fail.empty())
			{
				char buf[256];
				sprintf(buf, "%d out of %d failed.\n", fail.size(), gFaceSelection.size());
				DoUserAlert(buf);
	//			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);
				gEdgeSelection.clear();
				gFaceSelection = fail;
				gVertexSelection.clear();
			}
			
			return;
		}
	case 3:
		for (set<GISFace *>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel)
			(*fsel)->mPolyObjs.clear();
		return;
	case 5:
		try {
			for (set<GISFace *>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel)
			{
//				SimplifyCoastlineFace(gMap, *fsel);
			}
		} catch (...) {
		}
		break;
	case 6:
		SimplifyMap(gDocument->gMap, true);
		gEdgeSelection.clear();
		gFaceSelection.clear();
		gVertexSelection.clear();
		break;
	case 7:
		{
			set<GISFace *> kill_f;
			int ctr;
			
			PROGRESS_START(WED_ProgressFunc, 0, 3, "Accumulating Faces")
			ctr = 0;
			for (set<GISFace *>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel, ++ctr)
			{
				PROGRESS_CHECK(WED_ProgressFunc, 0, 3, "Accumulating Faces", ctr, gFaceSelection.size(), gFaceSelection.size() / 200)
				kill_f.insert(*fsel);
			}	
			PROGRESS_DONE(WED_ProgressFunc, 0, 3, "Accumulating Faces")

			PROGRESS_START(WED_ProgressFunc, 1, 3, "Accumulating Edges")
			set<GISHalfedge *> kill_e;			
			ctr = 0;
			for (Pmwx::Halfedge_iterator e = gDocument->gMap.halfedges_begin(); e != gDocument->gMap.halfedges_end(); ++e, ++ctr)
			if (e->mDominant)
			{
				PROGRESS_CHECK(WED_ProgressFunc, 1, 3, "Accumulating Edges", ctr, gDocument->gMap.number_of_halfedges(), gDocument->gMap.number_of_halfedges() / 200)
				if (kill_f.count(e->face()) &&
					kill_f.count(e->twin()->face()))
				kill_e.insert(e);
			}
			PROGRESS_DONE(WED_ProgressFunc, 1, 3, "Accumulating Edges")

			ctr = 0;
			PROGRESS_START(WED_ProgressFunc, 2, 3, "Deleting Edges")
			for (set<GISHalfedge *>::iterator kill = kill_e.begin(); kill != kill_e.end(); ++kill, ++ctr)
			{
				PROGRESS_CHECK(WED_ProgressFunc, 2, 3, "Deleting Edges", ctr, kill_e.size(), kill_e.size() / 200)
				gDocument->gMap.remove_edge(*kill);
			}			
			PROGRESS_DONE(WED_ProgressFunc, 2, 3, "Deleting Edges")
		}		
		gEdgeSelection.clear();
		gFaceSelection.clear();
		gVertexSelection.clear();
		break;
	case 8:
		{
			char	path[1024];
			path[0] = 0;
			if (gFaceSelection.size() == 1)
			if (GetFilePathFromUser(getFile_Open, "Please pick an .XES file to open for roads", "Open", 8, path))
			{
				MFMemFile * fi = MemFile_Open(path);
				if (fi)
				{
					Pmwx		overMap;
					ReadXESFile(fi, &overMap, NULL, NULL, NULL, WED_ProgressFunc);

                    Point2 master1, master2, slave1, slave2;
                    CalcBoundingBox(gDocument->gMap, master1, master2);
                    CalcBoundingBox(overMap, slave1, slave2);
                    
                    Vector2 delta(slave1, master1);

                    for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
                    	overMap.UnindexVertex(i);

                    for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
                        i->point() += delta;

                    for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
                    	overMap.ReindexVertex(i);
					
					SwapFace(gDocument->gMap, overMap, *gFaceSelection.begin(), WED_ProgressFunc);
					
					WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);
					MemFile_Close(fi);
				}					
			}
		}
		gEdgeSelection.clear();
		gFaceSelection.clear();
		gVertexSelection.clear();
		break;
	case 9:
		for (set<GISFace *>::iterator f = gFaceSelection.begin(); f != gFaceSelection.end(); ++f)
		{
//			(*f)->mTerrainType = terrain_Water;
			(*f)->mAreaFeature.mFeatType = feat_Park;
		}
		for (set<GISHalfedge *>::iterator e = gEdgeSelection.begin(); e != gEdgeSelection.end(); ++e)
		{
//			(*e)->mSegments.clear();
		}
		
		break;
	}
	DebugAssert(gDocument->gMap.is_valid());
	WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);
	gEdgeSelection.clear();
}
	
char *	WED_SelectionTool::GetStatusText(void)
{
	static char buf[1024];
	int n = 0;

	if (mIsDrag && (mMouseX != mMouseStartX || mMouseY != mMouseStartY))
	{
		int xMin = min(mMouseX, mMouseStartX);
		int yMin = min(mMouseY, mMouseStartY);
		int xMax = max(mMouseX, mMouseStartX);
		int yMax = max(mMouseY, mMouseStartY);
		
		double l1 = LonLatDistMeters(GetZoomer()->XPixelToLon(xMin), GetZoomer()->YPixelToLat(yMin), GetZoomer()->XPixelToLon(xMax), GetZoomer()->YPixelToLat(yMin));
		double l2 = LonLatDistMeters(GetZoomer()->XPixelToLon(xMin), GetZoomer()->YPixelToLat(yMin), GetZoomer()->XPixelToLon(xMin), GetZoomer()->YPixelToLat(yMax));
		double d = LonLatDistMeters(GetZoomer()->XPixelToLon(mMouseStartX), GetZoomer()->YPixelToLat(mMouseStartY), GetZoomer()->XPixelToLon(mMouseX), GetZoomer()->YPixelToLat(mMouseY));		
		double a = l1 * l2;
		
		n += sprintf(buf+n, "%dm %dmx%dm=%d sq m ", (int) d, (int) l1, (int) l2, (int) a);
		
		if (gDocument->gDem.count(dem_Elevation) != 0)
		{
			double p1 = gDocument->gDem[dem_Elevation].value_linear(GetZoomer()->XPixelToLon(mMouseX), 	 GetZoomer()->YPixelToLat(mMouseY	  ));
			double p2 = gDocument->gDem[dem_Elevation].value_linear(GetZoomer()->XPixelToLon(mMouseStartX), GetZoomer()->YPixelToLat(mMouseStartY));
			if (p1 != NO_DATA && p2 != NO_DATA)
			{
				double drop = fabs(p2 - p1);
				double gradient = 1000.0 * drop / d;
				n += sprintf(buf+n,"g=%.1fm/km ", gradient);
			}
		}
	}
	
	n += sprintf(buf+n, "Vertices: %d Edges: %d Faces: %d Features: %d ",
		gVertexSelection.size(), gEdgeSelection.size(), gFaceSelection.size(), gPointFeatureSelection.size());
	if (gFaceSelection.size() == 1)
	{
		Pmwx::Face_handle	the_face = *(gFaceSelection.begin());

		double area = GetMapFaceAreaMeters(the_face);
		n += sprintf(buf+n, "%d sq m ", (int) area);
//		if (the_face->mTerrainType != terrain_Natural)
			n += sprintf(buf+n,"Art.Terrain:%s ", FetchTokenString(the_face->mTerrainType));
			
		if (the_face->mAreaFeature.mFeatType != NO_VALUE)
		{
			n += sprintf(buf+n, "Area Feature:%s ", FetchTokenString(the_face->mAreaFeature.mFeatType));
		}
		
		if (the_face->mParams.count(af_Height))
		{
			n += sprintf(buf+n, "AGL:%lf ", the_face->mParams[af_Height]);
		}
	}
	if (gEdgeSelection.size() == 1)
	{
		double len = GetMapEdgeLengthMeters(*gEdgeSelection.begin());
		n += sprintf(buf+n, "%.1f m ", len);
		for (GISNetworkSegmentVector::iterator seg = (*gEdgeSelection.begin())->mSegments.begin(); seg != (*gEdgeSelection.begin())->mSegments.end(); ++seg)
		{
			n += sprintf(buf+n, "%s->%s ", FetchTokenString(seg->mFeatType),FetchTokenString(seg->mRepType));
		}
		if ((*gEdgeSelection.begin())->mTransition != 0)
		{
			n += sprintf(buf+n,"Beach=%d ", (*gEdgeSelection.begin())->mTransition);
		}
		if ((*gEdgeSelection.begin())->mMark != 0)
		{
			n += sprintf(buf+n,"Marked ");
		}
		if ((*gEdgeSelection.begin())->twin()->mMark != 0)
		{
			n += sprintf(buf+n,"TwinMarked ");
		}
	}
	if (gPointFeatureSelection.size() == 1)
	{
		GISPointFeature_t * f = &gPointFeatureSelection.begin()->first->mPointFeatures[gPointFeatureSelection.begin()->second];
		n += sprintf(buf+n, "%s ", FetchTokenString(f->mFeatType));
		if (f->mParams.find(pf_Height) != f->mParams.end())
			n += sprintf(buf+n, "agl=%f ",f->mParams[pf_Height]);
	}
	if (gVertexSelection.size() == 1 && (*(gVertexSelection.begin()))->mTunnelPortal)
	{
		n += sprintf(buf+n, " (tunnel portal)");
	}

	if (gVertexSelection.size() == 1)
	{
		n += sprintf(buf+n, "%lf,%lf %016llX,%016llX ", (*(gVertexSelection.begin()))->point().x,(*(gVertexSelection.begin()))->point().y,(*(gVertexSelection.begin()))->point().x,(*(gVertexSelection.begin()))->point().y);
	}
	
	{
		int mx, my;
		XPLMGetMouseLocation(&mx, &my);
		Bbox2	vis_area(Point2(GetZoomer()->XPixelToLon(mx),GetZoomer()->YPixelToLat(my)));
		set<int>	apts;
		FindAirports(vis_area, gDocument->gAptIndex, apts);
		for (set<int>::iterator e = apts.begin(); e != apts.end(); ++e)
		{
			int k = *e;			
			if (vis_area.overlap(gDocument->gApts[k].bounds))
			{
				n += sprintf(buf+n, "%s %s ", gDocument->gApts[k].icao.c_str(), gDocument->gApts[k].name.c_str());
			}
		}
	}

	

	
	return buf;
}

bool	WED_SelectionTool::GetRectMapCoords(double coords[4])
{
	double	startLon = GetZoomer()->XPixelToLon(mMouseStartX);
	double	nowLon = GetZoomer()->XPixelToLon(mMouseX);
	double	startLat = GetZoomer()->YPixelToLat(mMouseStartY);
	double	nowLat = GetZoomer()->YPixelToLat(mMouseY);
	coords[0] = min(startLon, nowLon);
	coords[2] = max(startLon, nowLon);
	coords[1] = min(startLat, nowLat);
	coords[3] = max(startLat, nowLat);
	if (mMouseX == mMouseStartX && mMouseY == mMouseStartY) return false;
	return true;
}

static void InsertFaceInSet(Pmwx::Face_handle f, set<Pmwx::Face_handle> * s)
{
	s->insert(f);
}

static void ToggleFaceInSet(Pmwx::Face_handle f, set<Pmwx::Face_handle> * s)
{
	if (s->find(f) == s->end())
		s->insert(f);
	else
		s->erase(f);
}

static void InsertEdgeInSet(Pmwx::Halfedge_handle f, set<Pmwx::Halfedge_handle> * s)
{
	s->insert(f);
}

static void ToggleEdgeInSet(Pmwx::Halfedge_handle f, set<Pmwx::Halfedge_handle> * s)
{
	if (s->find(f) == s->end())
		s->insert(f);
	else
		s->erase(f);
}


static void InsertVertexInSet(Pmwx::Vertex_handle f, set<Pmwx::Vertex_handle> * s)
{
	s->insert(f);
}

static void ToggleVertexInSet(Pmwx::Vertex_handle f, set<Pmwx::Vertex_handle> * s)
{
	if (s->find(f) == s->end())
		s->insert(f);
	else
		s->erase(f);
}

struct	NearestVertexToPt_t {
	double				lat;
	double				lon;
	double				dist;
	Pmwx::Vertex_handle	v;
	bool				found;
};

static void	MakeVertexClosestMaybe(Pmwx::Vertex_handle v, NearestVertexToPt_t * s)
{
	double dx = s->lon - v->point().x;
	double dy = s->lat - v->point().y;
	double dist = dx * dx + dy * dy;
	if (!s->found || dist < s->dist)
	{
		s->found = true;
		s->v = v;
		s->dist = dist;
	}
}

struct	NearestEdgeToPt_t {
	double					lat;
	double					lon;
	double					dist;
	Pmwx::Halfedge_handle	e;
	bool					found;
};

static void	MakeEdgeClosestMaybe(Pmwx::Halfedge_handle v, NearestEdgeToPt_t * s)
{
	Point2	p = Point2(s->lon, s->lat);
	Segment2	seg(v->source()->point(), v->target()->point());
	Point2 proj = seg.projection(p);
	if (!seg.collinear_has_on(proj)) return;
	
	double dist = CGAL::to_double(Segment2(p, proj).squared_length());
	if (!s->found || dist < s->dist)
	{
		s->found = true;
		s->e = v;
		s->dist = dist;
	}
}

struct	NearestPFSToPt_t {
	double				lat;
	double				lon;
	double				dist;
	PointFeatureSelection	v;
	bool				found;
};

static void	MakePFSClosestMaybe(PointFeatureSelection& v, NearestPFSToPt_t * s)
{
	double dx = s->lon - (v.first->mPointFeatures[v.second].mLocation.x);
	double dy = s->lon - (v.first->mPointFeatures[v.second].mLocation.y);
	double dist = dx * dx + dy * dy;
	if (!s->found || dist < s->dist)
	{
		s->found = true;
		s->v = v;
		s->dist = dist;
	}
}


void	WED_SelectionTool::DoSelectionPreview()
{
		vector<GISFace *> 		faceitems;
		vector<GISHalfedge *> 	halfedgeitems;
		vector<GISVertex *> 	vertexitems;

	switch(gSelectionMode) {
	case wed_Select_Face:
		{
			if ((mModifiers & (xplm_ShiftFlag + xplm_ControlFlag)) == 0)
				gFaceSelection.clear();
			double	bounds[4];
			if (GetRectMapCoords(bounds))
			{
				gDocument->gMap.FindFaceFullyInRect(
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]),
							faceitems);
				ApplyRange(faceitems.begin(), faceitems.end(), (mModifiers & xplm_ControlFlag) ? InsertFaceInSet : ToggleFaceInSet, &gFaceSelection);
			} else {
				gDocument->gMap.FindFaceTouchesPt(
							Point2(bounds[0], bounds[1]), faceitems);
				ApplyRange(faceitems.begin(), faceitems.end(), (mModifiers & xplm_ControlFlag) ? InsertFaceInSet : ToggleFaceInSet, &gFaceSelection);
			}
		}
		break;
	case wed_Select_Edge:
		{
			if ((mModifiers & (xplm_ShiftFlag + xplm_ControlFlag)) == 0)
				gEdgeSelection.clear();
			double	bounds[4];
			if (GetRectMapCoords(bounds))
			{
				gDocument->gMap.FindHalfedgeFullyInRect(
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]), 
							halfedgeitems);
				ApplyRange(halfedgeitems.begin(), halfedgeitems.end(), (mModifiers & xplm_ControlFlag) ? InsertEdgeInSet : ToggleEdgeInSet, &gEdgeSelection);
			} else {
				NearestEdgeToPt_t t;
				t.found = false;
				t.lon = bounds[0];
				t.lat = bounds[1];			
				gDocument->gMap.FindHalfedgeTouchesRect(
							Point2(GetZoomer()->XPixelToLon(mMouseX - kPointClickSlop), GetZoomer()->YPixelToLat(mMouseY - kPointClickSlop)),
							Point2(GetZoomer()->XPixelToLon(mMouseX + kPointClickSlop), GetZoomer()->YPixelToLat(mMouseY + kPointClickSlop)), halfedgeitems);
				ApplyRange(halfedgeitems.begin(), halfedgeitems.end(),
							MakeEdgeClosestMaybe,
							&t);
				if (t.found)
				{
					if (mModifiers & xplm_ControlFlag)
						InsertEdgeInSet(t.e, &gEdgeSelection);
					else
						ToggleEdgeInSet(t.e, &gEdgeSelection);
				}							
			}
		}
		break;
	case wed_Select_Vertex:
		{
			if ((mModifiers & (xplm_ShiftFlag + xplm_ControlFlag)) == 0)
				gVertexSelection.clear();
			double	bounds[4];
			if (GetRectMapCoords(bounds))
			{
				gDocument->gMap.FindVerticesFullyInRect(
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]), vertexitems);
				ApplyRange(vertexitems.begin(), vertexitems.end(),
							(mModifiers & xplm_ControlFlag) ? InsertVertexInSet : ToggleVertexInSet,
							&gVertexSelection);
			} else {
				NearestVertexToPt_t t;
				t.found = false;
				t.lon = bounds[0];
				t.lat = bounds[1];
				gDocument->gMap.FindVerticesFullyInRect(
							Point2(GetZoomer()->XPixelToLon(mMouseX - kPointClickSlop), GetZoomer()->YPixelToLat(mMouseY - kPointClickSlop)),
							Point2(GetZoomer()->XPixelToLon(mMouseX + kPointClickSlop), GetZoomer()->YPixelToLat(mMouseY + kPointClickSlop)), vertexitems);
				ApplyRange(vertexitems.begin(), vertexitems.end(),
							MakeVertexClosestMaybe,
							&t);
				if (t.found)
				{
					if (mModifiers & xplm_ControlFlag)
						InsertVertexInSet(t.v, &gVertexSelection);
					else
						ToggleVertexInSet(t.v, &gVertexSelection);
				}
			}
		}
		break;	
	case wed_Select_PointFeatures:
		{
			if ((mModifiers & (xplm_ShiftFlag + xplm_ControlFlag)) == 0)
				gPointFeatureSelection.clear();
			double	bounds[4];
			if (GetRectMapCoords(bounds))
			{
				vector<Pmwx::Face_handle>	faces;
				gDocument->gMap.FindFaceTouchesRect(
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]), faces);
				for (vector<Pmwx::Face_handle>::iterator i = faces.begin(); i != faces.end(); ++i)
				{
					for (int j = 0; j  < (*i)->mPointFeatures.size(); ++j)
					{
						double dx = ((*i)->mPointFeatures[j].mLocation.x);
						double dy = ((*i)->mPointFeatures[j].mLocation.y);
						if (bounds[0] <= dx && bounds[1] <= dy && bounds[2] > dx && bounds[3] > dy)
						{
							PointFeatureSelection pfs(*i, j);
							if (mModifiers & xplm_ControlFlag || gPointFeatureSelection.find(pfs) == gPointFeatureSelection.end())
								gPointFeatureSelection.insert(pfs);
							else
								gPointFeatureSelection.erase(pfs);
						}
					}
				}
			} else {
				NearestPFSToPt_t t;
				t.found = false;
				t.lon = bounds[0];
				t.lat = bounds[1];
				vector<Pmwx::Face_handle>	faces;
				gDocument->gMap.FindFaceTouchesRect(
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]), faces);
				for (vector<Pmwx::Face_handle>::iterator i = faces.begin(); i != faces.end(); ++i)
				{
					for (int j = 0; j  < (*i)->mPointFeatures.size(); ++j)
					{
						PointFeatureSelection pfs(*i, j);					
							MakePFSClosestMaybe(pfs,&t);
					}
				}
				if (t.found)
				{
					if (mModifiers & xplm_ControlFlag || gPointFeatureSelection.find(t.v) == gPointFeatureSelection.end())
						gPointFeatureSelection.insert(t.v);
					else
						gPointFeatureSelection.erase(t.v);
				}
			}
		}					
	}
}
