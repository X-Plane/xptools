/*
 * Copyright (c) 2019, Laminar Research.
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

#include "GISTool_MobileProcessingCmds.h"
#include "GISTool_Globals.h"
#include "GISTool_Utils.h"
#include "Agp.h"
#include "AssertUtils.h"
#include "MapDefs.h"
#include "BitmapUtils.h"
#include "GISUtils.h"
#include "MapCreate.h"
#include "MapOverlay.h"
#include "PlatformUtils.h"
#include "FileUtils.h"
#include "DEMTables.h"
#include "MapAlgs.h"
#include "ObjTables.h"
#include "MapTopology.h"
#include "MobileAutogenAlgs.h"

using dsf_assignment = vector<vector<tile_assignment>>;

static Pmwx s_autogen_grid;
static constexpr int s_pseudorand[] = {918,422,359,512,181,657,814,87,418,288,944,295,56,755,709,56,211,394,408,936,959,752,143,866,664,511,434,562,81,899,22,758,803,145,578,648,874,841,60,738,275,507,899,941,263,156,346,722,889,124,988,458,318,447,189,532,557,209,98,946,909,629,375,246,812,563,489,744,890,334,95,808,249,967,608,803,428,258,962,747,864,875,645,58,518,124,794,868,125,896,203,501,801,557,353,65,646,759,347,413,50,608,442,289,183,34,104,196,458,430,375,992,308,515,120,203,888,626,652,411,495,64,960,991,588,398,815,107,813,948,410,186,444,748,724,195,373,165,474,989,934,580,221,953,542,338,990,819,754,454,360,308,888,634,326,30,599,399,970,3,405,415,712,40,204,779,554,379,145,318,229,540,633,945,215,161,351,457,32,304,210,874,664,0,302,24,492,818,605,760,574,490,282,761,360,992,120,802,449,312,130,573,599,696,12,946,785,82,129,471,438,924,879,224,122,97,420,260,497,581,360,589,7,390,547,985,359,604,408,802,847,388,653,466,148,708,160,924,655,274,508,595,469,964,73,580,490,533,700,0,17,473,842,383,709,735,728,713,931,57,5,555,484,226,216,787,66,753,880,211,434,262,855,389,60,26,889,257,903,65,514,825,868,376,191,617,396,331,681,545,771,469,154,566,36,674,84,771,890,487,15,259,709,103,861,309,359,172,778,336,373,532,365,996,40,28,242,539,854,67,415,178,525,767,243,360,73,175,231,989,26,48,88,41,58,979,496,524,827,889,310,58,629,441,813,606,618,344,537,485,108,885,412,472,572,452,832,829,748,147,798,174,756,293,466,890,170,158,196,107,702,976,451,868,213,429,316,672,808,826,421,444,681,868,525,848,217,261,753,836,589,703,927,523,806,284,518,266,370,168,233,718,985,775,326,484,376,507,76,41,678,233,427,927,505,176,601,259,613,386,784,768,271,902,651,474,265,733,80,286,820,32,715,234,237,653,381,288,922,515,195,329,234,602,725,851,174,117,873,112,650,856,411,883,8,869,490,559,222,513,802,930,884,75,707,513,982,471,764,487,638,805,605,447,765,464,371,143,279,643,764,475,240,767,36,823,763,507,713,739,571,891,355,275,741,689,705,403,688,797,438,181,567,593,98,258,723,288,31,291,585,27,169,753,536,290,284,731,331,463,437,725,530,369,401,485,445,748,449,379,693,104,208,1000,899,900,888,964,4,791,278,791,265,23,507,178,812,356,713,738,950,299,218,84,84,981,444,119,991,464,488,545,853,967,72,917,868,286,11,511,533,386,833,805,214,35,228,289,294,831,469,400,520,549,419,2,747,777,492,919,672,448,404,627,540,773,952,143,83,735,598,54,190,502,559,651,712,380,576,804,401,105,435,298,992,366,222,582,911,888,672,179,755,860,521,948,821,391,237,952,210,694,558,346,240,5,864,846,201,285,609,293,536,157,514,340,694,427,504,669,154,115,623,869,983,910,205,200,651,952,21,249,957,959,31,405,401,392,751,740,437,386,122,542,506,459,400,952,113,202,184,297,994,567,976,628,1,739,636,791,966,717,420,252,184,384,656,457,606,991,830,704,790,689,105,41,964,399,858,129,606,356,334,19,400,708,736,496,756,429,163,596,133,442,845,682,350,551,37,73,319,782,696,85,477,16,889,586,798,720,441,835,212,862,864,595,185,960,744,935,267,870,94,368,281,110,647,622,599,992,286,420,10,632,612,945,742,977,313,415,273,503,768,86,685,314,406,784,767,572,954,241,649,120,930,258,801,154,531,909,986,576,855,435,452,553,145,366,512,847,183,255,40,99,164,92,882,230,643,499,782,393,830,653,868,196,741,88,714,88,13,352,600,602,398,276,417,564,382,907,323,698,919,795,859,775,369,635,434,502,87,197,941,785,599,624,226,464,847,541,707,798,780,517,668,348,132,268,408,624,550,938,650,141,537,697,445,729,66,961,67,887,864,943,233,644,558,113,557,33,883,103,169,865,325,541,204,534,135,896,123,650,983,849,890,114,501,513,163,741,29,793,693,954,19,706,203,194,7,946,284,981,474,13,351,195,982,741,64,877,420,936,964,67,810,64,95,30,240,519,388,908,603,690,511,284,564,818,346,505,7,49,616,213,720,822,244,854,432,400,95,985,741,469,981,854,768,521,440,723,63,333,833,919,27,374,406,504,920,692,871,353,110,121,150,776,188,325,263,73,704,150,291,165,858,225,5,793,471,184,235,481,777,888,173,941,142,600,311,747};
static const array<int, 3> s_terrain_types_to_not_touch = {terrain_Water, terrain_VisualWater, terrain_Airport};
static const map<string, Bbox2> s_obj_bounds_mtrs = read_mobile_obj_ground_bounds();

struct ag_terrain_dsf_description {
	int dsf_lon; // the min longitude in the DSF
	int dsf_lat; // the min latitude  in the DSF
	int divisions_lon;
	int divisions_lat;
	ag_terrain_style style;
};
static ag_terrain_dsf_description s_dsf_desc;


struct special_ter_repeat_rule {
	special_ter_repeat_rule(int min, int max, int ter_1) : min_radius(min), target_max_radius(max) { compatible_terrains.push_back(ter_1); compatible_terrains.push_back(ter_1 + 1); }
	int min_radius; // in terms of grid squares
	int target_max_radius; // in terms of grid squares
	vector<int> compatible_terrains;
};


static inline int MAJORITY_RULES(int a, int b, int c, int d)
{
	int la = 1, lb = 1, lc = 1, ld = 1;
	if (a == b) ++la, ++lb;
	if (a == c) ++la, ++lc;
	if (a == d) ++la, ++ld;
	if (b == c) ++lb, ++lc;
	if (b == d) ++lb, ++ld;
	if (c == d) ++lc, ++ld;

	if (la >= lb && la >= lc && la >= ld) return a;
	if (lb >= la && lb >= lc && lb >= ld) return b;
	if (lc >= la && lc >= lb && lc >= ld) return c;
	if (ld >= la && ld >= lb && ld >= lc) return d;
	return a;
}

static inline int MAJORITY_RULES(vector<int> values)
{
	DebugAssert(values.size() == 4);
	return MAJORITY_RULES(values[0], values[1], values[2], values[3]);
}

template<typename HalfEdge>
static double halfedge_length(const HalfEdge &he)
{
	return sqrt(
			CGAL::to_double(
					CGAL::squared_distance(
							he->source()->point(),
							he->target()->point()
					)
			)
	);
}


static int choose_ortho_terrain_us(int land_use, double mean_urbanization)
{
	constexpr double min_urbanization_for_any_ortho = 0.001;
	constexpr double min_urbanization_for_ortho_on_non_matching_land_use = 0.1;
	if(land_use != lu_globcover_WATER &&
			mean_urbanization > min_urbanization_for_any_ortho)
	{
		switch(land_use)
		{
			case lu_globcover_URBAN_CROP_TOWN:
			case lu_globcover_URBAN_SQUARE_CROP_TOWN:
			case lu_globcover_URBAN_SQUARE_TOWN:
			case lu_globcover_URBAN_TOWN:
				return terrain_PseudoOrthoTown;
			case lu_globcover_URBAN_LOW:
			case lu_globcover_URBAN_MEDIUM:
			case lu_globcover_URBAN_SQUARE_LOW:
			case lu_globcover_URBAN_SQUARE_MEDIUM:
				return mean_urbanization < 2 ? terrain_PseudoOrthoTown : terrain_PseudoOrthoOuter;
			case lu_globcover_URBAN_HIGH:
			case lu_globcover_URBAN_SQUARE_HIGH:
				return mean_urbanization <= 0.35 ? terrain_PseudoOrthoOuter : terrain_PseudoOrthoInner;
			case lu_globcover_INDUSTRY:
			case lu_globcover_INDUSTRY_SQUARE:
				return terrain_PseudoOrthoIndustrial;
			// Tyler says: this would be kind of nice, but our tiling algorithm doesn't support the specific "park" sub-type of inner city
			/*
			case lu_globcover_WETLAND_BROADLEAVED_OPEN:
			case lu_globcover_WETLAND_GRASSLAND:
			case lu_globcover_WETLAND_SHRUB_CLOSED:
				if(mean_urbanization > 0.55)
					return terrain_PseudoOrthoInnerPark;
				INTENTIONAL_FALLTHROUGH;
			*/
			default:
				if(mean_urbanization < min_urbanization_for_ortho_on_non_matching_land_use)
				{
					return NO_VALUE;
				}
				else if(mean_urbanization <= 0.25)
				{
					return terrain_PseudoOrthoTown;
				}
				else if(mean_urbanization <= 0.65)
				{
					return terrain_PseudoOrthoOuter;
				}
				else
				{
					return terrain_PseudoOrthoInner;
				}
				break;
		}
	}
	return NO_VALUE;
}

