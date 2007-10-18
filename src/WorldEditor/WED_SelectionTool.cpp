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

#include <geos/operation/buffer/BufferOp.h>

#include "WED_MapZoomer.h"
#include "WED_Globals.h"
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
#include "WED_MapView.h"

#include "WED_DrawMap.h"
#include "PlatformUtils.h"

#include "XPLMGraphics.h"
#include "MemFileUtils.h"
#include "Hydro.h"
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

#include "BitmapUtils.h"
#include "TensorRoads.h"

#include "ObjPlacement.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

const	int kPointClickSlop = 4;

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
	return 6;
}

void	WED_SelectionTool::GetNthButtonName(int n, string& s)
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
				gMap.remove_edge(*nukeme);
				DebugAssert(gMap.is_valid());
			}
		}
		break;
	case 1:
		for (set<GISFace *>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel)
		{
			CleanFace(gMap, *fsel);
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
				/*
				GISFace * the_face = *fsel;				
				GISHalfedge * iter, * stop;
				int ct = 0;
				iter = stop = the_face->outer_ccb();
				do {
					++ct;
					iter = iter->next();
				} while (iter != stop);
				
				geos::geom::CoordinateSequence * s = geos::geom::GeometryFactory::getDefaultInstance()->getCoordinateSequenceFactory()->create(ct+1,2);
				ct = 0;
				iter = stop = the_face->outer_ccb();
				do {
					s->setOrdinate(ct,0,iter->target()->point().x);
					s->setOrdinate(ct,1,iter->target()->point().y);
					++ct;
					iter = iter->next();
				} while (iter != stop);
					s->setOrdinate(ct,0,iter->target()->point().x);
					s->setOrdinate(ct,1,iter->target()->point().y);
				
				geos::geom::LinearRing * outer_ccb_ring = geos::geom::GeometryFactory::getDefaultInstance()->createLinearRing(s);
				
				vector<geos::geom::Geometry *>	holes(the_face->holes_count());
				int i = 0;
				for (Pmwx::Holes_iterator h = the_face->holes_begin(); h != the_face->holes_end(); ++h, ++i)
				{
					ct = 0;
					iter = stop = *h;
					do {
						++ct;
						iter = iter->next();
					} while (iter != stop);
					
					geos::geom::CoordinateSequence * s = geos::geom::GeometryFactory::getDefaultInstance()->getCoordinateSequenceFactory()->create(ct+1,2);
					ct = 0;
					iter = stop = *h;
					do {
						s->setOrdinate(ct,0,iter->target()->point().x);
						s->setOrdinate(ct,1,iter->target()->point().y);
						++ct;
						iter = iter->next();
					} while (iter != stop);
					s->setOrdinate(ct,0,iter->target()->point().x);
					s->setOrdinate(ct,1,iter->target()->point().y);
					
					holes[i] =  geos::geom::GeometryFactory::getDefaultInstance()->createLinearRing(s);
				}
				
				std::vector<geos::geom::Geometry *> *vholes = new std::vector<geos::geom::Geometry *>(holes.begin(), holes.end());
				geos::geom::Geometry * poly =  geos::geom::GeometryFactory::getDefaultInstance()->createPolygon(outer_ccb_ring, vholes);

//				geos::geom::Geometry * buf = geos::operation::buffer::BufferOp::bufferOp(poly, -30.0 * MTR_TO_NM * NM_TO_DEG_LAT , 8, geos::operation::buffer::BufferOp::CAP_SQUARE);
				geos::operation::buffer::BufferOp bufOp(poly);
				bufOp.setQuadrantSegments(1);
				bufOp.setEndCapStyle(geos::operation::buffer::BufferOp::CAP_SQUARE);
				geos::geom::Geometry * buf = bufOp.getResultGeometry(-30.0 * MTR_TO_NM * NM_TO_DEG_LAT);


				dump_geos_recursive(buf, (the_face)->mPolyObjs);
				
				delete poly;
				delete buf;
				*/
				
				/*
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
				*/
				
				GISFace * the_face = *fsel;
				Pmwx	result_map;
				
				try {
					InsetPmwx(the_face, result_map,30.0 * MTR_TO_NM * NM_TO_DEG_LAT);
				} catch(...) {
					fail.insert(the_face);
				}
				
				for (Pmwx::Face_iterator f = result_map.faces_begin(); f != result_map.faces_end(); ++f)
				if (!f->is_unbounded())
				if (f->mTerrainType == 1)
				{
					GISPolyObjPlacement_t	obj;
					FaceToComplexPolygon(f,obj.mShape,NULL, NULL, NULL);
					obj.mRepType = NO_VALUE;
					obj.mDerived = true;
					obj.mHeight = 1.0;

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
		{
			ImageInfo	img;
			CreateNewBitmap(512,512,3,&img);
			double		bounds[4];
			for (set<GISFace *>::iterator fsel = gFaceSelection.begin(); fsel != gFaceSelection.end(); ++fsel)
				BuildRoadsForFace(gMap, gDem[dem_Elevation], *fsel,  WED_ProgressFunc, &img, bounds);

			gMapView->SetFlowImage(img,bounds);
			DestroyBitmap(&img);
		}
		return;
	}
	DebugAssert(gMap.is_valid());
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
	double dy = s->lat - (v.first->mPointFeatures[v.second].mLocation.y);
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
				gMap.FindFaceFullyInRect(
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]),
							faceitems);
				ApplyRange(faceitems.begin(), faceitems.end(), (mModifiers & xplm_ControlFlag) ? InsertFaceInSet : ToggleFaceInSet, &gFaceSelection);
			} else {
				gMap.FindFaceTouchesPt(
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
				gMap.FindHalfedgeFullyInRect(
							Point2(bounds[0], bounds[1]),
							Point2(bounds[2], bounds[3]), 
							halfedgeitems);
				ApplyRange(halfedgeitems.begin(), halfedgeitems.end(), (mModifiers & xplm_ControlFlag) ? InsertEdgeInSet : ToggleEdgeInSet, &gEdgeSelection);
			} else {
				NearestEdgeToPt_t t;
				t.found = false;
				t.lon = bounds[0];
				t.lat = bounds[1];			
				gMap.FindHalfedgeTouchesRectFast(
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
				gMap.FindVerticesTouchesRect(
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
				gMap.FindVerticesTouchesRect(
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
				gMap.FindFaceTouchesRectFast(
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
				gMap.FindFaceTouchesRectFast(
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
