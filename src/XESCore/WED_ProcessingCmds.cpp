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
#include "WED_ProcessingCmds.h"
#include "MeshAlgs.h"
#include "MemFileUtils.h"
#include "DEMAlgs.h"
#include "EuroRoads.h"
#include "ParamDefs.h"
#include "XESIO.h"
#include "Hydro.h"
#include "WED_PrefsDialog.h"
#include "XPLMMenus.h"
#include "PlatformUtils.h"
#include "MapAlgs.h"
#include "WED_Progress.h"
#include "SceneryPackages.h"
#include "WED_Msgs.h"
#include "WED_Globals.h"
#include "WED_Selection.h"
#include "WED_Notify.h"
#include "NetPlacement.h"
#include "WED_DrawMap.h"
#include "DSFBuilder.h"
#include "PerfUtils.h"
#include "Zoning.h"
#include "Airports.h"
#include "Beaches.h"
#include "WED_Assert.h"
#include "PlatformUtils.h"
#include "Forests.h"
#include "DEMTables.h"

#include "ObjPlacement.h"

extern ProcessingPrefs_t	gProcessingCmdPrefs = {
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


enum {
	procCmd_CalcSlope,
	procCmd_UpsampleEnviro,
	procCmd_HydroCorrect,
	procCmd_HydroSimplfiy,
	procCmd_DeriveDEMs,
	procCmd_AddUrbanRoads,
	procCmd_BuildRoads,
	procCmd_DoAirports,
	procCmd_DoZoning,
//	procCmd_LowResTri,
	procCmd_HiResTri,
	procCmd_AssignLUToMesh,
	procCmd_Divider1,	
	procCmd_RemoveDupes,
	procCmd_InstantiateFor,
	procCmd_InstantiateGT,
	procCmd_Divider2,
	procCmd_DoProcessing,
	procCmd_ExportDSFNew,
	procCmd_ExportDSFExisting,
	procCmd_Count	
};

const char *	kProcCmdNames [] = {
	"Calculate Terrain Slope",
	"Upsample Environment",
	"Hydro-Correct",
	"Simplify Coastlines",
	"Calculate Derived Raster Data",
	"Add Urban Roads",
	"Pick Road Types",
	"Process Airports",
	"Do Zoning",
//	"Create Low Res Mesh",
	"Create Hi Res Mesh",	
	"Apply Terrain To Mesh",
	"-",
	"Remove Duplicate Features",
	"Instantiate Forests",
	"Instantiate Face Objects",
	"-",
	"All Processing",
	"Export DSF to New Scenery Package...",
	"Export DSF to Existing Scenery Package...",
	0
};

static	const char	kCmdKeys [] = {
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
//	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,	// divider
	0,	0,
	'F',xplm_ControlFlag,
	'G',xplm_ControlFlag,
	0,	0,	// divider
	'P',xplm_ControlFlag,
	0,	0,
	0,	0,
	0,	0
};

static	XPLMMenuID	sProcessMenu = NULL;

static	void	WED_HandleProcMenuCmd(void *, void * i);
static	void 	WED_UpdateProcCmds(void);
static	void 	WED_NotifyProcCmds(int catagory, int message, void * param);

void	RegisterProcessingCommands(void)
{
	int n;
	sProcessMenu = XPLMCreateMenu("Calculate", NULL, 0, WED_HandleProcMenuCmd, NULL);
	n = 0;
	while (kProcCmdNames[n])
	{
		XPLMAppendMenuItem(sProcessMenu, kProcCmdNames[n], (void *) n, 1);
		if (kCmdKeys[n*2])
			XPLMSetMenuItemKey(sProcessMenu,n,kCmdKeys[n*2],kCmdKeys[n*2+1]);
		++n;
	}
	WED_RegisterNotifyFunc(WED_NotifyProcCmds);
	WED_UpdateProcCmds();
}

static	void	WED_HandleProcMenuCmd(void *, void * i)
{
	int cmd = (int) i;
	
	try {
		switch(cmd) {
//		case procCmd_LowResTri:
//			TriangulateMesh(gMap, gTriangulationLo, gDem, WED_ProgressFunc, false);		
//			break;
		case procCmd_HiResTri:
			TriangulateMesh(gMap, gTriangulationHi, gDem, WED_ProgressFunc);		
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_TriangleHiChange, NULL);
			break;
		case procCmd_DoAirports:
			ProcessAirports(gApts, gMap, gDem[dem_Elevation], gDem[dem_UrbanTransport], true, true, WED_ProgressFunc);
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			break;
		case procCmd_DoZoning:
			ZoneManMadeAreas(gMap, gDem[dem_LandUse], gDem[dem_Slope],gApts, WED_ProgressFunc);
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorMetaChange, NULL);
			break;
//		case procCmd_DoBeaches:
//			CreateBeaches(gMap);
//			break;
		case procCmd_UpsampleEnviro:
			UpsampleEnvironmentalParams(gDem,WED_ProgressFunc);
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			break;	
		case procCmd_CalcSlope:
			CalcSlopeParams(gDem, true, WED_ProgressFunc);
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			break;
		case procCmd_HydroCorrect:
			{
				char	buf[1024];
				buf[0] = 0;
				if (!GetFilePathFromUser(getFile_Open, "Please pick a shape file", "Preview", 6, buf)) break;
						
				HydroReconstruct(gMap,  gDem,buf,WED_ProgressFunc);
				WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);
				WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			}
			break;
		case procCmd_HydroSimplfiy:
			SimplifyCoastlines(gMap, 5000.0 / (DEG_TO_NM_LAT * DEG_TO_NM_LAT * NM_TO_MTR * NM_TO_MTR), WED_ProgressFunc);
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);
			break;
		case procCmd_DeriveDEMs:
			DeriveDEMs(gMap, gDem,WED_ProgressFunc);			
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange, NULL);
			break;
		case procCmd_AddUrbanRoads:
			{
				char	path[1024];
				path[0] = 0;
				if (!gReplacementRoads.empty())
				{
					string path = gReplacementRoads;
					MFMemFile * fi = MemFile_Open(path.c_str());
					if (fi)
					{
						Pmwx		overMap;
						ReadXESFile(fi, &overMap, NULL, NULL, NULL, WED_ProgressFunc);

                        Point2 master1, master2, slave1, slave2;
                        CalcBoundingBox(gMap, master1, master2);
                        CalcBoundingBox(overMap, slave1, slave2);
                        
                        Vector2 delta(slave1, master1);

                        for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
                        	overMap.UnindexVertex(i);

                        for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
                            i->point() += delta;

                        for (Pmwx::Vertex_iterator i = overMap.vertices_begin(); i != overMap.vertices_end(); ++i)
                        	overMap.ReindexVertex(i);
 						
						AddEuroRoads(gMap, overMap, gDem[dem_Slope], gDem[dem_LandUse], lu_usgs_URBAN_IRREGULAR, WED_ProgressFunc);
						WED_Notifiable::Notify(wed_Cat_File, wed_Msg_VectorChange, NULL);
						MemFile_Close(fi);
					}					
				}
			}
			break;
		case procCmd_RemoveDupes:
			{
				if (gFaceSelection.empty())
					RemoveDuplicatesAll(gMap, WED_ProgressFunc);
				else {
					for (set<Pmwx::Face_handle>::iterator i = gFaceSelection.begin(); i != gFaceSelection.end(); ++i)
					if (!(*i)->IsWater())
					if (!(*i)->is_unbounded())
						RemoveDuplicates(*i);
				}
			}
			break;
		case procCmd_InstantiateGT:
			{
				if (gFaceSelection.empty())
					InstantiateGTPolygonAll(gMap, gDem, gTriangulationHi, WED_ProgressFunc);
				else {
					for (set<Pmwx::Face_handle>::iterator i = gFaceSelection.begin(); i != gFaceSelection.end(); ++i)
					if (!(*i)->IsWater())
					if (!(*i)->is_unbounded())
						InstantiateGTPolygon(*i, gDem, gTriangulationHi);
					
				}
			}
			DumpPlacementCounts();
			break;
		case procCmd_InstantiateFor:
			{
				StElapsedTime	timer("Total forest time.\n");
				int c = gFaceSelection.size();
				if (c == 0)
				{
					set<GISFace *> faces;
					for (Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
					if (!f->is_unbounded())
					if (!f->IsWater())
						faces.insert(f);
					GenerateForests(gMap, faces			, gTriangulationHi, WED_ProgressFunc);
				} else {
					GenerateForests(gMap, gFaceSelection, gTriangulationHi, WED_ProgressFunc);
				}
			}
			break;
		case procCmd_BuildRoads:
			CalcRoadTypes(gMap, gDem[dem_Elevation], gDem[dem_UrbanDensity],WED_ProgressFunc);
			break;
		case procCmd_AssignLUToMesh:
			AssignLandusesToMesh(gDem,gTriangulationHi,WED_ProgressFunc);
//			AssignLandusesToMesh(gDem,gTriangulationLo,false,WED_ProgressFunc);
			WED_Notifiable::Notify(wed_Cat_File, wed_Msg_TriangleHiChange, NULL);
			break;
		case procCmd_DoProcessing:		
			if (gProcessingCmdPrefs.do_calc_slope				)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_CalcSlope	   );
			if (gProcessingCmdPrefs.do_upsample_environment		)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_UpsampleEnviro );
			if (gProcessingCmdPrefs.do_hydro_correct			)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_HydroCorrect   );
			if (gProcessingCmdPrefs.do_hydro_simplify			)	WED_HandleProcMenuCmd(NULL, (void *)		procCmd_HydroSimplfiy  );
			if (gProcessingCmdPrefs.do_derive_dems				)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_DeriveDEMs     );
			if (gProcessingCmdPrefs.do_add_urban_roads			)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_AddUrbanRoads  );
			if (gProcessingCmdPrefs.do_build_roads				)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_BuildRoads	   );
			if (gProcessingCmdPrefs.do_airports					)	WED_HandleProcMenuCmd(NULL, (void *)		procCmd_DoAirports	   );
			if (gProcessingCmdPrefs.do_zoning					)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_DoZoning	   );
			if (gProcessingCmdPrefs.do_triangulate				)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_HiResTri	   );
			if (gProcessingCmdPrefs.do_assign_landuse			)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_AssignLUToMesh );
			if (gProcessingCmdPrefs.build_3d_forests			)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_InstantiateFor );
			if (gProcessingCmdPrefs.remove_duplicate_objs		)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_RemoveDupes    );
			if (gProcessingCmdPrefs.place_buildings				)	WED_HandleProcMenuCmd(NULL, (void *) 		procCmd_InstantiateGT  );
			break;
		case procCmd_ExportDSFNew:
		case procCmd_ExportDSFExisting:
			{	

				char	buf[1024], buf2[1024];
				strcpy(buf, "New Scenery Package");
				if (GetFilePathFromUser((cmd == procCmd_ExportDSFExisting) ? getFile_PickFolder : getFile_Save, 
										(cmd == procCmd_ExportDSFExisting) ? "Please pick your scenery package" : "Please name your scenery package", 
										(cmd == procCmd_ExportDSFExisting) ? "Update" : "Create", 5, buf))
				{
					if (cmd != procCmd_ExportDSFExisting) strcat(buf, DIR_STR);
					CreatePackageForDSF(buf, (int) gDem[dem_LandUse].mWest,(int) gDem[dem_LandUse].mSouth, buf2);				
					BuildDSF(buf2, gDem[dem_LandUse],gTriangulationHi, /*gTriangulationLo,*/ gMap, WED_ProgressFunc);
				}
			}
			break;
		}
	} catch (wed_assert_fail_exception& e) {
	} catch (exception& e) {
		DoUserAlert(e.what());
	} catch (...) {
		DoUserAlert("The operation was aborted due to an unexpected internal error.");
	}
		

}

