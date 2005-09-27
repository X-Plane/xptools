// Mesh generation prefs

PREFS_KEY_INT  ("MESH",	"MAX_MOUNTAIN_POINTS", 	gMeshPrefs.max_mountain_points)
PREFS_KEY_FLOAT("MESH",	"CLIFF_HEIGHT", 		gMeshPrefs.cliff_height)
PREFS_KEY_FLOAT("MESH",	"MAX_ERROR", 			gMeshPrefs.max_error)
PREFS_KEY_INT  ("MESH",	"FOWLER_LITTLE", 		gMeshPrefs.fowler_little)
PREFS_KEY_FLOAT("MESH", "REP_BLOB_SIZE",		gMeshPrefs.rep_switch_m)

// DEM Prefs

PREFS_KEY_INT  ("DEM",	"LOCAL_RANGE", 			gDemPrefs.local_range)
PREFS_KEY_FLOAT("DEM",	"TEMP_PERCENTILE",		gDemPrefs.temp_percentile)
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

PREFS_KEY_STR	("PROCESSING", "TERRAIN_SPREADSHEET", gNaturalTerrainFile)
PREFS_KEY_STR	("PROCESSING", "LANDUSE_FILE", gLanduseTransFile)

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
