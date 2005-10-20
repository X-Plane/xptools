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
	int row_num;
	if (tokens[0] == "OBJ_PROP")
	{
		if (TokenizeLine(tokens, " eefffffffffffffiefff",
			&info.feature, &info.terrain,

			&info.temp_min, &info.temp_max, 
			&info.rain_min, &info.rain_max, 
			&info.slope_min, &info.slope_max,
			
			&info.urban_dense_min, &info.urban_dense_max,
			&info.urban_radial_min, &info.urban_radial_max,
			&info.urban_trans_min, &info.urban_trans_max,
			
			&info.freq, &info.max_num,
			&info.obj_name,
			&info.width_min, 
			&info.depth_min, 
			&info.height_min) != 21) return false;
		
		info.obj_type = rep_Obj;
		info.width_max = info.width_min;
		info.depth_max = info.depth_min;
		info.height_max = info.height_min;
		row_num = gRepTable.size();
		gRepTable.push_back(info);
	}
	else if (tokens[0] == "OBS_PROP")
	{
		string base_name;
		int		height_min, height_max;
		if (TokenizeLine(tokens, " eefffffffffffffisffii",
			&info.feature, &info.terrain,

			&info.temp_min, &info.temp_max, 
			&info.rain_min, &info.rain_max, 
			&info.slope_min, &info.slope_max,
		
			&info.urban_dense_min, &info.urban_dense_max,
			&info.urban_radial_min, &info.urban_radial_max,
			&info.urban_trans_min, &info.urban_trans_max,
			
			&info.freq, &info.max_num,
			&base_name,
			&info.width_min, 
			&info.depth_min, 
			&height_min, &height_max) != 22) return false;
		
		info.obj_type = rep_Obj;
		info.width_max = info.width_min;
		info.depth_max = info.depth_min;
		
		if (height_min % 10) printf("WARNING: object %s min height %d not multiple of 10 meters.\n", base_name.c_str(), height_min);
		if (height_max % 10) printf("WARNING: object %s max height %d not multiple of 10 meters.\n", base_name.c_str(), height_max);
		
		for (int h = height_min; h <= height_max; h += 10)
		{
			info.height_max = info.height_min = h;
			char	obj_name[256];
			sprintf(obj_name,"%s%d", base_name.c_str(), h);
			info.obj_name = LookupTokenCreate(obj_name);
			row_num = gRepTable.size();
			gRepTable.push_back(info);
		}
	}
	else 
	{
		if (TokenizeLine(tokens, " eefffffffffffffieffffff",
			&info.feature, &info.terrain,

			&info.temp_min, &info.temp_max, 
			&info.rain_min, &info.rain_max, 
			&info.slope_min, &info.slope_max,
			
			&info.urban_dense_min, &info.urban_dense_max,
			&info.urban_radial_min, &info.urban_radial_max,
			&info.urban_trans_min, &info.urban_trans_max,
			
			&info.freq, &info.max_num,
			&info.obj_name,
			&info.width_min, &info.width_max, 
			&info.depth_min, &info.depth_max, 
			&info.height_min, &info.height_max) != 24) return false;
		
		info.obj_type = rep_Fac;	
		row_num = gRepTable.size();
		gRepTable.push_back(info);
	}
	
			

	// WE NEED TO REVISIT THIS GUY	
	if (gRepFeatureIndex.count(info.obj_name) > 0)
	{
		RepInfo_t& master(gRepTable[gRepFeatureIndex[info.obj_name]]);
		if (master.freq      != info.freq		)	printf("WARNING: inconsistent frequency for object %s\n", FetchTokenString(info.obj_name));
		if (master.max_num   != info.max_num	)	printf("WARNING: inconsistent max num for object %s\n", FetchTokenString(info.obj_name));
		if (master.width_min != info.width_min	)	printf("WARNING: inconsistent width for object %s\n", FetchTokenString(info.obj_name));
		if (master.width_max != info.width_max	)	printf("WARNING: inconsistent width for object %s\n", FetchTokenString(info.obj_name));
		if (master.height_min != info.height_min	)	printf("WARNING: inconsistent height for object %s\n", FetchTokenString(info.obj_name));
		if (master.height_max != info.height_max	)	printf("WARNING: inconsistent height for object %s\n", FetchTokenString(info.obj_name));
		if (master.depth_min != info.depth_min	)	printf("WARNING: inconsistent depth for object %s\n", FetchTokenString(info.obj_name));
		if (master.depth_max != info.depth_max	)	printf("WARNING: inconsistent depth for object %s\n", FetchTokenString(info.obj_name));

		if (master.obj_type != info.obj_type	)	printf("WARNING: inconsistent type for object %s\n", FetchTokenString(info.obj_name));
	} else
		gRepFeatureIndex[info.obj_name] = row_num;

