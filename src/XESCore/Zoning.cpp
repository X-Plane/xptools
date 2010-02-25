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

#include "MapDefs.h"
#include "Zoning.h"
#include "MapAlgs.h"
#include "DEMDefs.h"
#include "DEMTables.h"
#include "GISUtils.h"
#include "ObjTables.h"
#include "ParamDefs.h"
#include "MapAlgs.h"
#include "PolyRasterUtils.h"
#include "AptDefs.h"
#include "ObjPlacement2.h"
#include "ConfigSystem.h"

// NOTE: all that this does is propegate parks, forestparks, cemetaries and golf courses to the feature type if
// it isn't assigned.

// This is how big a block can be and still "grow" sky scrapers from one big one, in square meters.  The idea is to keep 
// a single tall building in a block from turning into a sea of tall buildings.
#define MAX_OBJ_SPREAD 100000

ZoningRuleTable				gZoningRules;
set<int>					gZoningTypes;


static bool	ReadZoningRule(const vector<string>& tokens, void * ref)
{
	ZoningRule_t	r;
	
	if(TokenizeLine(tokens," effiiffffffffffffiiiiiSSe",
		&r.terrain,
		&r.size_min,&r.size_max,
		&r.side_min,			&r.side_max,
		&r.slope_min,			&r.slope_max,
		&r.urban_min,			&r.urban_max,
		&r.forest_min,			&r.forest_max,
		&r.urban_avg_min,		&r.urban_avg_max,
		&r.forest_avg_min,		&r.forest_avg_max,
		&r.bldg_min,			&r.bldg_max,
		&r.req_water,
		&r.req_train,
		&r.req_road,
		&r.hole_ok,
		&r.crud_ok,
		&r.require_features,
		&r.consume_features,
		&r.zoning) != 26)	return false;

	r.slope_min = 1.0 - cos(r.slope_min * DEG_TO_RAD);
	r.slope_max = 1.0 - cos(r.slope_max * DEG_TO_RAD);
	gZoningRules.push_back(r);
	gZoningTypes.insert(r.zoning);

	return true;
}

void LoadZoningRules(void)
{
	gZoningRules.clear();
	gZoningTypes.clear();

	RegisterLineHandler("ZONING_RULE", ReadZoningRule, NULL);
	LoadConfigFile("zoning.txt");	
}

template <typename T>
inline bool	check_rule(T minv, T maxv, T actv)
{
	return ((minv == maxv) && (minv == 0) || (actv >= minv && actv <= maxv));
}

inline bool any_match(const set<int>& lhs, const set<int>& rhs)
{
	set<int>::const_iterator l = lhs.begin(), r = rhs.begin();

	while(l != lhs.end() && r != rhs.end())	// While we're not off the end of either
	{
		if(*l < *r)							// Advance the lesser
			++l;
		else if (*l > *r)
			++r;
		else 
			return true;					// Any match = a win
	}
	return false;							// if one is EOF we're not going to get a match.
}

inline bool all_match(const set<int>& lhs, const set<int>& rhs)
{
	if(lhs.size() != rhs.size()) return false;
	
	set<int>::const_iterator l = lhs.begin(), r = rhs.begin();

	while(l != lhs.end() && r != rhs.end())
	{
		if(*l < *r) return false;			// Mismatch?  Bail.
		if(*l > *r) return false;
		++l, ++r;							// match, continue.
	}	
	return l == lhs.end() && r == rhs.end();// We match if we ran out of BOTH at the same time.
}

inline bool remove_these(set<int>& stuff, const set<int>& nuke_these)
{
	for(set<int>::const_iterator i = nuke_these.begin(); i != nuke_these.end(); ++i)
		stuff.erase(*i);
}

static int		PickZoningRule(
						int			terrain,
						float		area,
						int			sides,
						float		max_slope,
						float		urban,
						float		forest,
						float		urban_avg,
						float		forest_avg,
						float		bldg_hgt,
						int			has_water,
						int			has_train,
						int			has_road,
						int			has_hole,
						set<int>&	features)
{
	for(ZoningRuleTable::const_iterator r = gZoningRules.begin(); r != gZoningRules.end(); ++r)
	{
		if(r->terrain == NO_VALUE || terrain == terrain)
		if(check_rule(r->size_min, r->size_max, area))
		if(check_rule(r->side_min, r->side_max, sides))
		if(check_rule(r->slope_min, r->slope_max, max_slope))
		if(check_rule(r->urban_min, r->urban_max, urban))
		if(check_rule(r->forest_min, r->forest_max, forest))
		if(check_rule(r->urban_avg_min, r->urban_avg_max, urban_avg))
		if(check_rule(r->forest_avg_min, r->urban_avg_max, forest_avg))
		if(check_rule(r->bldg_min, r->bldg_max, bldg_hgt))
		if(has_water || !r->req_water)
		if(has_train || !r->req_train)
		if(has_road || !r->req_road)
		if(!has_hole || r->hole_ok)		
		if(r->require_features.empty() || any_match(r->require_features, features))
		if(r->crud_ok || all_match(r->consume_features, features))
		{
			remove_these(features, r->consume_features);
			return r->zoning;
		}
	}
	return NO_VALUE;
}
						



typedef int(* Feature_Is_f)(int);

