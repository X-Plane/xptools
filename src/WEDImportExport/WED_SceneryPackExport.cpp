/*
 * Copyright (c) 2014, Laminar Research.
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

#include "WED_SceneryPackExport.h"

#include "DSFLib.h"
#include "IResolver.h"
#include "ILibrarian.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "WED_ToolUtils.h"
#include "WED_HierarchyUtils.h"

#include "WED_AptIE.h"
#include "WED_DSFExport.h"
#include "WED_DSFImport.h"
#include "WED_Document.h"
#include "WED_GatewayExport.h"
#include "WED_Group.h"
#include "WED_GroupCommands.h"
#include "WED_Menus.h"
#include "WED_UIDefs.h"
#include "WED_Validate.h"

#include <iostream>

void	WED_ExportPackToPath(WED_Thing * root, IResolver * resolver, const string& in_path, set<WED_Thing *>& problem_children)
{
	int result = DSF_Export(root, resolver, in_path,problem_children);
	if (result == -1)
		return;

	string	apt = in_path + "Earth nav data" DIR_STR "apt.dat";
	string	apt_dir = in_path + "Earth nav data";

	FILE_make_dir_exist(apt_dir.c_str());
	WED_AptExport(root, apt.c_str());
}


int		WED_CanExportPack(IResolver * resolver)
{
	return 1;
}

#include "WED_Airport.h"
#include "WED_AirportBoundary.h"
#include "WED_LinePlacement.h"
#include "WED_EnumSystem.h"
#include "WED_ExclusionZone.h"
#include "WED_RampPosition.h"
#include "WED_Runway.h"
#include "WED_Sealane.h"
#include "WED_TruckDestination.h"
#include "WED_TruckParkingLocation.h"
#include "WED_ForestPlacement.h"
#include "WED_ObjPlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_MetaDataKeys.h"
#include "WED_MetaDataDefaults.h"
#include "WED_Menus.h"
#include "GISUtils.h"
#include <chrono>
#include "WED_ConvertCommands.h"
#include "WED_PackageMgr.h"

namespace
{
	template<class T>
	WED_Thing* CreateThing(WED_Archive* parent)
	{
		return T::CreateTyped(parent);
	}
}

void dummyPrintf(void * ref, const char * fmt, ...) { return; }

static void	DoHueristicAnalysisAndAutoUpgrade(IResolver* resolver)
{
	LOG_MSG("I/exp Starting upgrade heuristics\n");
	WED_Thing * wrl = WED_GetWorld(resolver);
	vector<WED_Airport*> apts;
	CollectRecursiveNoNesting(wrl, back_inserter(apts),WED_Airport::sClass);    // ATTENTION: all code here assumes 'normal' hierachies and no hidden items,
																				//	i.e. apts 1 level down, groups next, then items in them at 2 levels down.
	WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);                      // Speeds up recursive collecting, avoids recursing too deep.
	ISelection * sel = WED_GetSelect(resolver);
	
	int deleted_illicit_icao = 0;
	int added_local_codes = 0;
	int added_country_codes = 0;
	int grass_statistics[4] = { 0 };

//	auto climate_map = LoadAirportClimates();

	auto t0 = chrono::high_resolution_clock::now();

	for (auto apt_itr = apts.begin(); apt_itr != apts.end(); ++apt_itr)
	{
		auto t2 = chrono::high_resolution_clock::now();
		string ICAO_code;

		//-- Erase implausible ICAO and now undesired closed tags (the [X] in the name is now official) ----
		if ((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataICAO))
		{
			ICAO_code = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataICAO);

			bool illicit = ICAO_code.size() != 4 || toupper(ICAO_code[0]) < 'A' || toupper(ICAO_code[0]) > 'Z';
			for (int i = 1; i < 4; i++)
				illicit |= toupper(ICAO_code[i]) < 'A' || toupper(ICAO_code[i]) > 'Z';
			if(illicit)
			{
				wrl->StartCommand("Delete bad icao");
				(*apt_itr)->EditMetaDataKey(META_KeyName(wed_AddMetaDataICAO),"");
				wrl->CommitCommand();
				deleted_illicit_icao++;
				ICAO_code = "";
			}
		}
		
		// Per Philipp's email of 11/22/2021
		// In case the airport_ID is the ONLY meta data for the airports code - this is taken by X-plane as 
		// the ICAO code. So if that the casse and the airport_ID is not a legal ICAO, add a local code entry to 
		// prevent this 'inheriting' of the illegal ICAO code, causing name collisions with other airports.
		if (ICAO_code.empty())
		{
			string local_code, faa_code;
			if((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataFAA))
				faa_code = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataFAA);
			if((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataLocal))
				local_code = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataLocal);
			
			if(local_code.empty() && faa_code.empty())
			{
				string apt_ID;
				(*apt_itr)->GetICAO(apt_ID);
				bool illicit = apt_ID.size() != 4 || toupper(apt_ID[0]) < 'A' || toupper(apt_ID[0]) >= 'Z';
				for (int i = 1; i < 4; i++)
					illicit |= toupper(apt_ID[i]) < 'A' || toupper(apt_ID[i]) > 'Z';
				if(illicit)
				{
					wrl->StartCommand("Add local code");
					(*apt_itr)->AddMetaDataKey(META_KeyName(wed_AddMetaDataLocal),apt_ID);
					wrl->CommitCommand();
					added_local_codes++;
				}
			}
		}

		//-- upgrade Country Metadata -------------
		added_country_codes += add_iso3166_country_metadata(**apt_itr);

		//-- upgrade Ramp Positions with XP10.45 data to get parked A/C -------------
		wrl->StartCommand("Upgrade Ramp Positions");
		if (wed_upgrade_ramps(*apt_itr))
			wrl->CommitCommand();
		else
			wrl->AbortCommand();

#if 0  // this was good in 10.45, but not needed any for gateway airports as of 2022
		//-- Agp and obj upgrades to create more ground traffic --------------------------------
		vector<WED_TruckParkingLocation*> parking_locations;
		CollectRecursive(*apt_itr, back_inserter(parking_locations));

		vector<WED_TruckDestination*>     truck_destinations;
		CollectRecursive(*apt_itr, back_inserter(truck_destinations));

		bool found_truck_evidence = false;
		found_truck_evidence |= !parking_locations.empty();
		found_truck_evidence |= !truck_destinations.empty();

		if (found_truck_evidence == false)
		{
			vector<WED_ObjPlacement*> all_objs;
			vector<WED_AgpPlacement*> agp_placements;
			CollectRecursive(*apt_itr, back_inserter(all_objs));

			for (vector<WED_AgpPlacement*>::iterator obj_itr = all_objs.begin(); obj_itr != all_objs.end(); ++obj_itr)
			{
				string agp_resource;
				(*obj_itr)->GetResource(agp_resource);
				if (FILE_get_file_extension(agp_resource) == ".agp")
					agp_placements.push_back(*obj_itr);
			}

			vector<WED_ObjPlacement*> out_added_objs;
			wrl->StartCommand("Break Apart Special Agps");
			int num_replaced = wed_break_apart_special_agps(agp_placements, rmgr, out_added_objs);
			if (num_replaced == 0)
				wrl->AbortCommand();
			else
			{
				wrl->CommitCommand();
				LOG_MSG("Broke apart %d agp at %s\n", num_replaced, ICAO_code.c_str());
			}

			if (num_replaced > 0 || out_added_objs.size() > 0 || WED_CanReplaceVehicleObj(*apt_itr))
				WED_DoReplaceVehicleObj(resolver,*apt_itr);
		}
#endif
		//-- Remove leading zero's from runways within the FAA's jurisdiction, except some mil bases ------
		Bbox2 apt_box;
		(*apt_itr)->GetBounds(gis_Geo, apt_box);

		if ((*apt_itr)->GetAirportType() == type_Airport)
		{
			string ICAO_region;
			if ((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataRegionCode))
				ICAO_region = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataRegionCode);

			//	if (FAA_bounds.inside(apt_box.p1()))
			if ((apt_box.p1.x() <  -67.0 && apt_box.p1.y() > 24.5 && apt_box.p1.y() < 49.0) ||
				(apt_box.p1.x() < -131.4 && apt_box.p1.y() > 16.0))
			if(ICAO_region != "CY" && ICAO_region != "MM")
			if(ICAO_code != "KEDW" && ICAO_code != "9L2" && ICAO_code != "KFFO" && ICAO_code != "KSSC" && ICAO_code != "KCHS")
			{
				vector<WED_Runway*> rwys;
				CollectRecursive(*apt_itr, back_inserter(rwys));
				for (auto r : rwys)
					if(r->GetSurface() < surf_Grass)
					{
						string r_nam;
						r->GetName(r_nam);
						if (r_nam[0] == '0')
						{
							wrl->StartCommand("Remove runway zeros");
							r->SetName(r_nam.substr(1));
							wrl->CommitCommand();
						}
					}
			}
		}

		//-- Break up jetway AGP's, convert jetway objects into facades for XP12 moving jetways -------------
		wrl->StartCommand("Upgrade Jetways");
		if (int count = WED_DoConvertToJW(*apt_itr))
		{
			wrl->CommitCommand();
			LOG_MSG("Upgraded %d JW at %s\n", count, ICAO_code.c_str());
		}
		else
			wrl->AbortCommand();

#if TYLER_MODE == 11
		// translate new pavement polygons into XP11 equivalents (run/taxiways have that done in aptio.cpp)
		// as well as a few essential and well known new XP12 objects. These "back-translations" 
		// of new art assets will some day get out of hand and backporting to XP11 will end ...
		#define XP12PATH  "lib/airport/ground/"
		#define XP12N  strlen(XP12PATH)
		vector<IHasResource*> xp12_art;
		CollectRecursive(*apt_itr, back_inserter(xp12_art), IgnoreVisiblity, [](WED_Thing* t)->bool 
			{
				if(auto r = dynamic_cast<IHasResource*>(t))
				{
					string res;
					r->GetResource(res);
					return res.compare(0, XP12N, XP12PATH) == 0 || 
						   res.compare(0, strlen("lib/vehicles/"), "lib/vehicles/") == 0 ||
						   res.compare(0, strlen("lib/airport/control_towers/"), "lib/airport/control_towers/") == 0;
				}
				return false;
			}, "", 2);

		if (xp12_art.size())
		{
			wrl->StartCommand("Translate XP12 art");
			for (auto p : xp12_art)
			{
				string res;
				p->GetResource(res);
				if      (res.compare(XP12N, strlen("pavement/asphalt_L"), "pavement/asphalt_L") == 0)
					res = "lib/airport/pavement/asphalt_1L.pol";
				else if (res.compare(XP12N, strlen("pavement/asphalt_D"), "pavement/asphalt_D") == 0)
					res = "lib/airport/pavement/asphalt_1D.pol";
				else if (res.compare(XP12N, strlen("pavement/asphalt"), "pavement/asphalt") == 0)
					res = "lib/airport/pavement/asphalt_3D.pol";
				else if (res.compare(XP12N, strlen("pavement/concrete_L"), "pavement/concrete_L") == 0)
					res = "lib/airport/pavement/concrete_1L.pol";
				else if (res.compare(XP12N, strlen("pavement/concrete"), "pavement/concrete") == 0)
					res = "lib/airport/pavement/concrete_1D.pol";
				else if (res.compare(0, strlen("lib/vehicles/static/trucks/"), "lib/vehicles/static/trucks/") == 0)
					res = "lib/airport/Common_Elements/Vehicles/Cargo_Trailer.obj";
				else if (res.compare(0, strlen("lib/airport/control_towers/"), "lib/airport/control_towers/") == 0)
					res = "lib/airport/Modern_Airports/Control_Towers/Modern_Tower_1.agp";
				else 
					continue;
				p->SetResource(res);
			}
			wrl->CommitCommand();
		}
#else
		// mow the grass
		vector<WED_Group*> terFX;
		CollectRecursive(*apt_itr, back_inserter(terFX), IgnoreVisiblity, [](WED_Thing* t)->bool 
			{
				string res;
				t->GetName(res);
				return res == "Terrain FX";
			},
			WED_Group::sClass, 1);
		if(terFX.empty())
		{
			wrl->StartOperation("Mow Grass");
			if(WED_DoMowGrass(*apt_itr, grass_statistics))
			{
				LOG_MSG("Mowed grass at %s\n", ICAO_code.c_str());
				wrl->CommitOperation();
			}
			else
				wrl->AbortOperation();
		}
		// convert tree objects into a forest
		vector<WED_ObjPlacement*> tree_objs;
		CollectRecursive(*apt_itr, back_inserter(tree_objs), IgnoreVisiblity, [](WED_Thing* objs)->bool {
			string res;
			static_cast<WED_ObjPlacement*>(objs)->GetResource(res);
			return res.compare(0, strlen("lib/g10/forests/autogen"), "lib/g10/forests/autogen") == 0 ||
				   res.compare(0, strlen("lib/airport/Common_Elements/Miscellaneous/Tree"), "lib/airport/Common_Elements/Miscellaneous/Tree") == 0;
			},
			WED_ObjPlacement::sClass, 2);
		if (tree_objs.size() >= 3)
		{
			wrl->StartCommand("Select");
			sel->Clear();
			sel->Insert(vector<ISelectable*>(tree_objs.begin(), tree_objs.end()));
			wrl->CommitCommand();
			WED_DoConvertToForest(resolver);
			LOG_MSG("Converted Trees into Forests at %s\n", ICAO_code.c_str());
		}
		// convert long deprecated 2D only forests into contemporary 3D forests.
		vector<WED_ForestPlacement*> forests;
		CollectRecursive(*apt_itr, back_inserter(forests), IgnoreVisiblity, [](WED_Thing* thing)->bool {
			string res;
			static_cast<WED_ForestPlacement*>(thing)->GetResource(res);
			return res.compare(0, strlen("lib/g10/forests/AG_"), "lib/g10/forests/AG_") == 0;
			},
			WED_ForestPlacement::sClass, 2);
		if (!forests.empty())
		{
			wrl->StartCommand("Convert deprecated 2D forests");
			for(auto fst : forests)
				fst->SetResource("lib/vegetation/trees/deciduous/maple_medium.for");
			wrl->CommitCommand();
			LOG_MSG("Converted AG_* forests to 3D at %s\n", ICAO_code.c_str());
		}
		// add soft edges to all pavement
		vector<WED_Group*> pavFX;
		CollectRecursive(*apt_itr, back_inserter(pavFX), IgnoreVisiblity, [](WED_Thing* t)->bool
			{
				string res;
				t->GetName(res);
				return res == "Pavement FX";
			},
			WED_Group::sClass, 1);
		if (pavFX.empty())
		{
			wrl->StartOperation("SoftEdge all pavement");
			if (false) // ToDo !!!!
			{
				LOG_MSG("SoftEdged pavemnts  at %s\n", ICAO_code.c_str());
				wrl->CommitOperation();
			}
			else
				wrl->AbortOperation();
		}
/*		// add soft edges for airport grass
		vector<WED_AirportBoundary*> bdy;
		CollectRecursive(*apt_itr, back_inserter(bdy), IgnoreVisiblity, TakeAlways, WED_AirportBoundary::sClass, 2);
		if (bdy.size() && (*apt_itr)->GetAirportType() == type_Airport)
		{
			vector<WED_LinePlacement*> grass_lines;
			CollectRecursive(*apt_itr, back_inserter(grass_lines), IgnoreVisiblity, [](WED_Thing* lin)->bool {
				string res;
				static_cast<WED_LinePlacement*>(lin)->GetResource(res);
				return res.compare(0, strlen("lib/g10/terrain10/apt_border_"), "lib/g10/terrain10/apt_border_") == 0;
				},
				WED_LinePlacement::sClass, 2);
			if (grass_lines.empty() && climate_map.count(ICAO_code))
			{
				wrl->StartCommand("Create AptGrass Soft Edges");
				sel->Clear();
				sel->Insert(vector<ISelectable*>(bdy.begin(), bdy.end()));
				WED_DoDuplicate(resolver, false);
				WED_DoConvertTo(resolver, &CreateThing<WED_LinePlacement>, false);
				// change to particular line type
				string grass_line_res = string("lib/g10/terrain10/apt_border_") + climate_map[ICAO_code] + ".lin";
				int n_sel = sel->GetSelectionCount();
				for (int i = 0; i < n_sel; i++)
				{
					if (auto t = dynamic_cast<IHasResource*>(sel->GetNthSelection(i)))
						t->SetResource(grass_line_res);
				}
				wrl->CommitCommand();
				LOG_MSG("Added AptGrass Edges at %s\n", ICAO_code.c_str());
			}
		}
*/		//
		// The "big xp12 gateway reset" - remove certain features unless the submission is "recent" as
		//  measureds by the scenery ID (i.e. a cutoff point in time after which ONLY Xp12 ready sceneries were accepted) 
		// or presence of certain, XP12 only art assets
		//
		if ((*apt_itr)->GetSceneryID() < 99000 && terFX.empty() && pavFX.empty())
		{
			// nuke all large terrain polygons unless at high lattitudes (cuz there is no gobal scenery there ...)
			if (apt_box.p1.y() < 73.0 && apt_box.p1.y() > -60.0)
			{
				vector<WED_PolygonPlacement*> terrain_polys;
				CollectRecursive(*apt_itr, back_inserter(terrain_polys), IgnoreVisiblity, [](WED_Thing* t)->bool
					{
						string res;
						static_cast<WED_PolygonPlacement*>(t)->GetResource(res);
						return res.compare(0, strlen("lib/g10/terrain10/"), "lib/g10/terrain10/") == 0 ||
							res.compare(0, strlen("lib/g8/pol/"), "lib/g8/pol/") == 0;
					},
					WED_PolygonPlacement::sClass, 2);
				if (terrain_polys.size())
				{
					set<WED_Thing*> things;
					for (auto p : terrain_polys)
					{
						Bbox2 bounds;
						p->GetBounds(gis_Geo, bounds);
						if (LonLatDistMeters(bounds.bottom_left(), bounds.top_right()) > 20.0)      // passes at least 10m lettering drawn with snow texture
							CollectRecursive(p, inserter(things, things.end()), IgnoreVisiblity, TakeAlways);
					}
					wrl->StartCommand("Delete Terrain Polys");
					WED_RecursiveDelete(things);
					wrl->CommitCommand();
					LOG_MSG("Deleted %zd terrain polys at %s\n", terrain_polys.size(), ICAO_code.c_str());
				}
			}

			// nuke all "Grunge" draped objects
			vector<WED_ObjPlacement*> grunge_objs;
			CollectRecursive(*apt_itr, back_inserter(grunge_objs), IgnoreVisiblity, [](WED_Thing* objs)->bool {
				string res;
				static_cast<WED_ObjPlacement*>(objs)->GetResource(res);
				return res.compare(0, strlen("lib/airport/Common_Elements/Parking/Grunge"), "lib/airport/Common_Elements/Parking/Grunge") == 0;
				},
				WED_ObjPlacement::sClass, 2);
			if (grunge_objs.size())
			{
				wrl->StartCommand("Delete Grunge Objects");
				set<WED_Thing*> things(grunge_objs.begin(), grunge_objs.end());
				WED_RecursiveDelete(things);
				wrl->CommitCommand();
				LOG_MSG("Deleted %zd Grunges at %s\n", grunge_objs.size(), ICAO_code.c_str());
			}

			// nuke ALL exclusions at airports, but only for 2D stuff like Beaches, Roads, Polygons, Lines at Sea/Heliports
			vector<WED_ExclusionZone*> exclusions;
			CollectRecursive(*apt_itr, back_inserter(exclusions), IgnoreVisiblity, TakeAlways,
				WED_ExclusionZone::sClass, 2);
			if (exclusions.size())
			{
				wrl->StartCommand("Remove XP11 era exclusions");
				set<WED_Thing*> ex_set;
				int reduced_ex = 0;
				if((*apt_itr)->GetAirportType() == type_Airport)
				{
					for(auto e : exclusions)
						ex_set.insert(e);
				}
				else
				{
					for (auto e : exclusions)
					{
						set<int> ex;
						e->GetExclusions(ex);
						ex.erase(exclude_Bch);
						ex.erase(exclude_Net);
						ex.erase(exclude_Pol);
						ex.erase(exclude_Lin);
						if(ex.size())
						{
							e->SetExclusions(ex);
							reduced_ex++;
						}
						else
							ex_set.insert(e);
					}
				}
				WED_RecursiveDelete(ex_set);
				wrl->CommitCommand();
				LOG_MSG("I/XP12 Deleted %d Exclusions at %s\n", (int) exclusions.size() + reduced_ex, ICAO_code.c_str());
			}

			// nuke all per-airport flatten
			AptInfo_t apt_info;
			(*apt_itr)->Export(apt_info);
			auto it = std::find(apt_info.meta_data.begin(), apt_info.meta_data.end(), make_pair(string("flatten"), string("1")));
			if (it != apt_info.meta_data.end())
			{
				wrl->StartCommand("Remove XP11 era flatten");
				apt_info.meta_data.erase(it);
				(*apt_itr)->Import(apt_info, dummyPrintf, nullptr);
				wrl->CommitCommand();
				LOG_MSG("I/XP12 Deleted Always Flatten at %s\n", ICAO_code.c_str());
			}
		}

