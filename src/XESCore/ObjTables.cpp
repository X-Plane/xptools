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
#include "ObjTables.h"
#include "ConfigSystem.h"
#include "EnumSystem.h"
#include "ParamDefs.h"

RepTable						gRepTable;
RepFeatureIndex					gRepFeatureIndex;
FeatureInfoTable				gFeatures;
//FeatureToRepTable				gFeatureToRep;
//set<int>						gFeatureAsFacade;

static set<int>					sKnownFeatures;

RepAreaIndex					gFacadeAreaIndex;
RepAreaIndex					gObjectAreaIndex;
RepUsageTable					gRepUsage;
int								gRepUsageTotal = 0;

bool	ReadRepLine(const vector<string>& tokens, void * ref)
{
	RepInfo_t	info;
	if (TokenizeLine(tokens, " eeeeefffffffffffffffffffieffiffffffff",
			&info.feature,
			&info.landuse,
			&info.climate,
			&info.terrain,
			&info.zoning,			
			&info.elev_min,
			&info.elev_max,
			&info.temp_min,
			&info.temp_max,
			&info.slope_min,
			&info.slope_max,
			&info.relelev_min,
			&info.relelev_max,
			&info.elevrange_min,
			&info.elevrange_max,
			&info.urban_dense_min,
			&info.urban_dense_max,
			&info.urban_prop_min,
			&info.urban_prop_max,
			&info.urban_radial_min,
			&info.urban_radial_max,
			&info.urban_trans_min,
			&info.urban_trans_max,

			&info.freq,
			&info.max_num,

			&info.obj_name,
			&info.obj_width,
			&info.obj_depth,

			&info.fac_allow,
			&info.fac_swall_min,
			&info.fac_swall_max,
			&info.fac_lwall_min,
			&info.fac_lwall_max,
			&info.fac_area_min,
			&info.fac_area_max,
			&info.fac_agl_min,
			&info.fac_agl_max) != 38)
				return false;
			
	int row_num = gRepTable.size();
	gRepTable.push_back(info);
	
	if (gRepFeatureIndex.count(info.obj_name) > 0)
	{
		RepInfo_t& master(gRepTable[gRepFeatureIndex[info.obj_name]]);
		if (master.freq      != info.freq		)	printf("WARNING: inconsistent frequency for object %s\n", FetchTokenString(info.obj_name));
		if (master.max_num   != info.max_num	)	printf("WARNING: inconsistent max num for object %s\n", FetchTokenString(info.obj_name));
		if (master.obj_width != info.obj_width	)	printf("WARNING: inconsistent width for object %s\n", FetchTokenString(info.obj_name));
		if (master.obj_depth != info.obj_depth	)	printf("WARNING: inconsistent depth for object %s\n", FetchTokenString(info.obj_name));
		if (master.fac_allow != info.fac_allow	)	printf("WARNING: inconsistent facade flag for object %s\n", FetchTokenString(info.obj_name));
		if (master.fac_allow && info.fac_allow)
		{
			if (master.fac_swall_min != info.fac_swall_min	)	printf("WARNING: inconsistent swall min for facade %s\n", FetchTokenString(info.obj_name));
			if (master.fac_swall_max != info.fac_swall_max	)	printf("WARNING: inconsistent swall max for facade %s\n", FetchTokenString(info.obj_name));
			if (master.fac_lwall_min != info.fac_lwall_min	)	printf("WARNING: inconsistent lwall min for facade %s\n", FetchTokenString(info.obj_name));
			if (master.fac_lwall_max != info.fac_lwall_max	)	printf("WARNING: inconsistent lwall max for facade %s\n", FetchTokenString(info.obj_name));
			if (master.fac_area_min != info.fac_area_min	)	printf("WARNING: inconsistent area min for facade %s\n", FetchTokenString(info.obj_name));
			if (master.fac_area_max != info.fac_area_max	)	printf("WARNING: inconsistent area max for facade %s\n", FetchTokenString(info.obj_name));
			if (master.fac_agl_min != info.fac_agl_min		)	printf("WARNING: inconsistent agl min for facade %s\n", FetchTokenString(info.obj_name));
			if (master.fac_agl_max != info.fac_agl_max		)	printf("WARNING: inconsistent agl max for facade %s\n", FetchTokenString(info.obj_name));
		}
	} else
		gRepFeatureIndex[info.obj_name] = row_num;

	if (info.fac_allow)
		gFacadeAreaIndex.insert(RepAreaIndex::value_type(info.fac_area_min, row_num));

	gObjectAreaIndex.insert(RepAreaIndex::value_type(info.obj_width * info.obj_depth, row_num));
	
	if (info.feature != NO_VALUE)
		sKnownFeatures.insert(info.feature);

	return true;
}

