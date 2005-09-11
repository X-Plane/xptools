// Mesh generation prefs

PREFS_KEY_INT  ("MESH",	"MAX_MOUNTAIN_POINTS", 	gMeshPrefs.max_mountain_points)
PREFS_KEY_FLOAT("MESH",	"CLIFF_HEIGHT", 		gMeshPrefs.cliff_height)
PREFS_KEY_FLOAT("MESH",	"MAX_ERROR", 			gMeshPrefs.max_error)
PREFS_KEY_INT  ("MESH",	"FOWLER_LITTLE", 		gMeshPrefs.fowler_little)
PREFS_KEY_FLOAT("MESH", "REP_BLOB_SIZE",		gMeshPrefs.rep_switch_m)

// DEM Prefs

PREFS_KEY_INT  ("DEM",	"LOCAL_RANGE", 			gDemPrefs.local_range)

// Viewing Prefs

PREFS_KEY_INT  ("VIEW",	"SHOW_MAP",			sShowMap)
PREFS_KEY_INT  ("VIEW",	"SHOW_MESH_HI",		sShowMeshTrisHi)
PREFS_KEY_INT  ("VIEW",	"SHOW_MESH_ALPHA",	sShowMeshAlphas)
PREFS_KEY_INT  ("VIEW",	"SHOW_AIRPORTS",	sShowAirports)
PREFS_KEY_INT  ("VIEW",	"SHOW_SHADING",		sShowShading)
PREFS_KEY_FLOAT("VIEW",	"SUN_AZIMUTH",		sShadingAzi)
PREFS_KEY_FLOAT("VIEW",	"SUN_DECLINATION",	sShadingDecl)

// General Processing steps and overall config

PREFS_KEY_INT	("PROCESSING", "HYDRO_CORRECT", gWedPrefs.hydro_correct)
PREFS_KEY_INT	("PROCESSING", "HYDRO_SIMPLIFY", gWedPrefs.hydro_simplify)
PREFS_KEY_INT	("PROCESSING", "BORDER_MATCH", gMeshPrefs.border_match)

PREFS_KEY_STR	("PROCESSING", "TERRAIN_SPREADSHEET", gNaturalTerrainFile)
PREFS_KEY_STR	("PROCESSING", "LANDUSE_FILE", gLanduseTransFile)

// ROADS

