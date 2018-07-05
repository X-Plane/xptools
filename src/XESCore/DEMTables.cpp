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
#include "DEMTables.h"

#include "EnumSystem.h"
#include "DEMDefs.h"
#include "Zoning.h"
#include <ctype.h>
//#include "CoverageFinder.h"

//static int s_is_city = 0;
//static int s_is_forest = 0;


EnumColorTable				gEnumColors;
ColorBandTable				gColorBands;
set<int>					gEnumDEMs;


RegionalizationVector			gRegionalizations;
NaturalTerrainRuleVector		gNaturalTerrainRules;
NaturalTerrainInfoMap			gNaturalTerrainInfo;
BeachInfoTable					gBeachInfoTable;
BeachIndex						gBeachIndex;

LandUseTransTable				gLandUseTransTable;

TexProjTable					gTexProj;


//static set<int>			sForests;

static set<int>			sAirports;

string	gNaturalTerrainFile;
string	gLanduseTransFile;
string	gReplacementClimate;
string 	gReplacementRoads;

static map<string,CliffInfo_t>		sCliffs;

#define R_VARY 0

inline double cosdeg(double deg)
{
	if (deg == 0.0) return 1.0;
	if (deg == 90.0) return 0.0;
	if (deg == 180.0) return -1.0;
	if (deg == -90.0) return 0.0;
	if (deg == -180.0) return -1.0;
	return cos(deg * DEG_TO_RAD);
}

static string	MakeLit(const string& inName)
{
	string lit(inName);
	lit.insert(lit.length() - 4, "_LIT");
	return lit;
}

static string	MakeCompo(const string& inName)
{
	string lit(inName);
	lit.insert(lit.length() - 4, "2");
	return lit;
}

void MakeRVariant(string& io_string)
{
	io_string.insert(io_string.length()-4,"_R");
}


static	bool	LowerCheckName(string& ioName)
{
	string::size_type dir_char = ioName.find_last_of("\\/:");
	if (dir_char == ioName.npos) dir_char = 0;
	else						 dir_char++;
	for (string::size_type n = dir_char; n < ioName.length(); ++n)
		ioName[n] = tolower(ioName[n]);
	if ((ioName.size() - dir_char) > 27)
	{
		printf("The file name part of %s is too long - max limit for terrain names is 27 chars.\n", ioName.c_str());
		return false;
	}
	return true;
}

bool	ReadEnumColor(const vector<string>& tokens, void * ref)
{
	RGBColor_t	col;
	if (tokens.size() != 3 || !TokenizeColor(tokens[2], col))
	{
		DebugAssertPrintf("Parse error for enum colors.\n");
		return false;
	}

	int token;
	if (!TokenizeEnum(tokens[1],token, "Unknown token %s\n")) return false;
	if (gEnumColors.find(token) != gEnumColors.end())
		printf("WARNING: duplicate token %s\n", tokens[1].c_str());
	gEnumColors.insert(EnumColorTable::value_type(token, col));
	return true;
}

bool	ReadEnumBand(const vector<string>& tokens, void * ref)
{
	int				dem_key;
	float 			band_key;
	DEMColorBand_t	band;

	if (TokenizeLine(tokens, " effcc",
		&dem_key, &band.lo_value, &band.hi_value,
		&band.lo_color, &band.hi_color) != 6) return false;

	band_key = band.hi_value;

	ColorBandMap& table = gColorBands[dem_key];
	if (table.count(band_key) != 0)
		printf("WARNING: duplicate color band: %s/%f %f to %f\n",
			FetchTokenString(dem_key), band_key, band.lo_value, band.hi_value);
	table.insert(ColorBandMap::value_type(band_key, band));
	return true;

}

bool	ReadEnumDEM(const vector<string>& tokens, void * ref)
{
	int	dem;
	if (TokenizeLine(tokens, " e", &dem) != 2) return false;
	gEnumDEMs.insert(dem);
	return true;
}

bool	ReadBeachInfo(const vector<string>& tokens, void * ref)
{
	BeachInfo_t	info;

	if (TokenizeLine(tokens, " fffffffffffffifiii",
				&info.min_rain,
				&info.max_rain,
				&info.min_temp,
				&info.max_temp,
				&info.min_lat,
				&info.max_lat,
				&info.min_slope,
				&info.max_slope,
				&info.min_sea,
				&info.max_sea,
				&info.max_turn_convex,
				&info.max_turn_concave,
				&info.min_len,
				&info.require_open,
				&info.min_area,
				&info.require_airport,
				&info.x_beach_type,
				&info.x_backup) != 19) return false;

	info.max_turn_convex = cosdeg(info.max_turn_convex);
	info.max_turn_concave = cosdeg(info.max_turn_concave);

	info.min_slope=cosdeg(info.min_slope);
	info.max_slope=cosdeg(info.max_slope);			// NOTE: because we are storing cosigns, a flat beach is 1.0, so we are reversing the
	swap(info.min_slope,info.max_slope);			// ordering here.
	DebugAssert(gBeachIndex.count(info.x_beach_type) == 0);
	gBeachIndex[info.x_beach_type] = gBeachInfoTable.size();
	gBeachInfoTable.push_back(info);
	return true;
}

static void 	AddRuleInfoPair(NaturalTerrainRule_t& rule, NaturalTerrainInfo_t& info)
{
	NaturalTerrainInfoMap::iterator i = gNaturalTerrainInfo.find(rule.name);
	if(i == gNaturalTerrainInfo.end())
		gNaturalTerrainInfo.insert(NaturalTerrainInfoMap::value_type(rule.name, info));
	else
	{
//		if (i->second.forest_type	!= info.forest_type	)	printf("ERROR:  terrain 'forest type' does not match.  name = %s, layers = %s vs %s\n", FetchTokenString(rule.name), FetchTokenString(i->second.forest_type),FetchTokenString(info.forest_type));
		if (i->second.layer    		!= info.layer    	)	printf("ERROR:  terrain 'layer' does not match.  name = %s, layers = %d vs %d\n", FetchTokenString(rule.name), i->second.layer,info.layer);
		if (i->second.xon_dist 		!= info.xon_dist 	)	printf("ERROR:  terrain 'xon_dist' does not match.  name = %s, layers = %f vs %f\n", FetchTokenString(rule.name), i->second.xon_dist,info.xon_dist);
		if (i->second.base_tex    	!= info.base_tex    )	printf("ERROR:  terrain 'base_tex' does not match.  name = %s, layers = %s vs %s\n", FetchTokenString(rule.name), i->second.base_tex.c_str(),info.base_tex.c_str());
		if (i->second.base_res		!= info.base_res	)	printf("ERROR:  terrain 'base_res' does not match.  name = %s, layers = %lfx%lf vs %lfx%lf\n", FetchTokenString(rule.name), i->second.base_res.x(),i->second.base_res.y(),info.base_res.x(),info.base_res.y());

//		if(i->second != info)
//		{
//			printf("WARNING: terrain type %s has inconsistent rules.\n", FetchTokenString(rule.name));
//		}
	}
	
	gNaturalTerrainRules.push_back(rule);
}

