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
#include "WED_FileCommands.h"
#include "XPLMMenus.h"
#include "DEMTables.h"
#include "ObjTables.h"
#include "XESIO.h"
#include "PlatformUtils.h"
#include "AptIO.h"
#include "MemFileUtils.h"
#include "WED_Notify.h"
#include "MapAlgs.h"
#include "WED_Msgs.h"
#include "WED_Globals.h"
#include "WED_Selection.h"
#include "WED_Progress.h"
#include "WED_Import.h"
#include "WED_Export.h"
#include "WED_PrefsDialog.h"
#include "WED_Assert.h"
#include "PlatformUtils.h"

/****************************************************************************************
 * FILE MENU UI
 ****************************************************************************************/


enum {
	fileCmd_New,
	fileCmd_Open,
	fileCmd_OpenApt,
	fileCmd_OpenSpreadsheet,
	fileCmd_OpenLandUse,
	fileCmd_OpenClimateXES,
	fileCmd_OpenRoadXES,
	fileCmd_OpenObjSpreadsheet,
	fileCmd_Sep1,
	fileCmd_Save,
	fileCmd_SaveAs,
	fileCmd_Revert,
	fileCmd_Sep2,
	fileCmd_Import,
	fileCmd_Export,
	fileCmd_Sep3,
	fileCmd_Prefs,
	fileCmd_Count
};

const char *	kFileCmdNames [] = {
	"New",
	"Open...",
	"Open Airport...",
	"Open Terrain Spreadsheet...",
	"Open LandUse Translation...",
	"Pick Climate XES...",
	"Pick Road XES...",
	"Pick OBj Spreadsheet..",
	"-",
	"Save",
	"Save As...",
	"Revert...",
	"-",
	"Import...",
	"Export...",
	"-",
	"Preferences...",
	0
};

static	const char	kCmdKeys [] = {
	'N',	xplm_ControlFlag,
	'O',	xplm_ControlFlag,
	0,		0,
	'O',	xplm_ControlFlag + xplm_OptionAltFlag,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	0,		0,
	'S',	xplm_ControlFlag,
	'S',	xplm_ControlFlag + xplm_ShiftFlag,
	0,		0,
	0,		0,	// divider
	'I',	xplm_ControlFlag,
	'E',	xplm_ControlFlag,
	0,		0,	// Divider,
	',',	xplm_ControlFlag,
	0,		0
};

static	XPLMMenuID	sFileMenu = NULL;

static	void	WED_HandleFileMenuCmd(void *, void * i);
static	void	WED_NotifyFileMenus(int catagory, int message, void * param);
static	void	WED_RecalcFileMenus(void);
static 	bool	DoSaveIfNeeded(void);


bool	DoSaveIfNeeded(void)
{
#if TODO
FIX THIS!
#endif
	return true;
}

void	RegisterFileCommands(void)
{
	int n;
	sFileMenu = XPLMCreateMenu("File", NULL, 0, WED_HandleFileMenuCmd, NULL);
	n = 0;
	while (kFileCmdNames[n])
	{
		XPLMAppendMenuItem(sFileMenu, kFileCmdNames[n], (void *) n, 1);
		if (kCmdKeys[n*2])
			XPLMSetMenuItemKey(sFileMenu,n,kCmdKeys[n*2],kCmdKeys[n*2+1]);
		++n;
	}
	WED_RegisterNotifyFunc(WED_NotifyFileMenus);
	WED_RecalcFileMenus();
}

