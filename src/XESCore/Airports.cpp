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

#include "Airports.h"
#include "MapDefs.h"
#include "MapPolygon.h"
#include "MapTopology.h"
#include "MapAlgs.h"
#include "MapOverlay.h"
#include "AptDefs.h"
#include "AssertUtils.h"
#include "XESConstants.h"
#include "GISUtils.h"
#include "ParamDefs.h"
#include "DEMDefs.h"
#include "DEMAlgs.h"
#include "MapAlgs.h"
#include "CompGeomUtils.h"
#include "MapBuffer.h"
#include <CGAL/convex_hull_2.h>
#include <CGAL/Sweep_line_2_algorithms.h>
#include <CGAL/Boolean_set_operations_2/Gps_polygon_validation.h>
#if DEV
#include "GISTool_Globals.h"
#endif

// Maximum error for bezier from seg to panic!
#define MAX_ERR_APT_BEZ_CHECK 0.0
#define OKAY_WITH_BEZ_BORDERS true


#define WANT_NEW_BORDER_RULES 0

#define KILL_IF_APT_LEAK 1

#define DEBUG_FLATTENING 0

#if DEBUG_FLATTENING
#include "GISTool_Globals.h"
#endif

#define AIRPORT_BEZIER_TESS	(25.0 / (DEG_TO_NM_LAT * NM_TO_MTR))



#define	AIRPORT_OUTER_SIMPLIFY	(25.0 / (DEG_TO_NM_LAT * NM_TO_MTR))		// would be 100 but keep low to preserve outer hull!
#define AIRPORT_INNER_SIMPLIFY	(25.0 / (DEG_TO_NM_LAT * NM_TO_MTR))
#define	AIRPORT_OUTER_FILLGAPS	(200.0 / (DEG_TO_NM_LAT * NM_TO_MTR))
#define AIRPORT_INNER_FILLGAPS	(50.0 / (DEG_TO_NM_LAT * NM_TO_MTR))
#define AIRPORT_OUTER_FILL_AREA		(400.0 * 400.0 / (DEG_TO_NM_LAT * NM_TO_MTR * DEG_TO_NM_LAT * NM_TO_MTR))
#define AIRPORT_INNER_FILL_AREA		(40.0 * 40.0 / (DEG_TO_NM_LAT * NM_TO_MTR * DEG_TO_NM_LAT * NM_TO_MTR))

enum apt_fill_mode {
	fill_nukeroads,		// Hard splat of roads underneath pavement...only for when user made the boundaries.
	fill_water2apt,		// Water becomes airport - tightest radius - ensure airport under runways.
	fill_water2dirt,	// Slightly wider...make sure we have buffer around water.  Area in this but not above becomes embankment.
	fill_dirt2apt		// widest - if there is land, declare it part of the airport, but only if already ry.
};


static void GetPadWidth(
		bool						is_rwy,
		double&						pad_width_m,
		double&						pad_length_m,
		apt_fill_mode				fill_water)
{
	if (fill_water == fill_nukeroads)
	{
		pad_width_m = 10.0;
		pad_length_m = 10.0;
	}
	else if (fill_water == fill_dirt2apt)
	{
		pad_width_m = is_rwy ? 50.0 : 40.0;
		pad_length_m = is_rwy ? 150.0 : 40.0;
	} else if (fill_water == fill_water2apt) {
		pad_width_m = is_rwy ? 20.0 : 15.0;
		pad_length_m = is_rwy ? 20.0 : 15.0;
	} else {
		pad_width_m = is_rwy ? 30.0 : 25.0;
		pad_length_m = is_rwy ? 30.0 : 25.0;
	}
}