bool HandleFlags(const vector<string>& tokens, void * ref)
{
	if(tokens[0] == "TERRAIN_IS_CITY")
	{
//		s_is_city = atoi(tokens[1].c_str());
		return true;
	}
	if(tokens[0] == "TERRAIN_FOREST")
	{
//		s_is_forest = atoi(tokens[1].c_str());
		return true;
	}
	return false;
}
bool	ReadNewTerrainInfo(const vector<string>& tokens, void * ref)
{
	if(tokens[0] == "TERRAIN_RULE")
	{
		set<int>	lu_set;
		string		lu_set_string;
		string		zone_string;
		NaturalTerrainRule_t	rule;
		if(TokenizeLine(tokens," esseeeffffffffffiffffffe",
			&rule.terrain, &zone_string, &lu_set_string, 
			&rule.soil_style,&rule.agri_style,&rule.clim_style,
			&rule.elev_min,
			&rule.elev_max,
			&rule.slope_min,
			&rule.slope_max,
			&rule.temp_min,
			&rule.temp_max,
			&rule.temp_rng_min,
			&rule.temp_rng_max,
			&rule.rain_min,
			&rule.rain_max,
			&rule.near_water,
			&rule.rel_elev_min,
			&rule.rel_elev_max,
			&rule.elev_range_min,
			&rule.elev_range_max,
			&rule.lat_min,
			&rule.lat_max,
			&rule.name) != 25)
				return false;
		
		rule.zoning = LookupToken(zone_string.c_str());
		if(rule.zoning == -1)
		{
			rule.zoning = atoi(zone_string.c_str());
			if(rule.zoning == 0)
			{
				return false;
			}
		} else {
			if (rule.zoning != NO_VALUE)
				Assert(!"hrm.");
		}

		
		rule.urban_density_min = rule.urban_density_max = 0.0;
		rule.urban_radial_min = rule.urban_radial_max = 0.0;
		rule.urban_trans_min = rule.urban_trans_max = 0.0;
		rule.urban_square = 0;
		rule.slope_heading_min = rule.slope_heading_max = 0.0;
		rule.variant = 0;
		if (rule.elev_min > rule.elev_max)
			{ fprintf(stderr, "Illegal elevation\n"); return false; }
		if (rule.slope_min > rule.slope_max)	
			{ fprintf(stderr, "Illegal slope\n"); return false; }
		if (rule.temp_min > rule.temp_max)		
			{ fprintf(stderr, "Illegal temperature\n"); return false; }
		if (rule.temp_rng_min > rule.temp_rng_max)
			{ fprintf(stderr, "Illegal temperature range\n"); return false; }
		if (rule.rain_min > rule.rain_max)		
			{ fprintf(stderr, "Illegal rain\n"); return false; }
		if (rule.rel_elev_min > rule.rel_elev_max)
			{ fprintf(stderr, "Illegal relative elevation\n"); return false; }
		if (rule.elev_range_min > rule.elev_range_max)
			{ fprintf(stderr, "Illegal elevation range\n"); return false; }

		rule.slope_min = 1.0 - cosdeg(rule.slope_min);
		rule.slope_max = 1.0 - cosdeg(rule.slope_max);

		if(!TokenizeEnumSet(lu_set_string, lu_set))
		{
			fprintf(stderr, "Illegal landuse.\n");
			return false;
		}	
		
		if(rule.zoning > 4 && gZoningInfo.count(rule.zoning) == 0)
		{
			fprintf(stderr,"Zoning type %s is unknown.\n", FetchTokenString(rule.zoning));
			return false;
		}
		if(gNaturalTerrainInfo.count(rule.name) == 0)
		{
			fprintf(stderr,"Rule final terrain name %s is not a real terrain name.\n", FetchTokenString(rule.name));
			return false;
		}
		
		if(gNaturalTerrainInfo[rule.name].regionalization > 0)
		{
			fprintf(stderr,"ERROR: rule uses terrain %s which is NOT a base - it is regionalized as %s\n", FetchTokenString(rule.name), gRegionalizations[gNaturalTerrainInfo[rule.name].regionalization].variant_prefix.c_str());
			return false;
		}

		if(lu_set.empty())
		{
			rule.landuse = NO_VALUE;
			gNaturalTerrainRules.push_back(rule);
		}
		else
		for(set<int>::iterator lu = lu_set.begin(); lu != lu_set.end(); ++lu)
		{
			rule.landuse = *lu;
			gNaturalTerrainRules.push_back(rule);
		}
		return true;
	}
	else if(tokens[0] == "CLIFF_INFO")
	{
		CliffInfo_t info;
		string	id;
		if(TokenizeLine(tokens," sPPffffss",
			&id,
			&info.hill_res,
			&info.cliff_res,
			&info.hill_angle1,
			&info.hill_angle2,
			&info.cliff_angle1,
			&info.cliff_angle2,
			&info.hill_tex,
			&info.cliff_tex
			) != 10)
		return false;
		
		if(sCliffs.count(id))
		{
			fprintf(stderr,"ERROR: cliff %s defined twice.\n", id.c_str());
			return false;
		}
		sCliffs[id] = info;		
		return true;
	}

	else if(tokens[0] == "REGIONALIZATION")
	{
		// REGIONALIZATION prefix string
		Regionalization_t r;
		if(TokenizeLine(tokens," ss",&r.variant_prefix,&r.region_png) != 3)
			return false;
		
		for(RegionalizationVector::iterator rr = gRegionalizations.begin(); rr != gRegionalizations.end(); ++rr)
		if(rr->variant_prefix == r.variant_prefix)
		{
			fprintf(stderr,"WARNING: variant prefix %s included twice.\n",r.variant_prefix.c_str());
			return false;
		}
		
		gRegionalizations.push_back(r);
		return true;		
	}

	else if (tokens[0] == "TERRAIN_INFO")
	{
		//	TERRAIN_INFO			NAME	LAYER	XON		RGB		BASE TEX	BORDER_TEX	RES	LIT		COMPO		MODE		MODE PARAMS
		int		has_lit;
		int		ter_name;
		int		has_compo;
		string	shader_mode;

		NaturalTerrainInfo_t	info;
//		info.is_city = s_is_city;
//		info.is_forest = s_is_forest;
		if(TokenizeLine(tokens," eifcessPisfss",
			&ter_name,	
			&info.layer,
			&info.xon_dist,
			&info.map_rgb,
			&info.autogen_mode,
			&info.base_tex,
			&info.border_tex,
			&info.base_res,
			&has_lit,
			&info.decal,
			&info.normal_scale,
			&info.normal,
			&shader_mode) != 14)
		return false;
		
		if(info.autogen_mode != BARE &&
			info.autogen_mode != FOREST &&
			info.autogen_mode != URBAN)
		{
			fprintf(stderr,"ERROR: terrain info %s has bad autogen mode %s.\n", FetchTokenString(ter_name), FetchTokenString(info.autogen_mode));
			return false;
			
		}

		info.regionalization = OLD_SERGIO_RULES ? 0 : -1;
		string ter_string(FetchTokenString(ter_name));
		for(int r = 0; r < gRegionalizations.size(); ++r)
		{
			if(ter_string.size() >= gRegionalizations[r].variant_prefix.size() &&
				strncmp(ter_string.c_str(),gRegionalizations[r].variant_prefix.c_str(),gRegionalizations[r].variant_prefix.size()) == 0)
			{
				info.regionalization = r;
			}
		}
		if(info.regionalization == -1)
		{
			fprintf(stderr,"ERROR: terrain info %s has unknowon region prefix\n", ter_string.c_str());
			return false;
		}
		if(info.decal == "-") info.decal.clear();
		if(info.normal == "-") info.normal.clear();
		
		if(!info.normal.empty() && info.normal_scale <= 0.0)
		{
			fprintf(stderr,"ERROR: normal %s has illegal scale %f\n", info.normal.c_str(),info.normal_scale);
			return false;
		}

		if(has_lit)
			info.lit_tex = MakeLit(info.base_tex);
		
				if(shader_mode == "NORMAL")		info.shader = shader_normal;
		else	if(shader_mode == "VARY")		info.shader = shader_vary;
		else	if(shader_mode == "SLOPE")		info.shader = shader_slope;
		else	if(shader_mode == "SLOPE2")		info.shader = shader_slope2;
		else	if(shader_mode == "HEADING")	info.shader = shader_heading;
		else	if(shader_mode == "TILE")		info.shader = shader_tile;
		else	if(shader_mode == "COMPOSITE")	info.shader = shader_composite;
		else	{ fprintf(stderr,"Illegal shader: %s.\n",shader_mode.c_str()); return false; }
		
		string id;
		switch(info.shader) {
		case shader_slope:
		case shader_slope2:
			if(TokenizeLine(tokens,"              s", &id) != 15) return false;
			if(sCliffs.count(id) == 0) { fprintf(stderr,"Unknown cliff type: %s\n", id.c_str()); return false; }
			
			info.cliff_info = sCliffs[id];
			break;
		case shader_tile:
			if(TokenizeLine(tokens,"              iii",
				&info.tiles_x,
				&info.tiles_y,
				&has_compo) != 17) return false;
			if(has_compo)info.compo_tex = MakeCompo(info.base_tex);	
			break;
		case shader_vary:
			info.compo_tex = MakeCompo(info.base_tex);	
			break;			
		case shader_composite:
			if(TokenizeLine(tokens,"              sPsffffff",
				&info.compo_tex,
				&info.comp_res,
				&info.noise_tex,
				&info.composite_params[0],
				&info.composite_params[1],
				&info.composite_params[2],
				&info.composite_params[3],
				&info.composite_params[4],
				&info.composite_params[5]) != 23) return false;
			break;
		}
		
		info.proj_angle = proj_Down;
		info.custom_ter = 0;

		if(gNaturalTerrainInfo.count(ter_name) != 0) { fprintf(stderr, "Duplicate terrain %s\n",FetchTokenString(ter_name)); return false; }
		
		gNaturalTerrainInfo.insert(NaturalTerrainInfoMap::value_type(ter_name, info));

		return true;
	}
	else
		return false;
}