static int choose_ortho_terrain_euro(int land_use, double mean_urbanization)
{
	static const double min_urbanization_for_any_ortho = 0.001;
	static const double min_urbanization_for_ortho_on_non_matching_land_use = 0.1;
	if(land_use != lu_globcover_WATER &&
			mean_urbanization > min_urbanization_for_any_ortho)
	{
		switch(land_use)
		{
			case lu_globcover_URBAN_CROP_TOWN:
			case lu_globcover_URBAN_SQUARE_CROP_TOWN:
			case lu_globcover_URBAN_SQUARE_TOWN:
			case lu_globcover_URBAN_TOWN:
			case lu_globcover_URBAN_LOW:
			case lu_globcover_URBAN_MEDIUM:
			case lu_globcover_URBAN_SQUARE_LOW:
			case lu_globcover_URBAN_SQUARE_MEDIUM:
			case lu_globcover_URBAN_HIGH:
			case lu_globcover_URBAN_SQUARE_HIGH:
				return terrain_PseudoOrthoEuro;
			case lu_globcover_INDUSTRY:
			case lu_globcover_INDUSTRY_SQUARE:
				return terrain_PseudoOrthoEuroSortaIndustrial;
			default:
				if(mean_urbanization > 0.25)
				{
					return terrain_PseudoOrthoEuro;
				}
				break;
		}
	}
	return NO_VALUE;
}

static int choose_ortho_terrain(int land_use, double mean_urbanization, ag_terrain_style style)
{
	return style == style_europe ? choose_ortho_terrain_euro(land_use, mean_urbanization) : choose_ortho_terrain_us(land_use, mean_urbanization);
}

static ag_terrain_dsf_description initialize_autogen_pmwx()
{
	const DEMGeo & climate_style(gDem[dem_ClimStyle]);
	DebugAssertWithExplanation(climate_style.mWidth > 0 && climate_style.mHeight > 0, "No climate data available");

	const Bbox_2 bounding_box = Bbox_2(climate_style.mWest, climate_style.mSouth, climate_style.mEast, climate_style.mNorth);
	vector<Segment_2> grid_params;
	const int degrees_lon = intround(climate_style.mEast  - climate_style.mWest);
	const int degrees_lat = intround(climate_style.mNorth - climate_style.mSouth);
	DebugAssertWithExplanation(flt_abs(climate_style.mEast  - climate_style.mWest  - degrees_lon) < 0.01, "Working area should be an integer number of degrees longitude");
	DebugAssertWithExplanation(flt_abs(climate_style.mNorth - climate_style.mSouth - degrees_lat) < 0.01, "Working area should be an integer number of degrees latitude");
	Assert(degrees_lon * degrees_lat == 1);

	const double lon_min = climate_style.mWest;
	const double lat_min = climate_style.mSouth;

	const ag_terrain_style style = choose_style(lon_min, lat_min);

	const int divisions_lon = divisions_longitude_per_degree(g_desired_ortho_dim_m[style], lat_min + 0.5);
	for(int x = 0; x <= divisions_lon; ++x)
	{
		double lon = lon_min + ((double)x / divisions_lon);
		grid_params.push_back(Segment_2(Point_2(lon, lat_min), Point_2(lon, lat_min + 1)));
	}

	const int divisions_lat = divisions_latitude_per_degree(g_desired_ortho_dim_m[style]);
	for(int y = 0; y <= divisions_lat; ++y)
	{
		const double lat = lat_min + ((double)y / divisions_lat);
		grid_params.push_back(Segment_2(Point_2(lon_min, lat), Point_2(lon_min + 1, lat)));
	}

#if DEV
	for(vector<Segment_2>::const_iterator i = grid_params.begin(); i != grid_params.end(); ++i)
	for(vector<Segment_2>::const_iterator j = grid_params.begin(); j != grid_params.end(); ++j)
	{
		DebugAssert(i == j || *i != *j); // ensure no duplicate segments!
	}
#endif


	// These are effectively the gridlines for every grid square
	vector<vector<Pmwx::Halfedge_handle> >	halfedge_handles;
	// Edges will be adjacent to faces where we want to set the terrain type
	Map_CreateReturnEdges(s_autogen_grid, grid_params, halfedge_handles);

#if DEV
	// Sanity check: No zero area faces in the AG grid!
	for(Pmwx::Face_handle f = s_autogen_grid.faces_begin(); f != s_autogen_grid.faces_end(); ++f)
	{
		if(!f->is_unbounded())
		{
			const Polygon2 ben_face = cgal2ben(f, lon_min, lat_min);
			if(ben_face.area() <  0) { cout << "Negative area: " << ben_face.wolfram_alpha() << "\n"; }
			if(ben_face.area() == 0) { cout << "Zero area: "     << ben_face.wolfram_alpha() << "\n";
				cout << "GetMapFaceAreaMeters(f): " << GetMapFaceAreaMeters(f) << "\n"; }
			DebugAssert(ben_face.is_ccw());
			DebugAssert(ben_face.area() > one_square_meter_in_degrees || !barf_on_tiny_map_faces());
		}
	}
#endif

	return {intround(lon_min), intround(lat_min), divisions_lon, divisions_lat, style};
}