// Calculate the quad polygonal bounds of a runway, padded by a certain number of meters.
static void ExpandRunway(
				const AptRunway_t*		rwy,
				double					pad_width_meters,
				double					pad_length_meters,
				Point2					pts[4])
{
	double	aspect = cos(rwy->ends.midpoint().y() * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;

	double rwy_len = LonLatDistMetersWithScale(rwy->ends.p1.x(), rwy->ends.p1.y(), rwy->ends.p2.x(), rwy->ends.p2.y(), DEG_TO_MTR_LON, DEG_TO_MTR_LAT);
	Vector2	rwy_dir(rwy->ends.p1,  rwy->ends.p2);
	rwy_dir.dx *= DEG_TO_MTR_LON;
	rwy_dir.dy *= DEG_TO_MTR_LAT;

	rwy_dir.normalize();

	Vector2	rwy_right = rwy_dir.perpendicular_cw();
	Vector2	rwy_left = rwy_dir.perpendicular_ccw();
	rwy_right *= (rwy->width_mtr * 0.5 + pad_width_meters);
	rwy_left *= (rwy->width_mtr * 0.5 + pad_width_meters);

	rwy_left.dx *= MTR_TO_DEG_LON;
	rwy_left.dy *= MTR_TO_DEG_LAT;
	rwy_right.dx *= MTR_TO_DEG_LON;
	rwy_right.dy *= MTR_TO_DEG_LAT;

	Segment2	real_ends;
	real_ends.p1 = rwy->ends.midpoint(-(pad_length_meters + rwy->blas_mtr[0]) / rwy_len);
	real_ends.p2 = rwy->ends.midpoint(1.0 + (pad_length_meters + rwy->blas_mtr[1]) / rwy_len);

	pts[3] = real_ends.p1 + rwy_left;
	pts[2] = real_ends.p2 + rwy_left;
	pts[1] = real_ends.p2 + rwy_right;
	pts[0] = real_ends.p1 + rwy_right;
}

// Calculate the quad polygonal bounds of a runway, padded by a certain number of meters.
static void ExpandRunway(
				const AptHelipad_t* 	rwy,
				double					pad_width_meters,
				double					pad_length_meters,
				Point2					pts[4])
{
	double	aspect = cos(rwy->location.y() * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;

	double	heading_corrected = (rwy->heading) * DEG_TO_RAD;
	Vector2	delta,cross;
	delta.dx = (rwy->length_mtr * 0.5 + pad_length_meters) * sin(heading_corrected);
	delta.dy = (rwy->length_mtr * 0.5 + pad_length_meters) * cos(heading_corrected);
	cross.dx = (rwy->width_mtr * 0.5 + pad_width_meters) * cos(heading_corrected);
	cross.dy = (rwy->width_mtr * 0.5 + pad_width_meters) *-sin(heading_corrected);

	delta.dx *= MTR_TO_DEG_LON;
	delta.dy *= MTR_TO_DEG_LAT;
	cross.dx *= MTR_TO_DEG_LON;
	cross.dy *= MTR_TO_DEG_LAT;

	pts[3] = rwy->location - delta - cross;
	pts[2] = rwy->location + delta - cross;
	pts[1] = rwy->location + delta + cross;
	pts[0] = rwy->location - delta + cross;
}


// Calculate the quad polygonal bounds of a runway, padded by a certain number of meters.
static void ExpandRunway(
				const AptPavement_t * 	rwy,
				double					pad_width_meters,
				double					pad_length_meters,
				Point2					pts[4])
{
	double	aspect = cos(rwy->ends.midpoint().y() * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;

	double rwy_len = LonLatDistMetersWithScale(rwy->ends.p1.x(), rwy->ends.p1.y(), rwy->ends.p2.x(), rwy->ends.p2.y(), DEG_TO_MTR_LON, DEG_TO_MTR_LAT);
	Vector2	rwy_dir(rwy->ends.p1,  rwy->ends.p2);
	rwy_dir.dx *= DEG_TO_MTR_LON;
	rwy_dir.dy *= DEG_TO_MTR_LAT;

	rwy_dir.normalize();

	Vector2	rwy_right = rwy_dir.perpendicular_cw();
	Vector2	rwy_left = rwy_dir.perpendicular_ccw();
	rwy_right *= (rwy->width_ft * 0.5 * FT_TO_MTR + pad_width_meters);
	rwy_left *= (rwy->width_ft * 0.5 * FT_TO_MTR + pad_width_meters);

	rwy_left.dx *= MTR_TO_DEG_LON;
	rwy_left.dy *= MTR_TO_DEG_LAT;
	rwy_right.dx *= MTR_TO_DEG_LON;
	rwy_right.dy *= MTR_TO_DEG_LAT;

	Segment2	real_ends;
	real_ends.p1 = rwy->ends.midpoint(-(pad_length_meters + rwy->blast1_ft * FT_TO_MTR) / rwy_len);
	real_ends.p2 = rwy->ends.midpoint(1.0 + (pad_length_meters + rwy->blast2_ft * FT_TO_MTR) / rwy_len);

	pts[3] = real_ends.p1 + rwy_left;
	pts[2] = real_ends.p2 + rwy_left;
	pts[1] = real_ends.p2 + rwy_right;
	pts[0] = real_ends.p1 + rwy_right;
}

// Given an apt.dat layout in memory and a fill-mode, calculate the basic outline.
void BurnInAirport(
				const AptInfo_t * 	inAirport,
				Polygon_set_2&		outArea,
				apt_fill_mode		inFillWater)
{
	// STEP 1
	// Go through and burn every runway into the layout.  We will introduce
	// a series of connected faces with each quad we burn in.  We accumulate
	// them all so we have the area of all pavement that was introduced.

	outArea.clear();

	if (!inAirport->boundaries.empty() && 
#if WANT_NEW_BORDER_RULES 	
		inFillWater != fill_nukeroads
#else
		inFillWater == fill_dirt2apt
#endif		
		)
	{
		// Precomputed boundary?  Use it!

		for (AptBoundaryVector::const_iterator b = inAirport->boundaries.begin(); b != inAirport->boundaries.end(); ++b)
		{
			vector<vector<Bezier2> >	bez_poly;
			AptPolygonToBezier(b->area, bez_poly, OKAY_WITH_BEZ_BORDERS);
			for (vector<vector<Bezier2> >::iterator w = bez_poly.begin(); w != bez_poly.end(); ++w)
			{
				Polygon_2	winding;
				BezierToSegments(*w, winding,MAX_ERR_APT_BEZ_CHECK);				
				if(w==bez_poly.begin())
				{
					if(!winding.is_counterclockwise_oriented())
						winding.reverse_orientation();
					outArea.join(winding);
				}
				else
				{
					if(!winding.is_counterclockwise_oriented())
						winding.reverse_orientation();
					outArea.difference(winding);
				}
			}
		}
	}
	else
	{
		vector<Polygon_2>	poly_vec;
		for (int rwy = 0; rwy < inAirport->pavements.size(); ++rwy)
		if (inAirport->pavements[rwy].surf_code != apt_surf_water)
		{
			Point2	corners[4];
			double	pad_width;
			double	pad_height;
			GetPadWidth(inAirport->pavements[rwy].name != "xxx", pad_width, pad_height, inFillWater);
			ExpandRunway(&inAirport->pavements[rwy],pad_width, pad_height, corners);

			Polygon_2	poly;
			poly.push_back(ben2cgal<Point_2>(corners[0]));
			poly.push_back(ben2cgal<Point_2>(corners[1]));
			poly.push_back(ben2cgal<Point_2>(corners[2]));
			poly.push_back(ben2cgal<Point_2>(corners[3]));
			Traits_2 traits;
			DebugAssert(CGAL::is_valid_polygon(poly,traits));
			poly_vec.push_back(poly);
		}
		for (int rwy = 0; rwy < inAirport->runways.size(); ++rwy)
		if (inAirport->runways[rwy].surf_code != apt_surf_water)
		{
			Point2	corners[4];
			double	pad_width;
			double	pad_height;
			GetPadWidth(true,pad_width, pad_height, inFillWater);
			ExpandRunway(&inAirport->runways[rwy],pad_width, pad_height, corners);

			Polygon_2	poly;
			poly.push_back(ben2cgal<Point_2>(corners[0]));
			poly.push_back(ben2cgal<Point_2>(corners[1]));
			poly.push_back(ben2cgal<Point_2>(corners[2]));
			poly.push_back(ben2cgal<Point_2>(corners[3]));

			Traits_2 traits;
			DebugAssert(CGAL::is_valid_polygon(poly,traits));
			poly_vec.push_back(poly);

		}
		for (int rwy = 0; rwy < inAirport->helipads.size(); ++rwy)
		if (inAirport->helipads[rwy].surface_code != apt_surf_water)
		{
			Point2	corners[4];
			double	pad_width;
			double	pad_height;
			GetPadWidth(false,pad_width, pad_height, inFillWater);
			ExpandRunway(&inAirport->helipads[rwy],pad_width, pad_height, corners);

			Polygon_2	poly;
			poly.push_back(ben2cgal<Point_2>(corners[0]));
			poly.push_back(ben2cgal<Point_2>(corners[1]));
			poly.push_back(ben2cgal<Point_2>(corners[2]));
			poly.push_back(ben2cgal<Point_2>(corners[3]));

			Traits_2 traits;
			DebugAssert(CGAL::is_valid_polygon(poly,traits));
			poly_vec.push_back(poly);
		}
		for (AptTaxiwayVector::const_iterator b = inAirport->taxiways.begin(); b != inAirport->taxiways.end(); ++b)
		if(b->surface_code != apt_surf_transparent)
		{
			vector<vector<Bezier2> >	bez_poly;
			Polygon_2					winding;
			AptPolygonToBezier(b->area, bez_poly, true);
			BezierToSegments(bez_poly.front(), winding,0.0);
			Polygon_2					convex_hull;
			
			vector<Polygon_2>			winding_fixed;
			MakePolygonSimple(winding,winding_fixed);

			poly_vec.insert(poly_vec.end(),winding_fixed.begin(),winding_fixed.end());
		}

		for(vector<Polygon_2>::iterator p  = poly_vec.begin(); p != poly_vec.end(); ++p)
		{
			Traits_2 traits;
			DebugAssert(CGAL::is_valid_polygon(*p,traits));
		}
		DebugAssert(outArea.is_valid());
		outArea.join(poly_vec.begin(), poly_vec.end());
		DebugAssert(outArea.is_valid());
	}

	if(inFillWater != fill_dirt2apt && inFillWater != fill_nukeroads && inAirport->boundaries.empty())
	{
		// In water-filling mode, we also fill in holes.  IN other words, if we form a ring of taxiways into the water, we fill in the
		// area inside the ring so that airports don't have little lakes inside them.
		// But in dirt-filling mode, we do NOT do this.  This is because some very large airports might (theoretically) fully surround bits of city
		// with their 2-mile long runways.
		
		// If the user provided a layout, we do not do this.  If users bothered to make a hole, assume it is there for a reason.

		for(Arrangement_2::Face_iterator f = outArea.arrangement().faces_begin(); f != outArea.arrangement().faces_end(); ++f)
		if(!f->is_unbounded())
			f->set_contained(true);
		outArea.remove_redundant_edges();
		outArea.fix_curves_direction();
	}
	
	if(inFillWater == fill_water2apt && !inAirport->boundaries.empty())
	{
		// If the user provided a boundary then we have to inset slightly for the inner one because we need two
		// distinct rings and the user provides only one.  If the airport isn't water-bounded, this gets
		// optimized away.
		Polygon_set_2 orig(outArea);
		BufferPolygonSet(orig, 5.0 * MTR_TO_DEG_LAT, outArea);
	}
	
	if (!inAirport->boundaries.empty() && inFillWater == fill_nukeroads)
	{
		// 'clip' to boundary.
		Polygon_set_2	clip_area;

		for (AptBoundaryVector::const_iterator b = inAirport->boundaries.begin(); b != inAirport->boundaries.end(); ++b)
		{
			vector<vector<Bezier2> >	bez_poly;
			AptPolygonToBezier(b->area, bez_poly, OKAY_WITH_BEZ_BORDERS);
			for (vector<vector<Bezier2> >::iterator w = bez_poly.begin(); w != bez_poly.end(); ++w)
			{
				Polygon_2	winding;
				BezierToSegments(*w, winding,MAX_ERR_APT_BEZ_CHECK);				
				if(w==bez_poly.begin())
				{
					if(!winding.is_counterclockwise_oriented())
						winding.reverse_orientation();
					clip_area.join(winding);
				}
				else
				{
					if(!winding.is_counterclockwise_oriented())
						winding.reverse_orientation();
					clip_area.difference(winding);
				}
			}
		}		
		outArea.intersection(clip_area);
	}
}


//bug since we are using 'overlay' method to preserve roads we are not filling in water - thus JFK is too fucking wet?

// Given a destination map and faces from a DIFFERENT map, we produce a NEW set of SIMPLER faces that are made
// by reducing the polygon complexity.  THEN dump the results into our inDstMap map, and return the actual 
// faces that we made...clients need that.
void	SimplifyAirportAreasAndSplat(Pmwx& inDstMap, Polygon_set_2& in_area, bool do_simplify, set<Face_handle>& outDstFaces, apt_fill_mode inFillWater, Locator * loc)
{
	Polygon_set_2	area(in_area);
	if(do_simplify)
	{
		FillPolygonGaps(area, inFillWater != fill_dirt2apt ? AIRPORT_INNER_FILLGAPS : AIRPORT_OUTER_FILLGAPS);
		SafeMakeMoreConvex(area, inFillWater != fill_dirt2apt ? AIRPORT_INNER_FILL_AREA : AIRPORT_OUTER_FILL_AREA);
		SimplifyPolygonMaxMove(area, inFillWater != fill_dirt2apt ? AIRPORT_INNER_SIMPLIFY : AIRPORT_OUTER_SIMPLIFY);//, true, false);
		DebugAssert(area.arrangement().unbounded_face()->contained() == false);
		#if DEV
		for(Pmwx::Edge_iterator e = area.arrangement().edges_begin(); e != area.arrangement().edges_end(); ++e)
		{
			DebugAssert(e->face() != e->twin()->face());
		}
		#endif
	}
	if (inFillWater == fill_dirt2apt || (inFillWater != fill_nukeroads && !do_simplify))
	{
		// Merge in airports, leaving roads, etc.
		MapMergePolygonSet(inDstMap, area, &outDstFaces, loc);
		for (set<Face_handle>::iterator f = outDstFaces.begin(); f != outDstFaces.end(); ++f)
		{
			DebugAssert(!(*f)->is_unbounded());
			DebugAssert((*f)->data().mTerrainType != 0xDEADBEEF);
			
			// the fill rules:
			// 1. We fill if we are dry OR we are in one of the water->fill modes (water2dirt, water2apt, nuke roads, which is really water2apt)
			// 2. We aren't already airport -avoid down-grading terrain
			// -> nukeroads and water2apt get inner terrain, the rest get outer.
			
			if(!(*f)->data().IsWater() || inFillWater != fill_dirt2apt)
			if((*f)->data().mTerrainType != terrain_Airport)
				(*f)->data().mTerrainType = (inFillWater == fill_water2apt || inFillWater == fill_nukeroads) ?  terrain_Airport : terrain_AirportOuter;	// Airport outer - this is POSSIBLE airport terrain, unless we are under water.  We will resolve this later.

			(*f)->data().mAreaFeature.mFeatType = NO_VALUE;		//Remove area features but do not set LU yet.
		}
		#if KILL_IF_APT_LEAK
		Assert((outDstFaces.size()+1) < inDstMap.number_of_faces());
		#endif
	}
	else
	{
		// Splat-overlay, set two levels of airport terrain.
		MapOverlayPolygonSet(inDstMap, area, loc, &outDstFaces);
		for (set<Face_handle>::iterator f = outDstFaces.begin(); f != outDstFaces.end(); ++f)
		{
			DebugAssert(!(*f)->is_unbounded());
			DebugAssert((*f)->data().mTerrainType != 0xDEADBEEF);

			if(!(*f)->data().IsWater() || inFillWater != fill_dirt2apt)
			if((*f)->data().mTerrainType != terrain_Airport)
				(*f)->data().mTerrainType = (inFillWater == fill_water2apt || inFillWater == fill_nukeroads) ?  terrain_Airport : terrain_AirportOuter;	// Airport outer - this is POSSIBLE airport terrain, unless we are under water.  We will resolve this later.

			(*f)->data().mAreaFeature.mFeatType = NO_VALUE;		//Remove area features but do not set LU yet.
		}
		#if KILL_IF_APT_LEAK
		Assert((outDstFaces.size()+1) < inDstMap.number_of_faces());
		#endif
	}
}

bool NeighborsWater(Face_handle f)
{
	Pmwx::Ccb_halfedge_circulator circ,stop;
	circ = stop = f->outer_ccb();
	do {
		++circ;
		if (circ->twin()->face()->data().IsWater()) return true;
	} while (circ != stop);
	for(Pmwx::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
	{
		circ = stop = *h;
		do {
			++circ;
			if (circ->twin()->face()->data().IsWater()) return true;
		} while (circ != stop);
	}
	return false;
}

static void mask_with(DEMGeo& dst, const DEMGeo& src)
{
	DEMGeo::iterator d;
	DEMGeo::const_iterator s;
	for(d = dst.begin(), s = src.begin(); d != dst.end(); ++s, ++d)
	if(*s == DEM_NO_DATA)
		*d = DEM_NO_DATA;
}

void ProcessAirports(const AptVector& apts, Pmwx& ioMap, DEMGeo& elevation, DEMGeo& transport, bool crop, bool dems, bool kill_rivers, ProgressFunc prog)
{
//	double wet_area = 0;
//	for(Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
//	if(!f->is_unbounded())
//	if(f->data().IsWater())
//	{
//		wet_area += GetMapFaceAreaMeters(f,NULL);
//	}
//	printf("BEFORE: %lf sq meters.\n", wet_area);

	int x1, x2, x, y1, y2, y;
	Point_2 p1, p2;
	if (crop)
		CalcBoundingBox(ioMap, p1, p2);

	set<Face_handle>		simple_faces;

	DEMGeo		working(elevation.mWidth,elevation.mHeight);
	DEMGeo		transport_src(transport.mWidth, transport.mHeight);

	#if DEBUG_FLATTENING
	gDem[dem_Wizard] = elevation;
	gDem[dem_Wizard] = DEM_NO_DATA;
	gDem[dem_Wizard1] = elevation;
	gDem[dem_Wizard1] = DEM_NO_DATA;
	#endif

	if (dems)
	{
		transport_src.copy_geo_from(transport);
		transport_src = 1.0;
	}

	PROGRESS_START(prog, 0, 1, "Burning in airports...")

	// First we will burn in airport landuse onto the big map for every airport.
	// Pass 1 - water-fill...a tight boundary to ensure land under everyone...only do this
	// if we do NOT have a user-specified boundary.
	for (int n = 0; n < apts.size(); ++n)
	if (apts[n].kind_code == apt_airport)
	{
		PROGRESS_SHOW(prog, 0, 1, "Burning in airports...", n, apts.size()*2);
		Polygon_set_2	foo;
		BurnInAirport(&apts[n], foo, fill_water2dirt);					// Produce a map that is the airport boundary.
		DebugAssert(foo.arrangement().unbounded_face()->contained() == false);
		if(!foo.is_empty())																// Check for empty airport (e.g. all sea plane lanes or somthing.)
			SimplifyAirportAreasAndSplat(ioMap, foo, apts[n].boundaries.empty(), simple_faces, fill_water2dirt, NULL);		// Simplify the airport surface area a bit.
	}

	for (int n = 0; n < apts.size(); ++n)
	if (apts[n].kind_code == apt_airport)
	{
		PROGRESS_SHOW(prog, 0, 1, "Burning in airports...", n, apts.size()*2);
		Polygon_set_2	foo;
		BurnInAirport(&apts[n], foo, fill_water2apt);					// Produce a map that is the airport boundary.
		DebugAssert(foo.arrangement().unbounded_face()->contained() == false);
		if(!foo.is_empty())																// Check for empty airport (e.g. all sea plane lanes or somthing.)
			SimplifyAirportAreasAndSplat(ioMap, foo, apts[n].boundaries.empty(), simple_faces, fill_water2apt, NULL);		// Simplify the airport surface area a bit.
	}

#if WANT_NEW_BORDER_RULES
	for (int n = 0; n < apts.size(); ++n)
	if (apts[n].kind_code == apt_airport)
	if(!apts[n].boundaries.empty())
	{
		PROGRESS_SHOW(prog, 0, 1, "Burning in airports...", n, apts.size()*2);
		Polygon_set_2	foo;
		BurnInAirport(&apts[n], foo, fill_nukeroads);					// Produce a map that is the airport boundary.
		DebugAssert(foo.arrangement().unbounded_face()->contained() == false);
		if(!foo.is_empty())																// Check for empty airport (e.g. all sea plane lanes or somthing.)
			SimplifyAirportAreasAndSplat(ioMap, foo, apts[n].boundaries.empty(), simple_faces, fill_nukeroads, NULL);		// Simplify the airport surface area a bit.
	}
#endif

	// Pass 2 - wide boundaries, kill roads but not water, and burn DEM.
	// BUT...if we have user-specified boundaries, this is the only pass and we do fill water.

#if DEV && OPENGL_MAP	
	gDem[dem_Wizard] = elevation;
	gDem[dem_Wizard] = DEM_NO_DATA;
	gDem[dem_Wizard1] = gDem[dem_Wizard];
	gDem[dem_Wizard2] = gDem[dem_Wizard];
	gDem[dem_Wizard3] = gDem[dem_Wizard];
	gDem[dem_Wizard4] = gDem[dem_Wizard];
	gDem[dem_Wizard5] = gDem[dem_Wizard];
	gDem[dem_Wizard6] = gDem[dem_Wizard];
#endif
	
	for (int n = 0; n < apts.size(); ++n)
	if (apts[n].kind_code == apt_airport)
	{
		PROGRESS_SHOW(prog, 0, 1, "Burning in airports...", n+apts.size(), apts.size()*2);
		Polygon_set_2	foo;
		BurnInAirport(&apts[n], foo, fill_dirt2apt);
		DebugAssert(foo.arrangement().unbounded_face()->contained() == false);
		if(!foo.is_empty())																// Check for empty airport (e.g. all sea plane lanes or somthing.)
		{
			SimplifyAirportAreasAndSplat(ioMap, foo, apts[n].boundaries.empty(), simple_faces, fill_dirt2apt, NULL);
			if (dems)
			{
				working = DEM_NO_DATA;
				if (ClipDEMToFaceSet(simple_faces, elevation, working, x1, y1, x2, y2))
				{
					working.copy_geo_from(elevation);
					#if PHONE
						x1-=2;
						y1-=2;
						x2+=2;
						y2+=2;
						SpreadDEMValues(working, 2, x1, y1, x2, y2);
					#else
						--x1;
						--y1;
						++x2;
						++y2;
//						SpreadDEMValues(working, 1, x1, y1, x2, y2);
						dem_copy_buffer_one(elevation, working, DEM_NO_DATA);
					#endif
					DEMGeo		airport_area;
					working.subset(airport_area, x1, y1, x2-1,y2-1);
					
					#if DEBUG_FLATTENING
					gDem[dem_Wizard].overlay(airport_area, x1,y1);
					#endif

#if DEV && OPENGL_MAP && 0
					DEMGeo a(airport_area);
					a += (-(float) apts[n].elevation_ft * FT_TO_MTR);
					gDem[dem_Wizard].overlay(a, x1,y1);
					float sigma[6] = { 1.0, 2.0, 3.0, 4.0, 6.0, 8.0 };
					for(int k = 0; k < 6; ++k)					
					{
						DEMGeo t(airport_area);
						GaussianBlurDEM(t,sigma[k]);
						mask_with(t,airport_area);
						t += (-(float) apts[n].elevation_ft * FT_TO_MTR);
						gDem[dem_Wizard1+k].overlay(t, x1,y1);
						
					}
#endif					
					GaussianBlurDEM(airport_area,3.0);
					#if DEBUG_FLATTENING
					gDem[dem_Wizard1].overlay(airport_area, x1,y1);
					#endif
					
					#if PHONE
					for(y = 0; y < airport_area.mHeight; ++y)
					for(x = 0; x < airport_area.mWidth ; ++x)
						if(airport_area.get(x,y) != DEM_NO_DATA)
							airport_area(x,y) = (float) apts[n].elevation_ft * FT_TO_MTR;
					#endif
					for (y = y1; y < y2; ++y)
					for (x = x1; x < x2; ++x)
					{
						if (working.get(x,y) != DEM_NO_DATA && airport_area(x-x1,y-y1) != DEM_NO_DATA)
							working(x,y) = airport_area(x-x1,y-y1);
					}
				}
				elevation.overlay(working);
				ClipDEMToFaceSet(simple_faces, transport_src, transport, x1, y1, x2, y2);
			}
		}
	}
	
	PROGRESS_DONE(prog, 0, 1, "Burning in airports...")

	// Promote outer airport when possible.  Basically...if the terrain type is outer airport AND we are not near
	// water, just call it airport.  This will wipe out an unneeded border between the two levels of water->not water fill.
	for(Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
	if (f->data().mTerrainType == terrain_AirportOuter)
	if (!f->is_unbounded())
	{
		if(!IsAdjacentWater(f, false))
			f->data().mTerrainType = terrain_Airport;
	}

	// Now - any remaining outer airport is there because it is adjacent to water and the edge is important.  Mark it as a "forced-burn-in" case for later.
	for(Pmwx::Edge_iterator he = ioMap.edges_begin(); he != ioMap.edges_end(); ++he)
	if ((he->face()->data().mTerrainType == terrain_Airport && he->twin()->face()->data().mTerrainType == terrain_AirportOuter) ||
		(he->face()->data().mTerrainType == terrain_AirportOuter && he->twin()->face()->data().mTerrainType == terrain_Airport))
	{
		he->data().mParams[he_MustBurn] = 1.0;
	}


	// Okay.  We have preserved our areas...outer boundary that is left is adjacent to water.  Promote it too now.
	for(Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
	if (f->data().mTerrainType == terrain_AirportOuter)
	if (!f->is_unbounded())
		f->data().mTerrainType = terrain_Airport;

	if (crop)
	{
		CropMap(ioMap, CGAL::to_double(p1.x()), CGAL::to_double(p1.y()), CGAL::to_double(p2.x()), CGAL::to_double(p2.y()), false, prog);
		SimplifyMap(ioMap, kill_rivers, prog);
	}
	
	
	
//	wet_area = 0;
//	for(Pmwx::Face_iterator f = ioMap.faces_begin(); f != ioMap.faces_end(); ++f)
//	if(!f->is_unbounded())
//	if(f->data().IsWater())
//	{
//		wet_area += GetMapFaceAreaMeters(f,NULL);
//	}
//	printf("AFTER: %lf sq meters.\n", wet_area);
	

}
void	GenBoundary(
				AptInfo_t * 	ioAirport)
{
	if (!ioAirport->boundaries.empty()) return;

		Polygon_set_2			area;

	BurnInAirport(ioAirport, area, fill_dirt2apt);

	Pmwx	foo(area.arrangement());

	for(Pmwx::Face_iterator f = foo.faces_begin(); f != foo.faces_end(); ++f)
	if (!f->is_unbounded())
	if(f->contained())
	{
		ioAirport->boundaries.push_back(AptBoundary_t());
		AptPolygon_t * p = &ioAirport->boundaries.back().area;
		Pmwx::Ccb_halfedge_circulator circ(f->outer_ccb()),stop(f->outer_ccb());
		do {
			++circ;
			p->push_back(AptLinearSegment_t());
			p->back().code = (circ == stop ? apt_rng_seg : apt_lin_seg);
			p->back().pt = cgal2ben(circ->target()->point());
		} while(circ != stop);

		// This is not true because we intentionally leave holes in airports that might surround an area...whether this is a good idea or not I do not know.
//		DebugAssert(f->holes_begin() == f->holes_end());
	}
}

inline Point2	ctrl_for_pt(const AptLinearSegment_t * lin)
{
	if(lin->code == apt_rng_crv || lin->code == apt_lin_crv || lin->code == apt_end_crv)
	{
		return lin->ctrl; 
	}
	else 
		return lin->pt;
}


void	AptPolygonToBezier(
				const AptPolygon_t&			inPoly,
				vector<vector<Bezier2> >&	outPoly,
				bool						bez_ok)
{
	outPoly.clear();
	DebugAssert(!inPoly.empty());
	outPoly.push_back(vector<Bezier2>());
	vector<Bezier2> * l = &outPoly.back();

	bool has_first = false;
	Point2	fp, fc, lp, lc;
	for (AptPolygon_t::const_iterator pt = inPoly.begin(); pt != inPoly.end(); ++pt)
	{
		if(!bez_ok)
		if(pt->code == apt_rng_crv || pt->code == apt_lin_crv || pt->code == apt_end_crv)
			throw "No curves!";
		
		if (!has_first)
		{
			has_first = true;
			lp = fp = pt->pt;
			lc = fc = ctrl_for_pt(&*pt);
		}
		else
		{
			l->push_back(Bezier2(lp,lc,pt->pt + Vector2(ctrl_for_pt(&*pt),pt->pt),pt->pt));
			lp = pt->pt;
			lc = ctrl_for_pt(&*pt);
		}
		DebugAssert(pt->code != apt_end_seg);
		DebugAssert(pt->code != apt_end_crv);
		if(pt->code == apt_rng_seg || pt->code == apt_rng_crv)
		{
			DebugAssert(!outPoly.back().empty());
			l->push_back(Bezier2(lp,lc,fp + Vector2(fc,fp),fp));
			has_first = false;
		outPoly.push_back(vector<Bezier2>());
		l = &outPoly.back();
		}
	}
	DebugAssert(!outPoly.empty());
	DebugAssert(outPoly.back().empty());
	outPoly.pop_back();
	DebugAssert(!outPoly.back().empty());

	DebugAssert(has_first == false);
}

int	bz_tess(const Bezier2& b)
{
	double l =
		sqrt(Vector2(b.p1,b.c1).squared_length())+
		sqrt(Vector2(b.c1,b.c2).squared_length())+
		sqrt(Vector2(b.c2,b.p2).squared_length());
	return max(l / AIRPORT_BEZIER_TESS, 3.0);
}

void	BezierToSegments(
				const vector<Bezier2>&		inWinding,
				Polygon_2&					outWinding,
				float						inSimplify)
{
	DebugAssert(!inWinding.empty());
	outWinding.clear();
	for(vector<Bezier2>::const_iterator b = inWinding.begin(); b != inWinding.end(); ++b)
	{
		if (b->p1 == b->p2 && (b->p1 == b->c1 || b->p2 == b->c2)) continue;
		if(b->p1 == b->c1 && b->p2 == b->c2)
		{
			if(outWinding.is_empty() || outWinding[outWinding.size()-1] != ben2cgal<Point_2>(b->p1))
				outWinding.push_back(ben2cgal<Point_2>(b->p1));
		}
		else
		{
			Segment2	s(b->p1,b->p2);
			int tess = bz_tess(*b);
			for (int n = 0; n < tess; ++n)
			{
				Point2 bpb(b->midpoint((float)n / (float)tess));
				Point_2 bp(ben2cgal<Point_2>(b->midpoint((float)n / (float)tess)));
				if(outWinding.is_empty() || outWinding[outWinding.size()-1] != bp)
				{
					if(inSimplify)
					{
						if(sqrt(s.squared_distance_supporting_line(bpb)) > inSimplify * MTR_TO_DEG_LAT)
						{
							#if OPENGL_MAP && DEV
								debug_mesh_line(bpb,s.projection(bpb),1,0,0,1,0,0);
							#else
								throw "too curvy curve!";
							#endif
						}
					}
					outWinding.push_back(bp);
				}
			}
		}
	}
	#if DEV
	DebugAssert(!outWinding.is_empty());
	DebugAssert(outWinding[0] != outWinding[outWinding.size()-1]);
	for(int n = 1; n < outWinding.size(); ++n)
		DebugAssert(outWinding[n-1] != outWinding[n]);
	#endif
//	if(inSimplify)
//		SimplifyPolygonMaxMove(outWinding, inSimplify / (DEG_TO_NM_LAT * NM_TO_MTR), true, true);
}