#if OLD_SERGIO_RULES
static inline bool case_insensitive_compare(char a, char b) { return toupper(a) == toupper(b); }
bool	ReadNaturalTerrainInfo(const vector<string>& tokens, void * ref)
{
	NaturalTerrainInfo_t	info;
	NaturalTerrainRule_t	rule;
	
//	info.is_city = s_is_city;
//	info.is_forest = s_is_forest;
	int						forest_type;	// no longer used
	string					ter_name, tex_name, proj;

	int						has_lit = 0;;
	int						auto_vary;									// 0 = none. 1 = 4 variations, all fake. 2 = 4 variations, 2 & 2. 3 = 4 variations for HEADING.

	rule.zoning = NO_VALUE;
	rule.clim_style = NO_VALUE;
	rule.agri_style = NO_VALUE;
	rule.soil_style = NO_VALUE;
	rule.urban_density_min = rule.urban_density_max = 0.0;
	rule.urban_radial_min = rule.urban_radial_max = 0.0;
	rule.urban_trans_min = rule.urban_trans_max = 0.0;
	rule.urban_square = 0;
	info.map_rgb.rgb[0] = info.map_rgb.rgb[1] = info.map_rgb.rgb[2] = 0.5;
	rule.temp_rng_min = rule.temp_rng_max = 0.0;
	info.custom_ter = tex_not_custom;
	info.regionalization = 0;

	if (tokens[0] == "STERRAIN")
	{
		if (TokenizeLine(tokens, " eeffffffffiffffffffffisifsPssefff",
			&rule.terrain,
			&rule.landuse,
			&rule.elev_min,
			&rule.elev_max,
			&rule.slope_min,
			&rule.slope_max,

			&rule.temp_min,
			&rule.temp_max,
			&rule.rain_min,
			&rule.rain_max,
			&rule.near_water,

			&rule.slope_heading_min,
			&rule.slope_heading_max,
			&rule.rel_elev_min,
			&rule.rel_elev_max,
			&rule.elev_range_min,
			&rule.elev_range_max,
			&rule.temp_rng_min,
			&rule.temp_rng_max,

			&rule.lat_min,
			&rule.lat_max,
			&auto_vary,

			&ter_name,
			&info.layer,
			&info.xon_dist,
			&tex_name,
			&info.base_res,
			&proj,
			&info.border_tex,
			&forest_type,
			&info.map_rgb.rgb[0],
			&info.map_rgb.rgb[1],
			&info.map_rgb.rgb[2]
			) < 31) return false;
//		info.xon_hack = true;
	} else 	if (tokens[0] == "MTERRAIN") {


		if (TokenizeLine(tokens, " eeeffffffffiffffffffffffiffisifsiPssefff",
			&rule.terrain,
			&rule.landuse,
			&rule.clim_style,
			&rule.elev_min,
			&rule.elev_max,
			&rule.slope_min,
			&rule.slope_max,

			&rule.temp_min,
			&rule.temp_max,
//			&rule.temp_rng_min,
//			&rule.temp_rng_max,
			&rule.rain_min,
			&rule.rain_max,
			&rule.near_water,
			&rule.slope_heading_min,
			&rule.slope_heading_max,
			&rule.rel_elev_min,
			&rule.rel_elev_max,
			&rule.elev_range_min,
			&rule.elev_range_max,

			&rule.urban_density_min,
			&rule.urban_density_max,
			&rule.urban_radial_min,
			&rule.urban_radial_max,
			&rule.urban_trans_min,
			&rule.urban_trans_max,
			&rule.urban_square,

			&rule.lat_min,
			&rule.lat_max,
			&auto_vary,

			&ter_name,
			&info.layer,
			&info.xon_dist,
			&tex_name,
			&has_lit,
	//		&info.comp_tex,
			&info.base_res,
	//		&info.comp_res,
	//		&info.base_alpha_invert,
	//		&info.comp_alpha_invert,
			&proj,
			&info.border_tex,
			&forest_type,
	//		&info.forest_ratio,
			&info.map_rgb.rgb[0],
			&info.map_rgb.rgb[1],
			&info.map_rgb.rgb[2]
			) != 41) return false;
//		info.xon_hack = false;

	} else {

		if (TokenizeLine(tokens, " eeeffffffffffiffffffffffffiffisifsPssefff",
			&rule.terrain,
			&rule.landuse,
			&rule.clim_style,
			&rule.elev_min,
			&rule.elev_max,
			&rule.slope_min,
			&rule.slope_max,

			&rule.temp_min,
			&rule.temp_max,
			&rule.temp_rng_min,
			&rule.temp_rng_max,
			&rule.rain_min,
			&rule.rain_max,
			&rule.near_water,
			&rule.slope_heading_min,
			&rule.slope_heading_max,
			&rule.rel_elev_min,
			&rule.rel_elev_max,
			&rule.elev_range_min,
			&rule.elev_range_max,

			&rule.urban_density_min,
			&rule.urban_density_max,
			&rule.urban_radial_min,
			&rule.urban_radial_max,
			&rule.urban_trans_min,
			&rule.urban_trans_max,
			&rule.urban_square,

			&rule.lat_min,
			&rule.lat_max,
			&auto_vary,

			&ter_name,
			&info.layer,
			&info.xon_dist,
			&tex_name,
	//		&info.comp_tex,
			&info.base_res,
	//		&info.comp_res,
	//		&info.base_alpha_invert,
	//		&info.comp_alpha_invert,
			&proj,
			&info.border_tex,
			&forest_type,
	//		&info.forest_ratio,
			&info.map_rgb.rgb[0],
			&info.map_rgb.rgb[1],
			&info.map_rgb.rgb[2]
			) != 42) return false;
//		info.xon_hack = false;

	}
	if (rule.elev_min > rule.elev_max)	return false;
	if (rule.slope_min > rule.slope_max)	return false;
	if (rule.temp_min > rule.temp_max)	return false;
	if (rule.temp_rng_min > rule.temp_rng_max)	return false;
	if (rule.rain_min > rule.rain_max)	return false;
	if (rule.slope_heading_min > rule.slope_heading_max)	return false;
	if (rule.rel_elev_min > rule.rel_elev_max)	return false;
	if (rule.elev_range_min > rule.elev_range_max)	return false;
	if (rule.urban_density_min > rule.urban_density_max)	return false;
	if (rule.urban_radial_min > rule.urban_radial_max)	return false;
	if (rule.urban_trans_min > rule.urban_trans_max)	return false;
	if (rule.urban_square < 0 || rule.urban_square > 2)	return false;
	if (rule.lat_min > rule.lat_max)					return false;
	if (auto_vary != 0 && auto_vary != 1 && auto_vary != 2) 	return false;

	info.map_rgb.rgb[0] /= 255.0;
	info.map_rgb.rgb[1] /= 255.0;
	info.map_rgb.rgb[2] /= 255.0;

//	if(info.forest_type != NO_VALUE)
//		sForests.insert(info.forest_type);

//	int orig_forest = info.forest_type;
//	if (info.forest_type != NO_VALUE)	info.forest_type = LookupTokenCreate(ter_name.c_str());
//	if (info.forest_type != NO_VALUE)
//	{
//		if(sForests.count(info.forest_type) > 0)	if(sForests[info.forest_type] != orig_forest)
//			printf("WARNING: terrain %s has conflicting forest types %s and %s.\n", 
//								FetchTokenString(info.forest_type),
//								FetchTokenString(orig_forest),
//								FetchTokenString(sForests[info.forest_type]));
//		sForests[info.forest_type] = orig_forest;
//	}

	const string ortho_substring = "orthophoto";
	const bool is_pseudo_ortho = ter_name.end() != std::search(ter_name.begin(), ter_name.end(),
																ortho_substring.begin(), ortho_substring.end(),
																case_insensitive_compare);
	if(is_pseudo_ortho)
	{
		DebugAssertWithExplanation(info.layer > 999,
				"You've hit Tyler's shitty code for doing Mobile \"autogen terrain\", "
				"but didn't specify a ridiculous layer... perhaps you didn't intend for this?");
		info.custom_ter = tex_custom_pseudo_ortho;
	}
						info.proj_angle = proj_Down;
	if (proj == "NS")	info.proj_angle = proj_NorthSouth;
	if (proj == "EW")	info.proj_angle = proj_EastWest;
	if (proj == "HDG")	auto_vary = 3;

	string::size_type nstart = tex_name.find_last_of("\\/:");
	if (nstart == tex_name.npos)	nstart = 0; else nstart++;
	if (tex_name.size()-nstart > 31)
		printf("WARNING: base tex %s too long.\n", tex_name.c_str());

	if (rule.slope_min == rule.slope_max &&	rule.slope_min != 0.0)
		printf("WARNING: base tex %s has slope min and max both of %f\n", ter_name.c_str(), rule.slope_min);

	if (info.proj_angle != proj_Down)
	if (rule.slope_min < 30.0)
		printf("WARNING: base tex %s is projected but min slope is %f\n", ter_name.c_str(), rule.slope_min);

	if (info.base_res.x() == 0.0 || info.base_res.y() == 0.0)
		printf("WARNING: bad base res on texture %s\n", ter_name.c_str());

	if (info.proj_angle == proj_NorthSouth)
	if (!(rule.slope_heading_min == 0.0 && rule.slope_heading_max == 0.0 ||
		rule.slope_heading_min == 0.0 && rule.slope_heading_max == 45.0 ||
		rule.slope_heading_min == 135.0 && rule.slope_heading_max == 180.0))
		printf("WARNING: base tex %s is projected north-south but has bad headings.\n",ter_name.c_str());

	if (info.proj_angle == proj_EastWest)
	if (!(rule.slope_heading_min == 0.0 && rule.slope_heading_max == 0.0 ||
		rule.slope_heading_min == 45.0 && rule.slope_heading_max == 135.0))
		printf("WARNING: base tex %s is projected east-west but has bad headings.\n",ter_name.c_str());

	// We use 1-cos notation, which keeps our order constant.
	rule.slope_min = 1.0 - cosdeg(rule.slope_min);
	rule.slope_max = 1.0 - cosdeg(rule.slope_max);

	// Slope heading - uses 1 for north and -1 for south internally.
	// So that's just cosign, but that does mean that Sergio's order (0-180)
	// is going to get reversed.
	rule.slope_heading_min = cosdeg(rule.slope_heading_min);
	rule.slope_heading_max = cosdeg(rule.slope_heading_max);
	swap(rule.slope_heading_min, rule.slope_heading_max);

	// Ben says: this creates the basic terrain type for any non-scree case.  This case will handle
	// both auto-vary and NOT auto-vary.  Note that auto-vary in our code is ALWAYS truly "auto-vary".
	// In other words, as of now (v9.0 + MT2.0) we always use on-shader to vary terrain, _NOT_ 4 separate
	// layers.  As of MT2/V9, we still use 4 layers to do HEADING-based terrain.
	if (auto_vary < 3)
	{
		rule.variant = 0;			// Auto case - we don't use a variant-selector!
//		rule.related = -1;			// Auto case - we don't need related.

		string rep_name = ter_name;
		if(auto_vary > 0) rep_name += "_av";

		LowerCheckName(rep_name);
		rule.name = LookupTokenCreate(rep_name.c_str());
		info.base_tex = tex_name;
		info.autogen_mode = BARE;
		if(auto_vary == 2)								// Auto-vary with two textures - convention is "2" on end of second texture.
		{
			info.compo_tex = MakeCompo(info.base_tex);
		}
		if(auto_vary > 0)
			info.shader = shader_vary;							// Auto-vary FLAG set when we use auto-vary.
		else
			info.shader = shader_normal;

		if (has_lit)			info.lit_tex = MakeLit(info.base_tex);
		else					info.lit_tex.clear();

		#if R_VARY
		MakeRVariant(info.base_tex);
		if(!info.vary_tex.empty())
		MakeRVariant(info.vary_tex);
		#endif

		AddRuleInfoPair(rule, info);
	}

	info.compo_tex.clear();

	// BEN SAYS: we used to generate 4 rules for the top-down variant case (vary code = 1 or 2 in the spreadsheet).  This was NOT used by the sim since
	// auto-vary is used.  But we made the rules anyway so that a .ter file list for v9 would contain the v8 compatibility .ters.

	// Well, we are KILLING that code.  Now we only make the case 3 (scree) variants.  This is because the code must still generate v9 DSFs for MeshTool
	// but does NOT need to make an art asset list, which will be copied from v9 shipping when we package up next-gen art assets.

	// The variant is: 0 = none, 1 = vary by spatial blobs, 2 = vary by spatial blobs (2tex) 3 = vary by slope heading
	// The resulting codes in the struct are: 0 - no vary, 1-4 = spatial variants (all equal), 5-8 = heading variatns (N,E,S,W)
	// For now we only generate 5-8.
	if (auto_vary == 3)			// For the scree case
	{
		// -1 for related field means no relation.  Otherwise it is the index of the FIRST of four variants.
		// So...
//		info.related = -1;
		info.shader = shader_normal;
		for (int rep = 1; rep <= 4; ++rep)
		{
			rule.variant = rep + 4;
			info.map_rgb.rgb[2] += ((float) rep / 80.0);

			string rep_name = ter_name;
			rep_name += ('0' + rep);
			LowerCheckName(rep_name);
			rule.name = LookupTokenCreate(rep_name.c_str());

			string tex_vari = tex_name;
			tex_vari.erase(tex_vari.size()-4);
			tex_vari += ".dds";
			info.base_tex = tex_vari;

			if (has_lit)
				info.lit_tex = MakeLit(info.base_tex);
			else
				info.lit_tex.clear();

			#if R_VARY
			MakeRVariant(info.base_tex);
			#endif
			
			AddRuleInfoPair(rule, info);

		}
	}

	return true;
}
#endif

