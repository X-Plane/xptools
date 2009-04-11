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
#include "RF_ProcessingCmds.h"
#include "MeshAlgs.h"
#include "MemFileUtils.h"
#include "DEMAlgs.h"
#include "TensorRoads.h"
#include "EuroRoads.h"
#include "ParamDefs.h"
#include "XESIO.h"
#include "Hydro.h"
#include "RF_PrefsDialog.h"
#include "XPLMMenus.h"
#include "PlatformUtils.h"
#include "MapAlgs.h"
#include "RF_Progress.h"
#include "SceneryPackages.h"
#include "RF_Msgs.h"
#include "RF_Globals.h"
#include "RF_Selection.h"
#include "RF_Notify.h"
#include "NetPlacement.h"
#include "RF_DrawMap.h"
#include "DSFBuilder.h"
#include "PerfUtils.h"
#include "Zoning.h"
#include "ObjPlacement.h"
#include "ObjTables.h"
#include "Airports.h"
#include "Beaches.h"
#include "RF_Assert.h"
#include "PlatformUtils.h"
#include "Forests.h"
#include "DEMTables.h"
#include "GISTool_Globals.h"

#include "ObjPlacement.h"

ProcessingPrefs_t	gProcessingCmdPrefs = {
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

static	void	RF_HandleProcMenuCmd(void *, void * i);
static	void 	RF_UpdateProcCmds(void);
static	void 	RF_NotifyProcCmds(int catagory, int message, void * param);

void	RegisterProcessingCommands(void)
{
	int n;
	sProcessMenu = XPLMCreateMenu("Calculate", NULL, 0, RF_HandleProcMenuCmd, NULL);
	n = 0;
	while (kProcCmdNames[n])
	{
		XPLMAppendMenuItem(sProcessMenu, kProcCmdNames[n], (void *) n, 1);
		if (kCmdKeys[n*2])
			XPLMSetMenuItemKey(sProcessMenu,n,kCmdKeys[n*2],kCmdKeys[n*2+1]);
		++n;
	}
	RF_RegisterNotifyFunc(RF_NotifyProcCmds);
	RF_UpdateProcCmds();
}

static	void	RF_HandleProcMenuCmd(void *, void * i)
{
	long cmd = (long) i;

	try {
		switch(cmd) {
//		case procCmd_LowResTri:
//			TriangulateMesh(gMap, gTriangulationLo, gDem, RF_ProgressFunc, false);
//			break;
		case procCmd_HiResTri:
			TriangulateMesh(gMap, gTriangulationHi, gDem, "../rendering_data/OUTPUT-border",RF_ProgressFunc);
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_TriangleHiChange, NULL);
			break;
		case procCmd_DoAirports:
			ProcessAirports(gApts, gMap, gDem[dem_Elevation], gDem[dem_UrbanTransport], true, true, true, RF_ProgressFunc);
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
			break;
		case procCmd_DoZoning:
			ZoneManMadeAreas(gMap, gDem[dem_LandUse], gDem[dem_Slope],gApts, RF_ProgressFunc);
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorMetaChange, NULL);
			break;
//		case procCmd_DoBeaches:
//			CreateBeaches(gMap);
//			break;
		case procCmd_UpsampleEnviro:
			UpsampleEnvironmentalParams(gDem,RF_ProgressFunc);
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
			break;
		case procCmd_CalcSlope:
			CalcSlopeParams(gDem, true, RF_ProgressFunc);
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
			break;
		case procCmd_HydroCorrect:
			{
//				char	f1[1024];
//				f1[0] = 0;
//				if (!GetFilePathFromUser(getFile_Open, "Please pick a mask.zip file", "Preview", 6, f1, sizeof(f1))) break;
//
//				HydroReconstruct(gMap,  gDem,f1,"../rendering_data/OUTPUT-hydro",RF_ProgressFunc);
//				RF_Notifiable::Notify(RF_Cat_File, RF_Msg_VectorChange, NULL);
//				RF_Notifiable::Notify(RF_Cat_File, RF_Msg_RasterChange, NULL);
			}
			break;
		case procCmd_HydroSimplfiy:
			{
//				Bbox2	bounds;
//				CalcBoundingBox(gMap, bounds.p1, bounds.p2);
//				SimplifyCoastlines(gMap, bounds, RF_ProgressFunc);
//				RF_Notifiable::Notify(RF_Cat_File, RF_Msg_VectorChange, NULL);
			}
			break;
		case procCmd_DeriveDEMs:
			DeriveDEMs(gMap, gDem,gApts, gAptIndex, RF_ProgressFunc);
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_RasterChange, NULL);
			break;
		case procCmd_AddUrbanRoads:
			{
/*				char	path[1024];
				path[0] = 0;
				if (!gReplacementRoads.empty())
				{
					string path = gReplacementRoads;
					MFMemFile * fi = MemFile_Open(path.c_str());
					if (fi)
					{
						Pmwx		overMap;
						ReadXESFile(fi, &overMap, NULL, NULL, NULL, RF_ProgressFunc);

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

						AddEuroRoads(gMap, overMap, gDem[dem_Slope], gDem[dem_LandUse], lu_usgs_URBAN_IRREGULAR, RF_ProgressFunc);
						RF_Notifiable::Notify(RF_Cat_File, RF_Msg_VectorChange, NULL);
						MemFile_Close(fi);
					}
				}
*/
			BuildRoadsForFace(gMap, gDem[dem_Elevation], gDem[dem_Slope], gDem[dem_UrbanDensity], gDem[dem_UrbanRadial], gDem[dem_UrbanSquare], Face_handle(),  RF_ProgressFunc, NULL, NULL);
			}
			break;
		case procCmd_RemoveDupes:
			{
				if (gFaceSelection.empty())
					RemoveDuplicatesAll(gMap, RF_ProgressFunc);
				else {
					for (set<Pmwx::Face_handle>::iterator i = gFaceSelection.begin(); i != gFaceSelection.end(); ++i)
					if (!(*i)->data().IsWater())
					if (!(*i)->is_unbounded())
						RemoveDuplicates(*i);
				}
			}
			break;
		case procCmd_InstantiateGT:
			{
				vector<PreinsetFace>	insets;
				set<int>				the_types;

				GetObjTerrainTypes		(the_types);

				Bbox2	lim(gDem[dem_Elevation].mWest, gDem[dem_Elevation].mSouth, gDem[dem_Elevation].mEast, gDem[dem_Elevation].mNorth);


				if (gFaceSelection.empty())
				{
					GenerateInsets(gMap, gTriangulationHi, lim, the_types, true, insets, RF_ProgressFunc);
				} else {
					GenerateInsets(gFaceSelection, insets, RF_ProgressFunc);
				}
				InstantiateGTPolygonAll(insets, gDem, gTriangulationHi , RF_ProgressFunc);
			}
			DumpPlacementCounts();
			break;
		case procCmd_InstantiateFor:
			{
//				StElapsedTime	timer("Total forest time.\n");
//
//				vector<PreinsetFace>	insets;
//				set<int>				the_types;
//				GetAllForestLUs(the_types);
//				Bbox2	lim(gDem[dem_Elevation].mWest, gDem[dem_Elevation].mSouth, gDem[dem_Elevation].mEast, gDem[dem_Elevation].mNorth);
//				if (gFaceSelection.empty())
//				{
//					GenerateInsets(gMap, gTriangulationHi, lim, the_types, false, insets, RF_ProgressFunc);
//				} else {
//					GenerateInsets(gFaceSelection, insets, RF_ProgressFunc);
//				}
//
//				GenerateForests(gMap, insets, gTriangulationHi, RF_ProgressFunc);
			}
			break;
		case procCmd_BuildRoads:
			CalcRoadTypes(gMap, gDem[dem_Elevation], gDem[dem_UrbanDensity],RF_ProgressFunc);
			break;
		case procCmd_AssignLUToMesh:
			AssignLandusesToMesh(gDem,gTriangulationHi,"../rendering_data/OUTPUT-border",RF_ProgressFunc);
//			AssignLandusesToMesh(gDem,gTriangulationLo,false,RF_ProgressFunc);
			RF_Notifiable::Notify(rf_Cat_File, rf_Msg_TriangleHiChange, NULL);
			break;
		case procCmd_DoProcessing:
			if (gProcessingCmdPrefs.do_calc_slope				)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_CalcSlope	   );
			if (gProcessingCmdPrefs.do_upsample_environment		)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_UpsampleEnviro );
			if (gProcessingCmdPrefs.do_hydro_correct			)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_HydroCorrect   );
			if (gProcessingCmdPrefs.do_hydro_simplify			)	RF_HandleProcMenuCmd(NULL, (void *)		procCmd_HydroSimplfiy  );
			if (gProcessingCmdPrefs.do_derive_dems				)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_DeriveDEMs     );
			if (gProcessingCmdPrefs.do_add_urban_roads			)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_AddUrbanRoads  );
			if (gProcessingCmdPrefs.do_build_roads				)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_BuildRoads	   );
			if (gProcessingCmdPrefs.do_airports					)	RF_HandleProcMenuCmd(NULL, (void *)		procCmd_DoAirports	   );
			if (gProcessingCmdPrefs.do_zoning					)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_DoZoning	   );
			if (gProcessingCmdPrefs.do_triangulate				)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_HiResTri	   );
			if (gProcessingCmdPrefs.do_assign_landuse			)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_AssignLUToMesh );
			if (gProcessingCmdPrefs.build_3d_forests			)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_InstantiateFor );
			if (gProcessingCmdPrefs.remove_duplicate_objs		)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_RemoveDupes    );
			if (gProcessingCmdPrefs.place_buildings				)	RF_HandleProcMenuCmd(NULL, (void *) 		procCmd_InstantiateGT  );
			break;
		case procCmd_ExportDSFNew:
		case procCmd_ExportDSFExisting:
			{

				char	buf[1024], buf2[1024];
				strcpy(buf, "New Scenery Package");
				if (GetFilePathFromUser((cmd == procCmd_ExportDSFExisting) ? getFile_PickFolder : getFile_Save,
										(cmd == procCmd_ExportDSFExisting) ? "Please pick your scenery package" : "Please name your scenery package",
										(cmd == procCmd_ExportDSFExisting) ? "Update" : "Create", 5, buf, sizeof(buf)))
				{
//					if (cmd != procCmd_ExportDSFExisting)
					strcat(buf, DIR_STR);
					CreatePackageForDSF(buf, (int) gDem[dem_LandUse].mWest,(int) gDem[dem_LandUse].mSouth, buf2);
					BuildDSF(buf2, "-", gDem[dem_LandUse],gTriangulationHi, /*gTriangulationLo,*/ gMap, RF_ProgressFunc);
				}
			}
			break;
		}
	} catch (rf_assert_fail_exception& e) {
	} catch (exception& e) {
		DoUserAlert(e.what());
	} catch (...) {
		DoUserAlert("The operation was aborted due to an unexpected internal error.");
	}


}

void RF_NotifyProcCmds(int catagory, int message, void * param)
{
	if (catagory == rf_Cat_File)
		RF_UpdateProcCmds();
}

void RF_UpdateProcCmds(void)
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
		if (!e->data().mSegments.empty())
		{
			has_roads = (e->data().mSegments.front().mRepType != NO_VALUE);
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


