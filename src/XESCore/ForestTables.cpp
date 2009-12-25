/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "ForestTables.h"
#include "EnumSystem.h"
#include "DEMDefs.h"

ForestRuleVector	gForestRules;
ForestInfoMap gForestInfo;

static bool	ReadForestRule(const vector<string>& tokens, void * ref)
{
	if(tokens.size() != 7)
	{
		printf("Illegal forest rule.\n");
		return false;
	}
	ForestRule_t	r;

	if(TokenizeLine(tokens, "  ffffe", 
		&r.temp_min, &r.temp_max,
//		&r.temp_range_min, &r.temp_range_max,
		&r.rain_min, &r.rain_max,
		&r.forest_type) != 7)
	{
		printf("Illegal forest line.\n");
		return false;
	}
	
	gForestInfo[r.forest_type] = ForestInfo_t();

	set<int>	lus;
	
	if(!TokenizeEnumSet(tokens[1],lus))
	{
		printf("Illegal lu in %s\n", tokens[1].c_str());
		return false;
	}
	for(set<int>::iterator lu = lus.begin(); lu != lus.end(); ++lu)
	{
		r.landuse = *lu;
		gForestRules.push_back(r);
	}
	return true;
}
	

void	LoadForestTables(void)
{
	gForestInfo.clear();
	gForestRules.clear();
	RegisterLineHandler("FOREST_RULE", ReadForestRule, NULL);
	LoadConfigFile("forests.txt");
	
	set<int>	all_forests;
	for(ForestInfoMap::iterator i = gForestInfo.begin(); i != gForestInfo.end(); ++i)
		all_forests.insert(i->first);
	
	for(ForestRuleVector::iterator r = gForestRules.begin(); r != gForestRules.end(); ++r)
	{
		if(r->forest_type != NO_VALUE)
		{
			if(gForestInfo.count(r->forest_type) == 0)
				printf("WARNING: the rule yielding type %s references a type that is not a forest.\n", FetchTokenString(r->forest_type));
			else
				all_forests.erase(r->forest_type);
		}
	}
	for(set<int>::iterator a = all_forests.begin(); a != all_forests.end(); ++a)
	{
		printf("WARNING: forest type %s is not used by any rule.\n", FetchTokenString(*a));
	}
	
}

int		FindForest(
				int landuse,
				float temp,
//				float temp_range,
				float rain)
{
	DebugAssert(temp != DEM_NO_DATA);
//	DebugAssert(temp_range != DEM_NO_DATA);
	DebugAssert(rain != DEM_NO_DATA);
	for(ForestRuleVector::iterator r = gForestRules.begin(); r != gForestRules.end(); ++r)
	{
		if(r->landuse == NO_VALUE || landuse == NO_VALUE || r->landuse == landuse)
		if(r->temp_min == r->temp_max || (r->temp_min <= temp && temp <= r->temp_max ))
//		if(r->temp_range_min == r->temp_range_max || (r->temp_range_min <= temp_range && temp_range <= r->temp_range_max ))
		if(r->rain_min == r->rain_max || (r->rain_min <= rain && rain <= r->rain_max ))
		{
			return r->forest_type;
		}
	}
	return NO_VALUE;
}