static map<int, special_ter_repeat_rule> get_special_ter_repeat_rules_us()
{
	map<int, special_ter_repeat_rule> out;
	out.insert(make_pair(terrain_PseudoOrthoInnerPark,			special_ter_repeat_rule(3, 5, terrain_PseudoOrthoInner1)));
	out.insert(make_pair(terrain_PseudoOrthoInnerStadium,		special_ter_repeat_rule(3, 7, terrain_PseudoOrthoInner1)));
	out.insert(make_pair(terrain_PseudoOrthoOuterBuilding,		special_ter_repeat_rule(2, 5, terrain_PseudoOrthoOuter1)));
	out.insert(make_pair(terrain_PseudoOrthoOuterStadium,		special_ter_repeat_rule(3, 7, terrain_PseudoOrthoOuter1)));
	out.insert(make_pair(terrain_PseudoOrthoTownLgBuilding,		special_ter_repeat_rule(2, 3, terrain_PseudoOrthoTown1)));
	out.insert(make_pair(terrain_PseudoOrthoTownSpecial2,		special_ter_repeat_rule(2, 3, terrain_PseudoOrthoTown1)));
	out.insert(make_pair(terrain_PseudoOrthoIndustrialSpecial1,	special_ter_repeat_rule(2, 3, terrain_PseudoOrthoIndustrial1)));
	out.insert(make_pair(terrain_PseudoOrthoIndustrialSpecial2,	special_ter_repeat_rule(2, 3, terrain_PseudoOrthoIndustrial1)));
	return out;
}

static map<int, special_ter_repeat_rule> get_special_ter_repeat_rules_euro()
{
	map<int, special_ter_repeat_rule> out;
	out.insert(make_pair(terrain_PseudoOrthoEuroSemiInd,		special_ter_repeat_rule(3, 5, terrain_PseudoOrthoEuro1)));
	out.insert(make_pair(terrain_PseudoOrthoEuroIndustrial,		special_ter_repeat_rule(3, 7, terrain_PseudoOrthoEuro1)));
	return out;
}

static map<int, special_ter_repeat_rule> get_special_ter_repeat_rules(ag_terrain_style style)
{
	return style == style_europe ? get_special_ter_repeat_rules_euro() : get_special_ter_repeat_rules_us();
}

static bool has_matching_ter_enum_in_radius(int ter_enum, int min_radius, const grid_coord_desc &point, const dsf_assignment &tile_assignments)
{
	DebugAssert(!tile_assignments.empty());
	for(int x = max(point.x - min_radius, 0); x <= intmin2(point.x + min_radius, tile_assignments.size()    - 1); ++x)
	for(int y = max(point.y - min_radius, 0); y <= intmin2(point.y + min_radius, tile_assignments[x].size() - 1); ++y)
	{
		if(x != point.x && y != point.y &&
				tile_assignments[x][y].ter_enum == ter_enum)
		{
				return true;
		}
	}
	return false;
}

static void attempt_assign_special_ter_enum(int ter_enum, const map<int, special_ter_repeat_rule> &special_ter_repeat_rules, const grid_coord_desc &point, dsf_assignment &tile_assignments)
{
	DebugAssert(special_ter_repeat_rules.count(ter_enum));
	const special_ter_repeat_rule &rule = special_ter_repeat_rules.at(ter_enum);
	if(contains(rule.compatible_terrains, tile_assignments[point.x][point.y].ter_enum) &&
			!has_matching_ter_enum_in_radius(ter_enum, rule.min_radius, point, tile_assignments))
	{
		tile_assignments[point.x][point.y] = ter_enum;
		//cout << "Assigned special terrain " << FetchTokenString(ter_enum) << " at (" << point.x << ", " << point.y << ")" << endl;
	}
}

/**
 * A repeatable method for scattering tile placements in apparently random places.
 */
static int pseudorandom_in_range(const special_ter_repeat_rule &rule, const pair<int, int> &dsf, int dim, int dsf_delta_dim)
{
	DebugAssert(rule.min_radius < rule.target_max_radius);
	const int offset = int_abs(dim + dsf.first * dsf.second * dsf_delta_dim * rule.min_radius * rule.target_max_radius);
	const int pseudo_rand = s_pseudorand[offset % (sizeof(s_pseudorand) / sizeof(s_pseudorand[0]))];
	return rule.min_radius + pseudo_rand % (rule.target_max_radius - rule.min_radius);
}

static int find_terrain_rule_name(int ter_enum)
{
	const auto matching_rule = find_if(gNaturalTerrainRules.begin(), gNaturalTerrainRules.end(), [=](const NaturalTerrainRule_t & rule) { return rule.terrain == ter_enum; });
	return matching_rule == gNaturalTerrainRules.end() ? -1 : matching_rule->name;
}

static string ter_lib_path_to_png_path(string lib_path)
{
	if(lib_path.find("../autogen/US/") == 0)
	{
		str_replace_all(lib_path, "../autogen/US/", "");

		map<string, string> prefixes;
		prefixes["OUT_"] = "temp_city_sq_out/";
		prefixes["IND_"] = "temp_city_sq_ind/";
		prefixes["IN_"] = "temp_city_sq_in/City_";
		prefixes["TWN_"] = "temp_city_sq_twn/";
		for(map<string, string>::const_iterator prefix = prefixes.begin(); prefix != prefixes.end(); ++prefix)
		{
			if(lib_path.find(prefix->first) == 0)
			{
				lib_path = prefix->second + lib_path;
				break;
			}
		}
		DebugAssertWithExplanation(lib_path.find("temp_") == 0, "Failed to match prefix");

		str_replace_all(lib_path, ".ter", ".png");
		return "Global Scenery/Mobile_Autogen_Lib/US/Textures/orthogonal_land_textures/" + lib_path;
	}
	else if(lib_path.find("../autogen/Europe/") == 0)
	{
		str_replace_all(lib_path, "../autogen/Europe/", "");
		str_replace_all(lib_path, ".ter", ".png");
		return "Global Scenery/Mobile_Autogen_Lib/Europe/Textures/orthogonal_land_textures/" + lib_path;
	}
	else
	{
		DebugAssertWithExplanation(false, "Unknown lib path");
		return "";
	}
}

static string ter_lib_path_to_agp_disk_path(string lib_path)
{
	str_replace_all(lib_path, "../autogen", "");
	str_replace_all(lib_path, ".ter", ".agp");
	return "Global Scenery/Mobile_Autogen_Lib/" + lib_path;
}

static map<int, agp_t> read_mobile_agps()
{
	map<int, agp_t> agps; // maps terrain enum to the AGP describing its building placements
	for(int ter = terrain_PseudoOrthophoto; ter < terrain_PseudoOrthophotoEnd; ++ter)
	{
		const int rule_name = find_terrain_rule_name(ter);
		NaturalTerrainInfoMap::const_iterator ter_info = gNaturalTerrainInfo.find(rule_name);
		if(ter_info != gNaturalTerrainInfo.end())
		{
			const string &ter_lib_path = ter_info->second.base_tex;
			const string agp_disk_path = ter_lib_path_to_agp_disk_path(ter_lib_path);
			agp_t agp;
			const bool loaded = load_agp(agp_disk_path, agp);
			if(loaded)
			{
				agps.insert(make_pair(ter, agp));
			}
			else
			{
				fprintf(stderr, "Couldn't find AGP %s", agp_disk_path.c_str());
			}
			DebugAssert(loaded);
		}
	}
	return agps;
}