#endif
#if TYLER_MODE
		double percent_done = (double)distance(apts.begin(), apt_itr) / apts.size() * 100;
		printf("%0.0lf%% through heuristic at %s\n", percent_done, ICAO_code.c_str());

		auto t1 = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed = t1 - t2;
		LOG_MSG("Update %s took %lf sec\n", ICAO_code.c_str(), elapsed.count());
		t2 = t1;
//		if(distance(apts.begin(), apt_itr) == 15) break;
#endif
	}
#if TYLER_MODE == 11
	// Remove all remaining new XP12 stuff - so this needs to be run in an XP11 installation. 
	// Or items be copied to be local items in the Global Airports Scenery.
	WED_DoSelectMissingObjects(resolver);
	WED_DoClear(resolver);
#endif		
	LOG_MSG("Deleted %d illicit ICAO meta tags\n", deleted_illicit_icao);
	LOG_MSG("Added %d local code metas to prevent Airport_ID getting taken for ICAO\n", added_local_codes);
	LOG_MSG("Prefixed %d country meta data with iso3166 codes\n", added_country_codes);
	LOG_MSG("Mowed %d polys %d lines %d spots %d patches\n", grass_statistics[0], grass_statistics[1],grass_statistics[2],grass_statistics[3]);

	auto t1 = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = t1 - t0;
	LOG_MSG("I/exp Done with upgrade heuristics, took %lf sec\n", elapsed.count());
	LOG_FLUSH();
}

