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
#include "ObjTables.h"
#include "ParamDefs.h"


// NOTE: all that this does is propegate parks, forestparks, cemetaries and golf courses to the feature type if
// it isn't assigned.

const double	kSkyscraperAGL = 30.0;

void	ZoneManMadeAreas(
				Pmwx& 				ioMap, 
				const DEMGeo& 		inLanduse, 
				const DEMGeo& 		inSlope,
				ProgressFunc		inProg)
{
		Pmwx::Face_iterator face;
		
	if (inProg) inProg(0, 1, "Zoning Land", 0.0);	

	double total = ioMap.number_of_faces() * 2.0 + ioMap.number_of_halfedges();
	int ctr = 0;
		
	/*****************************************************************************
	 * PASS 1 - ZONING ASSIGNMENT VIA LAD USE DATA + FEATURES
	 *****************************************************************************/
	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	if (!face->is_unbounded())
	{
		if (inProg && (ctr % 1000) == 0)	inProg(0, 1, "Zoning Land", (double) ctr / total);

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
		
		// Special case: troll for airports.
//		for (GISPointFeatureVector::iterator pt_feature = face->mPointFeatures.begin(); 
//			 pt_feature != face->mPointFeatures.end(); ++pt_feature)
//		{
//			if (gFeatures.count(pt_feature->mFeatType) > 0)
//			{
//				int feat_terrain =gFeatures[pt_feature->mFeatType].terrain_type;
//				if (feat_terrain == terrain_Airport)
//					face->mTerrainType = max(face->mTerrainType, feat_terrain);
//			}
//		}			
		
		
		int count = 0;
		{
			Pmwx::Ccb_halfedge_circulator c, i;
			c = i = face->outer_ccb();
			do {
				++i, ++count;
			} while (c != i);
		}
		
		// ZONING ASSIGNMENT

/*		
		float			slope_min, slope_max, slope_avg;
		map<int, int>	lu_hist;
		float			hist_total;
		
		slope_avg = GetParamAverage(face, inSlope, &slope_min, &slope_max);
		hist_total = GetParamHistogram(face, inLanduse, lu_hist);
		
		// Theory of zoning allocation: we count up all DEM points that are crops vs. urban
		// vs. something else...our farm percentage and urban percentage come from here.  
		// Crops and town are counted 50-50, and since the urban threshhold must be over 50%
		// to get urban, crops and town never makes a city.
		
		float	crop_count = lu_hist[lu_usgs_COOL_CROPS_AND_TOWNS_IRREGULAR] * 0.5 +
							 lu_hist[lu_usgs_COOL_CROPS_AND_TOWNS_SQUARE] * 0.5 +
							 lu_hist[lu_usgs_CROPS_AND_TOWN_IRREGULAR] * 0.5 +
							 lu_hist[lu_usgs_CROPS_AND_TOWN_SQUARE] * 0.5 +
							 lu_hist[lu_usgs_CORN_AND_BEANS_CROPLAND_IRREGULAR] +
							 lu_hist[lu_usgs_CORN_AND_BEANS_CROPLAND_SQUARE] +
							 lu_hist[lu_usgs_RICE_PADDY_AND_FIELD_IRREGULAR] +
							 lu_hist[lu_usgs_RICE_PADDY_AND_FIELD_SQUARE] +
							 lu_hist[lu_usgs_COOL_IRRIGATED_CROPLAND_IRREGULAR] +
							 lu_hist[lu_usgs_COOL_IRRIGATED_CROPLAND_SQUARE] +
							 lu_hist[lu_usgs_HOT_IRRIGATED_CROPLAND_IRREGULAR] +
							 lu_hist[lu_usgs_HOT_IRRIGATED_CROPLAND_SQUARE] +
							 lu_hist[lu_usgs_COLD_IRRIGATED_CROPLAND] +
							 lu_hist[lu_usgs_CROP_AND_WATER_MIXTURES_IRREGULAR] * 0.5 +
							 lu_hist[lu_usgs_CROP_AND_WATER_MIXTURES_SQUARE] * 0.5 +
							 lu_hist[lu_usgs_BROADLEAF_CROPS] +
							 lu_hist[lu_usgs_GRASS_CROPS_IRREGULAR] +
							 lu_hist[lu_usgs_GRASS_CROPS_SQUARE] +
							 lu_hist[lu_usgs_CROPS_GRASS_SHRUBS_IRREGULAR] + 
							 lu_hist[lu_usgs_CROPS_GRASS_SHRUBS_SQUARE] + 
							 lu_hist[lu_usgs_EVERGREEN_TREE_CROP] +
							 lu_hist[lu_usgs_DECIDUOUS_TREE_CROP];
		float	urban_count =lu_hist[lu_usgs_URBAN_IRREGULAR] + 
							 lu_hist[lu_usgs_URBAN_SQUARE] + 
							 lu_hist[lu_usgs_COOL_CROPS_AND_TOWNS_IRREGULAR] * 0.5 +
							 lu_hist[lu_usgs_COOL_CROPS_AND_TOWNS_SQUARE] * 0.5 +
							 lu_hist[lu_usgs_CROPS_AND_TOWN_IRREGULAR] * 0.5 + 
							 lu_hist[lu_usgs_CROPS_AND_TOWN_IRREGULAR] * 0.5;
		
		if (hist_total > 0.0) crop_count /= hist_total; else crop_count = 0.0;
		if (hist_total > 0.0) urban_count /= hist_total; else urban_count = 0.0;
		
		// More zoning theory - check for tall buildings - these do not occur in a vacuum but rather
		// affect their zoning info.  Tall buildings (e.g. building features with associated AGL)
		// typically come from "good" data like FAA obstacles...use this to know more about the area.
		
		bool	has_industrial = false;
		bool	has_skyscraper = false;
		bool	has_apartments = false;
		double	max_known_agl = 0.0;
		int		best_feature_terrain = NO_VALUE;
		
		for (GISPointFeatureVector::iterator pt_feature = face->mPointFeatures.begin(); 
			 pt_feature != face->mPointFeatures.end(); ++pt_feature)
		{
			if (gFeatures.count(pt_feature->mFeatType) > 0)
			{
				best_feature_terrain = max(best_feature_terrain, gFeatures[pt_feature->mFeatType].terrain_type);
			}
		
			switch(pt_feature->mFeatType) {
			case feat_Refinery:
			case feat_Tank:
			case feat_Smokestack:
			case feat_Smokestacks:
			case feat_Industrial:
			case feat_Plant:
				has_industrial = true;
				break;

			case feat_ResidentialLowRise:
			case feat_ResidentialMidRise:
			case feat_ResidentialComplex:
			case feat_ResidentialLowValueComplex:
				has_apartments = true;
				break;
				
			case feat_Skyscraper:
			case feat_Building:
				if (pt_feature->mParams.count(pf_Height) > 0)
					max_known_agl = max(max_known_agl, pt_feature->mParams[pf_Height]);
				if (max_known_agl > kSkyscraperAGL)
					has_skyscraper = true;
				break;
			}
		}

		// Okay now we're ready to make some decisions.
		
		if (urban_count > 0.90)
		{
			// This really is urban...
				 if (has_skyscraper)	face->mTerrainType = terrain_Downtown;
			else if (has_industrial)	face->mTerrainType = terrain_Industrial;
			else if (has_apartments)	face->mTerrainType = terrain_Urban;
			else						face->mTerrainType = terrain_Residential;
		} 
		else if (has_skyscraper)
		{
			face->mTerrainType = terrain_OutlayHighrise;
		} 
		else if (crop_count > 0.90) 
		{
			face->mTerrainType = terrain_Farm;
		} 
		else if (crop_count > 0.30 && urban_count > 0.40) 
		{
			face->mTerrainType = terrain_FarmTown;
		} 
		else if (urban_count > 0.40)
		{
			face->mTerrainType = terrain_OutlayResidential;
		}
		else if (crop_count > 0.30)
		{
			face->mTerrainType = terrain_MixedFarm;
		}
		
		if (face->mTerrainType == terrain_Natural)	face->mTerrainType = NO_VALUE;
		face->mTerrainType = max(face->mTerrainType, best_feature_terrain);
		if (face->mTerrainType == NO_VALUE)	face->mTerrainType = terrain_Natural;
		
		// Enforce hills!
		if (slope_max > 0.3)
		{
			face->mTemp2 = 1;
		}
		*/
	}

	/*****************************************************************************
	 * PASS 3 - SPREAD OUT AND MASSAGE ZONING DATA
	 *****************************************************************************/
/*
	for (Pmwx::Halfedge_iterator edge = ioMap.halfedges_begin(); edge != ioMap.halfedges_end(); ++edge, ++edge, ++ctr, ++ctr)
	if (edge->face() != edge->twin()->face())
	{
		if (inProg && (ctr % 1000) == 0)	inProg(0, 1, "Zoning Land", (double) ctr / total);
		edge->face()->mTemp1 = max(edge->face()->mTemp1, edge->twin()->face()->mTerrainType);
		edge->twin()->face()->mTemp1 = max(edge->twin()->face()->mTemp1, edge->face()->mTerrainType);
	}

	for (face = ioMap.faces_begin(); face != ioMap.faces_end(); ++face, ++ctr)
	{
		if (inProg && (ctr % 1000) == 0)	inProg(0, 1, "Zoning Land", (double) ctr / total);
		TerrainTypeTuple promote(face->mTemp1, face->mTerrainType);		
		if (gTerrainPromoteTable.count(promote) > 0)
			face->mTerrainType = gTerrainPromoteTable[promote];

		// Do hills too		
		if (face->mTemp2 && face->mTerrainType > terrain_Marker_Artificial && face->mTerrainType < terrain_Marker_Features)
			face->mTerrainType++;
	}
*/
	if (inProg) inProg(0, 1, "Zoning Land", 1.0);		

}
