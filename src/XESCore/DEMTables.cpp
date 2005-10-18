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
#include "CoverageFinder.h"

EnumColorTable				gEnumColors;
ColorBandTable				gColorBands;
set<int>					gEnumDEMs;	



NaturalTerrainTable			gNaturalTerrainTable;
NaturalTerrainLandUseIndex	gNaturalTerrainLandUseIndex;
NaturalTerrainIndex			gNaturalTerrainIndex;
//TerrainPromoteTable			gTerrainPromoteTable;
//ManTerrainTable				gManTerrainTable;
BeachInfoTable				gBeachInfoTable;
LandUseTransTable			gLandUseTransTable;

static	void	ValidateNaturalTerrain(void);
static set<int>		sForests;

string	gNaturalTerrainFile;
string	gLanduseTransFile;
string	gReplacementClimate;
string 	gReplacementRoads;

inline double cosdeg(double deg)
{
	if (deg == 0.0) return 1.0;
	if (deg == 90.0) return 0.0;
	if (deg == 180.0) return -1.0;
	if (deg == -90.0) return 0.0;
	if (deg == -180.0) return -1.0;
	return cos(deg * DEG_TO_RAD);
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
		printf("Parse error for enum colors.\n");
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
	if (TokenizeLine(tokens, " ffefffi", 
		&info.min_slope, &info.max_slope,
		&info.terrain_type, 
		&info.min_sea, &info.max_sea, &info.min_len,		
		&info.x_beach_type) != 8) return false;
		
	info.min_slope=cosdeg(info.min_slope);
	info.max_slope=cosdeg(info.max_slope);
	// NOTE: because we are storing cosigns, a flat beach is 1.0, so we are reversing the 
	// ordering here.
	swap(info.min_slope,info.max_slope);
	gBeachInfoTable.push_back(info);
	return true;
}

