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
#include "RF_SelectionTool.h"

//#include <geos/operation/buffer/BufferOp.h>

#include "RF_MapZoomer.h"
#include "RF_Globals.h"
#include "RF_Selection.h"
#include "RF_Progress.h"
#include "MapTopology.h"
#include "MapPolygon.h"
#include "MapBuffer.h"
#include "GISTool_Globals.h"

#include "RF_Notify.h"
#include "RF_Msgs.h"
#include "ParamDefs.h"
#include "AptIO.h"
#include "XESIO.h"

#include "GISUtils.h"
#include "MapAlgs.h"
#include "Skeleton.h"
#include "AssertUtils.h"
#include "RF_MapView.h"

#include "RF_DrawMap.h"
#include "PlatformUtils.h"

#include "GUI_GraphState.h"
#include "MemFileUtils.h"
#include "Hydro.h"
/*
//#include <geos_c.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>

//#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequenceFactory.h>
//#include <geos/geom/IntersectionMatrix.h>
//#include <geos/io/WKTReader.h>
//#include <geos/io/WKBReader.h>
//#include <geos/io/WKBWriter.h>
//#include <geos/simplify/DouglasPeuckerSimplifier.h>
//#include <geos/simplify/TopologyPreservingSimplifier.h>
//#include <geos/operation/valid/IsValidOp.h>
//#include <geos/operation/polygonize/Polygonizer.h>
//#include <geos/operation/linemerge/LineMerger.h>
//#include <geos/operation/overlay/OverlayOp.h>
//#include <geos/geom/BinaryOp.h>
//#include <geos/version.h>
*/
#include "BitmapUtils.h"
#include "TensorRoads.h"

#include "ObjPlacement.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

const	int kPointClickSlop = 4;
/*
void dump_geos_recursive(const geos::geom::Geometry * g, GISPolyObjPlacementVector& objs)
{
	int kids = g->getNumGeometries();
	switch(g->getGeometryTypeId()) {
	case geos::geom::GEOS_MULTIPOINT:
	case geos::geom::GEOS_MULTILINESTRING:
	case geos::geom::GEOS_MULTIPOLYGON:
	case geos::geom::GEOS_GEOMETRYCOLLECTION:
		if (kids > 0)
		{
			const geos::geom::GeometryCollection * c = dynamic_cast<const geos::geom::GeometryCollection *>(g);
			if (c)
			for (int i = 0; i < kids; ++i)
			{
				dump_geos_recursive(c->getGeometryN(i),objs);
			}
		}
		break;
	case geos::geom::GEOS_POINT:
	case geos::geom::GEOS_LINESTRING:
	case geos::geom::GEOS_LINEARRING:
		{
			const geos::geom::LineString * ls = dynamic_cast<const geos::geom::LineString *>(g);
			if (ls)
			{
				const geos::geom::CoordinateSequence * s = ls->getCoordinatesRO();
				GISPolyObjPlacement_t	obj;

						obj.mRepType = NO_VALUE;
						obj.mDerived = true;
						obj.mHeight = 1.0;

						obj.mShape.resize(1);

						unsigned int np = s->getSize();;
					obj.mShape[0].resize(np);
					for (int n = 0; n < np; ++n)
					{
						obj.mShape[0][n] = Point2(s->getOrdinate(n,0),s->getOrdinate(n,1));
					}

					obj.mLocation = obj.mShape[0].centroid();
					objs.push_back(obj);
				}
		}
		break;
	case geos::geom::GEOS_POLYGON:
		{
			const geos::geom::Polygon * p = dynamic_cast<const geos::geom::Polygon *>(g);
			if (p)
			{
				for (int r = 0;r < p->getNumInteriorRing(); ++r)
					dump_geos_recursive(p->getInteriorRingN(r), objs);
				dump_geos_recursive(p->getExteriorRing(), objs);
			}
		}
		break;
	}
}

*/