//	if (info.fac_allow)
//		gFacadeAreaIndex.insert(RepAreaIndex::value_type(info.fac_area_min, row_num));

//	gObjectAreaIndex.insert(RepAreaIndex::value_type(info.obj_width * info.obj_depth, row_num));
	
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
	RegisterLineHandler("OBS_PROP", ReadRepLine, NULL);
	RegisterLineHandler("FAC_PROP", ReadRepLine, NULL);
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
					int				terrain,
					
					float			temp,
					float			rain,
					float			slope,
					float			urban_dense,
					float			urban_radial,
					float			urban_trans,
					
					float			inWidth,
					float			inDepth,
					float			inHeightMin,
					float			inHeightMax,
					
					bool			inLimitUsage,	// True if we DO want to apply freq rule limits.
					int *			outResults,					
					int				inMaxResults)
{
	int 						ret = 0;	
	for (int row = 0; row < gRepTable.size(); ++row)
	{
		RepInfo_t& rec = gRepTable[row];
		
		// Evaluate this choice
		if (rec.obj_type == rep_Fac)
		if ((rec.max_num == 0 || rec.max_num > gRepUsage[rec.obj_name]) &&
			(rec.freq == 0.0 || (rec.freq * (float) gRepUsageTotal >= gRepUsage[rec.obj_name])) &&
			
			// Enum rules
			(rec.feature == feature) &&
			(rec.terrain == NO_VALUE || rec.terrain == terrain) &&
			// Range Rules
			RANGE_RULE(temp) &&
			RANGE_RULE(slope) &&
			RANGE_RULE(rain) &&
			RANGE_RULE(urban_dense) &&
			RANGE_RULE(urban_radial) &&
			RANGE_RULE(urban_trans) &&
		
			(inWidth >= rec.width_min && inWidth <= rec.width_max) &&
			(inDepth >= rec.depth_min && inWidth <= rec.depth_max) &&
			(inHeightMin <= rec.height_max && inHeightMax >= rec.height_min))			
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
					int				terrain,
					
					float			temp,
					float			rain,
					float			slope,
					float			urban_dense,
					float			urban_radial,
					float			urban_trans,
					
					float			inWidth,
					float			inDepth,
					float			inHeightMin,
					float			inHeightMax,	// If min = max, we want an exact height!
					
					bool			inLimitUsage,	// True if we DO want to apply freq rule limits.
					int *			outResults,
					
					int				inMaxResults)
{
	int 						ret = 0;	
	
	// Objects are sorted by size.  We know by definition that an
	// object won't fit in a block to osmsall for it.  So we use
	// area as a quick-eval to skip past the most high priorty but
	// hugest objects.
	
	// Note that we cannot use side length as a heueristic
	// for placement.  Consider an antenna...the end of the antenna
	// is a TINY side the length that the road is wide.  But
	// since the antenna is in the smack middle of the facade, it
	// is conceivable that a huge object could fit there.
	
	for (int row = 0; row < gRepTable.size(); ++row)
	{
		RepInfo_t& rec = gRepTable[row];
		
		// Evaluate this choice
		if (rec.obj_type == rep_Obj)
		if ((rec.max_num == 0 || rec.max_num > gRepUsage[rec.obj_name]) &&
			(rec.freq == 0.0 || (rec.freq * (float) gRepUsageTotal >= gRepUsage[rec.obj_name])) &&
			
			// Enum rules
			(rec.feature == feature) &&
			(rec.terrain == NO_VALUE || rec.terrain == terrain) &&
			// Range Rules
			RANGE_RULE(slope) &&
			RANGE_RULE(temp) &&
			RANGE_RULE(rain) &&
			RANGE_RULE(urban_dense) &&
			RANGE_RULE(urban_radial) &&
			RANGE_RULE(urban_trans) &&
			// Obj Rules
			(inWidth >= rec.width_min && inWidth <= rec.width_max) &&
			(inDepth >= rec.depth_min && inDepth <= rec.depth_max) &&
			(inHeightMin <= rec.height_max && inHeightMax >= rec.height_min))			
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