bool	ReadNaturalTerrainInfo(const vector<string>& tokens, void * ref)
{
	NaturalTerrainInfo_t	info;
	string					name, proj;
	
	int						auto_vary;
	
	info.climate = NO_VALUE;
	info.urban_density_min = info.urban_density_max = 0.0;
	info.urban_radial_min = info.urban_radial_max = 0.0;
	info.urban_trans_min = info.urban_trans_max = 0.0;
	info.urban_square = 0;
	info.map_rgb[0] = info.map_rgb[1] = info.map_rgb[2] = 0.5;
	info.temp_rng_min = info.temp_rng_max = 0.0;
	
	if (tokens[0] == "STERRAIN")
	{
		if (TokenizeLine(tokens, " eeffffffffiffffffffffisifsfssefff",
			&info.terrain,
			&info.landuse,			
			&info.elev_min,
			&info.elev_max,
			&info.slope_min,
			&info.slope_max,

			&info.temp_min,
			&info.temp_max,
			&info.rain_min,
			&info.rain_max,
			&info.near_water,
			
			&info.slope_heading_min,
			&info.slope_heading_max,
			&info.rel_elev_min,
			&info.rel_elev_max,
			&info.elev_range_min,
			&info.elev_range_max,
			&info.temp_rng_min,
			&info.temp_rng_max,

			&info.lat_min,
			&info.lat_max,
			&auto_vary,

			&name,
			&info.layer,
			&info.xon_dist,
			&info.base_tex,
			&info.base_res,
			&proj,
			&info.border_tex,
			&info.forest_type,
			&info.map_rgb[0],
			&info.map_rgb[1],
			&info.map_rgb[2]
			) < 31) return false;	
	} else 	if (tokens[0] == "MTERRAIN") {

		
		if (TokenizeLine(tokens, " eeeffffffffiffffffffffffiffisifsfssefff",
			&info.terrain,
			&info.landuse,
			&info.climate,
			&info.elev_min,
			&info.elev_max,
			&info.slope_min,
			&info.slope_max,

			&info.temp_min,
			&info.temp_max,
//			&info.temp_rng_min,
//			&info.temp_rng_max,
			&info.rain_min,
			&info.rain_max,
			&info.near_water,
			&info.slope_heading_min,
			&info.slope_heading_max,
			&info.rel_elev_min,
			&info.rel_elev_max,
			&info.elev_range_min,
			&info.elev_range_max,

			&info.urban_density_min,
			&info.urban_density_max,
			&info.urban_radial_min,
			&info.urban_radial_max,
			&info.urban_trans_min,
			&info.urban_trans_max,
			&info.urban_square,

			&info.lat_min,
			&info.lat_max,
			&auto_vary,

			&name,
			&info.layer,
			&info.xon_dist,
			&info.base_tex,
	//		&info.comp_tex,
			&info.base_res,
	//		&info.comp_res,
	//		&info.base_alpha_invert,
	//		&info.comp_alpha_invert,
			&proj,
			&info.border_tex,
			&info.forest_type,
	//		&info.forest_ratio,
			&info.map_rgb[0],
			&info.map_rgb[1],
			&info.map_rgb[2]
			) != 40) return false;
	
	} else {
		
		if (TokenizeLine(tokens, " eeeffffffffffiffffffffffffiffisifsfssefff",
			&info.terrain,
			&info.landuse,
			&info.climate,
			&info.elev_min,
			&info.elev_max,
			&info.slope_min,
			&info.slope_max,

			&info.temp_min,
			&info.temp_max,
			&info.temp_rng_min,
			&info.temp_rng_max,
			&info.rain_min,
			&info.rain_max,
			&info.near_water,
			&info.slope_heading_min,
			&info.slope_heading_max,
			&info.rel_elev_min,
			&info.rel_elev_max,
			&info.elev_range_min,
			&info.elev_range_max,

			&info.urban_density_min,
			&info.urban_density_max,
			&info.urban_radial_min,
			&info.urban_radial_max,
			&info.urban_trans_min,
			&info.urban_trans_max,
			&info.urban_square,

			&info.lat_min,
			&info.lat_max,
			&auto_vary,

			&name,
			&info.layer,
			&info.xon_dist,
			&info.base_tex,
	//		&info.comp_tex,
			&info.base_res,
	//		&info.comp_res,
	//		&info.base_alpha_invert,
	//		&info.comp_alpha_invert,
			&proj,
			&info.border_tex,
			&info.forest_type,
	//		&info.forest_ratio,
			&info.map_rgb[0],
			&info.map_rgb[1],
			&info.map_rgb[2]
			) != 42) return false;
	}	
	if (info.elev_min > info.elev_max)	return false;
	if (info.slope_min > info.slope_max)	return false;
	if (info.temp_min > info.temp_max)	return false;
	if (info.temp_rng_min > info.temp_rng_max)	return false;
	if (info.rain_min > info.rain_max)	return false;
	if (info.slope_heading_min > info.slope_heading_max)	return false;
	if (info.rel_elev_min > info.rel_elev_max)	return false;
	if (info.elev_range_min > info.elev_range_max)	return false;
	if (info.urban_density_min > info.urban_density_max)	return false;
	if (info.urban_radial_min > info.urban_radial_max)	return false;
	if (info.urban_trans_min > info.urban_trans_max)	return false;
	if (info.urban_square < 0 || info.urban_square > 2)	return false;
	if (info.lat_min > info.lat_max)					return false;
	if (auto_vary != 0 && auto_vary != 1) 	return false;
	
	info.map_rgb[0] /= 255.0;
	info.map_rgb[1] /= 255.0;
	info.map_rgb[2] /= 255.0;
	
	if (info.forest_type != NO_VALUE)	sForests.insert(info.forest_type);
		
						info.proj_angle = proj_Down;
	if (proj == "NS")	info.proj_angle = proj_NorthSouth;
	if (proj == "EW")	info.proj_angle = proj_EastWest;
	if (proj == "HDG")	auto_vary = 2;
		
	string::size_type nstart = info.base_tex.find_last_of("\\/:");
	if (nstart == info.base_tex.npos)	nstart = 0; else nstart++;
	if (info.base_tex.size()-nstart > 31)
		printf("WARNING: base tex %s too long.\n", info.base_tex.c_str());

	if (info.slope_min == info.slope_max &&	info.slope_min != 0.0)
		printf("WARNING: base tex %s has slope min and max both of %f\n", name.c_str(), info.slope_min);

	if (info.proj_angle != proj_Down)
	if (info.slope_min < 30.0)
		printf("WARNING: base tex %s is projected but min slope is %f\n", name.c_str(), info.slope_min);

	if (info.proj_angle == proj_NorthSouth)
	if (!(info.slope_heading_min == 0.0 && info.slope_heading_max == 0.0 ||
		info.slope_heading_min == 0.0 && info.slope_heading_max == 45.0 ||
		info.slope_heading_min == 135.0 && info.slope_heading_max == 180.0))
		printf("WARNING: base tex %s is projected north-south but has bad headings.\n",name.c_str());

	if (info.proj_angle == proj_EastWest)
	if (!(info.slope_heading_min == 0.0 && info.slope_heading_max == 0.0 ||
		info.slope_heading_min == 45.0 && info.slope_heading_max == 135.0))
		printf("WARNING: base tex %s is projected east-west but has bad headings.\n",name.c_str());

	// We use 1-cos notation, which keeps our order constant.	
	info.slope_min = 1.0 - cosdeg(info.slope_min);
	info.slope_max = 1.0 - cosdeg(info.slope_max);
	
	// Slope heading - uses 1 for north and -1 for south internally.
	// So that's just cosign, but that does mean that Sergio's order (0-180)
	// is going to get reversed.
	info.slope_heading_min = cosdeg(info.slope_heading_min);
	info.slope_heading_max = cosdeg(info.slope_heading_max);
	swap(info.slope_heading_min, info.slope_heading_max);

	// AUTO-VARIATION - we take one rule and make four rules with variant codes.  Later the rule-finder will generate random codes to select rules spatially.
	// The auto-vary code is: 0 = none, 1 = vary by spatial blobs, 2 = vary by slope heading
	// The resulting codes in the struct are: 0 - no vary, 1-4 = spatial variants (all equal), 5-8 = heading variatns (N,E,S,W)
	if (auto_vary > 0)
	{
		for (int rep = 1; rep <= 4; ++rep)
		{
			info.variant = rep + (auto_vary == 2 ? 4 : 0);
			info.map_rgb[2] += ((float) rep / 80.0);

			string rep_name = name;
			rep_name += ('0' + rep);
			LowerCheckName(rep_name);
			info.name = LookupTokenCreate(rep_name.c_str());	
			
			int rn = gNaturalTerrainTable.size();
			gNaturalTerrainTable.push_back(info);

			gNaturalTerrainLandUseIndex.insert(NaturalTerrainLandUseIndex::value_type(info.landuse, rn));
			if (gNaturalTerrainIndex.count(info.name) == 0)
				gNaturalTerrainIndex[info.name] = rn;
		}

	} else {
		
		info.variant = 0;
		
		LowerCheckName(name);
		info.name = LookupTokenCreate(name.c_str());	
		
		int rn = gNaturalTerrainTable.size();
		gNaturalTerrainTable.push_back(info);

		gNaturalTerrainLandUseIndex.insert(NaturalTerrainLandUseIndex::value_type(info.landuse, rn));
		if (gNaturalTerrainIndex.count(info.name) == 0)
			gNaturalTerrainIndex[info.name] = rn;
	}
	
	return true;	
}

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
	gNaturalTerrainTable.clear();
	gNaturalTerrainLandUseIndex.clear();
	gNaturalTerrainIndex.clear();
