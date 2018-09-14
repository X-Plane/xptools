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
#include "NetTables.h"
#include "EnumSystem.h"
#include "ConfigSystem.h"
#include "AssertUtils.h"
#include "XESConstants.h"
#include "GISTool_Globals.h"
#include <math.h>

NetFeatureInfoTable				gNetFeatures;
NetRepInfoTable					gNetReps;
Feature2RepInfoTable			gFeature2Rep;
ForkRuleTable					gForkRules;
ChangeRuleTable					gChangeRules;
BridgeInfoTable					gBridgeInfo;
map<int,int>					gTwinRules;

LevelCrossingTable				gLevelCrossings;

ZonePromoteTable				gZonePromote;
RoadCountryTable				gRoadCountry;

set<int>						gPromotedZoningSet;


bool	RoadGeneralProps(const vector<string>& tokens, void * ref)
{
	int				feature_type;
	NetFeatureInfo	info;
	if (TokenizeLine(tokens, " efie", &feature_type,
		&info.density_factor, &info.is_oneway, &info.oneway_feature) != 5) return false;


	if (gNetFeatures.count(feature_type) > 0)
		printf("WARNING: duplicate token %s\n", tokens[1].c_str());


	gNetFeatures[feature_type] = info;
	return true;
}

bool	ReadRoadSpecificProps(const vector<string>& tokens, void * ref)
{
	int rep_type;
	NetRepInfo	info;
	info.export_type_draped = NO_VALUE;	// hack for mesh tool - allow draped param to not be attached!

	float	crease, max_rad;

	float edge_l, core_l, core_r, edge_r;

	if (TokenizeLine(tokens, " effffffeiifff",&rep_type,
		&edge_l, &core_l, &core_r, &edge_r,
		&info.pad, &info.building_percent, &info.use_mode, &info.is_oneway, &info.export_type_draped, &crease, &max_rad, &info.max_err) != 14)
	{
		return false;
	}
	info.semi_l = edge_l+core_l;
	info.semi_r = edge_r+core_r;
	
	info.crease_angle_cos=cos(crease * DEG_TO_RAD);
	info.min_defl_deg_mtr = max_rad > 0.0 ? (360.0 / (2 * PI * max_rad)) : 0.0f;
	
	if (gNetReps.count(rep_type) > 0)
		printf("WARNING: duplicate token %s\n", FetchTokenString(rep_type));

	gNetReps[rep_type] = info;
	
//	printf("%s: %f, %f\n", FetchTokenString(rep_type),info.semi_l, info.semi_r);
	return true;
}

bool	ReadRoadPick(const vector<string>& tokens, void * ref)
{
	Feature2RepInfo	info;
	int				feature_type;

	if (TokenizeLine(tokens, " effffffffe", &info.feature, 
		&info.min_density,&info.max_density, 
		&info.min_rail, &info.max_rail,
		&info.rain_min,
		&info.rain_max,
		&info.temp_min,
		&info.temp_max,
		&info.rep_type) != 11)	return false;

	gFeature2Rep.push_back(info);
	return true;
}

bool	ReadRoadPromoteZoning(const vector<string>& tokens, void * ref)
{
	gPromotedZoningSet.clear();
	if(TokenizeLine(tokens," S",&gPromotedZoningSet) != 2)
		return false;
	return true;
}

bool	ReadLevelCrossing(const vector<string>& tokens, void * ref)
{
	int t1, t2;
	if(TokenizeLine(tokens," ee", &t1, &t2) != 3)
		return false;
	gLevelCrossings[t1] = t2;
	return true;
}

bool	ReadRoadCountry(const vector<string>& tokens, void * ref)
{
	int src, dst;
	if(TokenizeLine(tokens, " ee", &src, &dst) != 3) return false;
	DebugAssert(gRoadCountry.count(src) == 0);
	gRoadCountry[src] = dst;
	return true;
}

bool	ReadRoadPromote(const vector<string>& tokens, void * ref)
{
	int	rt;
	ZoningPromote	p;
	if(TokenizeLine(tokens," eeee",&rt,&p.promote_left, &p.promote_right,&p.promote_both) != 5) return false;
	if(gZonePromote.count(rt) != 0)
		fprintf(stderr,"WARNING: promotion info for road %s included twice.\n", FetchTokenString(rt));
	gZonePromote[rt] = p;
	return true;
}

bool	ReadForkRule(const vector<string>& tokens, void * ref)
{
	ForkRule r;
	if(TokenizeLine(tokens," eeeeee",
						&r.trunk,
						&r.left,
						&r.right,
						&r.new_trunk,
						&r.new_left,
						&r.new_right) != 7)
	return false;
	gForkRules.push_back(r);
	return true;
				
}

bool	ReadChangeRule(const vector<string>& tokens, void * ref)
{
	ChangeRule r;
	if(TokenizeLine(tokens," eeee",
						&r.prev,
						&r.next,
						&r.new_mid) != 4)
	return false;
	gChangeRules.push_back(r);
	return true;
				
}