bool	ReadFeatureProps(const vector<string>& tokens, void * ref)
{
	FeatureInfo info;
	int	key;
	if (TokenizeLine(tokens, " efe", &key, &info.property_value, &info.terrain_type) != 4)
		return false;
	if (gFeatures.find(key) != gFeatures.end())
		printf("WARNING: duplicate key %s", tokens[1]);
	gFeatures[key] = info;
	return true;
}

/*
bool	ReadFeatureToRep(const vector<string>& tokens, void * ref)
{
	FeatureToRep_t i;
	int k;
	if (TokenizeLine(tokens, " eefffi", &k, &i.rep_type,
		&i.min_urban, &i.max_urban, &i.area_density, &i.area_max) != 7) return false;
	
	gFeatureToRep.insert(FeatureToRepTable::value_type(k, i));
	return true;
}
*/
void	LoadObjTables(void)
{
	gRepTable.clear();
	gRepFeatureIndex.clear();
	gFeatures.clear();
	sKnownFeatures.clear();
	gFacadeAreaIndex.clear();
	gObjectAreaIndex.clear();
	gRepUsage.clear();

	RegisterLineHandler("OBJ_PROP", ReadRepLine, NULL);
	RegisterLineHandler("FEAT_PROP", ReadFeatureProps, NULL);	
//	RegisterLineHandler("FEAT_2_OBJ", ReadFeatureToRep, NULL);
	LoadConfigFile("obj_properties.txt");
	LoadConfigFile("feat_properties.txt");
//	LoadConfigFile("feat_2_obj.txt");
	
//	for (FeatureToRepTable::iterator i = gFeatureToRep.begin(); i != gFeatureToRep.end(); ++i)
//	{
//		if (!gRepTable[gRepFeatureIndex[i->second.rep_type]].fac_name.empty())
//			gFeatureAsFacade.insert(i->first);
//	}
}

/************************************************************************************************
 * DATABASE OPERATIONS
 ************************************************************************************************/

