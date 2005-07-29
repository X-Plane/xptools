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

NetFeatureInfoTable				gNetFeatures;
NetEntityInfoTable				gNetEntities;
Road2NetInfoTable				gRoad2Net;
//set<int>						gBridgeTypes;

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
	
	if (TokenizeLine(tokens, " efffii",&entity_type,
		&info.width, &info.pad, &info.building_percent, &info.limited_access, &info.export_type) != 7)
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

void	LoadNetFeatureTables(void)
{
	gNetFeatures.clear();
	gNetEntities.clear();
	gRoad2Net.clear();
//	gBridgeTypes.clear();

	RegisterLineHandler("ROAD_GENERAL", RoadGeneralProps, NULL);
	RegisterLineHandler("ROAD_PROP", ReadRoadSpecificProps, NULL);
	RegisterLineHandler("ROAD_PICK", ReadRoadPick, NULL);
	LoadConfigFile("road_properties.txt");
//	LoadConfigFile("net_properties.txt");
//	LoadConfigFile("road_2_net.txt");
	
//	for (Road2NetInfoTable::iterator i = gRoad2Net.begin(); i != gRoad2Net.end(); ++i)
//	if (i->second.bridge)
//		gBridgeTypes.insert(i->second.entity_type);

//	for (Road2NetInfoTable::iterator i = gRoad2Net.begin(); i != gRoad2Net.end(); ++i)
//	if (!i->second.bridge)
//		gBridgeTypes.erase(i->second.entity_type);
	
//	for (NetEntityInfoTable::iterator i = gNetEntities.begin(); i != gNetEntities.end(); ++i)
//		printf("Net %s    %s.\n",
//			FetchTokenString(i->first),
//			gBridgeTypes.count(i->first) ? "yes" : "no");
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