bool	ReadRoadBridge(const vector<string>& tokens, void * ref)
{
	BridgeInfo	info;

	if (TokenizeLine(tokens, " efffffffififfffffffi",
		&info.rep_type,
		&info.min_length, &info.max_length,
		&info.min_seg_length, &info.max_seg_length,
		&info.min_seg_count, &info.max_seg_count,
		&info.curve_limit,
		&info.split_count, &info.split_length, &info.split_arch,
		&info.min_start_agl, &info.max_start_agl,
		&info.search_dist,   &info.pref_start_agl,
		&info.min_center_agl, &info.max_center_agl,
		&info.height_ratio, &info.road_slope,
		&info.export_type) != 21) return false;

		// Special case these - otherwise we get inexact values from deg-rad conversion.
		 if (info.curve_limit == 90.0)		info.curve_limit = 0.0;
	else if (info.curve_limit ==180.0)		info.curve_limit =-1.0;
	else if (info.curve_limit ==  0.0)		info.curve_limit = 1.0;
	else									info.curve_limit = cos(info.curve_limit * DEG_TO_RAD);

	gBridgeInfo.push_back(info);
	return true;
}

bool ReadTwinRule(const vector<string>& tokens, void * ref)
{
	int type_1, type_2;
	if(TokenizeLine(tokens, " ee",&type_1,&type_2) != 3) return false;
	
	if(gTwinRules.count(type_1) || 
		gTwinRules.count(type_2))
	{
		fprintf(stderr, "ERROR: duplicate twin rule.\n");
		return false;
	}
	gTwinRules[type_1] = type_2;
	gTwinRules[type_2] = type_1;
	return true;
}

void	LoadNetFeatureTables(void)
{
	gNetFeatures.clear();
	gNetReps.clear();
	gFeature2Rep.clear();
	gBridgeInfo.clear();
	gTwinRules.clear();
	gForkRules.clear();
	gChangeRules.clear();
	gZonePromote.clear();
	gPromotedZoningSet.clear();
	gLevelCrossings.clear();

	RegisterLineHandler("ROAD_GENERAL", RoadGeneralProps, NULL);
	RegisterLineHandler("ROAD_PROP", ReadRoadSpecificProps, NULL);
	RegisterLineHandler("ROAD_PICK", ReadRoadPick, NULL);
	RegisterLineHandler("FORK_RULE",ReadForkRule,NULL);
	RegisterLineHandler("CHANGE_RULE",ReadChangeRule,NULL);
	RegisterLineHandler("ROAD_BRIDGE", ReadRoadBridge, NULL);
	RegisterLineHandler("ROAD_TWIN", ReadTwinRule, NULL);
	RegisterLineHandler("ROAD_PROMOTE_ZONING", ReadRoadPromoteZoning, NULL);
	RegisterLineHandler("ROAD_PROMOTE", ReadRoadPromote, NULL);
	RegisterLineHandler("LEVEL_CROSSING", ReadLevelCrossing, NULL);
	RegisterLineHandler("ROAD_COUNTRY", ReadRoadCountry, NULL);
	LoadConfigFile(gRegion == rf_eu ? "road_properties_eu.txt" : "road_properties_us.txt");
}

//bool	IsSeparatedHighway(int feat_type)
//{
//	if (gNetFeatures.count(feat_type) == 0) return false;
//	return gNetFeatures[feat_type].oneway_feature != NO_VALUE;
//}
//
//int		SeparatedToOneway(int feat_type)
//{
//	if (gNetFeatures.count(feat_type) == 0) return feat_type;
//	int new_type = gNetFeatures[feat_type].oneway_feature;
//	if (new_type == NO_VALUE) return feat_type;
//	return new_type;
//}
//

bool	IsOneway(int rep_type)
{
	if (gNetReps.count(rep_type) == 0) return 0;
	return gNetReps[rep_type].is_oneway;
}

bool	IsTwinRoads(int rep_type1, int rep_type2)
{
	map<int,int>::iterator i = gTwinRules.find(rep_type1);
	return i != gTwinRules.end() && i->second == rep_type2;
}



int		FindBridgeRule(int rep_type, double len, double smallest_seg, double biggest_seg, int num_segments, double curve_dot, double agl1, double agl2)
{
	DebugAssert(len > 0.0);
	DebugAssert(smallest_seg > 0.0);
	DebugAssert(biggest_seg > 0.0);
	DebugAssert(smallest_seg <= biggest_seg);
	DebugAssert(num_segments > 0);
	DebugAssert(len >= biggest_seg);

	for (int n = 0; n < gBridgeInfo.size(); ++n)
	{
		BridgeInfo& rule = gBridgeInfo[n];
		if (rule.rep_type == rep_type &&
			(rule.curve_limit <= curve_dot) &&
			(rule.min_length == rule.max_length || (rule.min_length <= len && len <= rule.max_length)) &&
			(rule.min_start_agl == rule.max_start_agl || agl1 == -1.0 || (rule.min_start_agl <= agl1 && agl1 <= rule.max_start_agl)) &&
			(rule.min_start_agl == rule.max_start_agl || agl2 == -1.0 || (rule.min_start_agl <= agl2 && agl2 <= rule.max_start_agl)) &&
			(rule.min_seg_length == rule.max_seg_length || (rule.min_seg_length <= smallest_seg && biggest_seg <= rule.max_seg_length)) &&
			(rule.min_seg_count == rule.max_seg_count || (rule.min_seg_count <= num_segments && num_segments <= rule.max_seg_count))
		)
		{
			return n;
		}
	}
	return -1;
}
