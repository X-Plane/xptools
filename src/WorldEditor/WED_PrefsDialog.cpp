#include "WED_PrefsDialog.h"
#include "XPWidgetDialogs.h"
#include "XPWidgets.h"
#include "MemFileUtils.h"
#include "ConfigSystem.h"
#include "AssertUtils.h"

#include "WED_MapView.h"
#include "MeshAlgs.h"
#include "DEMAlgs.h"
#include "DEMTables.h"

#define PREFS_FILE	"WorldEditor.prf"

static XPWidgetID		sPrefsDialog = NULL;

static	int				sPrefsTab = 0;
static	int				sTest1 = 0;
static	int				sTest2 = 0;

WED_Prefs	gWedPrefs = { 0, 0 };

#define	TAG_MAX_MOUNTAINS 1001
#define TAG_CLIFF_HEIGHT  1002
#define TAG_FOWLER_LITTLE 1003

static void DoFowlerLittle(XPWidgetID);
void	WED_ShowPrefsDialog(void)
{
	if (sPrefsDialog == NULL)
	{
		sPrefsDialog = XPCreateWidgetLayout(
			0, XP_DIALOG_BOX, "Preferences", XP_DIALOG_CLOSEBOX, 1, 0, NULL,
				XP_COLUMN,
					XP_TABS, "General;View;Mesh;DEMs", &sPrefsTab,
						XP_COLUMN,
							XP_ROW, XP_CHECKBOX, "Hydro-Correct Maps", &gWedPrefs.hydro_correct, XP_END,
							XP_ROW, XP_CHECKBOX, "Simplify Coastlines", &gWedPrefs.hydro_simplify, XP_END,
							XP_ROW, XP_CHECKBOX, "Match Borders", &gMeshPrefs.border_match, XP_END,
						XP_END,
						XP_COLUMN,
							XP_ROW, XP_CAPTION, "Sun Azimuth", XP_EDIT_FLOAT, 6, 6, 0, &sShadingAzi, XP_END,
							XP_ROW, XP_CAPTION, "Sun Declination", XP_EDIT_FLOAT, 6, 6, 0, &sShadingDecl, XP_END,
						XP_END,
						XP_COLUMN,
							XP_ROW, XP_CAPTION, "Max Mountain Points:", XP_EDIT_INT, 15, 6, &gMeshPrefs.max_mountain_points, XP_TAG, TAG_MAX_MOUNTAINS, XP_END,
							XP_ROW, XP_CAPTION, "Cliff Height:", XP_EDIT_FLOAT, 15, 6, 1, &gMeshPrefs.cliff_height, XP_TAG, TAG_CLIFF_HEIGHT, XP_END,
							XP_ROW, XP_CAPTION, "Max Error:", XP_EDIT_FLOAT, 15, 6, 1, &gMeshPrefs.max_error, XP_END,
							XP_ROW, XP_CHECKBOX, "Use Fowler-Little", &gMeshPrefs.fowler_little, XP_NOTIFY, DoFowlerLittle, XP_TAG, TAG_FOWLER_LITTLE, XP_END,
						XP_END,
						XP_COLUMN,
							XP_ROW, XP_CAPTION, "Local Area Search:", XP_EDIT_INT, 5, 5, &gDemPrefs.local_range, XP_END,
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
	DoFowlerLittle(sPrefsDialog);
}

void DoFowlerLittle(XPWidgetID)
{
	bool old = gMeshPrefs.fowler_little;
	XPDataFromItem(sPrefsDialog, TAG_FOWLER_LITTLE);
	XPEnableByTag(sPrefsDialog, TAG_MAX_MOUNTAINS, !gMeshPrefs.fowler_little);
	XPEnableByTag(sPrefsDialog, TAG_CLIFF_HEIGHT, !gMeshPrefs.fowler_little);
	gMeshPrefs.fowler_little = old;
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
					(*sec)[tok[0]] = tok[1];
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

