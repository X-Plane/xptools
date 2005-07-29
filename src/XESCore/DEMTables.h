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
#ifndef DEMTABLES_H
#define DEMTABLES_H

#include "ConfigSystem.h"

/************************************************************************
 * ENUM COLORS
 ************************************************************************/

typedef hash_map<int, RGBColor_t>		EnumColorTable;
extern	EnumColorTable					gEnumColors;

struct	DEMColorBand_t {
	float			lo_value;
	RGBColor_t		lo_color;
	float			hi_value;
	RGBColor_t		hi_color;
};

typedef map<float, DEMColorBand_t>	ColorBandMap;
typedef hash_map<int, ColorBandMap>	ColorBandTable;
extern ColorBandTable				gColorBands;
	

enum {
	proj_Down,
	proj_NorthSouth,
	proj_EastWest
};

/************************************************************************
 * NATURAL TERRAIN RULES
 ************************************************************************/

struct	NaturalTerrainInfo_t {
	// RULES
	int				terrain;	//	e.g. natural_Terrain
	int				landuse;
	int				climate;
	float			elev_min;
	float			elev_max;
	float			slope_min;
	float			slope_max;
	float			temp_min;
	float			temp_max;
	float			rain_min;
	float			rain_max;
	int				near_water;
	float			slope_heading_min;
	float			slope_heading_max;
	float			rel_elev_min;
	float			rel_elev_max;
	float			elev_range_min;
	float			elev_range_max;
	float			urban_density_min;
	float			urban_density_max;
	float			urban_radial_min;
	float			urban_radial_max;
	float			urban_trans_min;
	float			urban_trans_max;
	
	// DEFS
	int				name;
	int				layer;
	float			xon_dist;
	// 2-D Texturing
	string			base_tex;
//	string			comp_tex;
	float			base_res;
//	float			comp_res;
//	int				base_alpha_invert;
//	int				comp_alpha_invert;
	int				proj_angle;	
	string			border_tex;
	
	// Forests!
	int				forest_type;
//	float			forest_ratio;
	
	float			map_rgb[3];
};	
typedef vector<NaturalTerrainInfo_t>	NaturalTerrainTable;			// Natural terrain rules ordered by rule priority
typedef multimap<int, int>				NaturalTerrainLandUseIndex;		// Index based on land use ranges
typedef map<int, int>					NaturalTerrainIndex;			// Index from .ter enum to line info!

extern	NaturalTerrainTable				gNaturalTerrainTable;
extern	NaturalTerrainLandUseIndex		gNaturalTerrainLandUseIndex;
extern	NaturalTerrainIndex				gNaturalTerrainIndex;
int		FindNaturalTerrain(
				int		terrain,
				int 	landuse, 
				int 	climate, 
				float 	elevation, 
				float 	slope,
				float	temp,
				float	rain,
				int		water,
				float	slopeheading,
				float	relelevation,
				float	elevrange,
				float	urban_density,
				float	urban_radial,
				float	urban_trans);

/************************************************************************
 * ZONED TERRAIN PROMOTIONS
 ************************************************************************/


typedef pair<int, int>										TerrainTypeTuple;
struct	HashTerrianTuple {
	std::size_t operator()(const TerrainTypeTuple& key) const {
		return key.first ^ (key.second << 13); }
};	
typedef hash_map<TerrainTypeTuple, int, HashTerrianTuple>	TerrainPromoteTable;
extern	TerrainPromoteTable									gTerrainPromoteTable;


/************************************************************************
 * MAN-MADE TERRAIN INFO
 ************************************************************************/
#if 0
struct ManTerrainInfo_t {
	string		terrain_name;
	float		base_res;
	string		base_tex;
	float		comp_res;
	string		comp_tex;
	string		lit_tex;
};
typedef hash_map<int, ManTerrainInfo_t>			ManTerrainTable;
extern ManTerrainTable							gManTerrainTable;
#endif
/************************************************************************
 * BEACH TERRAIN INFO
 ************************************************************************/
struct BeachInfo_t {
	float		min_slope;
	float		max_slope;
	int			terrain_type;			
	float		min_sea;
	float		max_sea;
	float		min_len;
	int			x_beach_type;
};
typedef vector<BeachInfo_t>		BeachInfoTable;
extern BeachInfoTable			gBeachInfoTable;

/************************************************************************
 * BEACH TERRAIN INFO
 ************************************************************************/

typedef hash_map<int, int> 	LandUseTransTable;
extern LandUseTransTable	gLandUseTransTable;

// Given two specific terrains, equivalent to priority(lsh) < priority(rhs).
// However please note that there is no equality of priority, e.g. 
// priority(a) == priorty((b) -> a == b
bool	LowerPriorityNaturalTerrain(int lhs, int rhs);
bool	IsForestType(int inType);

extern	string	gNaturalTerrainFile;
extern	string	gLanduseTransFile;

void	LoadDEMTables(void);
void	CheckDEMRuleCoverage(void);
void	GetNaturalTerrainColor(int terrain, float rgb[3]);

#endif