static bool HandleTranslate(const vector<string>& inTokenLine, void * inRef)
{
	int e1, e2;
	if (TokenizeLine(inTokenLine, " ee", &e1, &e2) == 3)
		gLandUseTransTable[e1] = e2;
	else
		return false;
	return true;
}


/*
bool	ReadPromoteTerrainInfo(const vector<string>& tokens, void * ref)
{
	pair<int, int> key;
	int			   value;
	if (TokenizeLine(tokens, " eee", &key.first, &key.second, &value) != 4)
		return false;

	if (gTerrainPromoteTable.count(key) != 0)
		printf("Warning: duplicate land use promotion key %s,%s\n",
			FetchTokenString(key.first),FetchTokenString(key.second));
	gTerrainPromoteTable[key] = value;
	return true;
}
*/

/*
bool	ReadManTerrainInfo(const vector<string>& tokens, void * ref)
{
	ManTerrainInfo_t	info;
	int					key;
	if (TokenizeLine(tokens, " esfsfss",
		&key, &info.terrain_name,
		&info.base_res, &info.base_tex,
		&info.comp_res, &info.comp_tex,
		&info.lit_tex) != 8) return false;
	if (gManTerrainTable.count(key) != 0)
		printf("Warning: duplicate man-made terrain info key %s\n",
			FetchTokenString(key));
	if (info.terrain_name == "-")	info.terrain_name.clear();
	if (!LowerCheckName(info.terrain_name)) return false;
	if (info.base_tex == "-")	info.base_tex.clear();
	if (info.comp_tex == "-")	info.comp_tex.clear();
	if (info.lit_tex  == "-")	info.lit_tex.clear();

	gManTerrainTable[key] = info;
	return true;
}*/