//	gTerrainPromoteTable.clear();
	gBeachInfoTable.clear();
	sForests.clear();
	gLandUseTransTable.clear();
	
	RegisterLineHandler("ENUM_COLOR", ReadEnumColor, NULL);
	RegisterLineHandler("COLOR_BAND", ReadEnumBand, NULL);
	RegisterLineHandler("COLOR_ENUM_DEM", ReadEnumDEM, NULL);
	RegisterLineHandler("BEACH", ReadBeachInfo, NULL);
	RegisterLineHandler("NTERRAIN", ReadNaturalTerrainInfo, NULL);
	RegisterLineHandler("STERRAIN", ReadNaturalTerrainInfo, NULL);
	RegisterLineHandler("MTERRAIN", ReadNaturalTerrainInfo, NULL);
//	RegisterLineHandler("PROMOTE_TERRAIN", ReadPromoteTerrainInfo, NULL);
	RegisterLineHandler("LU_TRANSLATE", HandleTranslate, NULL);

	if (gNaturalTerrainFile.empty())	LoadConfigFile("natural_terrain.txt");	
	else								LoadConfigFileFullPath(gNaturalTerrainFile.c_str());
	if (gLanduseTransFile.empty())		LoadConfigFile("landuse_translate.txt");
	else								LoadConfigFileFullPath(gLanduseTransFile.c_str());

	LoadConfigFile("enum_colors.txt");
	LoadConfigFile("beach_terrain.txt");
	
	ValidateNaturalTerrain();
}