static	void	WED_HandleFileMenuCmd(void *, void * i)
{
	try {
		int cmd = (int) i;
		switch(cmd) {
		case fileCmd_New:
			if (!DoSaveIfNeeded())	return;
			WED_FileNew();
			break;
		case fileCmd_Open:
			if (!DoSaveIfNeeded())	return;
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick an .XES file to open", "Open", 1, buf,sizeof(buf))) return;
				WED_FileOpen(buf);
			}
			break;
		case fileCmd_OpenApt:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick an apt.dat file to open", "Open", 10, buf,sizeof(buf))) return;
				gApts.clear();
				string err = ReadAptFile(buf, gApts);
				if (!err.empty())
					DoUserAlert(err.c_str());
				else
					IndexAirports(gApts,gAptIndex);
				WED_Notifiable::Notify(wed_Cat_File, wed_Msg_AirportsLoaded, NULL);
			}
			break;
		case fileCmd_OpenSpreadsheet:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick a spreadsheet", "Open", 6, buf,sizeof(buf))) return;
				gNaturalTerrainFile = buf;
				LoadDEMTables();
			}
			break;

		case fileCmd_OpenLandUse:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick a landuse translaion file", "Open", 7, buf,sizeof(buf))) return;
				gLanduseTransFile = buf;
				LoadDEMTables();
			}
			break;

		case fileCmd_OpenClimateXES:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick a climate file", "Open", 8, buf,sizeof(buf))) { gReplacementClimate.clear(); return; }
				gReplacementClimate = buf;
			}
			break;

		case fileCmd_OpenRoadXES:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick a road file", "Open", 8, buf,sizeof(buf))) { gReplacementRoads.clear(); return; }
				gReplacementRoads = buf;
			}
			break;

		case fileCmd_OpenObjSpreadsheet:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick an object spreadsheet", "Open", 26, buf,sizeof(buf))) return;
				gObjPlacementFile = buf;
				LoadObjTables();
			}
			break;

		case fileCmd_Save:
			if (gDirty)
			{
				if (gFilePath.empty())
				{
					char buf[1024];
					buf[0] = 0;
					if (!GetFilePathFromUser(getFile_Save, "Please name your .XES file", "Save", 1, buf,sizeof(buf))) return;
					gFilePath = buf;
				}
				WED_FileSave();
			}
			break;
		case fileCmd_SaveAs:
			{
				char buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Save, "Please rename your .XES file", "Save As", 1, buf,sizeof(buf))) return;
				gFilePath = buf;
				gDirty = true;
				WED_FileSave();
			}
			break;
		case fileCmd_Revert:
			if (gDirty && !gFilePath.empty())
			{
				char	buf[1024];
				sprintf(buf, "Are you sure you want to revert the file '%s'?  you will lose all changes since you last saved.",
					gFilePath.c_str());
				if (ConfirmMessage(buf, "Revert", "Cancel"))
					WED_FileOpen(gFilePath);
			}
			break;
		case fileCmd_Import:
			WED_ShowImportDialog();
			break;
		case fileCmd_Export:
			WED_ShowExportDialog();
			break;
		case fileCmd_Prefs:
			WED_ShowPrefsDialog();
			break;
		}
	} catch (wed_assert_fail_exception& e) {
	} catch (exception& e) {
		DoUserAlert(e.what());
	} catch (...) {
		DoUserAlert("The operation was aborted due to an unexpected internal error.");
	}
}

void	WED_NotifyFileMenus(int catagory, int message, void * param)
{
	switch(catagory) {
	case wed_Cat_File:
		WED_RecalcFileMenus();
		break;
	}
}

void	WED_RecalcFileMenus(void)
{
	XPLMSetMenuItemName(sFileMenu, fileCmd_Save, gFilePath.empty() ? "Save..." : "Save", 1);
	XPLMEnableMenuItem(sFileMenu,fileCmd_Save,   gDirty);
	XPLMEnableMenuItem(sFileMenu,fileCmd_Revert, gDirty);
}

/****************************************************************************************
 * FILE COMMAND ALGORITHMS
 ****************************************************************************************/

#define 	MAP_BUCKET_DEPTH	8

void	WED_FileNew(void)
{
	gMap.clear();
	gMap.is_valid();
	gDem.clear();
	gMeshPoints.clear();
	gMeshLines.clear();
	gFilePath.clear();
	gDirty = false;
//	gTriangulationLo.clear();
	gTriangulationHi.clear();
	gFaceSelection.clear();
	gEdgeSelection.clear();
	gVertexSelection.clear();
	gPointFeatureSelection.clear();

	float mapWest = -180.0;
	float mapSouth = -90.0;
	float mapEast = 180.0;
	float mapNorth = 90.0;
	

//	gMap.Index();

	WED_Notifiable::Notify(wed_Cat_File, wed_Msg_FileLoaded, NULL);
}