int		WED_CanExportPack(IResolver* resolver, string& ioname)
{
	int target_idx = gExportTarget - wet_xplane_900;
	if (target_idx > wet_latest_xplane)
		ioname = "Export to Scenery (w/Scenery Gateway heuristics)";
	else
		 ioname = string("Export to Scenery for ") + WED_GetTargetMenuName(target_idx);
	return 1;
}

void	WED_DoExportPack(WED_Document * resolver, WED_MapPane * pane)
{
#if TYLER_MODE
    // do any pre-export modifications here.
	DoHueristicAnalysisAndAutoUpgrade(resolver);
#else
	// Just don't ever export if we are invalid.  Avoid the case where we write junk to a file!
	// Special case: in Tyler's bulk-Gateway-export-mode, the suitability for export is to be established with other means,
	// ... and if the export blows up or something, it's Tyler's fault :(
	if(!WED_ValidateApt(resolver, pane))
		return;
	if (gExportTarget == wet_gateway)
	{
		auto uMgr = resolver->GetUndoMgr();
		uMgr->MarkUndo();
		DoHueristicAnalysisAndAutoUpgrade(resolver);
		if (uMgr->UndoToMark())
			DoUserAlert("Some of the upgrade heuristics applied during export could not be undone. Scenery was permanently altered by export.");
	}
#endif
	ILibrarian * l = WED_GetLibrarian(resolver);
	WED_Thing * w = WED_GetWorld(resolver);
	WED_Group * g = dynamic_cast<WED_Group*>(w);
	DebugAssert(g);
	set<WED_Thing *>	problem_children;

	string pack_base;
	l->LookupPath(pack_base);

	if(gExportTarget >= wet_xplane_1130 || TYLER_MODE)
	{
		w->StartOperation("Force GUI/closed Metatags");
		if(EnforceRecursive_MetaDataGuiLabel(w))
			w->CommitOperation();
		else
			w->AbortOperation();
	}

	WED_ExportPackToPath(g, resolver, pack_base, problem_children);

	if(!problem_children.empty())
	{
		DoUserAlert("One or more objects could not be exported - check for self intersecting polygons and closed-ring facades crossing DFS boundaries.");
		ISelection * sel = WED_GetSelect(resolver);
		(*problem_children.begin())->StartOperation("Select broken items.");
		sel->Clear();
		for(set<WED_Thing*>::iterator p = problem_children.begin(); p != problem_children.end(); ++p)
			sel->Insert(*p);
		(*problem_children.begin())->CommitOperation();
	}
}
