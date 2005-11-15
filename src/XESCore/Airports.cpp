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
#include "AptDefs.h"
#include "AssertUtils.h"
#include "XESConstants.h"
#include "GISUtils.h"
#include "ParamDefs.h"
#include "DEMDefs.h"
#include "DEMAlgs.h"
#include "MapAlgs.h"
#include "CompGeomUtils.h"
	
//#define AIRPORT_OUTER_SIMPLIFY	(40.0 / (DEG_TO_NM_LAT * NM_TO_MTR))
//#define AIRPORT_INNER_SIMPLIFY	(25.0 / (DEG_TO_NM_LAT * NM_TO_MTR))

#define	AIRPORT_OUTER_SIMPLIFY	(25.0 / (DEG_TO_NM_LAT * NM_TO_MTR))		// would be 100 but keep low to preserve outer hull!
#define AIRPORT_INNER_SIMPLIFY	(25.0 / (DEG_TO_NM_LAT * NM_TO_MTR))
#define	AIRPORT_OUTER_FILLGAPS	(200.0 / (DEG_TO_NM_LAT * NM_TO_MTR))
#define AIRPORT_INNER_FILLGAPS	(50.0 / (DEG_TO_NM_LAT * NM_TO_MTR))
#define AIRPORT_OUTER_FILL_AREA		(400.0 * 400.0 / (DEG_TO_NM_LAT * NM_TO_MTR * DEG_TO_NM_LAT * NM_TO_MTR))
#define AIRPORT_INNER_FILL_AREA		(40.0 * 40.0 / (DEG_TO_NM_LAT * NM_TO_MTR * DEG_TO_NM_LAT * NM_TO_MTR))
	
static void GetPadWidth(
		const AptPavement_t *		rwy,
		double&						pad_width_m,
		double&						pad_length_m,
		bool						fill_water)
{
	bool is_rwy = (rwy->name != "xxx");
	if (!fill_water)
	{
		pad_width_m = is_rwy ? 50.0 : 40.0;
		pad_length_m = is_rwy ? 150.0 : 40.0;
	} else {
		pad_width_m = is_rwy ? 30.0 : 25.0;
		pad_length_m = is_rwy ? 30.0 : 25.0;
	}
}


