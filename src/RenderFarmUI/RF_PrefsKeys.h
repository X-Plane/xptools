/*
 * Copyright (c) 2007, Laminar Research.
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

// Mesh generation prefs

PREFS_KEY_INT  ("MESH",	"MAX_POINTS", 			gMeshPrefs.max_points)
PREFS_KEY_FLOAT("MESH",	"MAX_ERROR", 			gMeshPrefs.max_error)
PREFS_KEY_FLOAT("MESH", "REP_BLOB_SIZE",		gMeshPrefs.rep_switch_m)
PREFS_KEY_FLOAT("MESH", "MAX_TRI_SIZE_M",		gMeshPrefs.max_tri_size_m)

// DEM Prefs

PREFS_KEY_INT  ("DEM",	"LOCAL_RANGE", 			gDemPrefs.local_range)
PREFS_KEY_FLOAT("DEM",	"TEMP_PERCENTILE",		gDemPrefs.temp_percentile)
PREFS_KEY_FLOAT("DEM",	"RAIN_DISTURB",			gDemPrefs.rain_disturb)
// Viewing Prefs

PREFS_KEY_INT  ("VIEW",	"SHOW_MAP",			sShowMap)
PREFS_KEY_INT  ("VIEW",	"SHOW_MESH_HI",		sShowMeshTrisHi)
PREFS_KEY_INT  ("VIEW",	"SHOW_MESH_ALPHA",	sShowMeshAlphas)
PREFS_KEY_INT  ("VIEW",	"SHOW_AIRPORTS",	sShowAirports)
PREFS_KEY_INT  ("VIEW",	"SHOW_SHADING",		sShowShading)
PREFS_KEY_FLOAT("VIEW",	"SUN_AZIMUTH",		sShadingAzi)
PREFS_KEY_FLOAT("VIEW",	"SUN_DECLINATION",	sShadingDecl)

// General Processing steps and overall config

PREFS_KEY_INT	("PROCESSING", "BORDER_MATCH", gMeshPrefs.border_match)
PREFS_KEY_INT	("PROCESSING", "OPTIMIZE_BORDERS", gMeshPrefs.optimize_borders)

PREFS_KEY_STR	("PROCESSING", "TERRAIN_SPREADSHEET", gNaturalTerrainFile)
PREFS_KEY_STR	("PROCESSING", "LANDUSE_FILE", gLanduseTransFile)
PREFS_KEY_STR	("PROCESSING", "REPLACE_CLIMATE", gReplacementClimate)
PREFS_KEY_STR	("PROCESSING", "ADD_ROADS", gReplacementRoads)

PREFS_KEY_STR	("PROCESSING", "OBJ_SPREADSHEET", gObjPlacementFile)

PREFS_KEY_INT	("PROCESSING", "DO_UPSAMPLE_ENVIRONMENT"	,gProcessingCmdPrefs.do_upsample_environment)
PREFS_KEY_INT	("PROCESSING", "DO_CALC_SLOPE"				,gProcessingCmdPrefs.do_calc_slope)
PREFS_KEY_INT	("PROCESSING", "DO_HYDRO_CORRECT"			,gProcessingCmdPrefs.do_hydro_correct)
PREFS_KEY_INT	("PROCESSING", "DO_HYDRO_SIMPLIFY"			,gProcessingCmdPrefs.do_hydro_simplify)
PREFS_KEY_INT	("PROCESSING", "DO_DERIVE_DEMS"				,gProcessingCmdPrefs.do_derive_dems)
PREFS_KEY_INT	("PROCESSING", "DO_ADD_URBAN_ROADS"			,gProcessingCmdPrefs.do_add_urban_roads)
PREFS_KEY_INT	("PROCESSING", "DO_BUILD_ROADS"				,gProcessingCmdPrefs.do_build_roads)
PREFS_KEY_INT	("PROCESSING", "DO_CUT_AIRPORTS"			,gProcessingCmdPrefs.do_airports)
PREFS_KEY_INT	("PROCESSING", "DO_ZONING"					,gProcessingCmdPrefs.do_zoning)
PREFS_KEY_INT	("PROCESSING", "DO_MESH_TRIANGULATE"		,gProcessingCmdPrefs.do_triangulate)
PREFS_KEY_INT	("PROCESSING", "DO_ASSIGN_LANDUSE"			,gProcessingCmdPrefs.do_assign_landuse)
PREFS_KEY_INT	("PROCESSING", "DO_REMOVE_DUPE_OBJS"		,gProcessingCmdPrefs.remove_duplicate_objs)
PREFS_KEY_INT	("PROCESSING", "DO_3D_FORESTS"				,gProcessingCmdPrefs.build_3d_forests)
PREFS_KEY_INT	("PROCESSING", "DO_PLACE_BUILDINGS"			,gProcessingCmdPrefs.place_buildings)

PREFS_KEY_INT	("DSF_EXPORT", "EXPORT_ROADS",				gDSFBuildPrefs.export_roads)

PREFS_KEY_FLOAT	("ROADS",	"ELEV_WEIGHT",					gRoadPrefs.elevation_weight)
PREFS_KEY_FLOAT	("ROADS",	"RADIAL_WEIGHT",				gRoadPrefs.radial_weight)
PREFS_KEY_FLOAT	("ROADS",	"SLOPE_AMP",					gRoadPrefs.slope_amp)
PREFS_KEY_FLOAT	("ROADS",	"DENSITY_AMP",					gRoadPrefs.density_amp)
