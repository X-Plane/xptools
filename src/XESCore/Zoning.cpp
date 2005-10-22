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

#include "Zoning.h"
#include "MapDefs.h"
#include "MapAlgs.h"
#include "DEMDefs.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "ObjTables.h"
#include "ParamDefs.h"
#include "AptDefs.h"

// NOTE: all that this does is propegate parks, forestparks, cemetaries and golf courses to the feature type if
// it isn't assigned.

void	ZoneManMadeAreas(
				Pmwx& 				ioMap, 
				const DEMGeo& 		inLanduse, 
				const DEMGeo& 		inSlope,
				const AptVector&	inApts,			
				ProgressFunc		inProg)
{
		Pmwx::Face_iterator face;
		
	PROGRESS_START(inProg, 0, 2, "Zoning terrain...")

	int total = ioMap.number_of_faces() * 2;
	int check = total / 100;
	int ctr = 0;
		
	/*****************************************************************************
	 * PASS 1 - ZONING ASSIGNMENT VIA LAD USE DATA + FEATURES
	 *****************************************************************************/
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	{
		PROGRESS_CHECK(inProg, 0, 1, "Zoning terrain...", ctr, total, check)
		
		double	max_height = 0.0;
		for (GISPointFeatureVector::iterator feat = face->mPointFeatures.begin(); feat != face->mPointFeatures.end(); ++feat)
		{
			if (feat->mParams.count(pf_Height))
			{
				max_height = max(max_height, feat->mParams[pf_Height]);
			}
		}
		face->mParams[af_HeightObjs] = max_height;

		// FEATURE ASSIGNMENT - first go and assign any features we might have.
		face->mTemp1 = NO_VALUE;
		face->mTemp2 = 0;

		// Quick bail - if we're assigned, we're done. - Moving this to first place because...
		// airports take the cake/
		if (face->mTerrainType != terrain_Natural) continue;
	
		switch(face->mAreaFeature.mFeatType) {
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

	}

	PROGRESS_DONE(inProg, 0, 2, "Zoning terrain...")
	PROGRESS_START(inProg, 1, 2, "Checking approach paths...")

	ctr = 0;	
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	if (face->mTerrainType != terrain_Airport)
	if (!face->IsWater())
	{
		PROGRESS_CHECK(inProg, 0, 1, "Checking approach paths...", ctr, total, check)
		set<GISFace *>	neighbors;
		FindAdjacentFaces(face, neighbors);
		Polygon2 me;
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			me.push_back(circ->target()->point());
			++circ;
		} while (circ != stop);
		
		Point2	myloc = me.centroid();
		
		double	my_agl = face->mParams[af_HeightObjs];
		double	max_agl = my_agl;
		
		for (set<GISFace *>::iterator niter = neighbors.begin(); niter != neighbors.end(); ++niter)
		{
			max_agl = max(max_agl, (*niter)->mParams[af_HeightObjs] * 0.5);
		}
		
		for (AptVector::const_iterator apt = inApts.begin(); apt != inApts.end(); ++apt)
		if (apt->kind_code == apt_Type_Airport)
		if (!apt->pavements.empty())
		{
			Point2 midp = apt->pavements.front().ends.midpoint();
			double dist = LonLatDistMeters(midp.x, midp.y, myloc.x, myloc.y);
			if (dist < 15000.0)
			for (AptPavementVector::const_iterator rwy = apt->pavements.begin(); rwy != apt->pavements.end(); ++rwy)
			if (rwy->name != "xxx")
			{
				midp = rwy->ends.midpoint();
				dist = LonLatDistMeters(midp.x, midp.y, myloc.x, myloc.y);
				
				Vector2	azi_rwy = Vector2(rwy->ends.p1, rwy->ends.p2);	azi_rwy.normalize();
				Vector2 azi_me = Vector2(midp, myloc);					azi_me.normalize();
				
				double dot = azi_rwy.dot(azi_me);
				
				double gs_elev = dist / 18.0;
				if (dot > 0.8 < dist < 700.0)
					max_agl = min(max_agl, gs_elev);
			}
		}
				
		my_agl = max(my_agl, max_agl);
		face->mParams[af_Height] = max_agl;
	}
	PROGRESS_DONE(inProg, 1, 2, "Checking approach paths...")
}