void	LoadDEMTables(void)
{
	gEnumColors.clear();
	gColorBands.clear();
	gEnumDEMs.clear();
	gNaturalTerrainRules.clear();
	gNaturalTerrainInfo.clear();
	gRegionalizations.clear();
	gBeachInfoTable.clear();
	gBeachIndex.clear();
//	sForests.clear();
	gLandUseTransTable.clear();

	sCliffs.clear();
//	s_is_city = 0;
//	s_is_forest = 0;

	RegisterLineHandler("ENUM_COLOR", ReadEnumColor, NULL);
	RegisterLineHandler("COLOR_BAND", ReadEnumBand, NULL);
	RegisterLineHandler("COLOR_ENUM_DEM", ReadEnumDEM, NULL);
	RegisterLineHandler("BEACH", ReadBeachInfo, NULL);
#if OLD_SERGIO_RULES	
	RegisterLineHandler("NTERRAIN", ReadNaturalTerrainInfo, NULL);
	RegisterLineHandler("STERRAIN", ReadNaturalTerrainInfo, NULL);
	RegisterLineHandler("MTERRAIN", ReadNaturalTerrainInfo, NULL);
#endif	
	RegisterLineHandler("TERRAIN_RULE", ReadNewTerrainInfo, NULL);
	RegisterLineHandler("REGIONALIZATION", ReadNewTerrainInfo, NULL);	
	RegisterLineHandler("TERRAIN_INFO", ReadNewTerrainInfo, NULL);
	RegisterLineHandler("CLIFF_INFO", ReadNewTerrainInfo, NULL);
//	RegisterLineHandler("PROMOTE_TERRAIN", ReadPromoteTerrainInfo, NULL);
	RegisterLineHandler("LU_TRANSLATE", HandleTranslate, NULL);
	RegisterLineHandler("TERRAIN_IS_CITY",HandleFlags,NULL);
	RegisterLineHandler("TERRAIN_FOREST", HandleFlags, NULL);

	if (gNaturalTerrainFile.empty())	LoadConfigFile("master_terrain.txt");
	else								LoadConfigFileFullPath(gNaturalTerrainFile.c_str());
	if (gLanduseTransFile.empty())		LoadConfigFile("landuse_translate.txt");
	else								LoadConfigFileFullPath(gLanduseTransFile.c_str());

	LoadConfigFile("enum_colors.txt");
	LoadConfigFile("beach_terrain.txt");

	sAirports.clear();
	for(int n = 0; n < gNaturalTerrainRules.size(); ++n)
	if(gNaturalTerrainRules[n].terrain == terrain_Airport)
		sAirports.insert(gNaturalTerrainRules[n].name);

	/*
	printf("---forests---\n");
	for (set<int>::iterator f = sForests.begin(); f != sForests.end(); ++f)
	{
		printf("%s\n", FetchTokenString(*f));
	}*/

	DebugAssertWithExplanation(!gNaturalTerrainRules.empty(), "No terrain rules defined by your config files");
}