bool	WED_FileOpen(const string& inPath)
{
	MFMemFile *	memFile = MemFile_Open(inPath.c_str());
	if (memFile)
	{
		gMap.clear();
		gDem.clear();
		gMeshPoints.clear();
		gMeshLines.clear();
		gFilePath.clear();
		gDirty = false;
//		gTriangulationLo.clear();
		gTriangulationHi.clear();
		gFaceSelection.clear();
		gEdgeSelection.clear();
		gVertexSelection.clear();
		gPointFeatureSelection.clear();

		ReadXESFile(memFile, &gMap, &gTriangulationHi, &gDem, &gApts, WED_ProgressFunc);
		IndexAirports(gApts, gAptIndex);
		MemFile_Close(memFile);
	} else
		return false;

	Point_2	sw, ne;
	CalcBoundingBox(gMap, sw, ne);
	double mapNorth = CGAL::to_double(ne.y());
	double mapSouth = CGAL::to_double(sw.y());
	double mapEast = CGAL::to_double(ne.x());
	double mapWest = CGAL::to_double(sw.x());

//	ReduceToWaterBodies(gMap);
/*
	for (Pmwx::Vertex_iterator vit = gMap.vertices_begin(); vit != gMap.vertices_end(); ++vit)
	{
		printf("Vertex at: %lf,%lf\n", CGAL::to_double(vit->point().x()),CGAL::to_double(vit->point().y()));
		Pmwx::Halfedge_around_vertex_const_circulator circ = vit->incident_halfedges();
		Pmwx::Halfedge_around_vertex_const_circulator start = circ;
		Pmwx::Halfedge_around_vertex_const_circulator prev, next;
		bool                                    eq1, eq2;


			CGAL::Arr_traits_basic_adaptor_2<Traits_2>	traits;
			CGAL::Arr_traits_basic_adaptor_2<Traits_2>::Is_between_cw_2  is_between_cw = traits.is_between_cw_2_object();

		do
		{
			prev = circ; --prev;
			next = circ; ++next;

			printf("Testing these 3 curves:\n"
					"%lf,%lf->%lf,%lf\n"
					"%lf,%lf->%lf,%lf\n"
					"%lf,%lf->%lf,%lf\n",
						CGAL::to_double(prev->source()->point().x()),CGAL::to_double(prev->source()->point().y()),
						CGAL::to_double(prev->target()->point().x()),CGAL::to_double(prev->target()->point().y()),

						CGAL::to_double(circ->source()->point().x()),CGAL::to_double(circ->source()->point().y()),
						CGAL::to_double(circ->target()->point().x()),CGAL::to_double(circ->target()->point().y()),

						CGAL::to_double(next->source()->point().x()),CGAL::to_double(next->source()->point().y()),
						CGAL::to_double(next->target()->point().x()),CGAL::to_double(next->target()->point().y()));

			if(prev != next)
			if (!is_between_cw (circ->curve(), (circ->direction() == CGAL::LARGER),
								prev->curve(), (prev->direction() == CGAL::LARGER),
								next->curve(), (next->direction() == CGAL::LARGER),
								vit->point(), eq1, eq2))

			{
				printf("Failed test...eq: %s. eq2: %s\n", eq1 ? "yes" : "no", eq2 ? "yes" : "no");
			}

				++circ;
				
			
		} while (circ != start);
		
		if (!gMap._is_valid (vit))
			Assert(!"Problem!");
	}
*/
	if (!gMap.is_valid())
	{
		gMap.clear();
		gFilePath.clear();
		gDirty = false;
		return false;;
	}

	gFilePath = inPath;
	gDirty = false;

	WED_Notifiable::Notify(wed_Cat_File, wed_Msg_FileLoaded, NULL);
	return true;
}

void	WED_FileSave(void)
{
	if (gDirty && !gFilePath.empty())
	{
#if TODO
	TODO SAFE FILE SAVE AND SWAP!
#endif
		WriteXESFile(gFilePath.c_str(), gMap, gTriangulationHi, gDem, gApts, WED_ProgressFunc);
		gDirty = false;
		WED_RecalcFileMenus();
	}
}
