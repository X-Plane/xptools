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
#include "WED_GroupCommands.h"
#include "WED_UIDefs.h"
#include "WED_Validate.h"

#include <iostream>

void	WED_ExportPackToPath(WED_Thing * root, IResolver * resolver, const string& in_path, set<WED_Thing *>& problem_children)
{
	int result = DSF_Export(root, resolver, in_path,problem_children);
	if (result == -1)
	{
		return;
	}

	string	apt = in_path + "Earth nav data" DIR_STR "apt.dat";
	string	apt_dir = in_path + "Earth nav data";

	FILE_make_dir_exist(apt_dir.c_str());
	WED_AptExport(root, apt.c_str());
}


int		WED_CanExportPack(IResolver * resolver)
{
	return 1;
}

#if TYLER_MODE

#include "WED_Airport.h"
#include "WED_EnumSystem.h"
#include "WED_Group.h"
#include "WED_RampPosition.h"
#include "WED_TruckDestination.h"
#include "WED_TruckParkingLocation.h"
#include "WED_ObjPlacement.h"
#include "WED_MetaDataKeys.h"
#include "WED_Menus.h"

static void	DoHueristicAnalysisAndAutoUpgrade(IResolver* resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	vector<WED_Airport*> apts;
	CollectRecursiveNoNesting(wrl, back_inserter(apts),WED_Airport::sClass);

	WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	
	int deleted_illicit_icao = 0;

	for (auto apt_itr = apts.begin(); apt_itr != apts.end(); ++apt_itr)
	{

		//--Erase implausible ICAO tags----------------------------------------------
		if ((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataICAO))
		{
			string ICAO_code = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataICAO);

			bool illicit = ICAO_code.size() != 4 || toupper(ICAO_code[0]) < 'A' || toupper(ICAO_code[0]) >= 'Z';
			for (int i = 1; i < 4; i++)
				illicit |= toupper(ICAO_code[i]) < 'A' || toupper(ICAO_code[i]) > 'Z';
			if(illicit)
			{
				wrl->StartCommand("Upgrade Ramp Positions");
				(*apt_itr)->EditMetaDataKey(META_KeyName(wed_AddMetaDataICAO),"");
				wrl->CommitCommand();
				deleted_illicit_icao++;
			}
		}

		//--Ramp Positions-----------------------------------------------------------
		vector<WED_RampPosition*> ramp_positions;
		CollectRecursive(*apt_itr, back_inserter(ramp_positions),WED_RampPosition::sClass);

		int non_empty_airlines_strs = 0;
		int non_op_none_ramp_starts = 0;
		for (vector<WED_RampPosition*>::iterator ramp_itr = ramp_positions.begin(); ramp_itr != ramp_positions.end(); ++ramp_itr)
		{
			if ((*ramp_itr)->GetAirlines() != "")
			{
				++non_empty_airlines_strs;
			}

			if ((*ramp_itr)->GetRampOperationType() != ramp_operation_None)
			{
				++non_op_none_ramp_starts;
			}
		}

		if (non_empty_airlines_strs == 0 || non_op_none_ramp_starts == 0)
		{
			wrl->StartCommand("Upgrade Ramp Positions");
			std::cout << "Upgrading Ramp Positions" << endl;
			int did_work = wed_upgrade_one_airport(*apt_itr, rmgr, sel);
			if (did_work == 0)
			{
				wrl->AbortCommand();
			}
			else
			{
				wrl->CommitCommand();
			}
		}
		//---------------------------------------------------------------------------

		//--Agp and obj upgrades-----------------------------------------------------
		vector<WED_TruckParkingLocation*> parking_locations;
		CollectRecursive(*apt_itr, back_inserter(parking_locations),WED_TruckParkingLocation::sClass);

		vector<WED_TruckDestination*>     truck_destinations;
		CollectRecursive(*apt_itr, back_inserter(truck_destinations),WED_TruckDestination::sClass);

		bool found_truck_evidence = false;
		found_truck_evidence |= !parking_locations.empty();
		found_truck_evidence |= !truck_destinations.empty();

		if (found_truck_evidence == false)
		{
			vector<WED_ObjPlacement*> all_objs;
			vector<WED_AgpPlacement*> agp_placements;
			CollectRecursive(*apt_itr, back_inserter(all_objs),WED_ObjPlacement::sClass);

			for (vector<WED_AgpPlacement*>::iterator obj_itr = all_objs.begin(); obj_itr != all_objs.end(); ++obj_itr)
			{
				string agp_resource;
				(*obj_itr)->GetResource(agp_resource);
				if (FILE_get_file_extension(agp_resource) == ".agp")
				{
					agp_placements.push_back(*obj_itr);
				}
			}

			vector<WED_ObjPlacement*> out_added_objs;
			wrl->StartCommand("Break Apart Special Agps");
			int num_replaced = wed_break_apart_special_agps(agp_placements, rmgr, out_added_objs);
			if (num_replaced == 0)
				wrl->AbortCommand();
			else
				wrl->CommitCommand();

			//Easy out
			if (num_replaced > 0 || out_added_objs.size() > 0)
			{
				WED_DoReplaceVehicleObj(resolver,*apt_itr);
			}
			else if (WED_CanReplaceVehicleObj(*apt_itr))
			{
				WED_DoReplaceVehicleObj(resolver,*apt_itr);
			}
		}

		double percent_done = (double)distance(apts.begin(), apt_itr) / apts.size() * 100;
		printf("%0.0f%% through heuristic\n", percent_done);
	}
	printf("Deleted %d illicit ICAO meta tags\n", deleted_illicit_icao);
}
#endif

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
