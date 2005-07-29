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
#include "TigerImport.h"
#include "ParamDefs.h"
#include "PerfUtils.h"
#include "ConfigSystem.h"
#include "MapAlgs.h"

// This tags all roads as having their CFCCs for enums - useful for raw data analysis
#define ENCODE_ROAD_CFCC 0

	/*			
			TODO: ADD COUNTERS OF ALL PHENOMENA
			TODO: build major catagories of land classes....
	*/


/*
 Census Feature Classification Codes - areas.  These codes are applied to points
 or areas and either result in a land class (basically a zoning decision that will
 autogen buildings later) or a point object.  The 'allow_from_point' field allows
 a single point object to apply a land class to the underlying polygon.  This is 
 appropriate for big things where the presence of the object would dominate the 
 block.  It is not appropriate for small things that might just be one single building
 instance in a land use.
 
 */
#define DO_CHECKS 0

struct	WaterCodeInfo_t {
	string		name;
	int			is_wet;
};

hash_map<string, WaterCodeInfo_t>	kWaterCodes;

bool	ReadWaterLine(const vector<string>& tokens, void * ref)
{	
	if (tokens.size() != 3) return false;
	WaterCodeInfo_t code;
	code.name = tokens[2];
	code.is_wet = 1;
	if (kWaterCodes.find(tokens[1]) != kWaterCodes.end())
		fprintf(stderr,"WARNING: duplicate water cfcc %s\n", tokens[1].c_str());	
	kWaterCodes[tokens[1]] = code;
	return true;
}


// Census features may be GT-polygon based or point-based...there isn't a lot of logic to the database except for some broad restrictions.
// (And since we are in the 'D' catagory virtually everything is point or area.)
//
//	Census feature type			use_gt_poly		require_gt_poly			action
//		Area					0				x						Add point feature based on GT-poly center-point
//		Area					1				x						Add area feature for GT-poly
//		Point					x				0						Add point feature based on landmark point
//		Point					x				1						Skip Feature
struct	FeatureInfo_t {
	string				name;
	int					feature_type;
	int					use_gt_poly;		// If this is set and we are an area feature in the census, make an area feature.
	int					require_gt_poly;	// If this is set and we are an area not an area feature in the census, ignore and dump the feature.
	int					water_ok;			// Can import on water
	int					water_required;		// Must import on water
};

hash_map<string,FeatureInfo_t>	kFeatureCodes;

bool	ReadFeatureLine(const vector<string>& tokens, void * ref)
{
	if (tokens.size() != 8) return false;
	FeatureInfo_t	info;
	info.name = tokens[2];
	info.feature_type = LookupToken(tokens[3].c_str());
	if (info.feature_type == -1)
	{
		fprintf(stderr,"WARNING: unknown enum %s\n", tokens[3].c_str());
		return false;
	}
	info.use_gt_poly = atoi(tokens[4].c_str());
	info.require_gt_poly = atoi(tokens[5].c_str());
	info.water_ok = atoi(tokens[6].c_str());
	info.water_required = atoi(tokens[7].c_str());
	if (kFeatureCodes.find(tokens[1]) != kFeatureCodes.end())
		fprintf(stderr,"WARNING: duplicate feature cfcc %s\n", tokens[1].c_str());	
	kFeatureCodes[tokens[1]] = info;
	return true;
}

struct	RoadInfo_t {
	string				cfcc;
	int					network_type;
	int					underpassing;
	int					tunnel;
};

hash_map<string, RoadInfo_t>	kRoadCodes;

bool	ReadRoadLine(const vector<string>& tokens, void * ref)
{
	if (tokens.size() != 5) return false;
	RoadInfo_t info;
	info.network_type = LookupToken(tokens[2].c_str());
	if (info.network_type == -1)
	{
		fprintf(stderr,"Unknown enum %s\n", tokens[2].c_str());
		return false;
	}
	info.underpassing = atoi(tokens[3].c_str());
	info.tunnel = atoi(tokens[4].c_str());
	if (kRoadCodes.find(tokens[1]) != kRoadCodes.end())
		fprintf(stderr,"WARNING: duplicate net cfcc %s\n", tokens[1].c_str());
	kRoadCodes[tokens[1]] = info;
	return true;
}