#pragma mark -

int	FindNaturalTerrain(
				int		terrain,
				int		zoning,
				int 	landuse,
				int		soil_style,
				int		agri_style,
				int		clim_style,
//				int 	climate,
//				float 	elevation,
				float 	slope,
				float 	slope_tri,
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
				int		urban_square,
				float	lat)
//				int		variant_blob,
//				int		variant_head)
{
	// Check for no data in the continuous floating point inputs!
//	DebugAssert(DEM_NO_DATA !=  	elevation);
	DebugAssert(DEM_NO_DATA !=  	slope);
	DebugAssert(DEM_NO_DATA !=  	slope_tri);
	DebugAssert(DEM_NO_DATA != 	temp);
	DebugAssert(DEM_NO_DATA != 	temp_rng);
	DebugAssert(DEM_NO_DATA != 	rain);
	DebugAssert(DEM_NO_DATA != 	slopeheading);
	DebugAssert(DEM_NO_DATA != 	relelevation);
	DebugAssert(DEM_NO_DATA != 	elevrange);
	DebugAssert(DEM_NO_DATA != 	urban_density);
	DebugAssert(DEM_NO_DATA != 	urban_radial);
	DebugAssert(DEM_NO_DATA != 	urban_trans);
	DebugAssert(DEM_NO_DATA != 	lat);
	DebugAssertWithExplanation(!gNaturalTerrainRules.empty(), "No terrain rules defined... you won't get far trying to FindNaturalTerrain() with no terrain rules to match!");

	// OPTIMIZE - figure out what the major keys should be.

	for (int rec_num = 0; rec_num < gNaturalTerrainRules.size(); ++rec_num)
	{
		const NaturalTerrainRule_t& rec = gNaturalTerrainRules[rec_num];

//		float slope_to_use = rec.proj_angle == proj_Down ? slope : slope_tri;
		float slope_to_use = slope_tri;
		
		#define MATCH_RANGE(x,vmin,vmax)	if(rec.vmin == rec.vmax || (rec.vmin <= x && x <= rec.vmax))
		#define MATCH_ENUM(x,field) if(rec.field == NO_VALUE || x == rec.field)
		
		MATCH_RANGE(temp,temp_min,temp_max)
		MATCH_RANGE(slope_to_use,slope_min,slope_max)
		MATCH_RANGE(rain,rain_min,rain_max)
		MATCH_RANGE(temp_rng,temp_rng_min,temp_rng_max)
		MATCH_RANGE(slopeheading,slope_heading_min,slope_heading_max)
//		if (rec.variant == 0 || rec.variant == variant_blob || rec.variant == variant_head)
		MATCH_ENUM(landuse,landuse)
		MATCH_ENUM(soil_style,soil_style)
		MATCH_ENUM(agri_style,agri_style)
		MATCH_ENUM(clim_style,clim_style)
		MATCH_ENUM(terrain,terrain)
		MATCH_ENUM(zoning,zoning)
		MATCH_RANGE(relelevation,rel_elev_min,rel_elev_max)
		MATCH_RANGE(elevrange,elev_range_min,elev_range_max)
		MATCH_RANGE(urban_density,urban_density_min,urban_density_max)
		MATCH_RANGE(urban_trans,urban_trans_min,urban_trans_max)
		if (rec.urban_square == 0 || urban_square == DEM_NO_DATA || rec.urban_square == urban_square)
		MATCH_RANGE(lat,lat_min,lat_max)
		if (!rec.near_water || water)
		MATCH_RANGE(urban_radial,urban_radial_min,urban_radial_max)		
		{
			return rec.name;
		}
	}

	return -1;
}

#pragma mark -

struct float_between_iterator {
	typedef set<float>					set_type;
	typedef set_type::const_iterator	iter_type;
	iter_type		i_;
	bool			h_;
	const set_type&	s_;

	float_between_iterator(const set_type& s) : s_(s), i_(s.begin()), h_(false)
	{
	}

	bool operator()(void) const {
		return i_ != s_.end();
	}

	float_between_iterator& operator++(void) {
		if (h_) {
			h_ = false;
			++i_; }
		else {
			h_ = true;
			iter_type j = i_;
			++j;
			if (j == s_.end())
				++i_;
		}
		return *this;
	}

	float operator * (void) const {
		if (h_)
		{
			iter_type j = i_;
			++j;
			if (j == s_.end()) return *i_;
			return (*i_ + *j) * 0.5;
		} else
			return *i_;
		}

};

#if 0