void WED_NotifyProcCmds(int catagory, int message, void * param)
{
	if (catagory == wed_Cat_File)
		WED_UpdateProcCmds();
}

void WED_UpdateProcCmds(void)
{
	bool	has_enviro = gDem.count(dem_Climate);
	bool	has_enviro_hi = has_enviro && gDem[dem_Climate].mWidth > 10;
	bool	has_slope = gDem.count(dem_Slope);
	bool	has_elev = gDem.count(dem_Elevation);
	bool	has_deriv_raster = gDem.count(dem_UrbanDensity);
	bool	has_lu = gDem.count(dem_LandUse);
//	bool	has_tri_lo = gTriangulationLo.number_of_faces() > 0;
	bool	has_tri_hi = gTriangulationHi.number_of_faces() > 0;
	bool	mesh_set = true;
	bool	has_roads = true;
	bool	has_zoning = true;	// ???
	bool	has_beaches = true; // ???
#if TODO
	This could use some more inspection!
#endif	
	for (CDT::Finite_faces_iterator f = gTriangulationHi.finite_faces_begin(); f != gTriangulationHi.finite_faces_end(); ++f)
	{
		if (f->info().terrain == NO_VALUE || f->info().terrain == terrain_Natural)
		{
			mesh_set = false;
			break;
		}
	}
	for (Pmwx::Halfedge_const_iterator e(gMap.halfedges_begin()); e != gMap.halfedges_end(); ++e)
	{
		if (!e->mSegments.empty())
		{
			has_roads = (e->mSegments.front().mRepType != NO_VALUE);
			break;
		}
	}
	
	XPLMEnableMenuItem(sProcessMenu, procCmd_CalcSlope,			has_elev);
	XPLMEnableMenuItem(sProcessMenu, procCmd_UpsampleEnviro, 	has_enviro);
	XPLMEnableMenuItem(sProcessMenu, procCmd_HydroCorrect,		has_lu && has_slope && has_enviro_hi);
	XPLMEnableMenuItem(sProcessMenu, procCmd_HydroSimplfiy,		has_lu && has_slope && has_enviro_hi);	
	XPLMEnableMenuItem(sProcessMenu, procCmd_DeriveDEMs,		has_lu && has_slope && has_enviro_hi);
	XPLMEnableMenuItem(sProcessMenu, procCmd_BuildRoads,		has_deriv_raster);
	XPLMEnableMenuItem(sProcessMenu, procCmd_DoAirports,		!gApts.empty());
	XPLMEnableMenuItem(sProcessMenu, procCmd_DoZoning,			has_roads);
//	XPLMEnableMenuItem(sProcessMenu, procCmd_DoBeaches,			has_zoning);
//	XPLMEnableMenuItem(sProcessMenu, procCmd_LowResTri,			has_deriv_raster && has_roads && has_zoning);
	XPLMEnableMenuItem(sProcessMenu, procCmd_HiResTri,			has_deriv_raster && has_roads && has_zoning);
	XPLMEnableMenuItem(sProcessMenu, procCmd_AssignLUToMesh,	/*has_tri_lo && */has_tri_hi && has_deriv_raster && has_roads);

	XPLMEnableMenuItem(sProcessMenu, procCmd_InstantiateGT,		has_roads && has_deriv_raster && has_enviro_hi && has_enviro &&& has_zoning);

	XPLMEnableMenuItem(sProcessMenu, procCmd_ExportDSFNew,		has_roads && has_deriv_raster && has_enviro_hi && has_enviro/* && has_tri_lo*/ && has_tri_hi && mesh_set);
	XPLMEnableMenuItem(sProcessMenu, procCmd_ExportDSFExisting, has_roads && has_deriv_raster && has_enviro_hi && has_enviro/* && has_tri_lo*/ && has_tri_hi && mesh_set);

	XPLMCheckMenuItem(sProcessMenu, procCmd_CalcSlope,			has_slope ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sProcessMenu, procCmd_UpsampleEnviro, 	has_enviro_hi ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sProcessMenu, procCmd_DeriveDEMs,			has_deriv_raster ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sProcessMenu, procCmd_BuildRoads,			(has_roads && has_deriv_raster) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sProcessMenu, procCmd_DoZoning,			has_zoning ? xplm_Menu_Checked : xplm_Menu_Unchecked);
//	XPLMCheckMenuItem(sProcessMenu, procCmd_DoBeaches,			has_beaches ? xplm_Menu_Checked : xplm_Menu_Unchecked);
//	XPLMCheckMenuItem(sProcessMenu, procCmd_LowResTri,			has_tri_lo ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sProcessMenu, procCmd_HiResTri,			has_tri_hi ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sProcessMenu, procCmd_AssignLUToMesh,		(mesh_set && has_tri_hi) ? xplm_Menu_Checked : xplm_Menu_Unchecked);


}