int	LookupWaterCFCC(const char * inCode)
{
	hash_map<string,WaterCodeInfo_t>::iterator i = kWaterCodes.find(inCode);
	if (i == kWaterCodes.end()) return 0;
	return i->second.is_wet;
}

RoadInfo_t * LookupNetCFCC(const char * inCode)
{
#if ENCODE_ROAD_CFCC
	static RoadInfo_t	info;
	info.cfcc = "HACK";
	info.network_type = NO_VALUE;
	info.underpassing = 0;
	info.tunnel = 0;
	return &info;
#endif

	hash_map<string,RoadInfo_t>::iterator i = kRoadCodes.find(inCode);
	if (i == kRoadCodes.end()) return NULL;
	return &i->second;
}

FeatureInfo_t * LookupFeatureCFCC(const char * inCode)
{
	hash_map<string, FeatureInfo_t>::iterator i = kFeatureCodes.find(inCode);
	if (i == kFeatureCodes.end()) return NULL;
	return &i->second;
}
	
void LoadTigerConfig() 
{
	RegisterLineHandler("TIGER_WATER", ReadWaterLine, NULL);
	RegisterLineHandler("TIGER_FEATURE", ReadFeatureLine, NULL);
	RegisterLineHandler("TIGER_NETWORK", ReadRoadLine, NULL);
	LoadConfigFile("tiger_import.txt");
}

