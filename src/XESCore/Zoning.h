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

#ifndef ZONING_H
#define ZONING_H

#include "AptDefs.h"
#include "MapDefs.h"
#include "ProgressUtils.h"

struct	DEMGeo;

struct ZoningRule_t {

	int			terrain;								// Required base terrain (or require natural)
	float		size_min,		size_max;				// Size of block in square meters
	int			side_min,		side_max;				// Number of sides of block
	float		slope_min,		slope_max;				// Max slope within block
	float		urban_min,		urban_max;				// Total sum of urban activity
	float		forest_min,		forest_max;				// Total sum of forest
	float		urban_avg_min,	urban_avg_max;			// Average urbanization level
	float		forest_avg_min,	forest_avg_max;			// Average forest level
	float		bldg_min,		bldg_max;				// Maximum building height (0 if none)

	int			req_water;								// Are we adjacent to water (not counting holes/lakes)
	int			req_train;								// Train tracks on outer border?
	int			req_road;								// Roads with direct access (primary/secondary, not highway or ramp)

	set<int>	require_features;						// One of these features MUST be present or we can't use the rule.
	set<int>	consume_features;						// These features get consumed by the act of zoning.
	int			crud_ok;								// We can live with "stuff" in our zone that we haven't consumed.
	int			hole_ok;								// Is there a hole in the block?
	
	int			zoning;
};

typedef vector<ZoningRule_t>	ZoningRuleTable;
extern ZoningRuleTable				gZoningRules;
extern set<int>						gZoningTypes;

struct FillRule_t {
	
	int			zoning;									// Base zoning we act upon
	
	float		size_min,		size_max;				// These params limit the kind of block we act upon
	int			side_min,		side_max;
	float		slope_min,		slope_max;
	float		bldg_min,		bldg_max;

	int			adj_terrain;							// These locate an adjacent edge with a certain property.   We will key
	int			adj_network;							// the feature to align to this edge.
	int			adj_zoning;

	int			block_id;								// art asset names of various things we can insert.
	int			string_id;
	int			point_id;
	int			veg_id;
};	

typedef vector<FillRule_t>		FillRuleTable;
extern FillRuleTable			gFillRules;

void	LoadZoningRules(void);


/*
 * Given a map and various raster parameters, go through the map and
 * assign terrain types to all polygons as appropriate.  This includes
 * terrain type based on area features or terrain types based on
 * urban zoning.
 *
 */
void	ZoneManMadeAreas(
				Pmwx& 				ioMap,
				const DEMGeo& 		inLanduse,
				const DEMGeo&		inForest,
				const DEMGeo& 		inSlope,
				const AptVector&	inApts,
				ProgressFunc		inProg);

#endif /* ZONING_H */
