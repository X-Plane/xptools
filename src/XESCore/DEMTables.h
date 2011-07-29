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

#include "CompGeomDefs2.h"
#include "ConfigSystem.h"
#include "ProgressUtils.h"
#include "AssertUtils.h"
#include "ParamDefs.h"

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

extern set<int>						gEnumDEMs;

enum proj_dir_t {
	proj_Down,
	proj_NorthSouth,
	proj_EastWest
};

enum shader_t {
	shader_normal,
	shader_vary,
	shader_slope,
	shader_slope2,
	shader_heading,
	shader_tile,
	shader_composite

};
	

/************************************************************************
 * NATURAL TERRAIN RULES
 ************************************************************************/

struct	NaturalTerrainRule_t {
	// RULES
	int				terrain;	//	e.g. natural_Terrain
	int				zoning;
	int				landuse;
	int				region;
	int				climate;
	float			elev_min;
	float			elev_max;
	float			slope_min;
	float			slope_max;
	float			temp_min;
	float			temp_max;
	float			temp_rng_min;
	float			temp_rng_max;
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
	int				urban_square;
	float			lat_min;
	float			lat_max;
//	int				variant;		// 0 = use all. 1-4 = flat variants. 5-8 = sloped variants, CW from N=5.  This is a SELECTOR for terrain.

	int				name;

};

struct CliffInfo_t {
	Point2			hill_res;
	Point2			cliff_res;
	float			hill_angle1;
	float			hill_angle2;
	float			cliff_angle1;
	float			cliff_angle2;	
	string			hill_tex;
	string			cliff_tex;
};

struct	Regionalization_t {
	string			variant_prefix;
	string			region_png;
};

struct	NaturalTerrainInfo_t {
	// DEFS
	int				regionalization;	// index into regionalization table for correct library output.
	int				layer;
	int				is_city;
	int				is_forest;
	float			xon_dist;
	int				custom_ter;
	proj_dir_t		proj_angle;			// Projection angle.  For legacy terrain this influences the xon distances.  For next-gen, always use "down"...the way the xon is scaled
										// is pretty reasonable for a next-gen terrain because xon distances are horizontal.

	// 2-D texturing common to pretty much everything.
	int				autogen_mode;
	string			base_tex;
	string			lit_tex;
	Point2			base_res;
	string			border_tex;			
	string			compo_tex;		/// ALL modes that can have a second texture jam it here.
	string			noise_tex;

	string			decal;
	float			normal_scale;
	string			normal;
	
	// Mode of shader we run in
	shader_t		shader;

	// Slope shader
	CliffInfo_t		cliff_info;

	// Tile shader
	int				tiles_x;
	int				tiles_y;

	// Composite shader
	float			composite_params[6];
	Point2			comp_res;	
	

	// 2-D Texturing we don't really use anymore?					
//	string			vary_tex_;			// variation tex if needed
//	int				auto_vary_;			// Use shaders to make variations.  This a flag to add the AUTO_VARY to the .ter and is only present to allow full .ter generation.

	// Forests!
//	int				forest_type;
//	float			forest_ratio;

	RGBColor_t		map_rgb;
};
typedef vector<NaturalTerrainRule_t>	NaturalTerrainRuleVector;			// Natural terrain rules ordered by rule priority
typedef map<int, NaturalTerrainInfo_t>	NaturalTerrainInfoMap;				// Index from .ter enum to line info!

typedef vector<Regionalization_t>		RegionalizationVector;

extern	RegionalizationVector			gRegionalizations;
extern	NaturalTerrainRuleVector		gNaturalTerrainRules;
extern	NaturalTerrainInfoMap			gNaturalTerrainInfo;

// This returns a rule NAME
int		FindNaturalTerrain(
				int		terrain,
				int		zoning,
				int 	landuse,
//				int 	climate,
//				float 	elevation,
				float 	slope,
				float	slope_tri,
				float	temp,
				float	temp_rng,
				float	rain,
				int		water,
				float	slopeheading,
				float	relelevation,
				float	elevrange,
				float	urban_density,
				float	urban_radial,
				float	urban_trans,
				int		urban_square,	// use 1=square, 2=irregulra NO_DATA
				float	lat);			// use NO_DATA!
//				int		variant_blob,
//				int		variant_head);	// use 0

// This routine creates a rule whereby if the "terrain" input type matches a real .ter file, we simply use it, period.
// This allows MeshTool to allow authors to direct-select final x-plane terrain types.  This is an optional init so we 
// don't have 500 extra rules in the table when making global scenery.
void	MakeDirectRules(void);

/************************************************************************
 * ZONED TERRAIN PROMOTIONS
 ************************************************************************/


//typedef pair<int, int>										TerrainTypeTuple;
//struct	HashTerrianTuple {
//	std::size_t operator()(const TerrainTypeTuple& key) const {
//		return key.first ^ (key.second << 13); }
//};
//typedef hash_map<TerrainTypeTuple, int, HashTerrianTuple>	TerrainPromoteTable;
//extern	TerrainPromoteTable									gTerrainPromoteTable;


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
	float		min_rain;
	float		max_rain;
	float		min_temp;
	float		max_temp;
	float		min_lat;
	float		max_lat;
	float		min_slope;
	float		max_slope;
	float		min_sea;
	float		max_sea;
	float		max_turn_convex;
	float		max_turn_concave;
	float		min_len;
	int			require_open;
	float		min_area;
	int			require_airport;
	int			x_beach_type;
	int			x_backup;
};
typedef vector<BeachInfo_t>		BeachInfoTable;
extern BeachInfoTable			gBeachInfoTable;
typedef map<int, int>			BeachIndex;
extern BeachIndex				gBeachIndex;