static map<string, int> register_obj_tokens_for_agps(const map<int, agp_t> & agps)
{
	map<string, int> obj_tokens; // maps agp_t::obj::name values to the global enums we register for them
	for(const auto & terrain_enum_and_agp : agps)
	for(const agp_t::obj & obj : terrain_enum_and_agp.second.objs)
	{
		const string no_ext = FILE_get_file_name_wo_extensions(obj.name);
		const int token = NewToken(no_ext.c_str());
		DebugAssert(token > NUMBER_OF_DEFAULT_TOKENS);
		obj_tokens.insert(make_pair(obj.name, token));
	}
	return obj_tokens;
}

static ortho_urbanization conform_terrain_to_expectations(const ortho_urbanization &non_matching_tile)
{
	vector<int> out = non_matching_tile.to_vector();
	if(non_matching_tile.count_sides(NO_VALUE) == 3)
	{
		for(vector<int>::iterator it = out.begin(); it != out.end(); ++it)
		{
			if(*it != NO_VALUE)
			{
				*it = terrain_PseudoOrthoTown;
				break;
			}
		}
	}
	return out;
}

static int to_scorable_value(int ortho_enum)
{
	switch(ortho_enum)
	{
		case NO_VALUE:									return 0;
		case terrain_PseudoOrthoTown:					return 1;
		case terrain_PseudoOrthoEuro:					return 1;
		case terrain_PseudoOrthoOuter:					return 2;
		case terrain_PseudoOrthoInner:					return 3;
		case terrain_PseudoOrthoIndustrial:				return 4;
		case terrain_PseudoOrthoEuroSortaIndustrial:	return 4;
		default: DebugAssert(!"illegal ortho enum");	return 9999;
	}
}

static const char * abbreviated_ortho_str(int ortho_enum)
{
	const char * full = FetchTokenString(ortho_enum);
	const char * sub_to_nuke = "terrain_PseudoOrtho";
	if(strstr(full, sub_to_nuke) == full)
	{
		return full + strlen(sub_to_nuke);
	}
	return full;
}

static void dump_tile_diff(const ortho_urbanization &from, const ortho_urbanization &to)
{
	if(from.top_left     == to.top_left    ) printf("\t%17s ",  abbreviated_ortho_str(from.top_left    )); else printf("\t%17s ",  stl_printf("%s->%s", abbreviated_ortho_str(from.top_left    ), abbreviated_ortho_str(to.top_left    )).c_str());
	if(from.top_right    == to.top_right   ) printf(  "%17s\n", abbreviated_ortho_str(from.top_right   )); else printf(  "%17s\n", stl_printf("%s->%s", abbreviated_ortho_str(from.top_right   ), abbreviated_ortho_str(to.top_right   )).c_str());
	if(from.bottom_left  == to.bottom_left ) printf("\t%17s ",  abbreviated_ortho_str(from.bottom_left )); else printf("\t%17s ",  stl_printf("%s->%s", abbreviated_ortho_str(from.bottom_left ), abbreviated_ortho_str(to.bottom_left )).c_str());
	if(from.bottom_right == to.bottom_right) printf(  "%17s\n", abbreviated_ortho_str(from.bottom_right)); else printf(  "%17s\n", stl_printf("%s->%s", abbreviated_ortho_str(from.bottom_right), abbreviated_ortho_str(to.bottom_right)).c_str());
}

static int score_distance(int ortho_enum_1, int ortho_enum_2)
{
	int scorable_1 = to_scorable_value(ortho_enum_1);
	int scorable_2 = to_scorable_value(ortho_enum_2);
	return int_abs(scorable_1 - scorable_2);
}

static int score_distance(const ortho_urbanization &tile_1, const ortho_urbanization &tile_2)
{
	return score_distance(tile_1.bottom_left, tile_2.bottom_left) +
			score_distance(tile_1.bottom_right, tile_2.bottom_right) +
			score_distance(tile_1.top_left, tile_2.top_left) +
			score_distance(tile_1.top_right, tile_2.top_right);
}

static int s_count_exact = 0;
static int s_count_nonexact = 0;
static int s_sum_nonexact_scores = 0;

static int choose_nearest_terrain(const ortho_urbanization &tile, const map<ortho_urbanization, int> & options)
{
	map<ortho_urbanization, int>::const_iterator ter_to_use = options.find(tile);
	if(ter_to_use != options.end())
	{
		++s_count_exact;
		return ter_to_use->second;
	}
	else // find the tile of minimum distance
	{
		map<ortho_urbanization, int>::const_iterator best_match = options.end();
		int best_score = 9999;
		for(map<ortho_urbanization, int>::const_iterator option = options.begin(); option != options.end(); ++option)
		{
			const int score = score_distance(tile, option->first);
			if(score < best_score)
			{
				best_match = option;
				best_score = score;
			}
		}
		++s_count_nonexact;
		s_sum_nonexact_scores += best_score;

		#if 0
			if(best_score > 1)
			{
				printf("Nonexact match to %s with score %d\n", abbreviated_ortho_str(best_match->second), best_score);
				dump_tile_diff(tile, best_match->first);
			}
		#endif
		DebugAssert(best_match != options.end());
		return best_match->second;
	}
}

static map<int, ortho_urbanization> omg_reverse_map(const map<ortho_urbanization, int> & in)
{
	map<int, ortho_urbanization> out;
	for(map<ortho_urbanization, int>::const_iterator it = in.begin(); it != in.end(); ++it)
	{
		out.insert(make_pair(it->second, it->first));
	}
	return out;
}


// less operator for Point
static inline bool operator< (const Point2 & lhs, const Point2 & rhs)
{
	return std::tie(lhs.x_, lhs.y_) < std::tie(rhs.x_, rhs.y_);
}