void	TIGERImport(
			const	ChainInfoMap&		chains,
			const	LandmarkInfoMap&	landmarks,
			const	PolygonInfoMap&		polygons,
			Pmwx&						ioMap,
			ProgressFunc				prog)
{
	static	bool first_time = true;
	if (first_time)
	{
		LoadTigerConfig();
		first_time = false;
	}
	for (ChainInfoMap::const_iterator chainIter = chains.begin(); chainIter != chains.end(); ++chainIter)
	{		
		RoadInfo_t * net_cfcc = LookupNetCFCC(chainIter->second.cfcc.c_str());		
		if (net_cfcc)
		{
			if (net_cfcc->tunnel)
			{
				chainIter->second.startNode->pm_vertex->mTunnelPortal = true;
				chainIter->second.endNode->pm_vertex->mTunnelPortal = true;
			} else {
				GISNetworkSegment_t nl;
				nl.mFeatType = net_cfcc->network_type;
#if ENCODE_ROAD_CFCC
				nl.mFeatType = LookupTokenCreate(chainIter->second.cfcc.c_str());
#endif				
				nl.mRepType = NO_VALUE;
				
				for (WTPM_Line::HalfedgeVector::const_iterator he = chainIter->second.pm_edges.first.begin(); 
					he != chainIter->second.pm_edges.first.end(); ++he)
				{			
					(*he)->mSegments.push_back(nl);
					(*he)->mParams[he_IsUnderpassing] = net_cfcc->underpassing;
				}
			}
		}
		int water_cfcc = LookupWaterCFCC(chainIter->second.cfcc.c_str());
		if (water_cfcc)
		{
			for (WTPM_Line::HalfedgeVector::const_iterator he = chainIter->second.pm_edges.first.begin(); 
				he != chainIter->second.pm_edges.first.end(); ++he)
			{			
				(*he)->mParams[he_IsRiver] = 1;
			}			
		}
		
		for (WTPM_Line::HalfedgeVector::const_iterator he = chainIter->second.pm_edges.first.begin(); he != chainIter->second.pm_edges.first.end(); ++he)
			(*he)->mParams[he_TIGER_TLID] = chainIter->first;
	
		for (WTPM_Line::HalfedgeVector::const_iterator he = chainIter->second.pm_edges.second.begin(); he != chainIter->second.pm_edges.second.end(); ++he)
			(*he)->mParams[he_TIGER_TLID] = chainIter->first;
	}
	
	for (PolygonInfoMap::const_iterator polyIter = polygons.begin(); polyIter != polygons.end(); ++polyIter)
	{
		if (polyIter->second.water)
			polyIter->second.pm_face->mTerrainType = terrain_Water;
	}
	
	Point2	sw, ne;
	CalcBoundingBox(ioMap, sw, ne);
 	ioMap.Index();
	int skip = 0;
	int	nolo = 0;
	for (LandmarkInfoMap::const_iterator landIter = landmarks.begin(); landIter != landmarks.end(); ++landIter)
	{
		FeatureInfo_t * land_cfcc = LookupFeatureCFCC(landIter->second.cfcc.c_str());
		if (land_cfcc)
		{
			bool	has_poly = !landIter->second.cenid_polyid.empty();
			if (land_cfcc->require_gt_poly && !has_poly) continue;
				// Lack of area feature from poitn landmark
			if (land_cfcc->use_gt_poly && has_poly)
			{
//				printf("Importing area feature %s %s\n", kFeatureCodes[cfcc].cfcc, kFeatureCodes[cfcc].name);
				// Area feature from area landmark
				for (vector<CENID_POLYID>::const_iterator i = landIter->second.cenid_polyid.begin(); i != landIter->second.cenid_polyid.end(); ++i)
				{
					PolygonInfoMap::const_iterator thePoly = polygons.find(*i);
					if (thePoly != polygons.end())
					{
						bool wet = thePoly->second.pm_face->IsWater();
						GISAreaFeature_t	feat;
						feat.mFeatType = land_cfcc->feature_type;
						if ((wet || !land_cfcc->water_required) && (!wet || land_cfcc->water_ok))
						{
							if (thePoly->second.pm_face->mAreaFeature.mFeatType != NO_VALUE)
								printf("WARNING: double feature, %s and %s\n", 
									FetchTokenString(thePoly->second.pm_face->mAreaFeature.mFeatType),
									FetchTokenString(feat.mFeatType));
							thePoly->second.pm_face->mAreaFeature = feat;
						} else
							++skip;	//printf("Skipped: wet = %d, feat = %s\n", wet, kFeatureCodes[cfcc].name);						
					}
				}
			} else if (has_poly) {
//				printf("Importing point feature for poly %s %s\n", kFeatureCodes[cfcc].cfcc, kFeatureCodes[cfcc].name);
				// Point feature from area landmark
				for (vector<CENID_POLYID>::const_iterator i = landIter->second.cenid_polyid.begin(); i != landIter->second.cenid_polyid.end(); ++i)
				{
					PolygonInfoMap::const_iterator thePoly = polygons.find(*i);
					if (thePoly != polygons.end())
					{
						bool wet = thePoly->second.pm_face->IsWater();
						GISPointFeature_t	feat;
						feat.mFeatType = land_cfcc->feature_type;
						feat.mInstantiated = false;
						feat.mLocation = thePoly->second.location;
						if ((wet || !land_cfcc->water_required) && (!wet || land_cfcc->water_ok))
							thePoly->second.pm_face->mPointFeatures.push_back(feat);
						else
							++skip;	//printf("Skipped: wet = %d, feat = %s\n", wet, kFeatureCodes[cfcc].name);						
					}
				}
			} else {
//				printf("Importing point feature for point %s %s\n", kFeatureCodes[cfcc].cfcc, kFeatureCodes[cfcc].name);
				// Point feature from point landmark
				vector<Pmwx::Face_handle>	v;
				ioMap.FindFaceTouchesPt(landIter->second.location, v);
				if (v.size() == 1)
				{
					bool wet = v[0]->IsWater();
					GISPointFeature_t	feat;
					feat.mFeatType = land_cfcc->feature_type;
					feat.mLocation = landIter->second.location;
					if ((wet || !land_cfcc->water_required) && (!wet || land_cfcc->water_ok))
						v[0]->mPointFeatures.push_back(feat);					
					else 
						++skip;	//printf("Skipped: wet = %d, feat = %s\n", wet, kFeatureCodes[cfcc].name);
				} else if (v.size() > 1)
					fprintf(stderr,"ERROR: Point feature matches multiple areas.\n");
				else
					nolo++;
			}
		}	
	}
#if DEV
	if (nolo) printf("Could not locate %d point features.\n", nolo);
#endif
	if (skip) printf("Skipped %d landmarks that were/were not in water when they should have been.\n", skip);
}	

