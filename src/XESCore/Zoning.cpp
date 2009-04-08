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

#include "MapDefsCGAL.h"
#include "Zoning.h"
#include "MapAlgs.h"
#include "DEMDefs.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "ObjTables.h"
#include "ParamDefs.h"
#include "AptDefs.h"

// NOTE: all that this does is propegate parks, forestparks, cemetaries and golf courses to the feature type if
// it isn't assigned.

#define MAX_OBJ_SPREAD 100000

void	ZoneManMadeAreas(
				Pmwx& 				ioMap,
				const DEMGeo& 		inLanduse,
				const DEMGeo& 		inSlope,
				const AptVector&	inApts,
				ProgressFunc		inProg)
{
		Pmwx::Face_iterator face;

	PROGRESS_START(inProg, 0, 3, "Zoning terrain...")

	int total = ioMap.number_of_faces() * 2;
	int check = total / 100;
	int ctr = 0;

	/*****************************************************************************
	 * PASS 1 - ZONING ASSIGNMENT VIA LAD USE DATA + FEATURES
	 *****************************************************************************/
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	{
		PROGRESS_CHECK(inProg, 0, 3, "Zoning terrain...", ctr, total, check)

		double mfam = GetMapFaceAreaMeters(face);

		double	max_height = 0.0;

		if (mfam < MAX_OBJ_SPREAD)
		for (GISPointFeatureVector::iterator feat = face->data().mPointFeatures.begin(); feat != face->data().mPointFeatures.end(); ++feat)
		{
			if (feat->mFeatType == feat_Building)
			if (feat->mParams.count(pf_Height))
			{
				max_height = max(max_height, feat->mParams[pf_Height]);
			}
		}
		face->data().mParams[af_HeightObjs] = max_height;

		// FEATURE ASSIGNMENT - first go and assign any features we might have.
		face->data().mTemp1 = NO_VALUE;
		face->data().mTemp2 = 0;

		// Quick bail - if we're assigned, we're done. - Moving this to first place because...
		// airports take the cake/

		if (face->data().mTerrainType != terrain_Natural) continue;
	
//		switch(face->data().mAreaFeature[0].mFeatType) {
////		case feat_MilitaryBase:	face->mTerrainType = terrain_MilitaryBase;	break;
////		case feat_TrailerPark:	face->mTerrainType = terrain_TrailerPark;	break;
////		case feat_Campground:	face->mTerrainType = terrain_Campground;	break;
////		case feat_Marina:		face->mTerrainType = terrain_Marina;		break;
//		case feat_GolfCourse:	face->data().mTerrainType = terrain_GolfCourse;	break;
//		case feat_Cemetary:		face->data().mTerrainType = terrain_Cemetary;		break;
////		case feat_Airport:		face->mTerrainType = terrain_Airport;		break;		
//		case feat_Park:			face->data().mTerrainType = terrain_Park;			break;
//		case feat_ForestPark:	face->data().mTerrainType = terrain_ForestPark;	break;
//		}

/*
		switch(face->data().mAreaFeature.mFeatType) {
//		case feat_MilitaryBase:	face->mTerrainType = terrain_MilitaryBase;	break;
//		case feat_TrailerPark:	face->mTerrainType = terrain_TrailerPark;	break;
//		case feat_Campground:	face->mTerrainType = terrain_Campground;	break;
//		case feat_Marina:		face->mTerrainType = terrain_Marina;		break;
		case feat_GolfCourse:	face->mTerrainType = terrain_GolfCourse;	break;
		case feat_Cemetary:		face->mTerrainType = terrain_Cemetary;		break;
//		case feat_Airport:		face->mTerrainType = terrain_Airport;		break;
		case feat_Park:			face->mTerrainType = terrain_Park;			break;
		case feat_ForestPark:	face->mTerrainType = terrain_ForestPark;	break;
		}
*/
	}

	PROGRESS_DONE(inProg, 0, 3, "Zoning terrain...")
#if 0
	PROGRESS_START(inProg, 1, 3, "Checking approach paths...")

	ctr = 0;
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	if (face->data().mTerrainType != terrain_Airport)
	if (!face->data().IsWater())
	{
		PROGRESS_CHECK(inProg, 1, 3, "Checking approach paths...", ctr, total, check)
		set<Face_handle>	neighbors;
		//FindAdjacentFaces(face, neighbors);
		{
			neighbors.clear();
			set<Halfedge_handle> e;
			FindEdgesForFace(face, e);
			for (set<Halfedge_handle>::iterator he = e.begin(); he != e.end(); ++he)
				if ((*he)->twin()->face() != face)
					neighbors.insert((*he)->twin()->face());
		}
		Polygon_2 me;
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			me.push_back(circ->target()->point());
			++circ;
		} while (circ != stop);
		
		Point_2	myloc = centroid(me);
		
		double	my_agl = face->data().mParams[af_HeightObjs];
		double	max_agl = my_agl;
		
		for (set<Face_handle>::iterator niter = neighbors.begin(); niter != neighbors.end(); ++niter)
		{
			max_agl = max(max_agl, (*niter)->data().mParams[af_HeightObjs] * 0.5);
		}

		for (AptVector::const_iterator apt = inApts.begin(); apt != inApts.end(); ++apt)
		if (apt->kind_code == apt_airport)
		if (!apt->pavements.empty())
		{
			Point_2 midp = CGAL::midpoint(apt->pavements.front().ends.source(),apt->pavements.front().ends.target());
			double dist = LonLatDistMeters(midp.x(), midp.y(), myloc.x(), myloc.y());
			if (dist < 15000.0)
			for (AptPavementVector::const_iterator rwy = apt->pavements.begin(); rwy != apt->pavements.end(); ++rwy)
			if (rwy->name != "xxx")
			{
				midp = CGAL::midpoint(rwy->ends.source(), rwy->ends.target());
				dist = LonLatDistMeters(midp.x(), midp.y(), myloc.x(), myloc.y());
				
				Vector_2	azi_rwy = normalize(Vector_2(rwy->ends.source(), rwy->ends.target()));
				Vector_2 azi_me = normalize(Vector_2(midp, myloc));
				
				double dot = azi_rwy * azi_me;
				
				double gs_elev = dist / 18.0;
				if (dot > 0.8 && dist < 700.0)
					max_agl = min(max_agl, gs_elev);
			}
		}

		my_agl = max(my_agl, max_agl);
		face->data().mParams[af_Height] = max_agl;
	}
	PROGRESS_DONE(inProg, 1, 3, "Checking approach paths...")
#endif
	PROGRESS_START(inProg, 2, 3, "Checking Water")
	ctr = 0;
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	if (face->data().IsWater())
	{
		bool is_open = false;
		PROGRESS_CHECK(inProg, 2, 3, "Checking Water", ctr, total, check)
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			if (circ->twin()->face()->is_unbounded())
			{
				is_open = true;
				break;
			}
			++circ;
		} while (circ != stop);
		
		face->data().mParams[af_WaterOpen] = is_open ? 1.0 : 0.0;		
		face->data().mParams[af_WaterArea] = GetMapFaceAreaMeters(face);
		
	}
	PROGRESS_DONE(inProg, 2, 3, "Checking Water")
}
