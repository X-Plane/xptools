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
#include "MapTopology.h"
//#include "GISTool_Globals.h"
#include "MeshAlgs.h"

// NOTE: all that this does is propegate parks, forestparks, cemetaries and golf courses to the feature type if
// it isn't assigned.

// This is how big a block can be and still "grow" sky scrapers from one big one, in square meters.  The idea is to keep 
// a single tall building in a block from turning into a sea of tall buildings.
#define MAX_OBJ_SPREAD 100000

#define ZONING_METRICS 0

ZoningRuleTable				gZoningRules;
ZoningInfoTable				gZoningInfo;
EdgeRuleTable				gEdgeRules;
FillRuleTable				gFillRules;
LandClassInfoTable			gLandClassInfo;
LandFillRuleTable			gLandFillRules;

static bool ReadLandClassRule(const vector<string>& tokens, void * ref)
{
	LandClassInfo_t info;
	int lc;
	if(TokenizeLine(tokens," eeff", &lc, &info.category, &info.urban_density, &info.veg_density) != 5) return false;
	
	if(gLandClassInfo.count(lc))
	{
		printf("ERROR: land class %s already in table.\n", FetchTokenString(lc));
		return false;
	}
	gLandClassInfo[lc] = info;
	return true;
}

static bool ReadEdgeRule(const vector<string>& tokens, void * ref)
{
	EdgeRule_t e;
	if(TokenizeLine(tokens," eeef",&e.zoning, &e.road_type, &e.resource_id, &e.width) != 5) return false;
	if(gZoningInfo.count(e.zoning) == 0)
		printf("WARNING: zoning type %s, required in rule for %s is unknown.\n",
			FetchTokenString(e.zoning),FetchTokenString(e.resource_id));
	gEdgeRules.push_back(e);
	return true;
}

static bool ReadFillRule(const vector<string>& tokens, void * ref)
{
	FillRule_t r;
	if(TokenizeLine(tokens, " effiiffie", &r.zoning,
			&r.size_min, &r.size_max,
			&r.side_min, &r.side_max,
			&r.slope_min,&r.slope_max,
			&r.hole_ok,
			&r.resource_id) != 10)  return false;
	
	if(gZoningInfo.count(r.zoning) == 0)
		printf("WARNING: zoning type %s, required in rule for %s is unknown.\n",
			FetchTokenString(r.zoning),FetchTokenString(r.resource_id));
	gFillRules.push_back(r);
	return true;
}

static bool ReadZoningInfo(const vector<string>& tokens, void * ref)
{
	ZoningInfo_t	info;
	int				zoning;
	if(TokenizeLine(tokens," efiiiie", &zoning,&info.max_slope,&info.need_lu,&info.fill_edge,&info.fill_area,&info.fill_veg,&info.terrain_type) != 8)
		return false;
	if(gZoningInfo.count(zoning) != 0)
	{
		printf("WARNING: duplicate zoning info for %s\n", FetchTokenString(zoning));
		return false;		
	}
	gZoningInfo[zoning] = info;
	return true;
}

