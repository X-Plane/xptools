#include "WED_PrefsDialog.h"
#include "XPWidgetDialogs.h"
#include "XPWidgets.h"
#include "MemFileUtils.h"
#include "ConfigSystem.h"
#include "AssertUtils.h"

#include "WED_MapView.h"
#include "MeshAlgs.h"
#include "DEMAlgs.h"
#include "ObjTables.h"
#include "DEMTables.h"
#include "WED_ProcessingCmds.h"
#include "DSFBuilder.h"
#define PREFS_FILE	"WorldEditor.prf"

static XPWidgetID		sPrefsDialog = NULL;

static	int				sPrefsTab = 0;
static	int				sTest1 = 0;
static	int				sTest2 = 0;

WED_Prefs	gWedPrefs = { 0 };

#define TAG_COMMAND_PREFS 1004

static void RestoreDefaultProcessing(XPWidgetID);
void	WED_ShowPrefsDialog(void)
{
	if (sPrefsDialog == NULL)
	{
		sPrefsDialog = XPCreateWidgetLayout(
			0, XP_DIALOG_BOX, "Preferences", XP_DIALOG_CLOSEBOX, 1, 0, NULL,
				XP_COLUMN,
					XP_TABS, "Processing;View;Mesh;DEMs;DSF", &sPrefsTab,
						XP_COLUMN, XP_TAG, TAG_COMMAND_PREFS,
							XP_ROW, XP_CHECKBOX,"Calculate Terrain Slope", 		&gProcessingCmdPrefs.do_calc_slope, 			XP_END,
							XP_ROW, XP_CHECKBOX,"Upsample Environment", 		&gProcessingCmdPrefs.do_upsample_environment, 	XP_END,
							XP_ROW, XP_CHECKBOX,"Hydro-Correct", 				&gProcessingCmdPrefs.do_hydro_correct, 			XP_END,
							XP_ROW, XP_CHECKBOX,"Simplify Coastlines", 			&gProcessingCmdPrefs.do_hydro_simplify, 		XP_END,
							XP_ROW, XP_CHECKBOX,"Calculate Derived Raster Data",&gProcessingCmdPrefs.do_derive_dems, 			XP_END,
							XP_ROW, XP_CHECKBOX,"Add Urban Roads",				&gProcessingCmdPrefs.do_add_urban_roads, 		XP_END,
							XP_ROW, XP_CHECKBOX,"Pick Road Types", 				&gProcessingCmdPrefs.do_build_roads, 			XP_END,
							XP_ROW, XP_CHECKBOX,"Process Airports", 			&gProcessingCmdPrefs.do_airports, 				XP_END,
							XP_ROW, XP_CHECKBOX,"Do Zoning", 					&gProcessingCmdPrefs.do_zoning, 				XP_END,
							XP_ROW, XP_CHECKBOX,"Create Hi Res Mesh", 			&gProcessingCmdPrefs.do_triangulate, 			XP_END,
							XP_ROW, XP_CHECKBOX,"Apply Terrain To Mesh", 		&gProcessingCmdPrefs.do_assign_landuse, 		XP_END,
							XP_ROW, XP_CHECKBOX,"Instantiate Forests",	 		&gProcessingCmdPrefs.remove_duplicate_objs, 	XP_END,
							XP_ROW, XP_CHECKBOX,"Remove Duplicate Features", 	&gProcessingCmdPrefs.build_3d_forests, 			XP_END,
							XP_ROW, XP_CHECKBOX,"Instantiate Face Objects", 	&gProcessingCmdPrefs.place_buildings, 			XP_END,
							XP_ROW, XP_BUTTON_ACTION, "Restore Default Processing Options", RestoreDefaultProcessing, XP_END,
						XP_END,
						XP_COLUMN,
							XP_ROW, XP_CAPTION, "Sun Azimuth", XP_EDIT_FLOAT, 6, 6, 0, &sShadingAzi, XP_END,
							XP_ROW, XP_CAPTION, "Sun Declination", XP_EDIT_FLOAT, 6, 6, 0, &sShadingDecl, XP_END,
						XP_END,
						XP_COLUMN,
							XP_ROW, XP_CAPTION, "Max Points:", XP_EDIT_INT, 15, 6, &gMeshPrefs.max_points, XP_END,
							XP_ROW, XP_CAPTION, "Max Error:", XP_EDIT_FLOAT, 15, 6, 1, &gMeshPrefs.max_error, XP_END,
							XP_ROW, XP_CAPTION, "Max Tri Size m:", XP_EDIT_FLOAT, 15, 6, 1, &gMeshPrefs.max_tri_size_m, XP_END,
							XP_ROW, XP_CAPTION, "Change Tex Length", XP_EDIT_FLOAT, 15, 6, 1, &gMeshPrefs.rep_switch_m, XP_END,
							XP_ROW, XP_CHECKBOX, "Match Borders", &gMeshPrefs.border_match, XP_END,
							XP_ROW, XP_CHECKBOX, "Optimize Transition Tris", &gMeshPrefs.optimize_borders, XP_END,
						XP_END,
						XP_COLUMN,
							XP_ROW, XP_CAPTION, "Local Area Search(1-8):", XP_EDIT_INT, 5, 5, &gDemPrefs.local_range, XP_END,
							XP_ROW, XP_CAPTION, "Temperature Elevation Calibration(0-1):", XP_EDIT_FLOAT, 5, 5, 1, &gDemPrefs.temp_percentile, XP_END,
							XP_ROW, XP_CAPTION, "Rain Variation(0-1):", XP_EDIT_FLOAT, 5, 5, 1, &gDemPrefs.rain_disturb, XP_END,
						XP_END,
						XP_COLUMN,
							XP_ROW, XP_CHECKBOX, "Export Roads", &gDSFBuildPrefs.export_roads, XP_END,
						XP_END,
					XP_END,
					XP_ROW, XP_BUTTON_CANCEL, "Cancel", XP_BUTTON_OK, "OK", XP_END,
				XP_END,
			XP_END);		
	}
	if (XPIsWidgetVisible(sPrefsDialog))
		XPBringRootWidgetToFront(sPrefsDialog);
	else {
		XPSendMessageToWidget(sPrefsDialog, xpMsg_DataToDialog, xpMode_Recursive, 0, 0);
		XPShowWidget(sPrefsDialog);
	}
}