#pragma mark -

int	FindNaturalTerrain(
				int		terrain,
				int 	landuse, 
				int 	climate, 
				float 	elevation, 
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
				float	lat,
				int		variant_blob,
				int		variant_head)
{
	// OPTIMIZE - figure out what the major keys should be.
	
	pair<NaturalTerrainLandUseIndex::iterator,NaturalTerrainLandUseIndex::iterator>	range;
	range = gNaturalTerrainLandUseIndex.equal_range(landuse);
	
	int best_typed = gNaturalTerrainTable.size();
	int best_untyped = gNaturalTerrainTable.size();
	int choice_typed = -1;
	int choice_untyped = -1;
	
	for (NaturalTerrainLandUseIndex::iterator i = range.first; i != range.second; ++i)
	{
		int rec_num = i->second;
		NaturalTerrainInfo_t& rec = gNaturalTerrainTable[rec_num];
		
		float slope_to_use = rec.proj_angle == proj_Down ? slope : slope_tri;
//		float slope_to_use = slope_tri;

		if (terrain == NO_VALUE || rec.terrain == NO_VALUE || terrain == rec.terrain)
		if (climate == NO_VALUE || rec.climate == NO_VALUE || climate == rec.climate)
		if (rec.slope_min == rec.slope_max || slope_to_use == NO_DATA || (rec.slope_min <= slope_to_use && slope_to_use <= rec.slope_max))
		if (rec.elev_min == rec.elev_max || elevation == NO_DATA || (rec.elev_min <= elevation && elevation <= rec.elev_max))
		if (rec.temp_min == rec.temp_max || temp == NO_DATA || (rec.temp_min <= temp && temp <= rec.temp_max))
		if (rec.temp_rng_min == rec.temp_rng_max || temp_rng == NO_DATA || (rec.temp_rng_min <= temp_rng && temp_rng <= rec.temp_rng_max))
		if (rec.rain_min == rec.rain_max || rain == NO_DATA || (rec.rain_min <= rain && rain <= rec.rain_max))
		if (rec.slope_heading_min == rec.slope_heading_max || slopeheading == NO_DATA || (rec.slope_heading_min <= slopeheading && slopeheading <= rec.slope_heading_max))
		if (rec.rel_elev_min == rec.rel_elev_max || relelevation == NO_DATA || (rec.rel_elev_min <= relelevation && relelevation <= rec.rel_elev_max))
		if (rec.elev_range_min == rec.elev_range_max || elevrange == NO_DATA || (rec.elev_range_min <= elevrange && elevrange <= rec.elev_range_max))
		if (rec.urban_density_min == rec.urban_density_max || urban_density == NO_DATA || (rec.urban_density_min <= urban_density && urban_density <= rec.urban_density_max))
		if (rec.urban_radial_min == rec.urban_radial_max || urban_radial == NO_DATA || (rec.urban_radial_min <= urban_radial && urban_radial <= rec.urban_radial_max))
		if (rec.urban_trans_min == rec.urban_trans_max || urban_trans == NO_DATA || (rec.urban_trans_min <= urban_trans && urban_trans <= rec.urban_trans_max))		
		if (rec.urban_square == 0 || urban_square == NO_DATA || rec.urban_square == urban_square)
		if (!rec.near_water || water)
		if (rec.lat_min == rec.lat_max || lat == NO_DATA || (rec.lat_min <= lat && lat <= rec.lat_max))
		if (rec.variant == 0 || variant_blob == 0 || variant_head == 0 || rec.variant == variant_blob || rec.variant == variant_head)
		{
			best_typed = rec_num;
			choice_typed = rec.name;
			break;
		}
	}

	range = gNaturalTerrainLandUseIndex.equal_range(NO_VALUE);
	
	for (NaturalTerrainLandUseIndex::iterator i = range.first; i != range.second; ++i)
	{
		int rec_num = i->second;
		NaturalTerrainInfo_t& rec = gNaturalTerrainTable[rec_num];

		float slope_to_use = rec.proj_angle == proj_Down ? slope : slope_tri;
//		float slope_to_use = slope_tri;

		if (terrain == NO_VALUE || rec.terrain == NO_VALUE || terrain == rec.terrain)
		if (climate == NO_VALUE || rec.climate == NO_VALUE || climate == rec.climate)
		if (rec.slope_min == rec.slope_max || slope_to_use == NO_DATA || (rec.slope_min <= slope_to_use && slope_to_use <= rec.slope_max))
		if (rec.elev_min == rec.elev_max || elevation == NO_DATA || (rec.elev_min <= elevation && elevation <= rec.elev_max))
		if (rec.temp_min == rec.temp_max || temp == NO_DATA || (rec.temp_min <= temp && temp <= rec.temp_max))
		if (rec.temp_rng_min == rec.temp_rng_max || temp_rng == NO_DATA || (rec.temp_rng_min <= temp_rng && temp_rng <= rec.temp_rng_max))
		if (rec.rain_min == rec.rain_max || rain == NO_DATA || (rec.rain_min <= rain && rain <= rec.rain_max))
		if (rec.slope_heading_min == rec.slope_heading_max || slopeheading == NO_DATA || (rec.slope_heading_min <= slopeheading && slopeheading <= rec.slope_heading_max))
		if (rec.rel_elev_min == rec.rel_elev_max || relelevation == NO_DATA || (rec.rel_elev_min <= relelevation && relelevation <= rec.rel_elev_max))
		if (rec.elev_range_min == rec.elev_range_max || elevrange == NO_DATA || (rec.elev_range_min <= elevrange && elevrange <= rec.elev_range_max))
		if (rec.urban_density_min == rec.urban_density_max || urban_density == NO_DATA || (rec.urban_density_min <= urban_density && urban_density <= rec.urban_density_max))
		if (rec.urban_radial_min == rec.urban_radial_max || urban_radial == NO_DATA || (rec.urban_radial_min <= urban_radial && urban_radial <= rec.urban_radial_max))
		if (rec.urban_trans_min == rec.urban_trans_max || urban_trans == NO_DATA || (rec.urban_trans_min <= urban_trans && urban_trans <= rec.urban_trans_max))
		if (rec.urban_square == 0 || urban_square == NO_DATA || rec.urban_square == urban_square)
		if (!rec.near_water || water)
		if (rec.lat_min == rec.lat_max || lat == NO_DATA || (rec.lat_min <= lat && lat <= rec.lat_max))
		if (rec.variant == 0 || variant_blob == 0 || variant_head == 0 || rec.variant == variant_blob || rec.variant == variant_head)
		{
			best_untyped = rec_num;
			choice_untyped = rec.name;
			break;
		}
	}

	if (choice_untyped != -1 && choice_typed != -1)
		return (best_untyped < best_typed) ? choice_untyped : choice_typed;
	if (choice_untyped != -1) return choice_untyped;
	return choice_typed;
}