static int DoMobileAutogenTerrain(const vector<const char *> &args)
{
	verify_map_bounds();
	DebugAssertWithExplanation(gDem.count(dem_UrbanDensity), "Tried to add autogen terrain with no DEM urbanization data; you probably need to change the order of your scenery gen commands");
	DebugAssertWithExplanation(gDem.count(dem_LandUse), "Tried to add autogen terrain with no DEM land use data; you probably need to change the order of your scenery gen commands");
	DebugAssertWithExplanation(gDem.count(dem_ClimStyle), "No climate data loaded");

	s_dsf_desc = initialize_autogen_pmwx();

	gObjLibPrefix = string("lib/mobile/autogen/") + (s_dsf_desc.style == style_us ? "US/" : "Europe/");

	const int dx = s_dsf_desc.divisions_lon;
	const int dy = s_dsf_desc.divisions_lat;
	dsf_assignment ortho_terrain_assignments(dx);
	for(int lon_offset = 0; lon_offset < dx; ++lon_offset)
	{
		ortho_terrain_assignments[lon_offset].resize(dy);
	}

	const map<ortho_urbanization, int> ter_with_transitions = get_terrain_transition_descriptions(s_dsf_desc.style);

	//--------------------------------------------------------------------------------------------------------
	// PASS 1
	// Choose a "starting point" orthophoto for each tile in the grid
	// based on land class and urban density data.
	// If we stopped here, it would look like shit, because:
	//   a) there would be no nice transitions between tile types
	//   b) we would never have used the "special" tiles of each type (e.g., sports stadiums)
	//   c) we wouldn't have done anything about standalone tiles, which just look awkward
	//--------------------------------------------------------------------------------------------------------
	map<ortho_urbanization, int> missing_transitions_count;
	for(int x = 0; x < dx; ++x)
	for(int y = 0; y < dy; ++y)
	{
		vector<int> land_uses(4, DEM_NO_DATA); // counterclockwise from lower left
		vector<float> urbanization(4, DEM_NO_DATA);
		bool has_some_urbanization_data = false;
		for(int corner_x = 0; corner_x < 2; ++corner_x)
		for(int corner_y = 0; corner_y < 2; ++corner_y)
		{
			const int corner_idx = corner_y == 0 ? corner_x : 3 - corner_x;
			const double lon = s_dsf_desc.dsf_lon + (double)(x + corner_x) / dx;
			const double lat = s_dsf_desc.dsf_lat + (double)(y + corner_y) / dy;
			const float urb = gDem[dem_UrbanDensity].value_linear(lon, lat);
			urbanization[corner_idx] = urb;
			has_some_urbanization_data |= urb != DEM_NO_DATA;

			const int land_use = intround(gDem[dem_LandUse].xy_nearest_raw(lon, lat));
			land_uses[corner_idx] = land_use;
		}

		if(has_some_urbanization_data)
		{
			vector<int> corners(4, NO_VALUE);
			for(int i = 0; i < 4; ++i)
			{
				corners[i] = choose_ortho_terrain(land_uses[i], urbanization[i], s_dsf_desc.style);
			}

			ortho_urbanization desired_urb_pattern(corners);
			if(!desired_urb_pattern.is_uniform() || desired_urb_pattern.bottom_left != NO_VALUE)
			{
				int ter_enum = choose_nearest_terrain(desired_urb_pattern, ter_with_transitions);
				ortho_terrain_assignments[x][y] = ter_enum;
			}
		}
	}

	printf("Exact matches: %d\n", s_count_exact);
	printf("Nonexact matches: %d\n", s_count_nonexact);
	printf("Average distance: %f\n", (float)s_sum_nonexact_scores / s_count_nonexact);

	//--------------------------------------------------------------------------------------------------------
	// PASS 2
	// Go through the existing map looking for point features which would correspond to our "special" orthophotos.
	//--------------------------------------------------------------------------------------------------------
	const map<int, special_ter_repeat_rule> special_ter_repeat_rules = get_special_ter_repeat_rules(s_dsf_desc.style); // Tyler says: for reasons unclear to me, we get UB deep within std::map::end() if this isn't const

	vector<int> large_building_features;
	large_building_features.push_back(feat_CommercialOffice);
	large_building_features.push_back(feat_CommercialShoppingPlaza);
	large_building_features.push_back(feat_Government);
	for(Pmwx::Face_handle f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	{
		if(!f->is_unbounded())
		{
			const GIS_face_data &fd = f->data();
			Polygon2 ben_poly = cgal2ben(f, s_dsf_desc.dsf_lon, s_dsf_desc.dsf_lat);
			if(ben_poly.area() > 0) // <= 0 is possible when the face extends beyond the DSF boundary, or when its points are "real" close together
			{
				const Point2 centroid = ben_poly.centroid();
				DebugAssert(s_dsf_desc.dsf_lon == floor(centroid.x()) || centroid.x() == s_dsf_desc.dsf_lon + 1);
				DebugAssert(s_dsf_desc.dsf_lat == floor(centroid.y()) || centroid.y() == s_dsf_desc.dsf_lat + 1);
				{
					const grid_coord_desc grid_pt = get_ortho_grid_xy(centroid, s_dsf_desc.style);
					tile_assignment &assignment = ortho_terrain_assignments[grid_pt.x][grid_pt.y];
					if(s_dsf_desc.style == style_us && // Europe doesn't have the special types we assign below
							assignment.ter_enum != NO_VALUE)
					{
						for(GISPointFeatureVector::const_iterator i = fd.mPointFeatures.begin(); i != fd.mPointFeatures.end(); ++i)
						{
							if(contains(large_building_features, i->mFeatType))
							{
								attempt_assign_special_ter_enum(terrain_PseudoOrthoOuterBuilding, special_ter_repeat_rules, grid_pt, ortho_terrain_assignments);
								attempt_assign_special_ter_enum(terrain_PseudoOrthoTownLgBuilding, special_ter_repeat_rules, grid_pt, ortho_terrain_assignments);
							}
							else if(i->mFeatType == feat_GolfCourse || i->mFeatType == feat_Campground || i->mFeatType == feat_Cemetary)
							{
								attempt_assign_special_ter_enum(terrain_PseudoOrthoInnerPark, special_ter_repeat_rules, grid_pt, ortho_terrain_assignments);
							}
						}
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------
	// PASS 3
	// Ensure we're using the "special" terrain types with "just enough" regularity...
	// They should look "randomly" placed, and not too frequent.
	// (You shouldn't have stadiums too close to each other!)
	// At the same time, we don't want to *never* use the special terrain types!
	//--------------------------------------------------------------------------------------------------------
	for(map<int, special_ter_repeat_rule>::const_iterator rule = special_ter_repeat_rules.begin(); rule != special_ter_repeat_rules.end(); ++rule)
	{
		const int dy_for_randomization = dx == dy ? dy + 13 : dy;
		for(int x = 0; x < dx; x += pseudorandom_in_range(rule->second, make_pair(s_dsf_desc.dsf_lon, s_dsf_desc.dsf_lat), x, dx))
		for(int y = 0; y < dy; y += pseudorandom_in_range(rule->second, make_pair(s_dsf_desc.dsf_lon, s_dsf_desc.dsf_lat), y, dy_for_randomization))
		{
			grid_coord_desc pt = {x, y, dx, dy, s_dsf_desc.dsf_lon, s_dsf_desc.dsf_lat};
			attempt_assign_special_ter_enum(rule->first, special_ter_repeat_rules, pt, ortho_terrain_assignments);
		}
	}

	//--------------------------------------------------------------------------------------------------------
	// PASS 4
	// Add rotations of analogous types
	//--------------------------------------------------------------------------------------------------------
	const map<int, ortho_urbanization> terrain_desc_by_enum = omg_reverse_map(ter_with_transitions);
	for(int x = 0; x < dx; ++x)
	for(int y = 0; y < dy; ++y)
	{
		ortho_terrain_assignments[x][y] = get_analogous_ortho_terrain(ortho_terrain_assignments[x][y].ter_enum, x, y, terrain_desc_by_enum);
	}

	//--------------------------------------------------------------------------------------------------------
	// PASS 5
	// Add transitions between types
	//--------------------------------------------------------------------------------------------------------

#if 0
	//--------------------------------------------------------------------------------------------------------
	// Debugging: output an image that joins all this together
	//--------------------------------------------------------------------------------------------------------
	// Prep images for outputting
	map<tile_assignment, ImageInfo> pngs; // maps (rotated) terrain types to their bitmaps
	for(int ter = terrain_PseudoOrthophoto; ter < terrain_PseudoOrthophotoEnd; ++ter)
	{
		const int rule_name = find_terrain_rule_name(ter);
		NaturalTerrainInfoMap::const_iterator ter_info = gNaturalTerrainInfo.find(rule_name);
		if(ter_info != gNaturalTerrainInfo.end())
		{
			const string &ter_lib_path = ter_info->second.base_tex;
			const string png_on_disk = ter_lib_path_to_png_path(ter_lib_path);
			int error = CreateBitmapFromPNG(png_on_disk.c_str(), &pngs[tile_assignment(ter, 0)], false, GAMMA_SRGB);
			if(error)
			{
				printf("Error loading %s\n", png_on_disk.c_str());
			}
			else
			{
				for(int rot = 270; rot > 0; rot -= 90)
				{
					const ImageInfo &copy_from = pngs[tile_assignment(ter, intwrap(rot + 90, 0, 359))];
					ImageInfo * copy_to = &pngs[tile_assignment(ter, rot)];
					*copy_to = copy_from;
					const long size = copy_from.width * copy_from.height * copy_from.channels;
					copy_to->data = (unsigned char *) malloc(size);
					memcpy(copy_to->data, copy_from.data, size);
					RotateBitmapCCW(&pngs[tile_assignment(ter, rot)]);
				}
			}
		}
	}

	const int output_x_min = 0;
	const int output_x_max = dx;
	const int output_y_min = 0;
	const int output_y_max = dy;

	const int compressed_dim_px = g_ortho_width_px[s_dsf_desc.style] / 2;
	{
		const dsf_assignment &grid = ortho_terrain_assignments;

		ImageInfo out_bmp = {};
		out_bmp.width = compressed_dim_px * (output_x_max - output_x_min);
		out_bmp.height = compressed_dim_px * (output_y_max - output_y_min);
		out_bmp.channels = 3; // rgb
		const long data_size = out_bmp.width * out_bmp.height * out_bmp.channels;
		out_bmp.data = new unsigned char[data_size];
		memset(out_bmp.data, 0, data_size * sizeof(out_bmp.data[0]));

		for(int x = output_x_min; x < output_x_max; ++x)
		for(int y = output_y_min; y < output_y_max; ++y)
		{
			const tile_assignment &assignment = grid[x][y];
			if(assignment.ter_enum > 0)
			{
				DebugAssert(assignment.ter_enum < 500000);
				DebugAssertWithExplanation(pngs.count(assignment), "Couldn't find PNG for terrain");
				const ImageInfo * png = &pngs[assignment];
				CopyBitmapSection(png, &out_bmp,
								  0, 0,
								  png->width, png->height,
								  compressed_dim_px * (x - output_x_min), compressed_dim_px * (y - output_y_min),
								  compressed_dim_px * (x - output_x_min + 1), compressed_dim_px * (y - output_y_min + 1));
			}
		}

		const string out_dir = stl_printf("Earth nav data" DIR_STR "%+03d%+04d" DIR_STR, latlon_bucket(s_dsf_desc.dsf_lat), latlon_bucket(s_dsf_desc.dsf_lon));
		FILE_make_dir_exist(out_dir.c_str());
		const string out_path = stl_printf("%s%+03d%+04d.dsf", out_dir.c_str(), s_dsf_desc.dsf_lat, s_dsf_desc.dsf_lon) + ".png";
		const int error = WriteBitmapToPNG(&out_bmp, out_path.c_str(), NULL, 0, GAMMA_SRGB);
		if(!error)
		{
			printf("Wrote %s\n", out_path.c_str());
		}
		else
		{
			printf("Error %d writing %s\n", error, out_path.c_str());
		}
	}

	// Dump to the console
	printf("Complete assignment set:\n");
	{
		for(int y = output_y_max - 1; y >= output_y_min; --y)
		{
			printf("%03d ", y);
			for(int x = output_x_min; x < output_x_max; ++x)
			{
				if(ortho_terrain_assignments[x][y].ter_enum == NO_VALUE)
				{
					printf("%20s ", " ");
				}
				else
				{
					printf("%20s ", abbreviated_ortho_str(ortho_terrain_assignments[x][y].ter_enum));
				}
			}
			printf("\n");
		}
	}
#endif // DEV

	//--------------------------------------------------------------------------------------------------------
	// FINALLY
	// Assign the selected terrain types to the Pmwx
	//--------------------------------------------------------------------------------------------------------
	for(Pmwx::Face_handle f = s_autogen_grid.faces_begin(); f != s_autogen_grid.faces_end(); ++f)
	{
		if(!f->is_unbounded())
		{
			GIS_face_data &fd = f->data();
			fd.mTerrainType = NO_VALUE;
			fd.mOverlayType = NO_VALUE;
			fd.mTemp1 = NO_VALUE;
			fd.mTemp2 = NO_VALUE;
			#if OPENGL_MAP
				memset(fd.mGLColor, sizeof(fd.mGLColor), 0);
			#endif

			Polygon2 ben_face = cgal2ben(f, s_dsf_desc.dsf_lon, s_dsf_desc.dsf_lat); // not *that* Ben face! https://secure.gravatar.com/ben2212171
			if(!ben_face.is_ccw())
			{
				sort(ben_face.begin(), ben_face.end());
				DebugAssert(ben_face.is_ccw());
			}
			const Point2 centroid = ben_face.centroid();
			const grid_coord_desc grid_pt = get_ortho_grid_xy(centroid, s_dsf_desc.style);
			{
				fd.mTemp1 = grid_pt.x;
				fd.mTemp2 = grid_pt.y;
				DebugAssert(ortho_terrain_assignments.size() > grid_pt.x);
				DebugAssert(ortho_terrain_assignments[grid_pt.x].size() > grid_pt.y);
				const tile_assignment &assignment = ortho_terrain_assignments[grid_pt.x][grid_pt.y];
				if(assignment.ter_enum != NO_VALUE)
				{
					f->set_contained(true);
					fd.mRotationDeg = assignment.rotation_deg;
					const bool needs_overlay =
							intrange(assignment.ter_enum, terrain_PseudoOrthoTownTransBottom, terrain_PseudoOrthoTownTransUR_Full) ||
							intrange(assignment.ter_enum, terrain_PseudoOrthoEuroTransBottom, terrain_PseudoOrthoEuroTransUR_Full);
					if(needs_overlay)
					{
						fd.mOverlayType = assignment.ter_enum;
					}
					else // cover the full tile, don't do an overlay
					{
						fd.mTerrainType = assignment.ter_enum;
					}

					Pmwx::Ccb_halfedge_circulator edge = f->outer_ccb();
					do {
						// Must burn EVERY grid square.  This is mandatory for overlays so they aren't optimized away,
						// and for base terrain so that adjacent terrain gets a dividing edge in the mesh.
						edge->data().mParams[he_MustBurn] = 1;
						--edge;
					} while(edge != f->outer_ccb());
				}
			}
		}
	}
	return 0;
}

static Point2 obj_placement_center_lon_lat(const agp_t::obj & obj, const agp_t & agp, const ag_terrain_dsf_description & dsf_desc, int grid_x, int grid_y, double clockwise_rotation_deg)
{
	DebugAssert(clockwise_rotation_deg - (int)clockwise_rotation_deg == 0);
	DebugAssert((int)clockwise_rotation_deg % 90 == 0);
	DebugAssert(dob_abs(agp.bounds_meters().xspan() - agp.bounds_meters().yspan()) < 0.01); // These AGPs damn well better be square!

	const Bbox2 agp_bounds_m = agp.bounds_meters()
			.rotated_by_degrees_cw(clockwise_rotation_deg)
			.bounds();
	const Point2 rotated_loc = Vector2(obj.x, obj.y).rotated_by_degrees_cw(clockwise_rotation_deg);

	const Bbox2 grid_square_bounds_latlon = Bbox2(
			(double)dsf_desc.dsf_lon + (double)(grid_x)     / dsf_desc.divisions_lon, (double)dsf_desc.dsf_lat + (double)(grid_y)     / dsf_desc.divisions_lat,
			(double)dsf_desc.dsf_lon + (double)(grid_x + 1) / dsf_desc.divisions_lon, (double)dsf_desc.dsf_lat + (double)(grid_y + 1) / dsf_desc.divisions_lat);
	const double out_lon = double_interp(agp_bounds_m.xmin(), grid_square_bounds_latlon.xmin(), agp_bounds_m.xmax(), grid_square_bounds_latlon.xmax(), rotated_loc.x());
	const double out_lat = double_interp(agp_bounds_m.ymin(), grid_square_bounds_latlon.ymin(), agp_bounds_m.ymax(), grid_square_bounds_latlon.ymax(), rotated_loc.y());
	const Point2 center(out_lon, out_lat);
	DebugAssert(grid_square_bounds_latlon.contains(center));
	return center;
}

static Polygon2 obj_placement(const agp_t::obj & obj, double obj_rotation_deg, const Point2 & placement_center_lon_lat)
{
	auto obj_bounds = s_obj_bounds_mtrs.find(obj.name);
	DebugAssert(obj_bounds != s_obj_bounds_mtrs.end());
	if(obj_bounds != s_obj_bounds_mtrs.end())
	{
		const Bbox2 & obj_bounds_mtrs = obj_bounds->second;
		const double lon_deg_per_mtr = m_to_degrees_longitude(placement_center_lon_lat.y());
		const Bbox2 obj_bounds_lon_lat(placement_center_lon_lat.x() + latitude_degrees_per_meter * obj_bounds_mtrs.xmin(), placement_center_lon_lat.y() + lon_deg_per_mtr * obj_bounds_mtrs.ymin(),
									   placement_center_lon_lat.x() + latitude_degrees_per_meter * obj_bounds_mtrs.xmax(), placement_center_lon_lat.y() + lon_deg_per_mtr * obj_bounds_mtrs.ymax());
		const Polygon2 out = obj_bounds_lon_lat.rotated_by_degrees_cw(obj_rotation_deg, placement_center_lon_lat);
		return out;
	}
	return {};
}

static vector<Polygon2> get_holes(Pmwx::Face_handle &f)
{
	vector<Polygon2> holes;
	for(auto hole = f->holes_begin(); hole != f->holes_end(); ++hole)
	{
		holes.emplace_back();
		auto he_circulator = *hole;
		auto stop = he_circulator;
		do {
			holes.back().push_back(cgal2ben(he_circulator->source()->point()));
			++he_circulator;
		} while (he_circulator != stop);
	}
	DebugAssert(holes.size() == f->number_of_holes());
	return holes;
}

static constexpr int ag_terrain_type(const GIS_face_data & face_data) // we can stick the AG terrain type in one of two places... this pulls it out, if applicable
{
	return face_data.mOverlayType == NO_VALUE ? face_data.mTerrainType : face_data.mOverlayType;
}

// Go through the map and clean up any teeny tiny faces that we've inadvertently induced.
// This generally happens due to an unlucky intersection between water and our grid, like this:
// ---------------------------------------\               \----------------
// |                   |                   \  Bastard of   \      |
// |                   |                    \   a lake      |     |
// |                   |                    |\  that cut a  |     |
// |  AG terrain grid  |      AG square 2   |/  tiiiiiiny   |     |
// |       square      |                    /  piece out   /      |
// |                   |                   /  of the grid /   AG  |
// |                   |                  \______________/ square |
// |                   |                    |                 3   |
// ------------------------------------------------------------------------
// If the cut this lake made is small enough (1 square meter is our cut off) it can cause perf issues.
//
// Our solution: merge the tiny pieces into the AG terrain that matches the neighbor on their longest side,
// then ask CGAL to simplify the mesh. This eliminates the teeny tiny face that *would* have gone all the
// way through the pipeline into the output DSF.
//
// @return The number of tiny faces we attempted to merge into their neighbors
static int simplify_tiny_faces_into_their_neighbors(Pmwx &map)
{
	int tiny_faces = 0;
	for(const auto &f : map.face_handles())
	if(!f->is_unbounded())
	{
		const Polygon2 ben_face = cgal2ben(f, s_dsf_desc.dsf_lon, s_dsf_desc.dsf_lat);
		if(ben_face.area() <= one_square_meter_in_degrees)
		{
			++tiny_faces;

			// Find our longest side *not* touching water
			Pmwx::Ccb_halfedge_circulator edge = f->outer_ccb();
			pair<Pmwx::Ccb_halfedge_circulator, double> edge_to_nuke = make_pair(edge, halfedge_length(edge));
			++edge;
			while(edge != f->outer_ccb())
			{
				GIS_face_data &twin_face_data = edge->twin()->face()->data();
				const int ter_enum = ag_terrain_type(twin_face_data);
				if(!contains(s_terrain_types_to_not_touch, ter_enum) && // if our twin face has anything *not* water ("no terrain" is fine!)
						(ter_enum != NO_VALUE || ag_terrain_type(twin_face_data) == NO_VALUE)) // this would be an acceptable terrain... we really want to merge into AG terrain
				{
					const double length = halfedge_length(edge);
					if(length > edge_to_nuke.second)
					{
						edge_to_nuke = make_pair(edge, length);
					}
				}
				++edge;
			}

			Point2 s = cgal2ben(edge_to_nuke.first->source()->point());
			Point2 d = cgal2ben(edge_to_nuke.first->target()->point());
			printf("Nuking edge: (%0.20f, %0.20f) -> (%0.20f, %0.20f)\n", s.x(), s.y(), d.x(), d.y());

			DebugAssert(edge_to_nuke.second > 0);

			edge_to_nuke.first->data().mParams.erase(he_MustBurn);
			edge_to_nuke.first->twin()->data().mParams.erase(he_MustBurn);

			GIS_face_data &face_data = f->data();
			face_data = edge_to_nuke.first->twin()->face()->data();
			face_data.mParams.erase(he_MustBurn);
			f->set_contained(false); // mark this face as no longer being essential
		}
	}

	if(tiny_faces)
	{
		SimplifyMap(map, false, nullptr);
		if(barf_on_tiny_map_faces())
		{
			DebugAssert(count_tiny_faces(map) < tiny_faces);
		}
	}
	return tiny_faces;
}

static int MergeTylersAg(const vector<const char *>& args)
{
#if DEV
	if(barf_on_tiny_map_faces())
	{
		DebugAssertWithExplanation(!count_tiny_faces(gMap), "There were one or more tiny faces in your input data");
		DebugAssertWithExplanation(!count_tiny_faces(s_autogen_grid), "There were one or more tiny faces in your autogen grid data");
	}
#endif


	// Tyler says:
	// I think we need 3 layers here (from bottom to top):
	// 0. The global map as it exists before we manipulate it---no faces marked as "contained"
	// 1. The AG map overlaid on top of that, with all faces marked as "contained"
	//     - This will blit over important features in urban areas like water and airports!
	// 2. The original map *again*, with its important features (like water & airports) marked as contained
	//     - This lets us recover what the AG tried to clear
	Pmwx intermediate_autogen_on_top;
	MapOverlay(gMap, s_autogen_grid, intermediate_autogen_on_top);

	for(Pmwx::Face_handle f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	{
		//DebugAssert(f->is_unbounded() || cgal2ben(f, s_dsf_desc.dsf_lon, s_dsf_desc.dsf_lat).area() > one_square_meter_in_degrees);
		GIS_face_data &fd = f->data();
		if(!f->is_unbounded() && contains(s_terrain_types_to_not_touch, fd.mTerrainType))
		{
			f->set_contained(true);
		}
	}

	Pmwx final;
	MapOverlay(intermediate_autogen_on_top, gMap, final);
	gMap = final;

	SimplifyMap(gMap, false, nullptr);
	// Now go through the map and clean up any teeny tiny faces that we've inadvertently induced.
	// We potentially repeat this a few times, since the first merge may end up just joining two
	// tiny faces to produce a *still* tiny face.
	constexpr int max_simplification_attempts = 10;
	const int initial_tiny_faces = simplify_tiny_faces_into_their_neighbors(gMap);
	if(initial_tiny_faces)
	{
		for(int simplification_attempt = 0;
			simplify_tiny_faces_into_their_neighbors(gMap) > 0 && simplification_attempt < max_simplification_attempts;
			++simplification_attempt)
		{ }
	}

#if DEV
	const int remaining_tiny_faces = count_tiny_faces(gMap);
	if(remaining_tiny_faces && barf_on_tiny_map_faces())
	{
		fprintf(stderr, "Tiny faces: %d (had %d before our simplification attempt)\n", remaining_tiny_faces, initial_tiny_faces);
		DebugAssertWithExplanation(remaining_tiny_faces == 0, "It appears Tyler screwed up the simplification step for tiny faces (above)");
	}
#endif

	//--------------------------------------------------------------------------------------------------------
	// Prep the AGPs we will read OBJ point positions from.
	// Mobile doesn't support AGPs directly, so instead we treat the AGPs as a *spec* from which we
	// read the relative locations of a bunch of OBJs; those OBJs then get baked directly into the DSF.
	//--------------------------------------------------------------------------------------------------------
	const map<int, agp_t> agps = read_mobile_agps(); // maps terrain enum to the AGP describing its building placements
	const map<string, int> obj_tokens = register_obj_tokens_for_agps(agps); // maps agp_t::obj::name values to the global enums we register for them

	//--------------------------------------------------------------------------------------------------------
	// Place OBJs
	// This must come *after* the map merge to ensure we don't stick buildings in the water or in airport boundaries
	//--------------------------------------------------------------------------------------------------------
	// TODO: Extend out airport bounds 
	int considered_objs = 0;
	int placed = 0;
	for(auto & f : gMap.face_handles())
	{
		GIS_face_data &fd = f->data();
		const int ter_enum = ag_terrain_type(fd);
		DebugAssert(ter_enum == NO_VALUE || !f->is_unbounded());
		if(ter_enum != NO_VALUE)
		{
			const Polygon2 ben_face = cgal2ben(f, s_dsf_desc.dsf_lon, s_dsf_desc.dsf_lat); // not *that* Ben face! https://secure.gravatar.com/ben2212171
			DebugAssert(ben_face.area() > one_square_meter_in_degrees || !barf_on_tiny_map_faces());

			// Place the associated OBJs based on this tile's AGP spec
			map<int, agp_t>::const_iterator agp = agps.find(ter_enum);
			if(agp != agps.end())
			{
				const vector<Polygon2> holes = get_holes(f);
				for(const agp_t::obj & obj : agp->second.objs)
				{

					auto should_place_obj = [&](const Polygon2 & face, const vector<Polygon2> & holes, const agp_t::obj & obj, const Point2 & center, const double obj_rotation) {
						if(face.size() == 4 && holes.size() == 0 && // the OBJs *deliberately* leak out of the face bounds in Euro terrain... that's okay as long as there's no funny business going on with this tile
								flt_abs(face.area() - face.bounds().area()) < 10 * one_square_meter_in_degrees)
						{
							return true;
						}
						else if(face.contains(center)) // ensure the OBJ is completely within the bounds, and doesn't intersect any holes
						{
							const Polygon2 obj_loc = obj_placement(obj, obj_rotation, center);
							return face.contains(obj_loc) &&
									none_of(holes.begin(), holes.end(), [&](const Polygon2 & hole) { return hole.bounds().overlap(obj_loc.bounds()); });
						}
						return false;
					};

					// Is this OBJ within this face's bounds?
					// Note that mTemp1 and mTemp2 were previously set to the containing grid point's x & y
					const Point2 center_lon_lat = obj_placement_center_lon_lat(obj, agp->second, s_dsf_desc, fd.mTemp1, fd.mTemp2, fd.mRotationDeg);
					const double final_rotation_deg = dobwrap(obj.r + fd.mRotationDeg, 0, 360);
					if(should_place_obj(ben_face, holes, obj, center_lon_lat, final_rotation_deg))
					{
						GISObjPlacement_t placement;
						map<string, int>::const_iterator it = obj_tokens.find(obj.name);
						DebugAssert(it != obj_tokens.end());
						placement.mRepType = it->second;
						DebugAssert(intrange(placement.mRepType, NUMBER_OF_DEFAULT_TOKENS + 1, gTokens.size() - 1));
						placement.mLocation = center_lon_lat;
						placement.mHeading = final_rotation_deg;
						placement.mDerived = true;
						fd.mObjs.push_back(placement);
						++placed;
					}
					++considered_objs;
				}
			}
		}
	}
	printf("Placed %d objs out of %d total considered\n", placed, considered_objs);
	return 0;
}

static int MakeMinAltFiles(const vector<const char *>& args)
{
	DebugAssert(args.size() == 1);
	const string parent_output_dir = args[0];
	DebugAssert(parent_output_dir.back() == '/');

	const DEMGeo & elevation_dem(gDem[dem_Elevation]);

	const string out_file_path = stl_printf("%sEarth nav data/%+03d%+04d/%+03.0f%+04.0f.alt",
											args[0],
											latlon_bucket(elevation_dem.mSouth), latlon_bucket(elevation_dem.mWest),
											elevation_dem.mSouth, elevation_dem.mWest);

	constexpr const int ALT_POSTS = 2;
	std::array<std::array<float, ALT_POSTS>, ALT_POSTS> alts;
	for(int y = 0; y < ALT_POSTS; ++y)
	for(int x = 0; x < ALT_POSTS; ++x)
	{
		alts[x][y] = -9.9e9f;
	}

	for(int y = 0; y < elevation_dem.mHeight; ++y)
	for(int x = 0; x < elevation_dem.mWidth; ++x)
	{
		const int x_bucket = intround(interp(0, 0, elevation_dem.mWidth  - 1, ALT_POSTS - 1, x));
		const int y_bucket = intround(interp(0, 0, elevation_dem.mHeight - 1, ALT_POSTS - 1, y));
		const float p = elevation_dem.mData[x + y * elevation_dem.mHeight];
		alts[x_bucket][y_bucket] = fltmax2(alts[x_bucket][y_bucket], p);
	}

	/*
	for(int y = ALT_POSTS - 1; y >= 0; --y)
	{
		for(int x = 0; x < ALT_POSTS; ++x)
		{
			printf("% 10.0f ", alts[x][y]);
		}
		printf("\n");
	}
	*/

	FILE * alt = fopen(out_file_path.c_str(), "wb");
	DebugAssertWithExplanation(alt, "Failed to open file for writing");
	if(!alt)
		return 1;

	fprintf(alt, "A\n1000\nALT\n\n");
	for(int y = 0; y < ALT_POSTS; ++y)
	for(int x = 0; x < ALT_POSTS; ++x)
	{
		double lon = interp(0, elevation_dem.mWest,  ALT_POSTS, elevation_dem.mEast,  (float)x + 0.5f);
		double lat = interp(0, elevation_dem.mSouth, ALT_POSTS, elevation_dem.mNorth, (float)y + 0.5f);
		fprintf(alt, "MAX_ALT %.8lf %.8lf %.0f\n", lon, lat, alts[x][y]);
	}
	fclose(alt);
	return 0;
}

static constexpr GISTool_RegCmd_t s_mobile_cmds[] = {
	{ "-autogenterrain",	0, 0, DoMobileAutogenTerrain,	"Mobile 'orthophoto'-based autogen.",	"" },
	{ "-merge_ag_terrain",	0, 0, MergeTylersAg,			"Merge AG terrain into the map.",		"" },
	{ "-export_alt_file",	1, 1, MakeMinAltFiles,			"Process DEMs into Mobile's minimum altitude (.alt) files.",		"" },
	{ 0, 0, 0, 0, 0, 0 }
};



void RegisterMobileProcessingCmds()
{
	GISTool_RegisterCommands(s_mobile_cmds);
}