#pragma mark -

typedef map<string, string>		IniSection;
typedef map<string, IniSection>	IniSectionMap;

static bool SaveIniSectionMap(const char * inFileName, const IniSectionMap& inMap)
{
	FILE * fi = fopen(inFileName, "w");
	if (fi == NULL) return false;
	for (IniSectionMap::const_iterator sec = inMap.begin(); sec != inMap.end(); ++sec)
	{
		fprintf(fi,"[%s]" CRLF, sec->first.c_str());
		for (IniSection::const_iterator kvp = sec->second.begin(); kvp != sec->second.end(); ++kvp)
			fprintf(fi,"%s=%s" CRLF, kvp->first.c_str(), kvp->second.c_str());
		fprintf(fi,CRLF);
	}
	fclose(fi);
	return true;
}

static	bool ToVec(const char * inBegin, const char * inEnd, void * inRef)
{
	vector<string> * vec = (vector<string> *) inRef;
	vec->push_back(string(inBegin, inEnd));
	return true;
}

static bool LoadIniSectionMap(const char * inFileName, IniSectionMap& outMap)
{
	outMap.clear();
	MFMemFile * fi = MemFile_Open(inFileName);
	if (fi == NULL) return false;
	MFTextScanner * sc = TextScanner_Open(fi);
	IniSection * sec = NULL;
	while (!TextScanner_IsDone(sc))
	{
		vector<string>	tok;
		TextScanner_TokenizeLine(sc, "=","\r\n", 2, ToVec, &tok);
		
		if (!tok.empty())
		{
			Assert(!tok[0].empty());
			if (tok[0][0] == '[')
			{
				string key = tok[0];
				key.erase(0, 1);
				key.erase(key.size()-1, 1);
				outMap[key] = IniSection();
				sec = &outMap[key];
			} else {
				if (sec)
				{
					if (tok.size() > 1)
						(*sec)[tok[0]] = tok[1];
					else
						(*sec)[tok[0]] = string();
				}
			}
		}
		
		TextScanner_Next(sc);
	}
	TextScanner_Close(sc);
	MemFile_Close(fi);
	return true;
}