/************************************************************************
 * BEACH TERRAIN INFO
 ************************************************************************/

typedef hash_map<int, int> 	LandUseTransTable;
extern LandUseTransTable	gLandUseTransTable;

// Given two specific terrains, equivalent to priority(lsh) < priority(rhs).
// However please note that there is no equality of priority, e.g.
// priority(a) == priorty((b) -> a == b
inline bool	LowerPriorityNaturalTerrain(int lhs, int rhs);			// Returns true if lhs is lower prio than rhs.  Lower prio is from lower layer or earlier rule if layers equal
//bool	IsForestType(int inType);								// Returns true if enum is for a forest
//inline bool 	IncompatibleProjection(int lhs, int rhs);				// Returns true if terrains are not projected the same way
//inline bool	AreVariants(int lhs, int rhs);							// Returns true if two terrains are variants of each other
//inline bool	HasVariant(int lhs);
//inline int		OtherVariant(int terrain);								// Returns a different variant of the terrain
//inline int		AnyVariant(int terrain);								// Returns any variant of the terrain randomly
//inline int		SpecificVariant(int terrain, int i);					// Use i (0-4) as a seed - get variant
//void	GetForestMapping(map<int,int>& forests);
//void			GetForestTypes(set<int>& forests);

bool	IsAirportTerrain(int t);


extern	string	gNaturalTerrainFile;
extern	string	gLanduseTransFile;
extern	string	gReplacementClimate;
extern 	string	gReplacementRoads;

void	LoadDEMTables(void);
//void	CheckDEMRuleCoverage(ProgressFunc func);
void	GetNaturalTerrainColor(int terrain, float rgb[3]);

/************************************************************************
 * CUSTOM TEXTURES
 ************************************************************************/

struct tex_proj_info {
	Point2	corners[4];
	Point2	ST[4];
};
typedef map<int,tex_proj_info>		TexProjTable;
extern TexProjTable					gTexProj;

enum {
    tex_not_custom = 0,
    tex_custom_no_water = 1,
    tex_custom_hard_water = 2,
    tex_custom_soft_water = 3
};

inline bool	LowerPriorityNaturalTerrain(int lhs, int rhs)
{
	// Fast case - if these are equal, don't even bother with a lookup, we know
	// that they can't be lower/higher prioritY!
	if (lhs == rhs) return false;

	if (lhs == terrain_Water) return true;
	if (rhs == terrain_Water) return false;

	DebugAssert(gNaturalTerrainInfo.count(lhs));
	DebugAssert(gNaturalTerrainInfo.count(rhs));

	int lhs_layer = gNaturalTerrainInfo[lhs].layer;
	int rhs_layer = gNaturalTerrainInfo[rhs].layer;

	// Lookups - if we have a layer difference, that goes.
	if (lhs_layer < rhs_layer) return true;
	if (lhs_layer > rhs_layer) return false;

	// Tie breaker - if the terrains are diferent but the layers are the same,
	// we have to enforce a layer difference somehow or else we will get haywire
	// results when we try to sort by layer priority.  So simply use their
	// index numbers as priority.  Better than nothing.
	return lhs < rhs;
}

/*
inline bool IncompatibleProjection(int lhs, int rhs)
{
	if (lhs == rhs) return false;
	return (gNaturalTerrainTable[gNaturalTerrainIndex[lhs]].proj_angle !=
			gNaturalTerrainTable[gNaturalTerrainIndex[rhs]].proj_angle);
}
*/
/*
inline bool	AreVariants(int lhs, int rhs)
{
	return false;
//	int v1 = gNaturalTerrainTable[gNaturalTerrainIndex[lhs]].related;
//	int v2 = gNaturalTerrainTable[gNaturalTerrainIndex[rhs]].related;
//	return (v1 == v2 && v1 != -1);
}
*/

/*
inline bool	HasVariant(int lhs)
{
	return false;
//	return gNaturalTerrainTable[gNaturalTerrainIndex[lhs]].related != -1;
}

inline int		OtherVariant(int terrain)
{
	return terrain;
//	int me_idx = gNaturalTerrainIndex[terrain];
//	int base = gNaturalTerrainTable[me_idx].related;
//	if (base == -1) return terrain;

//	if (base == me_idx) return gNaturalTerrainTable[base + 1 + rand() % 3].name;

//	int vary = gNaturalTerrainTable[base + rand() % 4].name;
//	if (vary == terrain) 	return gNaturalTerrainTable[base].name;
//	else					return gNaturalTerrainTable[vary].name;
}

inline int		AnyVariant(int terrain)
{
	return terrain;
//	int base = gNaturalTerrainTable[gNaturalTerrainIndex[terrain]].related;
//	if (base == -1) return terrain;
//	return gNaturalTerrainTable[base + rand() % 4].name;
}

inline int SpecificVariant(int terrain, int i)
{
	return terrain;
//	int base = gNaturalTerrainTable[gNaturalTerrainIndex[terrain]].related;
//	if (base == -1) return terrain;
//	return gNaturalTerrainTable[base + i].name;
}

*/

#endif