static void ExpandRunway(
				const AptPavement_t * 	rwy, 
				double					pad_width_meters,	
				double					pad_length_meters,
				Point2					pts[4])
{
	double	aspect = cos(rwy->ends.midpoint().y * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;
	
	double rwy_len = LonLatDistMetersWithScale(rwy->ends.p1.x, rwy->ends.p1.y, rwy->ends.p2.x, rwy->ends.p2.y, DEG_TO_MTR_LON, DEG_TO_MTR_LAT);
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

struct	AirportProcess_t {
	set<GISHalfedge *> *  edges;
	set<GISFace *> *	faces;
};

static void CollectEdges(GISHalfedge * old_e, GISHalfedge * new_e, void * ref)
{
	AirportProcess_t * info = (AirportProcess_t *) ref;
	set<GISHalfedge *> * edges = info->edges;
	if (old_e == NULL && new_e != NULL)	edges->insert(new_e);
	if (old_e != NULL && new_e == NULL)	edges->insert(old_e);	
	
	if (old_e == NULL && new_e != NULL)
	{
		GISFace * f1 = new_e->face();
		GISFace * f2 = new_e->twin()->face();
		if (info->faces->count(f1))
			info->faces->insert(f2);
		else if (info->faces->count(f2))
			info->faces->insert(f1);
	}
}

void BurnInAirport(
				const AptInfo_t * 	inAirport,
				Pmwx&				ioMap,
				bool				inFillWater,
				set<GISFace *>&		faces)
{
	// STEP 1
	// Go through and burn every runway into the layout.  We will introduce
	// a series of connected faces with each quad we burn in.  We accumulate
	// them all so we have the area of all pavement that was introduced.
	
	faces.clear();
	
	for (int rwy = 0; rwy < inAirport->pavements.size(); ++rwy)
	if (inAirport->pavements[rwy].surf_code != rwy_Surf_Waterway)
	{
		
		Point2	corners[4];
		double	pad_width;
		double	pad_height;
		GetPadWidth(&inAirport->pavements[rwy], pad_width, pad_height, inFillWater);
		ExpandRunway(&inAirport->pavements[rwy],pad_width, pad_height, corners);
		
		set<GISHalfedge *>	pavement;
		AirportProcess_t	info;
		info.edges = &pavement;
		info.faces = &faces;
		for (int n = 0; n < 4; ++n)
			ioMap.insert_edge(corners[n], corners[(n+1)%4], CollectEdges, &info);

		// Find the faces surrounded by our bounds, and accum them all.
		set<GISFace *>		new_faces;
		FindFacesForEdgeSet(pavement, new_faces);
		for (set<GISFace *>::iterator f = new_faces.begin(); f != new_faces.end(); ++f)
		{
			faces.insert(*f);
		}
	}

	// STEP 2
	// We're going to reduce the map down to the minimal edges by going through
	// and deleting internal edges.  We need to be careful to modify our face set
	// wh ich will shink.
	
	// Go through and find all of the halfedges in our layer, by iterating
	// on our faces.  But make sure we get the dominant ones so we don't get double
	// edges!
	set<GISHalfedge *>	all, nuke;

	for (set<GISFace *>::iterator f = faces.begin(); f != faces.end(); ++f)
	{
		set<GISHalfedge *> me;
		FindEdgesForFace(*f, me);
		for (set<GISHalfedge *>::iterator e = me.begin(); e != me. end(); ++e)
			if ((*e)->mDominant)
				all.insert(*e);
			else
				all.insert((*e)->twin());
	}
	
	// Go through and find all edges that have pavement on both sides - they're unneeded.
	for (set<GISHalfedge *>::iterator e = all.begin(); e != all.end(); ++e)
	{
		if (faces.count((*e)->face()) &&
			faces.count((*e)->twin()->face()))
		if (inFillWater || 
			((*e)->face()->mTerrainType != terrain_Water &&
			 (*e)->twin()->face()->mTerrainType != terrain_Water &&
			 (*e)->mSegments.empty()))
		nuke.insert(*e);
	}
	
	// Nuke these unneeded edges, and get rid of any faces we've rendered useless
	// and thus deleted.
	for (set<GISHalfedge *>::iterator e = nuke.begin(); e != nuke.end(); ++e)
	{
		faces.erase(ioMap.remove_edge(*e));
	}

	// STEP 3 
	// Mark all of our terrain as really airport!	If we're not filling water
	// then figure out who's not wet and we'll remove them from our face set later.

	set<GISFace *>	puddles;
	for (set<GISFace *>::iterator f = faces.begin(); f != faces.end(); ++f)
	{
		if (inFillWater || (*f)->mTerrainType != terrain_Water)
			(*f)->mTerrainType = inFillWater ? terrain_Airport : terrain_Airport;
		else
			puddles.insert(*f);
			
	}

	for (set<GISFace *>::iterator f = puddles.begin(); f != puddles.end(); ++f)
		faces.erase(*f);

	// STEP 4
	// Go through and remove all holes from our airport no matter what.  These
	// are areas 100% surrounded by runway pavement and are therefore considered
	// airport premsisis. 
	// (Note: this WILL possibly delete water, but that water is totally surrounded
	// by pavement - we can afford this hit.

	for (set<GISFace *>::iterator f = faces.begin(); f != faces.end(); ++f)
	{
		while ((*f)->holes_begin() != (*f)->holes_end())
		{
			GISFace * dead = ioMap.remove_edge(*((*f)->holes_begin()));
			if (dead)
			{
				DebugAssert(*f != dead);
				faces.erase(dead);
			}
		}
	}
}

void	SimplifyAirportAreas(Pmwx& inDstMap, const set<GISFace *>& inSrcFaces, set<GISFace *>& outDstFaces, bool inFillWater)
{
	for (set<GISFace *>::iterator i = inSrcFaces.begin(); i != inSrcFaces.end(); ++i)
	{
		Assert(!(*i)->is_unbounded());
		Assert((*i)->holes_begin() == (*i)->holes_end());
		DebugAssert((*i)->mTerrainType == terrain_Airport);
		
		Polygon2	orig;
		Pmwx::Ccb_halfedge_circulator iter, stop;
		iter = stop = (*i)->outer_ccb();
		do {
			orig.push_back(iter->target()->point());
			++iter;
		} while (iter != stop);
		
//		printf("BEFORE: %d\n", orig.size());
//		if (!inFillWater)
//			MakePolygonConvex(orig);
//		printf("AFTER: %d\n", orig.size());

		FillPolygonGaps(orig, inFillWater ? AIRPORT_INNER_FILLGAPS : AIRPORT_OUTER_FILLGAPS);

		SafeMakeMoreConvex(orig, inFillWater ? AIRPORT_INNER_FILL_AREA : AIRPORT_OUTER_FILL_AREA);
		
		DebugAssert(ValidatePolygonSimply(orig));
		SimplifyPolygonMaxMove(orig, inFillWater ? AIRPORT_INNER_SIMPLIFY : AIRPORT_OUTER_SIMPLIFY, true, false);
		DebugAssert(ValidatePolygonSimply(orig));

		Pmwx	temp;
		GISFace * ff = temp.insert_ring(temp.unbounded_face(), orig);
		ff->mTerrainType = terrain_Airport;
#if DEV
		try {		
#endif		
		if (inFillWater)
			OverlayMap(inDstMap, temp);
		else {
			MergeMaps(inDstMap, temp, inFillWater, &outDstFaces, false, NULL);	
			for (set<GISFace *>::iterator cleanMe = outDstFaces.begin(); cleanMe != outDstFaces.end(); ++cleanMe)
				(*cleanMe)->mAreaFeature.mFeatType = NO_VALUE;
		}
#if DEV
		} catch (...) {
			inDstMap = temp;
			throw;
		}
#endif							
	}
}

void ProcessAirports(const AptVector& apts, Pmwx& ioMap, DEMGeo& elevation, DEMGeo& transport, bool crop, bool dems, bool kill_rivers, ProgressFunc prog)
{
	int x1, x2, x, y1, y2, y;
	Point2 p1, p2;
	if (crop)
		CalcBoundingBox(ioMap, p1, p2);
	
	set<GISFace *>		faces, simple_faces;

	DEMGeo		working(elevation.mWidth,elevation.mHeight);
	DEMGeo		transport_src(transport.mWidth, transport.mHeight);
	
	if (dems)
	{
		transport_src.copy_geo_from(transport);
		transport_src = 1.0;
	}
	
	PROGRESS_START(prog, 0, 1, "Burning in airports...")
		
	for (int n = 0; n < apts.size(); ++n)
	if (apts[n].kind_code == apt_Type_Airport)
	{
		PROGRESS_SHOW(prog, 0, 1, "Burning in airports...", n, apts.size()*2);
		Pmwx	foo;
		foo.unbounded_face()->mTerrainType = terrain_Natural;
		BurnInAirport(&apts[n], foo, true, faces);
		SimplifyAirportAreas(ioMap, faces, simple_faces, true);
	}

	for (int n = 0; n < apts.size(); ++n)
	if (apts[n].kind_code == apt_Type_Airport)
	{
		PROGRESS_SHOW(prog, 0, 1, "Burning in airports...", n+apts.size(), apts.size()*2);	
		Pmwx	foo;
		foo.unbounded_face()->mTerrainType = terrain_Natural;
		BurnInAirport(&apts[n], foo, false, faces);
		simple_faces.clear();	
		SimplifyAirportAreas(ioMap, faces, simple_faces, false);
		if (dems)
		{
			working = NO_DATA;
			if (ClipDEMToFaceSet(simple_faces, elevation, working, x1, y1, x2, y2))
			{
				working.copy_geo_from(elevation);
				--x1;
				--y1;
				++x2;
				++y2;
				SpreadDEMValues(working, 1, x1, y1, x2, y2);
				DEMGeo		airport_area;
				working.subset(airport_area, x1, y1, x2-1,y2-1);
				vector<DEMGeo>	fft;
				DEMMakeFFT(airport_area, fft);
				if (fft.size() > 1)		fft[0] *= 0.0;
				if (fft.size() > 2)		fft[1] *= 0.0;
				if (fft.size() > 3)		fft[2] *= 0.0;
				FFTMakeDEM(fft,airport_area);
				for (y = y1; y < y2; ++y)
				for (x = x1; x < x2; ++x)
				{
					if (working.get(x,y) != NO_DATA && airport_area(x-x1,y-y1) != NO_DATA)
						working(x,y) = airport_area(x-x1,y-y1);
				}
			}		
			elevation.overlay(working);
			
			ClipDEMToFaceSet(simple_faces, transport_src, transport, x1, y1, x2, y2);
		}
	}

	PROGRESS_DONE(prog, 0, 1, "Burning in airports...")

	if (crop)
	{
		CropMap(ioMap, p1.x, p1.y, p2.x, p2.y, false, prog);
		SimplifyMap(ioMap, kill_rivers);
	}
	
}