static bool	ReadZoningRule(const vector<string>& tokens, void * ref)
{
	ZoningRule_t	r;
	if(TokenizeLine(tokens," effffffffffffefefiiiiiSSe",
		&r.terrain,
		&r.size_min,			&r.size_max,
		&r.slope_min,			&r.slope_max,
		&r.urban_avg_min,		&r.urban_avg_max,
		&r.forest_avg_min,		&r.forest_avg_max,
		&r.park_avg_min,		&r.park_avg_max,
		&r.bldg_min,			&r.bldg_max,

		&r.req_cat1, &r.req_cat1_min,
		&r.req_cat2, &r.req_cat2_min,
		
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
	
	
	if(gZoningInfo.count(r.zoning) < 1 && r.zoning != NO_VALUE)
		printf("WARNING: zoning rule output %s is unknown zoning type.\n", FetchTokenString(r.zoning));
	gZoningRules.push_back(r);

	return true;
}

static bool ReadLandFillRule(const vector<string>& tokens, void * ref)
{
	LandFillRule_t r;
	if(TokenizeLine(tokens," Sie",&r.required_zoning,&r.color, &r.terrain) != 4)	
		return false;
	
	gLandFillRules.push_back(r);
	
	return true;
}

void LoadZoningRules(void)
{
	gLandClassInfo.clear();
	gZoningRules.clear();
	gZoningInfo.clear();
	gEdgeRules.clear();
	gFillRules.clear();
	gLandFillRules.clear();

	RegisterLineHandler("LANDCLASS_INFO", ReadLandClassRule, NULL);
	RegisterLineHandler("ZONING_RULE", ReadZoningRule, NULL);
	RegisterLineHandler("ZONING_INFO", ReadZoningInfo, NULL);
	RegisterLineHandler("EDGE_RULE", ReadEdgeRule, NULL);
	RegisterLineHandler("FILL_RULE", ReadFillRule, NULL);
	RegisterLineHandler("LANDFILL_RULE", ReadLandFillRule, NULL);
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
						float		max_slope,
						float		urban_avg,
						float		forest_avg,
						float		park_avg,
						float		bldg_hgt,
						int			cat1,
						float		rat1,
						int			cat2,
						float		rat2,
						int			has_water,
						int			has_train,
						int			has_road,
						int			has_hole,
						set<int>&	features)
{
	for(ZoningRuleTable::const_iterator r = gZoningRules.begin(); r != gZoningRules.end(); ++r)
	{
		if(r->terrain == NO_VALUE || r->terrain == terrain)
		if(check_rule(r->size_min, r->size_max, area))
		if(check_rule(r->slope_min, r->slope_max, max_slope))
		if(check_rule(r->urban_avg_min, r->urban_avg_max, urban_avg))
		if(check_rule(r->forest_avg_min, r->forest_avg_max, forest_avg))
		if(check_rule(r->park_avg_min, r->park_avg_max, park_avg))
		if(check_rule(r->bldg_min, r->bldg_max, bldg_hgt))
		if(r->req_cat1 == NO_VALUE || (r->req_cat1 == cat1 && r->req_cat1_min <= rat1))
		if(r->req_cat2 == NO_VALUE || cat2 == NO_VALUE || (r->req_cat2 == cat2 && r->req_cat2_min <= rat2))
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
						


//----------------------------------------------------------------------------------------------------------------------------------------

struct zone_borders_t {
	set<double>		lock_pts[4];
};

int mark_is_locked(Vertex_handle v, NT coord, const set<double>& locked_pts)
{
	double coordf = CGAL::to_double(coord);
	
	set<double>::const_iterator k = locked_pts.lower_bound(coordf);
	double k1, k2;
	DebugAssert(!locked_pts.empty());
	if(locked_pts.size() == 1)
		k1 = k2 = *k;
	else if (k == locked_pts.begin())
		k1 = k2 = *k;
	else if (k == locked_pts.end())
	{
		--k;
		k1 = k2 = *k;		
	} else
	{
		k2 = *k;
		--k;
		k1 = *k;
	}

	const double epsi = 1.0 / 10000000.0;
	if(fabs(coordf - k1) < epsi || fabs(coordf - k2) < epsi)
	{
		v->data().mNeighborBurned = true;
		v->data().mNeighborNotBurned = false;
		return 1;
	} 
	else
	{
		v->data().mNeighborBurned = false;
		v->data().mNeighborNotBurned = true;
		return 0;
	}
}

static bool load_zone_border(const char * fname, zone_borders_t& border)
{
	for(int n = 0; n < 4; ++n)
		border.lock_pts[n].clear();
	FILE * fi = fopen(fname,"r");
	if(fi)
	{
		char buf[256];
		while(fgets(buf,sizeof(buf), fi))
		{	
			double l;
			if(sscanf(buf,"LOCK_WEST %lf",&l)==1)
				border.lock_pts[0].insert(l);
			if(sscanf(buf,"LOCK_SOUTH %lf",&l)==1)
				border.lock_pts[1].insert(l);
			if(sscanf(buf,"LOCK_EAST %lf",&l)==1)
				border.lock_pts[2].insert(l);
			if(sscanf(buf,"LOCK_NORTH %lf",&l)==1)
				border.lock_pts[3].insert(l);				
		}
		fclose(fi);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------


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


struct is_same_terrain_p { 
	int terrain_;
	is_same_terrain_p(int terrain) : terrain_(terrain) { }
	bool operator()(Face_handle f) const { return !f->is_unbounded() && f->data().mTerrainType == terrain_; } 
};

void	ColorFaces(set<Face_handle>&	io_faces);

void	ZoneManMadeAreas(
				Pmwx& 				ioMap,
				const DEMGeo& 		inLanduse,
				const DEMGeo&		inForest,
				const DEMGeo&		inPark,
				const DEMGeo& 		inSlope,
				const AptVector&	inApts,
				Pmwx::Face_handle	inDebug,
				ProgressFunc		inProg)
{
		Pmwx::Face_iterator face;

	PROGRESS_START(inProg, 0, 3, "Zoning terrain...")

	int total = ioMap.number_of_faces() * 2;
	int check = total / 100;
	int ctr = 0;
	
#if ZONING_METRICS
	map<int, map<int, int> > slope_zoning;
#endif	

	/*****************************************************************************
	 * PASS 1 - ZONING ASSIGNMENT VIA LAD USE DATA + FEATURES
	 *****************************************************************************/
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	if(!face->data().IsWater())
	if(inDebug == Pmwx::Face_handle() || face == inDebug)
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

		PolyRasterizer<double>	r;
		int x, y, x1, x2;
		y = SetupRasterizerForDEM(face, inLanduse, r);
		r.StartScanline(y);
		float count = 0, total_forest = 0, total_urban = 0, total_park = 0;
		map<int, int>		histo;
		
		while (!r.DoneScan())
		{
			while (r.GetRange(x1, x2))
			{
				for (x = x1; x < x2; ++x)
				{
					float e = inLanduse.get(x,y);
					float f = inForest.get(x,y);
					float p = inPark.get(x,y);
					count++;

					if(gLandClassInfo.count(e))
					{
						LandClassInfo_t& i(gLandClassInfo[e]);
						histo[i.category]++;
						total_urban += i.urban_density;
						total_forest += i.veg_density;						
						if(p != NO_VALUE)
							total_park += 1.0;
					} 
					else
					{
						histo[terrain_Natural]++;
						if(f != NO_VALUE)
							total_forest += 1.0;
//						if(p != NO_VALUE)
//							total_park += 1.0;
					}

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
			float p = inPark.xy_nearest(any.x(),any.y());

			count++;

			if(gLandClassInfo.count(e))
			{
				LandClassInfo_t& i(gLandClassInfo[e]);
				histo[i.category]++;
				total_urban += i.urban_density;
				total_forest += i.veg_density;			
				if(p != NO_VALUE)
					total_park += 1.0;
			} 
			else
			{
				histo[terrain_Natural]++;
				if(f != NO_VALUE)
					total_forest += 1.0;
				if(p != NO_VALUE)
					total_park += 1.0;
			}

		}
		
		multimap<int, int, greater<int> > histo2;
		for(map<int,int>::iterator i = histo.begin(); i != histo.end(); ++i)
			histo2.insert(multimap<int,int, greater<int> >::value_type(i->second,i->first));		

		PolyRasterizer<double>  r2;
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

		multimap<int, int, greater<int> >::iterator i = histo2.begin();
		if(histo2.size() > 0)
		{
			face->data().mParams[af_Cat1] = i->second;
			face->data().mParams[af_Cat1Rat] = (float) i->first / (float) count;
			++i;

			if(histo2.size() > 1)
			{
				face->data().mParams[af_Cat2] = i->second;
				face->data().mParams[af_Cat2Rat] = (float) i->first / (float) count;
				++i;

				if(histo2.size() > 2)
				{
					face->data().mParams[af_Cat3] = i->second;
					face->data().mParams[af_Cat3Rat] = (float) i->first / (float) count;
					++i;
				}
			}			
		}

		int zone = PickZoningRule(
						face->data().mTerrainType,
						mfam,
//						num_sides,
						max_slope,
						total_urban/(float)count,total_forest/(float)count,total_park/(float)count,
						max_height,
						face->data().mParams[af_Cat1],
						face->data().mParams[af_Cat1Rat],
						face->data().mParams[af_Cat2],
						face->data().mParams[af_Cat1Rat] + face->data().mParams[af_Cat2Rat],	// Really?  Yes.  This is the "high water mark" of BOTH cat 1 + cat 2.  That way
						has_water,																// We can say "80% industrial, 90% urban, and we cover 80I+10U and 90I+0U.  In other
						has_train,																// words when we can accept a mix, this lets the DOMINANT type crowd out the secondary.
						has_local,
						has_holes,
						my_pt_features);

		if(zone != NO_VALUE)
		{
			face->data().SetZoning(zone);
			int wanted_terrain = gZoningInfo[zone].terrain_type;
			if(wanted_terrain != NO_VALUE)
				face->data().mTerrainType = wanted_terrain;
#if ZONING_METRICS			
			double slope_d = acos(1.0 - max_slope) * RAD_TO_DEG;
			int slope_id = 3.0 * ceil(slope_d / 3.0);
			
			slope_zoning[zone][slope_id]++;
#endif			
			
		}
		face->data().mParams[af_HeightObjs] = max_height;

		face->data().mParams[af_UrbanAverage] = total_urban / (float) count;
		face->data().mParams[af_ForestAverage] = total_forest / (float) count;
		face->data().mParams[af_ParkAverage] = total_park / (float) count;
		face->data().mParams[af_SlopeMax] = max_slope;
		face->data().mParams[af_AreaMeters] = mfam;
		
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
	
#if ZONING_METRICS

	map<int,map<int, int> >		histo;

	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	{
		int z = face->data().GetZoning();
		if (z != NO_VALUE)
		{
			Pmwx::Ccb_halfedge_circulator circ, stop;
			circ = stop = face->outer_ccb();
			int c = 0;
			do {
				++c;
			} while(++circ != stop);
			if(c == 4)
			{
				circ = stop = face->outer_ccb();
				do
				{
					double m = LonLatDistMeters(
									CGAL::to_double(circ->source()->point().x()),
									CGAL::to_double(circ->source()->point().y()),
									CGAL::to_double(circ->target()->point().x()),
									CGAL::to_double(circ->target()->point().y()));
					int mi = round(m / 5.0) * 5.0;
					histo[z][mi]++;
				} while(++circ != stop);			
			}
		}
	}
	for(map<int,map<int, int> >::iterator z = histo.begin(); z != histo.end(); ++z)
	{
		printf("%s:\n", FetchTokenString(z->first));
		map<int,int>::iterator h;
		int t = 0;
		for(h = z->second.begin(); h != z->second.end(); ++h)
			t += h->second;
		for(h = z->second.begin(); h != z->second.end(); ++h)		
		if((100.0 * (float) h->second / (float) t) >= 5.0)
			printf("%d: %.0f%%\n", h->first, 100.0 * (float) h->second / (float) t);
	}

	printf("---slope---\n");
	for(map<int,map<int, int> >::iterator z = slope_zoning.begin(); z != slope_zoning.end(); ++z)
	{
		printf("%s:\n", FetchTokenString(z->first));
		map<int,int>::iterator h;
		int t = 0;
		for(h = z->second.begin(); h != z->second.end(); ++h)
			t += h->second;
		for(h = z->second.begin(); h != z->second.end(); ++h)		
		if((100.0 * (float) h->second / (float) t) >= 5.0)
			printf("%d: %.0f%%\n", h->first, 100.0 * (float) h->second / (float) t);
	}
	
	

	
	
#endif	

	NT west(inLanduse.mWest);
	NT east(inLanduse.mEast);
	NT north(inLanduse.mNorth);
	NT south(inLanduse.mSouth);
	

	{	
		zone_borders_t	n_west, n_south, n_east, n_north;
		
		char path[1024];

		make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest-1, inLanduse.mSouth,"zoning", path);
		bool has_west = load_zone_border(path, n_west);
		make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest+1, inLanduse.mSouth,"zoning", path);
		bool has_east = load_zone_border(path, n_east);

		make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest, inLanduse.mSouth-1,"zoning", path);
		bool has_south = load_zone_border(path, n_south);
		make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest, inLanduse.mSouth+1,"zoning", path);
		bool has_north = load_zone_border(path, n_north);

		int marked = 0;

		for(Pmwx::Vertex_iterator v = ioMap.vertices_begin(); v != ioMap.vertices_end(); ++v)
		{
			Point_2 p(v->point());
			if(p.x() == west && has_west)
				marked += mark_is_locked(v, p.y(), n_west.lock_pts[2]);
			else if (p.x() == east && has_east)
				marked += mark_is_locked(v, p.y(), n_east.lock_pts[0]);			
			else if (p.y() == south && has_south)			
				marked += mark_is_locked(v, p.x(), n_south.lock_pts[3]);
			else if (p.y() == north && has_north)			
				marked += mark_is_locked(v, p.x(), n_north.lock_pts[1]);
			else
				v->data().mNeighborBurned = v->data().mNeighborNotBurned = false;
		}

		printf("Marked: %d. Imported: %d.\n", marked, n_west.lock_pts[2].size() + n_east.lock_pts[0].size() + n_south.lock_pts[3].size() + n_north.lock_pts[1].size());
	}


	for(face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face)
	if(!face->is_unbounded())
	if(!face->data().IsWater())
	if(!face->data().HasParam(af_Variant))
	{		
		set<Face_handle>	the_blob;
		CollectionVisitor<Pmwx,Face_handle,is_same_terrain_p>	col(&the_blob, is_same_terrain_p(face->data().mTerrainType));
		VisitContiguousFaces<Pmwx,CollectionVisitor<Pmwx,Face_handle,is_same_terrain_p> >(face, col);
		ColorFaces(the_blob);
	}

	char zbpath[1024];
	make_cache_file_path("../rendering_data/OUTPUT-border/earth", inLanduse.mWest, inLanduse.mSouth,"zoning", zbpath);

	FILE * bf = fopen(zbpath,"w");
	if(bf)
	{
		set<double>	edge_l, edge_r, edge_b, edge_t;

		Pmwx::Ccb_halfedge_circulator circ, stop;
		DebugAssert(ioMap.unbounded_face()->number_of_holes() == 1);
		circ = stop = *ioMap.unbounded_face()->holes_begin();//->outer_ccb();
		do
		{
			if(must_burn_v(circ->target()))
			{
				Point_2 p(circ->target()->point());
				if(p.x() == west)
					edge_l.insert(CGAL::to_double(p.y()));
				else if(p.x() == east)
					edge_r.insert(CGAL::to_double(p.y()));
				else if(p.y() == south)
					edge_b.insert(CGAL::to_double(p.x()));
				else if(p.y() == north)
					edge_t.insert(CGAL::to_double(p.x()));
				else
				{
					DebugAssert(!"Point is not on an edge.");
				}
//				debug_mesh_point(cgal2ben(circ->target()->point()),1,1,1);
			}
		} while(++circ != stop);
		
		set<double>::iterator i;
		for(i = edge_l.begin(); i != edge_l.end(); ++i)
			fprintf(bf,"LOCK_WEST %.12lf\n",*i);
		for(i = edge_b.begin(); i != edge_b.end(); ++i)
			fprintf(bf,"LOCK_SOUTH %.12lf\n",*i);
		for(i = edge_r.begin(); i != edge_r.end(); ++i)
			fprintf(bf,"LOCK_EAST %.12lf\n",*i);
		for(i = edge_t.begin(); i != edge_t.end(); ++i)
			fprintf(bf,"LOCK_NORTH %.12lf\n",*i);
		fclose(bf);
	}

}

/***
 *
 **/
#pragma mark -

inline int MIN_NOVALUE(int a, int b)
{
	if(a == NO_VALUE) return b;
	if(b == NO_VALUE) return a;
	return min(a,b);
}


struct EdgeNode_t;

typedef multimap<float, EdgeNode_t *, greater<float> >	EdgeQ;

struct	FaceNode_t {
	
	FaceNode_t *			prev;
	FaceNode_t *			next;

	list<Face_handle>		real_faces;
	list<EdgeNode_t *>		edges;
	float					area;
	int						color;

};

struct EdgeNode_t {
	EdgeNode_t *	prev;
	EdgeNode_t *	next;

	FaceNode_t *	f1;
	FaceNode_t *	f2;
	float			length;		// Length of edge in meters
	int				count;		// Number of edges
	int				road_type;	// Lowest enum overlying road on this edge
	bool			must_lock;
	bool			must_merge;
	EdgeQ::iterator	self;
	
	FaceNode_t *	other(FaceNode_t * f) const { DebugAssert(f == f1 || f == f2); return (f == f1) ? f2 : f1; }
};

struct edge_contains_f {
	FaceNode_t * f_;
	edge_contains_f(FaceNode_t * f) : f_(f) { }
	bool operator()(const EdgeNode_t *& e) const { return e->f1 == f_ || e->f2 == f_; }
};

struct FaceGraph_t {
	
	FaceGraph_t() { edges = NULL; faces = NULL; }
	
	EdgeNode_t *	edges;
	FaceNode_t *	faces;	
	EdgeQ			queue;
	
	~FaceGraph_t() { clear(); }
	void			clear();
	FaceNode_t *	new_face();
	EdgeNode_t *	new_edge();
	void			delete_face(FaceNode_t * f);
	void			delete_edge(EdgeNode_t * e);
	bool			connected(FaceNode_t * f1, FaceNode_t * f2);
	void			merge(FaceNode_t * f1, FaceNode_t * f2, list<EdgeNode_t *>& merged_neighbors);
	
	void			enqueue(EdgeNode_t * en,float (* cost_func)(EdgeNode_t * en));
	EdgeNode_t *	pop(void);

};

FaceNode_t *	FaceGraph_t::new_face()
{
	FaceNode_t * n = new FaceNode_t;
	n->prev = NULL;
	n->next = faces;
	faces = n;
	if(n->next)
		n->next->prev = n;
	return n;
}

EdgeNode_t *	FaceGraph_t::new_edge()
{
	EdgeNode_t * n = new EdgeNode_t;
	n->prev = NULL;
	n->next = edges;
	if(n->next)
		n->next->prev = n;
	edges = n;
	return n;
}	


void			FaceGraph_t::delete_face(FaceNode_t * f)
{
	if(f == faces)
		faces = f->next;
	if(f->prev)	f->prev->next = f->next;
	if(f->next) f->next->prev = f->prev;
	delete f;
}

void			FaceGraph_t::delete_edge(EdgeNode_t * e)
{
	if(e->self != queue.end())
		queue.erase(e->self);
		
	if(e == edges)
		edges = e->next;
	if(e->prev)	e->prev->next = e->next;
	if(e->next) e->next->prev = e->prev;
	delete e;
}

bool			FaceGraph_t::connected(FaceNode_t * f1, FaceNode_t * f2)
{
	if(f1->edges.size() > f2->edges.size())
		swap(f1,f2);
	for(list<EdgeNode_t *>::iterator e = f1->edges.begin(); e != f1->edges.end(); ++e)
	if(((*e)->f1 == f1 && (*e)->f2 == f2) ||
	   ((*e)->f1 == f2 && (*e)->f2 == f1))
		return true;
	return false;
}

void			FaceGraph_t::merge(FaceNode_t * f1, FaceNode_t * f2, list<EdgeNode_t *>& out_neighbors)
{
	f1->area += f2->area;
	f1->real_faces.splice(f1->real_faces.end(),f2->real_faces);
	
	map<FaceNode_t *, EdgeNode_t *>						neighbors;
	map<FaceNode_t *, EdgeNode_t *>::iterator			ni;
	list<EdgeNode_t *>::iterator ei;
	EdgeNode_t * en;
	FaceNode_t * fn;
	
	
	// First: grab up all edges that we may keep.  This is our neighbor set.
	// We have one edge to f2 which we will cope with later.
	for(list<EdgeNode_t *>::iterator ei = f1->edges.begin(); ei != f1->edges.end(); ++ei)
	{
		en = * ei;
		fn = en->other(f1);
		if(fn != f2)
		{
			DebugAssert(neighbors.count(fn) == 0);
			neighbors[fn] = en;
		}
	}
	
	// Now we can "merge in" F2's edges.
	for(list<EdgeNode_t *>::iterator ei = f2->edges.begin(); ei != f2->edges.end(); ++ei)
	{
		en = *ei;
		fn = en->other(f2);
		if(fn == f1)
		{
			// This is the f1-f2 edge - it's GONE.
			DebugAssert(!en->must_lock);
			f1->edges.remove(en);
			delete_edge(en);
		}
		else if (neighbors.count(fn) > 0)
		{
			// Both f1 and f2 were connected to this face.  We need to merge these edges.
			EdgeNode_t * oe = neighbors[fn];
			oe->length += en->length;
			oe->count += en->count;
			if(en->must_lock)
				oe->must_lock = true;
			if(en->must_merge)
				oe->must_merge = true;
//			DebugAssert(!(oe->must_merge && oe->must_lock));
			if(oe->must_lock) 
				oe->must_merge = false;
//			#if !DEV
//				#error temp hack
//			#endif
			oe->road_type = MIN_NOVALUE(oe->road_type,en->road_type);
			fn->edges.remove(en);
			delete_edge(en);
			out_neighbors.push_back(oe);
		}
		else
		{
			// This is a face adjacent to f2 but not f1.  So now that f1 and f2 are merged,
			// f1 accumulates it in its neighborhood.
			neighbors[fn] = en;
			// Migrate links over...edge is literally moved, so this is the easy case.
			if(en->f1 == f2)
				en->f1 = f1;
			else if(en->f2 == f2)
				en->f2 = f1;
			f1->edges.push_back(en);
		}
	}
	
	delete_face(f2);	
}

void			FaceGraph_t::enqueue(EdgeNode_t * en, float (* cost_func)(EdgeNode_t * en))
{	
	if(queue.end() != en->self)
		queue.erase(en->self);
	float cost_now = cost_func(en);
	if(cost_now >= 0.0f)
	{
		en->self = queue.insert(EdgeQ::value_type(cost_now,en));
	}
	else	
		en->self = queue.end();
}


EdgeNode_t *	FaceGraph_t::pop(void)
{
	if(queue.empty())
		return NULL;
	EdgeNode_t * ret = queue.begin()->second;
	queue.erase(queue.begin());
	ret->self = queue.end();
	return ret;	
}





void		FaceGraph_t::clear(void)
{	
	while(faces)
	{
		FaceNode_t * f = faces;
		faces = faces->next;
		delete f;
	}
	while(edges)
	{
		EdgeNode_t * e = edges;
		edges = edges->next;
		delete e;
	}
}

#define MAX_AREA (1000.0 * 1000.0)
float	edge_cost_func(EdgeNode_t * en)
{
	if(en->must_lock) return -1.0f;
	if(en->must_merge) return 0.0f;
	if((en->f1->area + en->f2->area) > MAX_AREA)	return -1.0f;
//	if(Road_IsHighway(en->road_type) || Road_IsHighway(en->road_type))
//	   return en->length * 0.5;
	return en->length;
}

struct UnlinkedFace_p {
	set<Face_handle> *						universe_;
	UnlinkedFace_p(set<Face_handle> * universe) : universe_(universe) { }
	bool operator()(Face_handle who) const { 
		return universe_->count(who) > 0;
	}
};

void	ColorFaces(set<Face_handle>&	io_faces)
{
		FaceGraph_t								graph;
		set<Face_handle>::iterator				f, ff;
		FaceNode_t *							fn;
		EdgeNode_t *							en;
		map<Face_handle, FaceNode_t *>			face_mapping;
	
	/* First: build our graph structure */
	for(f = io_faces.begin(); f != io_faces.end(); ++f)
	{
		fn = graph.new_face();
		fn->real_faces.push_back(*f);
		fn->area = GetMapFaceAreaMeters(*f);
		fn->color = -1;
		face_mapping[*f] = fn;
	}
	
	for(f = io_faces.begin(); f != io_faces.end(); ++f)
	{
		set<Face_handle>	friends;
		CollectionVisitor<Pmwx,Face_handle,UnlinkedFace_p> face_collector(&friends, UnlinkedFace_p(&io_faces));
		VisitAdjacentFaces<Pmwx, CollectionVisitor<Pmwx,Face_handle,UnlinkedFace_p> >(*f, face_collector);
		for(ff = friends.begin(); ff != friends.end(); ++ff)
		{
			DebugAssert(face_mapping.count(*f ));
			DebugAssert(face_mapping.count(*ff));
			FaceNode_t * f1 = face_mapping[*f ];
			FaceNode_t * f2 = face_mapping[*ff];
			if(!graph.connected(f1,f2))
			{
				en = graph.new_edge();
				en->f1 = f1;
				en->f2 = f2;
				f1->edges.push_back(en);
				f2->edges.push_back(en);
				en->self = graph.queue.end();
				en->length = 0.0f;
				en->count = 0;
				en->must_merge = false;
				en->must_lock = false;
				en->road_type = NO_VALUE;
				set<Halfedge_handle>	edges;
				FindEdgesForFace<Pmwx>(*f, edges);
				for(set<Halfedge_handle>::iterator i = edges.begin(); i != edges.end(); ++i)
				if((*i)->twin()->face() == *ff)
				{
					en->length += GetMapEdgeLengthMeters(*i);
					en->count++;
					if((*i)->data().HasRoads())
					{
						en->road_type = MIN_NOVALUE(en->road_type, (*i)->data().mSegments.front().mFeatType);
						if((*i)->data().mSegments.front().mFeatType == powerline_Generic)
							en->must_merge = true;
					}
					if((*i)->twin()->data().HasRoads())
					{
						en->road_type = MIN_NOVALUE(en->road_type, (*i)->twin()->data().mSegments.front().mFeatType);
						if((*i)->twin()->data().mSegments.front().mFeatType == powerline_Generic)
							en->must_merge = true;
					}
					if((*i)->source()->data().mNeighborBurned ||
					   (*i)->target()->data().mNeighborBurned)
					{
						en->must_merge = false;
						en->must_lock = true;
					}
					if((*i)->source()->data().mNeighborNotBurned ||
					   (*i)->target()->data().mNeighborNotBurned)
					{
						en->must_merge = true;
						en->must_lock = false;
					}
				}
				
//				for(set<Halfedge_handle>::iterator i = edges.begin(); i != edges.end(); ++i)
//				if((*i)->twin()->face() == *ff)
//				if(en->must_merge)
//					debug_mesh_line(cgal2ben((*i)->source()->point()),cgal2ben((*i)->target()->point()),1,1,0,1,1,0);
				
			}
		}
	}
	
	/* Second: seed the initial PQ with edges. */
	
	for(en = graph.edges; en; en = en->next)
		graph.enqueue(en, edge_cost_func);
	
	/* Third: run the PQ until we have done our "merges". */
	while((en = graph.pop()) != NULL)
	{
		list<EdgeNode_t *>	neighbors;
		graph.merge(en->f1, en->f2, neighbors);
		for(list<EdgeNode_t *>::iterator n = neighbors.begin(); n != neighbors.end(); ++n)
			graph.enqueue(*n, edge_cost_func);
	}
	
	int highest = 0, total = 0, over_4 = 0;
	
	multimap<int, FaceNode_t *, greater<int> >		nodes_by_degree;
	
	for(en = graph.edges; en; en = en->next)
	{
		DebugAssert(!en->must_merge);
	}
	
	/* Fourth: color the remaining edges. */
	for(fn = graph.faces; fn; fn = fn->next)
	{

//		set<Face_handle>		faces;
//		set<Halfedge_handle>	edges;
//		faces.insert(fn->real_faces.begin(),fn->real_faces.end());			
//		FindEdgesForFaceSet<Pmwx>(faces,edges);
//		for(set<Halfedge_handle>::iterator e = edges.begin(); e != edges.end(); ++e)
//			debug_mesh_line(cgal2ben((*e)->source()->point()),cgal2ben((*e)->target()->point()),1,0,0,0,1,0);

		nodes_by_degree.insert(multimap<int, FaceNode_t *, greater<int> >::value_type(fn->edges.size(),fn));
	}
	
	for(multimap<int, FaceNode_t *, greater<int> >::iterator i = nodes_by_degree.begin(); i != nodes_by_degree.end(); ++i)
	{
		set<int> used;
		i->second->color = 0;
		for(list<EdgeNode_t *>::iterator ei = i->second->edges.begin(); ei != i->second->edges.end(); ++ei)
		{
			en = *ei;
			fn = en->other(i->second);
			if(fn->color != -1)	used.insert(fn->color);
		}
		while(used.count(i->second->color))
			++i->second->color;
		
		highest = max(highest,i->second->color);
		++total;
		if(i->second->color > 3)
			++over_4;
			
		set<int> zoning_we_have;	
			
		for(list<Face_handle>::iterator fh = i->second->real_faces.begin(); fh != i->second->real_faces.end(); ++fh)
		{
			(*fh)->data().mParams[af_Variant] = i->second->color;
			zoning_we_have.insert((*fh)->data().GetZoning());
		}		
		for(LandFillRuleTable::iterator r = gLandFillRules.begin(); r != gLandFillRules.end(); ++r)
		if(zoning_we_have == r->required_zoning && r->color == i->second->color)
		{		
			for(list<Face_handle>::iterator fh = i->second->real_faces.begin(); fh != i->second->real_faces.end(); ++fh)
			if((*fh)->data().mTerrainType == NO_VALUE)
				(*fh)->data().mTerrainType = r->terrain;
			break;
		}
	}
//	printf("Highest color: %d.  Total before: %d.  Total after: %d.  Over_4: %d.\n", highest, io_faces.size(), total,over_4);
}