static void KeyToFloat(IniSectionMap& inMap, const char * sec, const  char * key, float& v)
{
	if (inMap.count(sec) == 0) return;
	if (inMap[sec].count(key) == 0) return;
	v = atof(inMap[sec][key].c_str());
}

static void KeyToInt(IniSectionMap& inMap, const char * sec, const  char * key, int& v)
{
	if (inMap.count(sec) == 0) return;
	if (inMap[sec].count(key) == 0) return;
	v = atoi(inMap[sec][key].c_str());
}

static void KeyToStr(IniSectionMap& inMap, const char * sec, const  char * key, string& v)
{
	if (inMap.count(sec) == 0) return;
	if (inMap[sec].count(key) == 0) return;
	v = inMap[sec][key];
}

static void FloatToKey(IniSectionMap& inMap, const char * sec, const  char * key, float& v)
{
	char buf[64];
	sprintf(buf,"%f", v);
	inMap[sec][key] = buf;
}

static void IntToKey(IniSectionMap& inMap, const char * sec, const  char * key, int& v)
{
	char buf[64];
	sprintf(buf,"%d", v);
	inMap[sec][key] = buf;
}

static void StrToKey(IniSectionMap& inMap, const char * sec, const  char * key, string& v)
{
	inMap[sec][key] = v;
}

void	WED_LoadPrefs(void)
{
	IniSectionMap	prefs;
	string path = FindConfigFile(PREFS_FILE);
	if (LoadIniSectionMap(path.c_str(), prefs))
	{
		#define PREFS_KEY_INT(_S, _K, _V)	KeyToInt(prefs, _S, _K, _V);
		#define PREFS_KEY_FLOAT(_S, _K, _V)	KeyToFloat(prefs, _S, _K, _V);
		#define PREFS_KEY_STR(_S, _K, _V)	KeyToStr(prefs, _S, _K, _V);
		#include "WED_PrefsKeys.h"
		#undef PREFS_KEY_INT
		#undef PREFS_KEY_FLOAT
		#undef PREFS_KEY_STR
	}
}

void	WED_SavePrefs(void)
{
	IniSectionMap	prefs;

	#define PREFS_KEY_INT(_S, _K, _V)	IntToKey(prefs, _S, _K, _V);
	#define PREFS_KEY_FLOAT(_S, _K, _V)	FloatToKey(prefs, _S, _K, _V);
	#define PREFS_KEY_STR(_S, _K, _V) StrToKey(prefs, _S, _K, _V);
	#include "WED_PrefsKeys.h"
	#undef PREFS_KEY_INT
	#undef PREFS_KEY_FLOAT
	#undef PREFS_KEY_STR

	string path = FindConfigFile(PREFS_FILE);
	SaveIniSectionMap(path.c_str(), prefs);
}



static void RestoreDefaultProcessing(XPWidgetID)
{
	ProcessingPrefs_t temp = gProcessingCmdPrefs;

	ProcessingPrefs_t	defs = {
	/*		do_calc_slope			*/			1,
	/*		do_upsample_environment	*/			1,
	/*		do_hydro_correct		*/			0,
	/*		do_hydro_simplify		*/			0,
	/*		do_derive_dems			*/			1,
	/*		do_add_urban_roads		*/			0,
	/*		do_build_roads			*/			1,
	/*		do_airports				*/			1,
	/*		do_zoning				*/			1,
	/*		do_triangulate			*/			1,
	/*		do_assign_landuse		*/			1,
	/*		remove_duplicate_objs	*/			0,
	/*		place_buildings			*/			0,
	/*		build_3d_forests		*/			0 };	

	gProcessingCmdPrefs = defs;
	
	XPDataToItem(sPrefsDialog, 	TAG_COMMAND_PREFS);
	
	gProcessingCmdPrefs = temp;
	
}