#define RANGE_RULE(x)		(rec.x ## _min == rec.x ## _max || (rec.x ## _min <= x && x <= rec.x ## _max))

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
					
					int				inMaxResults)
{
	int 						ret = 0;	
	RepAreaIndex::iterator		recordIter, startIter;
	
	// These facades are sorted by minimum area, e.g. we fit in a block no smaller than X.
	// So we can start part way down the list skipping items that we're too small to consider.
	// (For example, the top of the list is an 'airport' - 99% of the time there's no way
	// our little block will be big enough to put one down.  Fast skip past it in LogN time.)
	// We use lower bound - that means we will be before ANY values that are smaller than us.
	// So all of the blocks that could be smaller than us come first.
	startIter = gFacadeAreaIndex.lower_bound(inArea);
	
	for (recordIter = startIter; recordIter != gFacadeAreaIndex.end(); ++ recordIter)
	{
		int row = recordIter->second;
		RepInfo_t& rec = gRepTable[row];
		
		// Evaluate this choice
		if ((rec.max_num == 0 || rec.max_num > gRepUsage[rec.obj_name]) &&
			(rec.freq == 0.0 || (rec.freq * (float) gRepUsageTotal >= gRepUsage[rec.obj_name])) &&
			
			// Enum rules
			(rec.feature == feature) &&
			(rec.landuse == NO_VALUE || rec.landuse == landuse) &&
			(rec.climate == NO_VALUE || rec.climate == climate) &&
			(rec.terrain == NO_VALUE || rec.terrain == terrain) &&
			(rec.zoning  == NO_VALUE || rec.zoning  == zoning ) &&
			// Range Rules
			RANGE_RULE(elev) &&
			RANGE_RULE(temp) &&
			RANGE_RULE(slope) &&
			RANGE_RULE(relelev) &&
			RANGE_RULE(elevrange) &&
			RANGE_RULE(urban_dense) &&
			RANGE_RULE(urban_prop) &&
			RANGE_RULE(urban_radial) &&
			RANGE_RULE(urban_trans) &&
			// Facade params
			rec.fac_area_min <= inArea &&
			rec.fac_area_max >= inArea &&
			rec.fac_swall_min <= inShortSideLen && inShortSideLen <= rec.fac_swall_max &&
			rec.fac_lwall_min <= inLongSideLen && inLongSideLen <= rec.fac_lwall_max)
		{
			outResults[ret] = row;
			++ret;
			if (ret >= inMaxResults) 
				return ret;
		}
	}
	return ret;
}


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
					
					int				inMaxResults)
{
	int 						ret = 0;	
	RepAreaIndex::iterator		recordIter, startIter;
	
	// Objects are sorted by size.  We know by definition that an
	// object won't fit in a block to osmsall for it.  So we use
	// area as a quick-eval to skip past the most high priorty but
	// hugest objects.
	
	// Note that we cannot use side length as a heueristic
	// for placement.  Consider an antenna...the end of the antenna
	// is a TINY side the length that the road is wide.  But
	// since the antenna is in the smack middle of the facade, it
	// is conceivable that a huge object could fit there.
	startIter = gObjectAreaIndex.lower_bound(inAreaMax);
	
	for (recordIter = startIter; recordIter != gObjectAreaIndex.end(); ++ recordIter)
	{
		int row = recordIter->second;
		RepInfo_t& rec = gRepTable[row];
		
		// Evaluate this choice
		if ((rec.max_num == 0 || rec.max_num > gRepUsage[rec.obj_name]) &&
			(rec.freq == 0.0 || (rec.freq * (float) gRepUsageTotal >= gRepUsage[rec.obj_name])) &&
			
			// Enum rules
			(rec.feature == feature) &&
			(rec.landuse == NO_VALUE || rec.landuse == landuse) &&
			(rec.climate == NO_VALUE || rec.climate == climate) &&
			(rec.terrain == NO_VALUE || rec.terrain == terrain) &&
			(rec.zoning  == NO_VALUE || rec.zoning  == zoning ) &&
			// Range Rules
			RANGE_RULE(elev) &&
			RANGE_RULE(temp) &&
			RANGE_RULE(slope) &&
			RANGE_RULE(relelev) &&
			RANGE_RULE(elevrange) &&
			RANGE_RULE(urban_dense) &&
			RANGE_RULE(urban_prop) &&
			RANGE_RULE(urban_radial) &&
			RANGE_RULE(urban_trans) &&
			// Obj Rules
			(rec.obj_width * rec.obj_depth) > inAreaMin &&
			(inMaxWidth == -1.0 || rec.obj_width <= inMaxWidth) &&
			(inMaxDepth == -1.0 || rec.obj_depth <= inMaxDepth))
		{
			outResults[ret] = row;
			++ret;
			if (ret >= inMaxResults) 
				return ret;
		}
	}
	return ret;
}

void IncrementRepUsage(int inRep)
{
	gRepUsage[inRep]++;
	gRepUsageTotal++;
}

void ResetUsages(void)
{
	gRepUsage.clear();
	gRepUsageTotal = 0;
}

bool IsWellKnownFeature(int inFeat)
{
	return sKnownFeatures.count(inFeat);
}