template<Feature_Is_f evaluator>
int evaluate_he(Pmwx::Halfedge_handle he)
{
	GISNetworkSegmentVector::iterator i;
	for(i = he->data().mSegments.begin(); i != he->data().mSegments.end(); ++i)
		if(evaluator(i->mFeatType))
			return 1;
	for(i = he->twin()->data().mSegments.begin(); i != he->twin()->data().mSegments.end(); ++i)
		if(evaluator(i->mFeatType))
			return 1;
	
}


inline void accum_lu(int lu, int fo, float& total_forest, float& total_urban)
{
	if(fo != NO_VALUE)
		++total_forest;
	switch((int) lu) { 
	#define URBAN(x) total_urban += (x); break;
	case lu_globcover_INDUSTRY:					
	case lu_globcover_INDUSTRY_SQUARE:			URBAN(1.0f);
	case lu_globcover_URBAN_SQUARE_HIGH:
	case lu_globcover_URBAN_HIGH:				URBAN(0.8f);
	case lu_globcover_URBAN_SQUARE_MEDIUM:
	case lu_globcover_URBAN_MEDIUM:				URBAN(0.6f);
	case lu_globcover_URBAN_SQUARE_LOW:
	case lu_globcover_URBAN_LOW:				URBAN(0.4f);
	case lu_globcover_URBAN_SQUARE_TOWN:
	case lu_globcover_URBAN_TOWN:				URBAN(0.2f);
	case lu_globcover_URBAN_SQUARE_CROP_TOWN:
	case lu_globcover_URBAN_CROP_TOWN:			URBAN(0.1f);
	}					
}


void	ZoneManMadeAreas(
				Pmwx& 				ioMap,
				const DEMGeo& 		inLanduse,
				const DEMGeo&		inForest,
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
	if(!face->data().IsWater())
	{
		PROGRESS_CHECK(inProg, 0, 3, "Zoning terrain...", ctr, total, check)

		double mfam = GetMapFaceAreaMeters(face);

		double	max_height = 0.0;
		set<int>	my_pt_features;

		if (mfam < MAX_OBJ_SPREAD)
		for (GISPointFeatureVector::iterator feat = face->data().mPointFeatures.begin(); feat != face->data().mPointFeatures.end(); ++feat)
		{
			my_pt_features.insert(feat->mFeatType);
			if (feat->mFeatType == feat_Building)
			{
				if (feat->mParams.count(pf_Height))
				{
					max_height = max(max_height, feat->mParams[pf_Height]);
				}
			} else {
				printf("Has other feature: %s\n", FetchTokenString(feat->mFeatType));
			}
		}
	
		int has_water = 0;
		int has_train = 0;
		int has_local = 0;		
	
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			if(evaluate_he<Road_IsTrain>(circ)) has_train = true;
			if(evaluate_he<Road_IsLocal>(circ) || 
				evaluate_he<Road_IsMainDrag>(circ)) has_local = true;			
			if(!circ->twin()->face()->is_unbounded() && circ->twin()->face()->data().IsWater()) has_water = 1;
		} while (++circ != stop);

		PolyRasterizer	r;
		int x, y, x1, x2;
		y = SetupRasterizerForDEM(face, inLanduse, r);
		r.StartScanline(y);
		float count = 0, total_forest = 0, total_urban = 0;
		while (!r.DoneScan())
		{
			while (r.GetRange(x1, x2))
			{
				for (x = x1; x < x2; ++x)
				{
					float e = inLanduse.get(x,y);
					float f = inForest.get(x,y);
					count++;
					accum_lu(e,f,total_forest,total_urban);
				}
			}
			++y;
			if (y >= inLanduse.mHeight) break;
			r.AdvanceScanline(y);
		}
		
		if(count == 0)
		{
			Point2 any = cgal2ben(face->outer_ccb()->source()->point());
			float e = inLanduse.xy_nearest(any.x(),any.y());
			float f = inForest.xy_nearest(any.x(),any.y());
			accum_lu(e,f,total_forest, total_urban);
			count++;
		}

		PolyRasterizer	r2;
		y = SetupRasterizerForDEM(face, inSlope, r2);
		r2.StartScanline(y);
		int scount = 0;
		float max_slope = 0.0;
		while (!r2.DoneScan())
		{
			while (r2.GetRange(x1, x2))
			{
				for (x = x1; x < x2; ++x)
				{
					float s = inSlope.get(x,y);
					max_slope = max(max_slope, s);
					++scount;
				}
			}
			++y;
			if (y >= inLanduse.mHeight) break;
			r2.AdvanceScanline(y);
		}
		
		if(scount == 0)
		{
			Point2 any = cgal2ben(face->outer_ccb()->source()->point());
			float s = inSlope.xy_nearest(any.x(),any.y());
			max_slope = s;
		}
		

		Polygon_with_holes_2	ra;

		GetTotalAreaForFaceSet(face,ra);

			
		bool has_holes = ra.has_holes();
		int num_sides = ra.outer_boundary().size();

		int zone = PickZoningRule(
						face->data().mTerrainType,
						mfam,
						num_sides,
						max_slope,
						total_urban,total_forest,
						total_urban/(float)count,total_forest/(float)count,
						max_height,
						has_water,
						has_train,
						has_local,
						has_holes,
						my_pt_features);

		if(zone != NO_VALUE)
			face->data().SetZoning(zone);
		face->data().mParams[af_HeightObjs] = max_height;
		face->data().mParams[af_OriginCode] = total_urban / (float) count;

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