#pragma mark -

bool	LowerPriorityNaturalTerrain(int lhs, int rhs)
{
	// Fast case - if these are equal, don't even bother with a lookup, we know
	// that they can't be lower/higher prioritY!
	if (lhs == rhs) return false;
	
	if (lhs == terrain_Water) return true;
	if (rhs == terrain_Water) return false;
	lhs = gNaturalTerrainIndex[lhs];
	rhs = gNaturalTerrainIndex[rhs];
	
	// Lookups - if we have a layer difference, that goes.
	if (gNaturalTerrainTable[lhs].layer < gNaturalTerrainTable[rhs].layer) return true;
	if (gNaturalTerrainTable[lhs].layer > gNaturalTerrainTable[rhs].layer) return false;
	
	// Tie breaker - if the terrains are diferent but the layers are the same, 
	// we have to enforce a layer difference somehow or else we will get haywire
	// results when we try to sort by layer priority.  So simply use their
	// index numbers as priority.  Better than nothing.
	return lhs < rhs;
}

void ValidateNaturalTerrain(void)
{
	map<int, int>	canonical;
	int				n, ref;
	for (n = 0; n < gNaturalTerrainTable.size(); ++n)
	{
		if (canonical.count(gNaturalTerrainTable[n].name) == 0)
			canonical[gNaturalTerrainTable[n].name] = n;
	}
	
	for (n = 0; n < gNaturalTerrainTable.size(); ++n)
	{
		ref = canonical[gNaturalTerrainTable[n].name];
		if (ref != n)
		{
			if (gNaturalTerrainTable[n].layer    		  != gNaturalTerrainTable[ref].layer    		)	printf("ERROR: land use lines %d and %d - terrain 'layer' does not match.  name = %s, layers = %d vs %d\n", ref, n, FetchTokenString(gNaturalTerrainTable[n].name), gNaturalTerrainTable[n].layer,gNaturalTerrainTable[ref].layer);
			if (gNaturalTerrainTable[n].xon_dist 		  != gNaturalTerrainTable[ref].xon_dist 		)	printf("ERROR: land use lines %d and %d - terrain 'xon_dist' does not match.  name = %s, layers = %d vs %d\n", ref, n, FetchTokenString(gNaturalTerrainTable[n].name), gNaturalTerrainTable[n].xon_dist,gNaturalTerrainTable[ref].xon_dist);
			if (gNaturalTerrainTable[n].base_tex    	  != gNaturalTerrainTable[ref].base_tex    	 	)	printf("ERROR: land use lines %d and %d - terrain 'base_tex' does not match.  name = %s, layers = %s vs %s\n", ref, n, FetchTokenString(gNaturalTerrainTable[n].name), gNaturalTerrainTable[n].base_tex.c_str(),gNaturalTerrainTable[ref].base_tex.c_str());
			if (gNaturalTerrainTable[n].base_res		  != gNaturalTerrainTable[ref].base_res		 	)	printf("ERROR: land use lines %d and %d - terrain 'base_res' does not match.  name = %s, layers = %d vs %d\n", ref, n, FetchTokenString(gNaturalTerrainTable[n].name), gNaturalTerrainTable[n].base_res,gNaturalTerrainTable[ref].base_res);
		}
	}	
}

bool	IsForestType(int inType)
{
	return sForests.count(inType) != 0;
}

void	CheckDEMRuleCoverage(void)
{
	CoverageFinder	cov(10);
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
}

void	GetNaturalTerrainColor(int terrain, float rgb[3])
{
	if (gNaturalTerrainIndex.count(terrain) == 0)
	{
		rgb[0] = rgb[2] = 1.0;
		rgb[1] = 0.0;
		return;
	}
	NaturalTerrainInfo_t& info = gNaturalTerrainTable[gNaturalTerrainIndex[terrain]];
	rgb[0] = info.map_rgb[0];
	rgb[1] = info.map_rgb[1];
	rgb[2] = info.map_rgb[2];
}
