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

#ifndef ForestTables_H
#define ForestTables_H

#include "ConfigSystem.h"

// For now we record very little about a given forest type.
struct	ForestInfo_t {
//	RGBColor_t		rgb;
};

struct	ForestRule_t {
	int			landuse;
	float		temp_min, temp_max;
//	float		temp_range_min, temp_range_max;
	float		rain_min, rain_max;
	
	int			forest_type;
};

typedef vector<ForestRule_t>	ForestRuleVector;
typedef	map<int, ForestInfo_t>	ForestInfoMap;

extern ForestRuleVector	gForestRules;
extern ForestInfoMap gForestInfo;

void	LoadForestTables(void);

int		FindForest(
				int landuse,
				float temp,
//				float temp_range,
				float rain);

inline bool IsForestType(int t) { return gForestInfo.count(t); }



#endif /* ForestTables_H */