void	CheckDEMRuleCoverage(ProgressFunc func)
{
/*	CoverageFinder	cov(10);
	cov.NameAxis(0, "Terrain");
	cov.NameAxis(1, "Landuse");
	cov.NameAxis(2, "Climate");
	cov.NameAxis(3, "Elevation");
	cov.NameAxis(4, "Slope");
	cov.NameAxis(5, "Temp");
	cov.NameAxis(6, "NearWater");
	cov.NameAxis(7, "SlopeHeading");
	cov.NameAxis(8, "RelElev");
	cov.NameAxis(9, "ElevRange");
	cov.AddAxisEnum(0, terrain_Natural, "terrain_Natural");
	cov.AddAxisEnum(0, terrain_Airport, "terrain_Airport");
	cov.AddAxisEnum(1, lu_usgs_URBAN_SQUARE, "lu_usgs_URBAN_SQUARE");
	cov.AddAxisEnum(1, lu_usgs_URBAN_IRREGULAR, "lu_usgs_URBAN_IRREGULAR");
	cov.AddAxisEnum(2, climate_TropicalRainForest, "climate_TropicalRainForest");
	cov.AddAxisEnum(2, climate_TropicalMonsoon, "climate_TropicalMonsoon");
	cov.AddAxisEnum(2, climate_TropicalDry, "climate_TropicalDry");
	cov.AddAxisEnum(2, climate_DrySteppe, "climate_DrySteppe");
	cov.AddAxisEnum(2, climate_DryDesert, "climate_DryDesert");
	cov.AddAxisEnum(2, climate_TemperateAny, "climate_TemperateAny");
	cov.AddAxisEnum(2, climate_TemperateSummerDry, "climate_TemperateSummerDry");
	cov.AddAxisEnum(2, climate_TemperateWinterDry, "climate_TemperateWinterDry");
	cov.AddAxisEnum(2, climate_TemperateWet, "climate_TemperateWet");
	cov.AddAxisEnum(2, climate_ColdAny, "climate_ColdAny");
	cov.AddAxisEnum(2, climate_ColdSummerDry, "climate_ColdSummerDry");
	cov.AddAxisEnum(2, climate_ColdWinterDry, "climate_ColdWinterDry");
	cov.AddAxisEnum(2, climate_PolarTundra, "climate_PolarTundra");
	cov.AddAxisEnum(2, climate_PolarFrozen, "climate_PolarFrozen");
	cov.AddAxisRange(3, 0.0, 1000.0);
	cov.AddAxisRange(4, 0.0, 1.0);
	cov.AddAxisRange(5, 0.0, 10.0);
	cov.AddAxisEnum(6, 0, "No");
	cov.AddAxisEnum(6, 1, "Yes");
	cov.AddAxisRange(7, -1.0, 1.0);
	cov.AddAxisRange(8, 0, 1.0);
	cov.AddAxisRange(9, 0, 300);

	int i;
	for (i = 0; i < gNaturalTerrainTable.size(); ++i)
	{
		NaturalTerrainInfo_t& r = gNaturalTerrainTable[i];
		if (r.terrain != NO_VALUE)							cov.AddAxisEnum(0, r.terrain, FetchTokenString(r.terrain));
		if (r.landuse != NO_VALUE)							cov.AddAxisEnum(1, r.landuse, FetchTokenString(r.landuse));
		if (r.climate != NO_VALUE)							cov.AddAxisEnum(2, r.landuse, FetchTokenString(r.climate));
		if (r.elev_min != r.elev_max)						cov.AddAxisRange(3, r.elev_min, r.elev_max);
		if (r.slope_min != r.slope_max)						cov.AddAxisRange(4, r.slope_min, r.slope_max);
		if (r.temp_min != r.temp_max)						cov.AddAxisRange(5, r.temp_min, r.temp_max);
		if (r.slope_heading_min != r.slope_heading_max)		cov.AddAxisRange(7, r.slope_heading_min, r.slope_heading_max);
		if (r.rel_elev_min != r.rel_elev_max)				cov.AddAxisRange(8, r.rel_elev_min, r.rel_elev_max);
		if (r.elev_range_min != r.elev_range_max)			cov.AddAxisRange(9, r.elev_range_min, r.elev_range_max);
	}

	cov.FinishAxes();

	for (i = 0; i < gNaturalTerrainTable.size(); ++i)
	{
		bool	has = false;
		cov.StartRule();
		NaturalTerrainInfo_t& r = gNaturalTerrainTable[i];
		if (r.terrain != NO_VALUE)							{ cov.AddRuleEnum (0, r.terrain);											has = true; } else cov.AddRuleAny(0);
		if (r.landuse != NO_VALUE)							{ cov.AddRuleEnum (1, r.landuse);											has = true; } else cov.AddRuleAny(1);
		if (r.climate != NO_VALUE)							{ cov.AddRuleEnum (2, r.landuse);											has = true; } else cov.AddRuleAny(2);
		if (r.elev_min != r.elev_max)						{ cov.AddRuleRange(3, r.elev_min, r.elev_max);								has = true; } else cov.AddRuleAny(3);
		if (r.slope_min != r.slope_max)						{ cov.AddRuleRange(4, r.slope_min, r.slope_max);							has = true; } else cov.AddRuleAny(4);
		if (r.temp_min != r.temp_max)						{ cov.AddRuleRange(5, r.temp_min, r.temp_max);								has = true; } else cov.AddRuleAny(5);
		if (r.near_water != 0)								{ cov.AddRuleEnum (6, 1); 													has = true; } else cov.AddRuleAny(6);
		if (r.slope_heading_min != r.slope_heading_max)		{ cov.AddRuleRange(7, r.slope_heading_min, r.slope_heading_max);			has = true; } else cov.AddRuleAny(7);
		if (r.rel_elev_min != r.rel_elev_max)				{ cov.AddRuleRange(8, r.rel_elev_min, r.rel_elev_max);						has = true; } else cov.AddRuleAny(8);
		if (r.elev_range_min != r.elev_range_max)			{ cov.AddRuleRange(9, r.elev_range_min, r.elev_range_max);					has = true; } else cov.AddRuleAny(9);
		cov.EndRule(has);
	}

	cov.OutputGaps();
*/

	set<int>		terrain, landuse, urban_square, near_water;
	set<float>		/*elev, */slope, temp, temp_rng, rain, slope_head, rel_elev, elev_range, urban_density, urban_radial, urban_trans, lat;

	terrain.insert(NO_VALUE);
	landuse.insert(NO_VALUE);

	for (int n = 0; n < gNaturalTerrainRules.size(); ++n)
	{
		NaturalTerrainRule_t& rec(gNaturalTerrainRules[n]);
		terrain.insert(rec.terrain);
		landuse.insert(rec.landuse);
//		elev.insert(rec.elev_min);
//		elev.insert(rec.elev_max);
		slope.insert(rec.slope_min);
		slope.insert(rec.slope_max);
		temp.insert(rec.temp_min);
		temp.insert(rec.temp_max);
		temp_rng.insert(rec.temp_rng_min);
		temp_rng.insert(rec.temp_rng_max);
		rain.insert(rec.rain_min);
		rain.insert(rec.rain_max);
		near_water.insert(rec.near_water);
		slope_head.insert(rec.slope_heading_min);
		slope_head.insert(rec.slope_heading_max);
		rel_elev.insert(rec.rel_elev_min);
		rel_elev.insert(rec.rel_elev_max);
		elev_range.insert(rec.elev_range_min);
		elev_range.insert(rec.elev_range_max);
		urban_density.insert(rec.urban_density_min);
		urban_density.insert(rec.urban_density_max);
		urban_radial.insert(rec.urban_radial_min);
		urban_radial.insert(rec.urban_radial_max);
		urban_trans.insert(rec.urban_trans_min);
		urban_trans.insert(rec.urban_trans_max);
		urban_square.insert(rec.urban_square);
		lat.insert(rec.lat_min);
		lat.insert(rec.lat_max);
	}

	int any_rule = gNaturalTerrainRules.back().name;

	printf("Landuse: %llu states.\n",(unsigned long long)landuse.size());
	printf("Terrain: %llu states.\n",(unsigned long long)terrain.size());
//	printf("Elev: %llu states.\n",(unsigned long long)elev.size() );
	printf("Slope: %llu states.\n",(unsigned long long)slope.size() );
	printf("Temp: %llu states.\n",(unsigned long long)temp.size() );
	printf("Temp Range: %llu states.\n",(unsigned long long)temp_rng.size() );
	printf("Rain: %llu states.\n",(unsigned long long)rain.size() );
	printf("Near Water: %llu states.\n",(unsigned long long)near_water.size());
	printf("Slope Headign: %llu states.\n",(unsigned long long)slope_head.size() );
	printf("Elevation Range: %llu states.\n",(unsigned long long)elev_range.size() );
	printf("Urban Density: %llu states.\n",(unsigned long long)urban_density.size() );
	printf("Urban Radial: %llu states.\n",(unsigned long long)urban_radial.size() );
	printf("Urban Trans: %llu states.\n",(unsigned long long)urban_trans.size() );
	printf("Urban Square: %llu states.\n",(unsigned long long)urban_square.size());
	printf("Latitude: %llu states.\n",(unsigned long long)lat.size());

	int total = landuse.size() *
				terrain.size() *
//				(elev.size() * 2 - 1) *
				(slope.size() * 2 - 1) *
				(temp.size() * 2 - 1) *
				(temp_rng.size() * 2 - 1)*
				(rain.size() * 2 - 1)*
				near_water.size() *
				(slope_head.size() * 2 - 1)*
				(elev_range.size() * 2 - 1)*
				(urban_density.size() * 2 - 1)*
				(urban_radial.size() * 2 - 1)*
				(urban_trans.size() * 2 - 1)*
				urban_square.size() *
				(lat.size() * 2 - 1);

	printf("Total pts to check: %d\n", total);
	int step = total / 200;

	PROGRESS_START(func, 0, 1, "Checking tables");

	int ctr = 0;

	for (set<int>::iterator lu = landuse.begin(); lu != landuse.end(); ++lu)
	for (set<int>::iterator ter = terrain.begin(); ter != terrain.end(); ++ter)
//	for (float_between_iterator el(elev); el(); ++el)
	for (float_between_iterator sd(slope); sd(); ++sd)
	for (float_between_iterator st(slope); st(); ++st)
	for (float_between_iterator t(temp); t(); ++t)
	for (float_between_iterator tr(temp_rng); tr(); ++tr)
	for (float_between_iterator r(rain); r(); ++r)
	for (set<int>::iterator nw = near_water.begin(); nw != near_water.end(); ++nw)
	for (float_between_iterator sh(slope_head); sh(); ++sh)
	for (float_between_iterator re(rel_elev); re(); ++re)
	for (float_between_iterator er(elev_range); er(); ++er)
	for (float_between_iterator ud(urban_density); ud(); ++ud)
	for (float_between_iterator ur(urban_radial); ur(); ++ur)
	for (float_between_iterator ut(urban_trans); ut(); ++ut)
	for (set<int>::iterator us = urban_square.begin(); us != urban_square.end(); ++us)
	for (float_between_iterator l(lat); l(); ++l)
	{
		PROGRESS_CHECK(func, 0, 1, "Checking tables", ctr, total, step);

		int found = FindNaturalTerrain(*ter, *lu, /*NO_VALUE,  *el, */ *sd, *st,
			*t, *tr, *r, *nw, *sh, *re, *er, *ud, *ur, *ut, *us, *l, 1, 5);

		if (found == any_rule)
		printf("Found %s rule on: ter=%s lu=%s el=XX sd=%f st=%f t=%f tr=%f r=%f w=%d sh=%f re=%f er=%f ud=XX ur=XX ut=%f us=%d l=%f\n",
			FetchTokenString(found),
			FetchTokenString(*ter), FetchTokenString(*lu), /* *el, */ *sd, *st,
			*t, *tr, *r, *nw, *sh, *re, *er, /* *ud, *ur, */ *ut, *us, *l);
		if (found == -1)
		printf("Found hole on: ter=%s lu=%s el=XX sd=%f st=%f t=%f tr=%f r=%f w=%d sh=%f re=%f er=%f ud=XX ur=XX ut=%f us=%d l=%f\n",
			FetchTokenString(*ter), FetchTokenString(*lu), /* *el, */ *sd, *st,
			*t, *tr, *r, *nw, *sh, *re, *er, /* *ud, *ur, */ *ut, *us, *l);

		++ctr;
	}
	PROGRESS_DONE(func, 0, 1, "Checking tables");
	printf("Total: %d\n", ctr);
}

