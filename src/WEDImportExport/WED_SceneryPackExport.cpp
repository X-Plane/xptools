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
#include "WED_AptIE.h"
#include "WED_DSFExport.h"
#include "IResolver.h"
#include "WED_ToolUtils.h"
#include "ILibrarian.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "WED_Group.h"
#include "WED_Validate.h"

#if TYLER_MODE || 1
#include "WED_RampPosition.h"
#include "WED_TruckDestination.h"
#include "WED_TruckParkingLocation.h"
#include "WED_GroupCommands.h"
#include <iterator>

#include "WED_EnumSystem.h"
#include "WED_Validate.h"
#endif

void	WED_ExportPackToPath(WED_Thing * root, IResolver * resolver, const string& in_path, set<WED_Thing *>& problem_children)
{
	DSF_Export(root, resolver, in_path,problem_children);

	string	apt = in_path + "Earth nav data" DIR_STR "apt.dat";
	string	apt_dir = in_path + "Earth nav data";

	FILE_make_dir_exist(apt_dir.c_str());
	WED_AptExport(root, apt.c_str());
}


int		WED_CanExportPack(IResolver * resolver)
{
	return 1;
}

static void	DoHueristicAnalysisAndAutoUpgrade(IResolver* resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	vector<WED_Airport*> apts;
	CollectRecursiveNoNesting(wrl, back_inserter(apts));

	WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);
	ISelection * sel = WED_GetSelect(resolver);

	for (vector<WED_Airport*>::iterator apt_itr = apts.begin(); apt_itr != apts.end(); ++apt_itr)
	{
		//--Ramp Positions-----------------------------------------------------------
		vector<WED_RampPosition*> ramp_positions;
		CollectRecursive(wrl, back_inserter(ramp_positions));

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
			int did_work = wed_upgrade_one_airport(*apt_itr, rmgr, sel);
			if (did_work == 0)
			{
				wrl->AbortCommand();
			}
			else
			{
				wrl->CommitCommand();
			}
			break;
		}
		//---------------------------------------------------------------------------

		//--Agp and obj upgrades-----------------------------------------------------
		vector<WED_TruckParkingLocation*> parking_locations;
		vector<WED_TruckDestination*>     truck_destinations;
		CollectRecursive(*apt_itr, back_inserter(parking_locations));
		CollectRecursive(*apt_itr, back_inserter(truck_destinations));

		/*if (parking_locations.empty() == false ||
		truck_destinations.empty() == false)
		{
		vector<WED_ObjPlacement*> all_objs;
		CollectRecursive(

		)
		for (vector<ISelectable*>::iterator itr = selected.begin(); itr != selected.end(); ++itr)
		{
		WED_AgpPlacement* agp = dynamic_cast<WED_AgpPlacement*>(*itr);
		if (agp != NULL)
		{
		string agp_resource;
		agp->GetResource(agp_resource);
		if (FILE_get_file_extension(agp_resource) != ".agp")
		{
		return false;
		}
		}
		else
		{
		return false;
		}
		}
		WED_DoBreakApartSpecialAgps(resolver);
		if (WED_CanReplaceVehicleObj(resolver))
		{
		WED_DoReplaceVehicleObj(resolver);
		}
		}
		}*/
	}
}

void	WED_DoExportPack(IResolver * resolver)
{
	DoHueristicAnalysisAndAutoUpgrade(resolver);
	// Just don't ever export if we are invalid.  Avoid the case where we write junk to a file!
	if(!WED_ValidateApt(resolver))
		return;

	ILibrarian * l = WED_GetLibrarian(resolver);
	WED_Thing * w = WED_GetWorld(resolver);
	WED_Group * g = dynamic_cast<WED_Group*>(w);
	DebugAssert(g);
	set<WED_Thing *>	problem_children;

	string pack_base;
	l->LookupPath(pack_base);

	WED_ExportPackToPath(g, resolver, pack_base, problem_children);


	if(!problem_children.empty())
	{
		DoUserAlert("One or more objects could not exported - check for self intersecting polygons and closed-ring facades crossing DFS boundaries.");
		ISelection * sel = WED_GetSelect(resolver);
		(*problem_children.begin())->StartOperation("Select broken items.");
		sel->Clear();
		for(set<WED_Thing*>::iterator p = problem_children.begin(); p != problem_children.end(); ++p)
			sel->Insert(*p);
		(*problem_children.begin())->CommitOperation();		
	}
}

