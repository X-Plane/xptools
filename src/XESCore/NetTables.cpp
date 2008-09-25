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
#include <math.h>

NetFeatureInfoTable				gNetFeatures;
NetEntityInfoTable				gNetEntities;
Road2NetInfoTable				gRoad2Net;
BridgeInfoTable					gBridgeInfo;


bool	RoadGeneralProps(const vector<string>& tokens, void * ref)
{
	int				feature_type;
	NetFeatureInfo	info;
	if (TokenizeLine(tokens, " efe", &feature_type,
		&info.density_factor, &info.one_way) != 4) return false;


	if (gNetFeatures.count(feature_type) > 0)
		printf("WARNING: duplicate token %s\n", tokens[1].c_str());


	gNetFeatures[feature_type] = info;
	return true;
}

bool	ReadRoadSpecificProps(const vector<string>& tokens, void * ref)
{
	int entity_type;
	NetEntityInfo	info;

	if (TokenizeLine(tokens, " effffeii",&entity_type,
		&info.width, &info.pad, &info.building_percent, &info.max_slope, &info.use_mode, &info.export_type_normal,&info.export_type_overpass) != 9)
		return false;

	if (gNetEntities.count(entity_type) > 0)
		printf("WARNING: duplicate token %s\n", FetchTokenString(entity_type));

	gNetEntities[entity_type] = info;
	return true;
}

bool	ReadRoadPick(const vector<string>& tokens, void * ref)
{
	Road2NetInfo	info;
	int				feature_type;

	if (TokenizeLine(tokens, " effe", &feature_type, &info.min_density,
		&info.max_density, &info.entity_type) != 5)	return false;

	gRoad2Net.insert(Road2NetInfoTable::value_type(feature_type, info));
	return true;
}

bool	ReadRoadBridge(const vector<string>& tokens, void * ref)
{
	BridgeInfo	info;

	if (TokenizeLine(tokens, " efffffffififfffffffi",
		&info.entity_type,
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

void	LoadNetFeatureTables(void)
{
	gNetFeatures.clear();
	gNetEntities.clear();
	gRoad2Net.clear();
	gBridgeInfo.clear();

	RegisterLineHandler("ROAD_GENERAL", RoadGeneralProps, NULL);
	RegisterLineHandler("ROAD_PROP", ReadRoadSpecificProps, NULL);
	RegisterLineHandler("ROAD_PICK", ReadRoadPick, NULL);
	RegisterLineHandler("ROAD_BRIDGE", ReadRoadBridge, NULL);
	LoadConfigFile("road_properties.txt");
}

bool	IsSeparatedHighway(int road_type)
{
	if (gNetFeatures.count(road_type) == 0) return false;
	return gNetFeatures[road_type].one_way != NO_VALUE;
}

int		SeparatedToOneway(int road_type)
{
	if (gNetFeatures.count(road_type) == 0) return road_type;
	int new_type = gNetFeatures[road_type].one_way;
	if (new_type == NO_VALUE) return road_type;
	return new_type;
}

int		FindBridgeRule(int entity_type, double len, double smallest_seg, double biggest_seg, int num_segments, double curve_dot, double agl1, double agl2)
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
		if (rule.entity_type == entity_type &&
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