template <typename __InputIterator, typename  __Ref>
void ApplyRange(__InputIterator begin, __InputIterator end, void (* func)(typename __InputIterator::value_type, __Ref), __Ref ref)
{
	for (__InputIterator i = begin; i != end; ++i)
		func(*i, ref);
}

RF_SelectionTool::RF_SelectionTool(RF_MapZoomer * inZoomer) : RF_MapTool(inZoomer)
{
	mIsDrag = false;
}

void	RF_SelectionTool::DrawFeedbackUnderlay(
							GUI_GraphState *	state,
				bool				inCurrent)
{
}

void	RF_SelectionTool::DrawFeedbackOverlay(
							GUI_GraphState *	state,
				bool				inCurrent)
{
	if (mIsDrag && (mMouseX != mMouseStartX || mMouseY != mMouseStartY))
	{
		int xMin = min(mMouseX, mMouseStartX);
		int yMin = min(mMouseY, mMouseStartY);
		int xMax = max(mMouseX, mMouseStartX);
		int yMax = max(mMouseY, mMouseStartY);

		state->SetState(0, 0, 0,    0, 1,   0, 0);
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

bool	RF_SelectionTool::HandleClick(
				XPLMMouseStatus		inStatus,
				int 				inX,
				int 				inY,
				int 				inButton,
				GUI_KeyFlags		inModifiers)
{
	if (inButton != 0) return 0;
	switch(inStatus) {
	case xplm_MouseDown:
		mModifiers = inModifiers;
		mMouseStartX = mMouseX = inX;
		mMouseStartY = mMouseY = inY;
		mIsDrag = true;
		mIsMoveVertices = false;
		mFaceSelection = gFaceSelection;
		mEdgeSelection = gEdgeSelection;
		mVertexSelection = gVertexSelection;
		mPointFeatureSelection = gPointFeatureSelection;
		DoSelectionPreview();
		if (gSelectionMode == rf_Select_Vertex)
		if ((mModifiers & (gui_ShiftFlag + gui_ControlFlag)) == 0)
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
			Vector_2	delta(lonNow - mMoveLon, latNow - mMoveLat);
			mMoveLon = lonNow;
			mMoveLat = latNow;
			for (set<Pmwx::Vertex_handle>::iterator v = gVertexSelection.begin(); v != gVertexSelection.end(); ++v)
			{
				Point_2 p = (*v)->point();
//				(*v)->point() = p + delta;
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
//				RF_PlaceBuildings(*gFaceSelection.begin());
		}
		if (mIsMoveVertices && (inX != mMouseX || inY != mMouseY))
		{
			double	lonNow = GetZoomer()->XPixelToLon(inX);
			double	latNow = GetZoomer()->YPixelToLat(inY);
			Vector_2	delta(lonNow - mMoveLon, latNow - mMoveLat);
			mMoveLon = lonNow;
			mMoveLat = latNow;
			for (set<Pmwx::Vertex_handle>::iterator v = gVertexSelection.begin(); v != gVertexSelection.end(); ++v)
			{
				Point_2 p = (*v)->point();
//				(*v)->set_point(p + delta);
			}
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
			mIsMoveVertices = false;
		}

		return 1;
	default:
		return 0;
	}
}

static int gStopPt = -1;

int		RF_SelectionTool::GetNumProperties(void)
{
	return 1;
}

void	RF_SelectionTool::GetNthPropertyName(int n, string& s)
{
	s = "Stop:";
}

double	RF_SelectionTool::GetNthPropertyValue(int n)
{
	return gStopPt;
}

void	RF_SelectionTool::SetNthPropertyValue(int, double v)
{
	gStopPt = v;
}

int		RF_SelectionTool::GetNumButtons(void)
{
	return 6;
}

void	RF_SelectionTool::GetNthButtonName(int n, string& s)
{
	switch(n) {
	case 0: s = "Delete"; break;
	case 1: s = "Clean"; break;
	case 2: s=  "Inset"; break;
	case 3: s=  "Clear"; break;
	case 4: s=  "Next"; break;
	case 5: s= "Roads"; break;
	}
}

void	RF_SelectionTool::NthButtonPressed(int n)
{
	GISFace * d = NULL;
	int ctr = 0;
	switch(n) {
	case 0:
		{
			set<Halfedge_handle>	nuke;
			for (set<Halfedge_handle>::iterator sel = gEdgeSelection.begin(); sel != gEdgeSelection.end(); ++sel)
				nuke.insert(he_get_same_direction(*sel));
			for (set<Halfedge_handle>::iterator nukeme = nuke.begin(); nukeme != nuke.end(); ++nukeme)
			{
				gMap.remove_edge(*nukeme);
				DebugAssert(gMap.is_valid());
			}
		}
		break;
	case 1:
		for (set<Face_handle>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel)
		{
			CleanFace(gMap, *fsel);
		}
		break;
	case 2:
	case 4:
		 {
			if (n == 4)
				++gStopPt;

				set<Face_handle>	fail;

			gMeshLines.clear();
			gMeshPoints.clear();
			bool was_empty = gFaceSelection.empty();
			if(was_empty)
			{
				for(Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
				if(!f->is_unbounded())
					gFaceSelection.insert(f);
			}
				Locator						loc(gMap);

			PROGRESS_START(RF_ProgressFunc,0,1,"Insetting")
			for (set<Face_handle>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel, ++ctr)
			{
				PROGRESS_SHOW(RF_ProgressFunc,0,1,"Insetting",ctr,gFaceSelection.size())
				Polygon_with_holes_2		bounds;
				PolyInset_t					lims;
				PolygonFromFace(*fsel, bounds, &lims, GetInsetForEdgeDegs, NULL);
				Polygon_set_2				bs;

				try {

					BufferPolygonWithHoles(bounds, &lims, 1.0 , bs);

					ValidateBuffer(gMap,*fsel, loc, bs);
				} catch (...) {
					fail.insert(*fsel);
					#if DEV
						debug_mesh_point(cgal2ben((*fsel)->outer_ccb()->source()->point()),1,1,0);
					#endif
				}

				vector<Polygon_with_holes_2>	all;

				bs.polygons_with_holes(back_insert_iterator<vector<Polygon_with_holes_2> >(all));

				for(vector<Polygon_with_holes_2>::iterator a = all.begin(); a != all.end(); ++a)
				{
					GISPolyObjPlacement_t	res;
					res.mShape = *a;
					res.mRepType = 1;
					res.mHeight = 20;
					res.mDerived = true;
					(*fsel)->data().mPolyObjs.push_back(res);
				}
			}
			PROGRESS_DONE(RF_ProgressFunc,0,1,"Insetting")
			if (!fail.empty())
			{
				char buf[256];
				sprintf(buf, "%d out of %d failed.\n", fail.size(), gFaceSelection.size());
				DoUserAlert(buf);
	//			RF_Notifiable::Notify(RF_Cat_File, RF_Msg_VectorChange, NULL);
				gEdgeSelection.clear();
				gFaceSelection = fail;
				gVertexSelection.clear();
			}
			else if(was_empty)
				gFaceSelection.clear();
			return;
		}
	case 3:
		for (set<Face_handle>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel)
			(*fsel)->data().mPolyObjs.clear();
		return;
	case 5:
		{
			ImageInfo	img;
			Face_handle f = gFaceSelection.empty() ? Face_handle() : *gFaceSelection.begin();
			if(f != Face_handle())CreateNewBitmap(512,512,3,&img);
			double		bounds[4];
			BuildRoadsForFace(gMap, gDem[dem_Elevation], gDem[dem_Slope], gDem[dem_UrbanDensity], gDem[dem_UrbanRadial], gDem[dem_UrbanSquare], f,  RF_ProgressFunc, f != Face_handle() ? &img : NULL, bounds);
			if(f != Face_handle()){
			gMapView->SetFlowImage(img,bounds);
			DestroyBitmap(&img);}
		}
	}
	DebugAssert(gMap.is_valid());
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
	gEdgeSelection.clear();
}

char *	RF_SelectionTool::GetStatusText(int x, int y)
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

		if (gDem.count(dem_Elevation) != 0)
		{
			double p1 = gDem[dem_Elevation].value_linear(GetZoomer()->XPixelToLon(mMouseX), 	 GetZoomer()->YPixelToLat(mMouseY	  ));
			double p2 = gDem[dem_Elevation].value_linear(GetZoomer()->XPixelToLon(mMouseStartX), GetZoomer()->YPixelToLat(mMouseStartY));
			if (p1 != DEM_NO_DATA && p2 != DEM_NO_DATA)
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
//		if (the_face->data().mTerrainType != terrain_Natural)
			n += sprintf(buf+n,"Art.Terrain:%s ", FetchTokenString(the_face->data().mTerrainType));

		if (the_face->data().mAreaFeature.mFeatType != NO_VALUE)
		{
			n += sprintf(buf+n, "Area Feature:%s ", FetchTokenString(the_face->data().mAreaFeature.mFeatType));
		}

		for(GISParamMap::iterator p = the_face->data().mParams.begin(); p != the_face->data().mParams.end(); ++p)
			n += sprintf(buf+n, "%s:%lf ", FetchTokenString(p->first),p->second);
	}
	if (gEdgeSelection.size() > 1)
	{
		double t = 0;
		for(set<Halfedge_handle>::iterator e = gEdgeSelection.begin(); e != gEdgeSelection.end(); ++e)
		t += GetMapEdgeLengthMeters(*e);
		n += sprintf(buf+n, "%.1f m ", t);
	}

	if (gEdgeSelection.size() == 1)
	{
		double len = GetMapEdgeLengthMeters(*gEdgeSelection.begin());
		n += sprintf(buf+n, "%.1f m ", len);
		for (GISNetworkSegmentVector::iterator seg = (*gEdgeSelection.begin())->data().mSegments.begin(); seg != (*gEdgeSelection.begin())->data().mSegments.end(); ++seg)
		{
			n += sprintf(buf+n, "%s->%s ", FetchTokenString(seg->mFeatType),FetchTokenString(seg->mRepType));
		}
		for (GISNetworkSegmentVector::iterator seg = (*gEdgeSelection.begin())->twin()->data().mSegments.begin(); seg != (*gEdgeSelection.begin())->twin()->data().mSegments.end(); ++seg)
		{
			n += sprintf(buf+n, "%s->%s ", FetchTokenString(seg->mFeatType),FetchTokenString(seg->mRepType));
		}

		for(GISParamMap::iterator p = (*gEdgeSelection.begin())->data().mParams.begin(); p != (*gEdgeSelection.begin())->data().mParams.end(); ++p)
			n += sprintf(buf+n, "%s:%lf ", FetchTokenString(p->first),p->second);
		for(GISParamMap::iterator p = (*gEdgeSelection.begin())->twin()->data().mParams.begin(); p != (*gEdgeSelection.begin())->twin()->data().mParams.end(); ++p)
			n += sprintf(buf+n, "%s:%lf ", FetchTokenString(p->first),p->second);

		if ((*gEdgeSelection.begin())->data().mTransition != 0)
		{
			n += sprintf(buf+n,"Beach=%d ", (*gEdgeSelection.begin())->data().mTransition);
		}
		if ((*gEdgeSelection.begin())->data().mMark != 0)
		{
			n += sprintf(buf+n,"Marked ");
		}
		if ((*gEdgeSelection.begin())->twin()->data().mMark != 0)
		{
			n += sprintf(buf+n,"TwinMarked ");
		}
	}
	if (gPointFeatureSelection.size() == 1)
	{
		GISPointFeature_t * f = &gPointFeatureSelection.begin()->first->data().mPointFeatures[gPointFeatureSelection.begin()->second];
		n += sprintf(buf+n, "%s ", FetchTokenString(f->mFeatType));
		for(GISParamMap::iterator p = f->mParams.begin(); p != f->mParams.end(); ++p)
			n += sprintf(buf+n, "%s:%lf ", FetchTokenString(p->first),p->second);
	}
	if (gVertexSelection.size() == 1 && (*(gVertexSelection.begin()))->data().mTunnelPortal)
	{
		n += sprintf(buf+n, " (tunnel portal)");
	}

	if (gVertexSelection.size() == 1)
	{
		n += sprintf(buf+n, "%lf,%lf %016llX,%016llX ",
			CGAL::to_double((*(gVertexSelection.begin()))->point().x()),
			CGAL::to_double((*(gVertexSelection.begin()))->point().y()),
			CGAL::to_double((*(gVertexSelection.begin()))->point().x()),
			CGAL::to_double((*(gVertexSelection.begin()))->point().y()));
	}

	{
		Bbox2	vis_area(Point2(GetZoomer()->XPixelToLon(x),GetZoomer()->YPixelToLat(y)));
		set<int>	apts;
		FindAirports(vis_area, gAptIndex, apts);
		for (set<int>::iterator e = apts.begin(); e != apts.end(); ++e)
		{
			int k = *e;
			if (vis_area.overlap(gApts[k].bounds))
			{
				n += sprintf(buf+n, "%s %s ", gApts[k].icao.c_str(), gApts[k].name.c_str());
			}
		}
	}




	return buf;
}

bool	RF_SelectionTool::GetRectMapCoords(double coords[4])
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
	double dx = s->lon - CGAL::to_double(v->point().x());
	double dy = s->lat - CGAL::to_double(v->point().y());
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
	Segment2	seg(cgal2ben(v->source()->point()), cgal2ben(v->target()->point()));
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
	double dx = s->lon - CGAL::to_double(v.first->data().mPointFeatures[v.second].mLocation.x());
	double dy = s->lat - CGAL::to_double(v.first->data().mPointFeatures[v.second].mLocation.y());
	double dist = dx * dx + dy * dy;
	if (!s->found || dist < s->dist)
	{
		s->found = true;
		s->v = v;
		s->dist = dist;
	}
}


void	RF_SelectionTool::DoSelectionPreview()
{
		vector<Face_handle> 		faceitems;
		vector<Halfedge_handle> 	halfedgeitems;
		vector<Vertex_handle>		vertexitems;

	switch(gSelectionMode) {
	case rf_Select_Face:
		{
			if ((mModifiers & (gui_ShiftFlag + gui_ControlFlag)) == 0)
				gFaceSelection.clear();
			double	bounds[4];
			if (GetRectMapCoords(bounds))
			{
				FindFaceFullyInRect(gMap,
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]),
							faceitems);
				ApplyRange(faceitems.begin(), faceitems.end(), (mModifiers & gui_ControlFlag) ? InsertFaceInSet : ToggleFaceInSet, &gFaceSelection);
			} else {
				FindFaceTouchesPt(gMap,
							Point2(bounds[0], bounds[1]), faceitems);
				ApplyRange(faceitems.begin(), faceitems.end(), (mModifiers & gui_ControlFlag) ? InsertFaceInSet : ToggleFaceInSet, &gFaceSelection);
			}
		}
		break;
	case rf_Select_Edge:
		{
			if ((mModifiers & (gui_ShiftFlag + gui_ControlFlag)) == 0)
				gEdgeSelection.clear();
			double	bounds[4];
			if (GetRectMapCoords(bounds))
			{
				FindHalfedgeFullyInRect(gMap,
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]),
							halfedgeitems);
				ApplyRange(halfedgeitems.begin(), halfedgeitems.end(), (mModifiers & gui_ControlFlag) ? InsertEdgeInSet : ToggleEdgeInSet, &gEdgeSelection);
			} else {
				NearestEdgeToPt_t t;
				t.found = false;
				t.lon = bounds[0];
				t.lat = bounds[1];
				FindHalfedgeTouchesRectFast(gMap,
							Point2(GetZoomer()->XPixelToLon(mMouseX - kPointClickSlop), GetZoomer()->YPixelToLat(mMouseY - kPointClickSlop)),
							Point2(GetZoomer()->XPixelToLon(mMouseX + kPointClickSlop), GetZoomer()->YPixelToLat(mMouseY + kPointClickSlop)), halfedgeitems);
				ApplyRange(halfedgeitems.begin(), halfedgeitems.end(),
							MakeEdgeClosestMaybe,
							&t);
				if (t.found)
				{
					if (mModifiers & gui_ControlFlag)
						InsertEdgeInSet(t.e, &gEdgeSelection);
					else
						ToggleEdgeInSet(t.e, &gEdgeSelection);
				}
			}
		}
		break;
	case rf_Select_Vertex:
		{
			if ((mModifiers & (gui_ShiftFlag + gui_ControlFlag)) == 0)
				gVertexSelection.clear();
			double	bounds[4];
			if (GetRectMapCoords(bounds))
			{
				FindVerticesTouchesRect(gMap,
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]), vertexitems);
				ApplyRange(vertexitems.begin(), vertexitems.end(),
							(mModifiers & gui_ControlFlag) ? InsertVertexInSet : ToggleVertexInSet,
							&gVertexSelection);
			} else {
				NearestVertexToPt_t t;
				t.found = false;
				t.lon = bounds[0];
				t.lat = bounds[1];
				FindVerticesTouchesRect(gMap,
							Point2(GetZoomer()->XPixelToLon(mMouseX - kPointClickSlop), GetZoomer()->YPixelToLat(mMouseY - kPointClickSlop)),
							Point2(GetZoomer()->XPixelToLon(mMouseX + kPointClickSlop), GetZoomer()->YPixelToLat(mMouseY + kPointClickSlop)), vertexitems);
				ApplyRange(vertexitems.begin(), vertexitems.end(),
							MakeVertexClosestMaybe,
							&t);
				if (t.found)
				{
					if (mModifiers & gui_ControlFlag)
						InsertVertexInSet(t.v, &gVertexSelection);
					else
						ToggleVertexInSet(t.v, &gVertexSelection);
				}
			}
		}
		break;
	case rf_Select_PointFeatures:
		{
			if ((mModifiers & (gui_ShiftFlag + gui_ControlFlag)) == 0)
				gPointFeatureSelection.clear();
			double	bounds[4];
			if (GetRectMapCoords(bounds))
			{
				vector<Pmwx::Face_handle>	faces;
				FindFaceTouchesRectFast(gMap,
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]), faces);
				for (vector<Pmwx::Face_handle>::iterator i = faces.begin(); i != faces.end(); ++i)
				{
					for (int j = 0; j  < (*i)->data().mPointFeatures.size(); ++j)
					{
						double dx = CGAL::to_double((*i)->data().mPointFeatures[j].mLocation.x());
						double dy = CGAL::to_double((*i)->data().mPointFeatures[j].mLocation.y());
						if (bounds[0] <= dx && bounds[1] <= dy && bounds[2] > dx && bounds[3] > dy)
						{
							PointFeatureSelection pfs(*i, j);
							if (mModifiers & gui_ControlFlag || gPointFeatureSelection.find(pfs) == gPointFeatureSelection.end())
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
				FindFaceTouchesRectFast(gMap,
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]), faces);
				for (vector<Pmwx::Face_handle>::iterator i = faces.begin(); i != faces.end(); ++i)
				{
					for (int j = 0; j  < (*i)->data().mPointFeatures.size(); ++j)
					{
						PointFeatureSelection pfs(*i, j);
							MakePFSClosestMaybe(pfs,&t);
					}
				}
				if (t.found)
				{
					if (mModifiers & gui_ControlFlag || gPointFeatureSelection.find(t.v) == gPointFeatureSelection.end())
						gPointFeatureSelection.insert(t.v);
					else
						gPointFeatureSelection.erase(t.v);
				}
			}
		}
	}
}