#endif

void	GetNaturalTerrainColor(int terrain, float rgb[3])
{
	if (gNaturalTerrainInfo.count(terrain) == 0)
	{
		rgb[0] = rgb[2] = 1.0;
		rgb[1] = 0.0;
		return;
	}
	NaturalTerrainInfo_t& info = gNaturalTerrainInfo[terrain];
	rgb[0] = info.map_rgb.rgb[0];
	rgb[1] = info.map_rgb.rgb[1];
	rgb[2] = info.map_rgb.rgb[2];
}

//bool	IsForestType(int inType)
//{
//	return sForests.count(inType) != 0;
//}

bool	IsAirportTerrain(int t)
{
	return sAirports.count(t) != 0;
}

/*
void			GetForestTypes(set<int>& forests)
{
	forests = sForests;
}
*/

//void	GetForestMapping(map<int,int>& forests)
//{
//	forests = sForests;
//}

void MakeDirectRules(void)
{
	NaturalTerrainRule_t	rule;
	for(NaturalTerrainInfoMap::iterator all_names = gNaturalTerrainInfo.begin(); all_names != gNaturalTerrainInfo.end(); ++all_names)
	{

		
		rule.terrain = all_names->first;
		rule.landuse = NO_VALUE;
		rule.clim_style = rule.soil_style = rule.agri_style = NO_VALUE;
//		rule.climate = NO_VALUE;
		rule.elev_min = rule.elev_max = 0.0f;
		rule.slope_min = rule.slope_max = 0.0;
		rule.temp_min = rule.temp_max = 0.0;
		rule.temp_rng_min = rule.temp_rng_max = 0.0;
		rule.rain_min = rule.rain_max = 0.0;
		rule.near_water = 0;
		rule.slope_heading_min = rule.slope_heading_max = 0.0;
		rule.rel_elev_min = rule.rel_elev_max = 0.0;
		rule.elev_range_min = rule.elev_range_max = 0.0;

		rule.urban_density_min= 0.0;
		rule.urban_density_max= 0.0;
		rule.urban_radial_min= 0.0;
		rule.urban_radial_max= 0.0;
		rule.urban_trans_min= 0.0;
		rule.urban_trans_max= 0.0;
		rule.urban_square = 0;

		rule.lat_min=0.0;
		rule.lat_max=0.0;
		rule.variant=0;		// 0 = use all. 1-4 = flat variants. 5-8 = sloped variants, CW from N=5
//		printf("Adding rule: %s->%s\n", FetchTokenString(rule.terrain), FetchTokenString(rule.name));

		rule.name = all_names->first;
		gNaturalTerrainRules.insert(gNaturalTerrainRules.begin(), rule);
	}	
}

