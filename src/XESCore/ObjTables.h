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
#ifndef OBJTABLES_H
#define OBJTABLES_H

struct	RepInfo_t {

	// RULEZ
	int		feature;
	int		landuse;
	int		climate;
	int		terrain;
	int		zoning;
	
	float	elev_min;
	float	elev_max;
	float	temp_min;
	float	temp_max;
	float	slope_min;
	float	slope_max;
	float	relelev_min;
	float	relelev_max;
	float	elevrange_min;
	float	elevrange_max;
	float	urban_dense_min;
	float	urban_dense_max;
	float	urban_prop_min;
	float	urban_prop_max;
	float	urban_radial_min;
	float	urban_radial_max;
	float	urban_trans_min;
	float	urban_trans_max;

	// AUTOGEN
	float	freq;
	int		max_num;

	// OBJECT
	int		obj_name;
	float	obj_width;
	float	obj_depth;
	
	// Facade		
	int		fac_allow;
	float	fac_swall_min;
	float	fac_swall_max;
	float	fac_lwall_min;
	float	fac_lwall_max;
	float	fac_area_min;
	float	fac_area_max;
	float	fac_agl_min;
	float	fac_agl_max;
};
typedef	vector<RepInfo_t>							RepTable;	

typedef hash_map<int, int>							RepFeatureIndex;
typedef multimap<float, int, greater<float> >		RepAreaIndex;
typedef hash_map<int, int>							RepUsageTable;
extern	RepTable		gRepTable;			// This is the actual master table
extern	RepFeatureIndex	gRepFeatureIndex;	// This indexes based on feature type (the object enum.)
extern	RepAreaIndex	gFacadeAreaIndex;	// This sorts facades by min area, big to small
extern	RepAreaIndex	gObjectAreaIndex;	// This sorts objects by area, big to small
extern	RepUsageTable	gRepUsage;			// This is a table of usages.
extern	int				gRepUsageTotal;

struct	FeatureInfo {
	float		property_value;
	int			terrain_type;
};
typedef hash_map<int, FeatureInfo>		FeatureInfoTable;
extern	FeatureInfoTable				gFeatures;

/*
struct	FeatureToRep_t {
	int			rep_type;
	float		min_urban;
	float		max_urban;
	float		area_density;
	int			area_max;
};
typedef hash_multimap<int, FeatureToRep_t>		FeatureToRepTable;
extern FeatureToRepTable		gFeatureToRep;
extern set<int>					gFeatureAsFacade;
*/
	
	
void	LoadObjTables(void);

// This routines returns facades that fit this profile sorted from biggest
// to smallest.  Note that they give you table indices, not feature types!
int	QueryUsableFacsBySize(
					// Rule inputs!
					int				feature,
					int				landuse,
					int				climate,
					int				terrain,
					int				zoning,
					
					float			elev,
					float			temp,
					float			slope,
					float			relelev,
					float			elevrange,
					float			urban_dense,
					float			urban_prop,
					float			urban_radial,
					float			urban_trans,
					
					// Criteria
					float			inArea,
					float			inShortSideLen,
					float			inLongSideLen,					
					bool			inLimitUsage,	// True if we DO want to apply freq rule limits.
					int *			outResults,
					
					int				inMaxResults);

// This routine returns objects that fit this profile sorted from biggest
// to smallest.

int QueryUsableObjsBySize(
					// Rule inputs!
					int				feature,
					int				landuse,
					int				climate,
					int				terrain,
					int				zoning,
					
					float			elev,
					float			temp,
					float			slope,
					float			relelev,
					float			elevrange,
					float			urban_dense,
					float			urban_prop,
					float			urban_radial,
					float			urban_trans,
					
					// Criteria
					float			inAreaMin,
					float			inAreaMax,
					float			inMaxWidth,
					float			inMaxDepth,
					bool			inLimitUsage,	// True if we DO want to apply freq rule limits.
					int *			outResults,
					
					int				inMaxResults);

void IncrementRepUsage(int inRep);
void ResetUsages(void);
bool IsWellKnownFeature(int inFeat);
	
#endif /* OBJTABLES_H */
