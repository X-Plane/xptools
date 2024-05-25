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

#include "WED_ShapePlacement.h"
#include "WED_ShapeNode.h"
#include "IGIS.h"

#include "WED_GroupCommands.h"
#include "WED_Menus.h"
#include "WED_UIDefs.h"
#include "WED_Validate.h"

#include <iostream>


static string escape(const string& str)
{
	string result;
	result.reserve(str.size());

	auto b = (unsigned char*) str.data();
	auto e = b + str.length();

	while (b < e)
	{
		switch (*b) 
		{
		case '<':	result += "&lt;";	break;
		case '>':	result += "&gt;";	break;
		case '&':	result += "&amp;";	break;
//		case '\'':	result += "&apos;";	break;        // not needed as all params are either XML text context or inside double quotes
		case '"':	result += "&quot;";	break;        // if switching to single quotes for parameters - double quotes could be passed through verbatim
		default:
			if (*b & 0xC0 == 0xC0) // UTF-8 multi-byte - copy verbatim
			{
				result += *b++;
				if (*b & 0xC0 == 0x80) // UTF-8 3-byte
					result += *b++;
				if (*b & 0xC0 == 0x80) // UTF-8 4-byte
					result += *b++;
				result += *b;
			}
			else if (*b >= ' ' || *b == '\t')         // skip all control chars or CR's save for tabs
				result += *b;
			break;
		}
		b++;
	}
	return result;
}

static void KmlExport(WED_Thing* root, const string& file)
{
	vector<WED_ShapePlacement*> shapes;
	CollectRecursive(root, back_inserter(shapes));

	if (!shapes.empty())
	{
		if(auto fo = fopen(file.c_str(), "w"))
		{
			fprintf(fo, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			            "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
			            "<Document>\n");
			for (auto s : shapes)
			{
				string name, desc;
				s->GetName(name);
				s->GetString(desc);
				fprintf(fo,		"  <Placemark>\n");
				fprintf(fo,		"    <name>%s</name>\n"
								"    <description>%s</description>\n", escape(name).c_str(), escape(desc).c_str());
				fprintf(fo,		"    <%s>\n", s->IsClosed() ? "Polygon" : "LineString");
				if (s->IsClosed())
					fprintf(fo, "      <outerBoundaryIs>\n"
								"      <LinearRing>\n");
				fprintf(fo, 	"      <coordinates>\n");

				int np = s->GetNumPoints();
				Point2 pt;
				for (int n = 0; n < np + s->IsClosed(); n++)
				{
					s->GetNthPoint(n % np)->GetLocation(gis_Geo, pt);
					double d = 0;
					if (auto p = dynamic_cast<WED_ShapeNode*>(s->GetNthChild(n % np)))
						d = p->GetZ();
					fprintf(fo, "      %.9lf,%.9lf,%.2lf\n", pt.x(), pt.y(), d);
				}
				fprintf(fo,		"      </coordinates>\n");
				if (s->IsClosed())
					fprintf(fo, "      </LinearRing>\n"
								"      </outerBoundaryIs>\n");
				fprintf(fo,		"    </%s>\n", s->IsClosed() ? "Polygon" : "LineString");
				fprintf(fo,		"  </Placemark>\n");
			}
			fprintf(fo, "</Document>\n"
			            "</kml>\n");
			fclose(fo);
		}
	}
}

static void OsmExport(WED_Thing* root, const string& file)
{
	vector<WED_ShapePlacement*> shapes;
	CollectRecursive(root, back_inserter(shapes));

	if (!shapes.empty())
	{
		if (auto fo = fopen(file.c_str(), "w"))
		{
			fprintf(fo,	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
						"<osm version=\"0.6\" generator=\"WorldEditor\">\n");
			for (auto shp : shapes)
			{
				vector<int> IDs;
				int id = 0;
				string name, desc;
				int np = shp->GetNumPoints();
				Point2 pt;
				for (int n = 0; n < np; n++)
				{
					shp->GetNthPoint(n)->GetLocation(gis_Geo, pt);
					double d = 0;
					if (auto p = dynamic_cast<WED_ShapeNode*>(shp->GetNthChild(n)))
					{
						p->GetName(name);
						d = p->GetZ();
						desc = p->GetString();
						id = p->GetID();
					}
					fprintf(fo,		"  <node id=\"%d\" lon=\"%.9lf\" lat=\"%.9lf\" version=\"1\">\n", id, pt.x(), pt.y());
					fprintf(fo,		"    <tag k=\"name\" v=\"%s\"/>\n"
									"    <tag k=\"z_value\" v=\"%.2lf\"/>\n", escape(name).c_str(), d);
					if(!desc.empty())
						fprintf(fo, "    <tag k=\"description\" v=\"%s\"/>\n", escape(desc).c_str());
					fprintf(fo,		"  </node>\n");
					IDs.push_back(id);
				}
				if(shp->IsClosed())
					IDs.push_back(IDs[0]);

				shp->GetName(name);
				shp->GetString(desc);
				fprintf(fo,			"  <way id=\"%d\" version=\"1\">\n", shp->GetID());
				for (auto i : IDs)
					fprintf(fo,		"    <nd ref=\"%d\"/>\n", i);
				fprintf(fo,			"    <tag k=\"name\" v=\"%s\"/>\n", escape(name).c_str());
				if(!desc.empty())
					fprintf(fo,		"    <tag k=\"description\" v=\"%s\"/>\n", escape(desc).c_str());
				fprintf(fo,			"  </way>\n");
			}
			fprintf(fo,				"</osm>\n");
			fclose(fo);
		}
	}
}


void	WED_ExportPackToPath(WED_Thing * root, IResolver * resolver, const string& in_path, set<WED_Thing *>& problem_children)
{
	int result = DSF_Export(root, resolver, in_path,problem_children);
	if (result == -1)
		return;

	string	apt = in_path + "Earth nav data" DIR_STR "apt.dat";
	string	apt_dir = in_path + "Earth nav data";

	FILE_make_dir_exist(apt_dir.c_str());
	WED_AptExport(root, apt.c_str());

#if !TYLER_MODE
	string kml = in_path + "doc.kml";
	KmlExport(root, kml);
	string osm = in_path + "doc.osm";
	OsmExport(root, osm);
#endif
}


int		WED_CanExportPack(IResolver * resolver)
{
	return 1;
}

#include "WED_Airport.h"
#include "WED_AirportBoundary.h"
#include "WED_ATCFlow.h"
#include "WED_ATCRunwayUse.h"
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
#include "IHasResource.h"
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
	CollectRecursiveNoNesting(wrl, back_inserter(apts), WED_Airport::sClass);   // ATTENTION: all code here assumes 'normal' hierachies and no hidden items,
																				//	i.e. apts 1 level down, groups next, then items in them at 2 levels down.
	WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);                      // Speeds up recursive collecting, avoids recursing too deep.
	ISelection * sel = WED_GetSelect(resolver);
	
	int deleted_illicit_icao = 0;
	int added_local_codes = 0;
	int added_country_codes = 0;
	int grass_statistics[4] = { 0 };

	auto t0 = chrono::high_resolution_clock::now();

	wrl->StartCommand("Gateway upgrade heuristics");
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
				(*apt_itr)->EditMetaDataKey(META_KeyName(wed_AddMetaDataICAO),"");
				deleted_illicit_icao++;
				// In many cases a "bad" ICAO code is rather an local code.
				string local_code, faa_code;
				if((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataLocal))
					local_code = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataLocal);
				if((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataFAA))
					faa_code = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataFAA);
				if(local_code.empty() && faa_code.empty())
					(*apt_itr)->AddMetaDataKey(META_KeyName(wed_AddMetaDataLocal),ICAO_code);
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
					(*apt_itr)->AddMetaDataKey(META_KeyName(wed_AddMetaDataLocal),apt_ID);
					added_local_codes++;
				}
			}
			(*apt_itr)->GetICAO(ICAO_code); // just for subsequent LOG_MSG(), so they report something meaningful
		}
		// -- tag heliports that are oilrigs, so the sim uses a more specific symbol in the map
		if ((*apt_itr)->GetAirportType() == type_Heliport)
		{
			vector<WED_ObjPlacement*> oilrigs;
			CollectRecursive(*apt_itr, back_inserter(oilrigs), IgnoreVisiblity, [](WED_Thing* objs)->bool {
				string res;
				static_cast<WED_ObjPlacement*>(objs)->GetResource(res);
				return res.compare(0, strlen("lib/ships/OilRig"), "lib/ships/OilRig") == 0 ||
					   res.compare(0, strlen("lib/ships/OilPlat"), "lib/ships/OilPlat") == 0;
				},
				WED_ObjPlacement::sClass, 2);
			if (oilrigs.size())
				if (!(*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataOilrig) ||
					 atoi((*apt_itr)->GetMetaDataValue(wed_AddMetaDataOilrig).c_str()) != 1)
				{
					(*apt_itr)->AddMetaDataKey(META_KeyName(wed_AddMetaDataOilrig), "1");
				}
		}

		//-- upgrade Country Metadata -------------
		added_country_codes += add_iso3166_country_metadata(**apt_itr);

		//-- upgrade Ramp Positions with XP10.45 data to get parked A/C -------------
		wed_upgrade_ramps(*apt_itr);

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
			int num_replaced = wed_break_apart_special_agps(agp_placements, rmgr, out_added_objs);
			if (num_replaced > 0)
				LOG_MSG("Broke apart %d agp at %s\n", num_replaced, ICAO_code.c_str());

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
							r->SetName(r_nam.substr(1));
					}
			}
		}

		//-- Break up jetway AGP's, convert jetway objects into facades for XP12 moving jetways -------------
		if (int count = WED_DoConvertToJW(*apt_itr))
			LOG_MSG("Upgraded %d JW at %s\n", count, ICAO_code.c_str());

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
		}
#else
		// mow the grass
		vector<WED_Thing*> terFX;
		CollectRecursive(*apt_itr, back_inserter(terFX), IgnoreVisiblity, [](WED_Thing* t)->bool 
			{
				string res;
#if TYLER_MODE
				t->GetName(res);
				return res == "Terrain FX";
			},
			WED_Group::sClass, 1);
#else
				if (auto tr = dynamic_cast<IHasResource*>(t))  // thats pretty slow - the reason why in TYLER_MODE we go for the group only
				{                                              // but for user exports we can't rely on that group to already exist.
					tr->GetResource(res);
					return res.find("terrain_FX") != string::npos;
				}
				return false;
			}
		);
#endif
		if(terFX.empty())
		{
			if(WED_DoMowGrass(*apt_itr, grass_statistics))
				LOG_MSG("Mowed grass at %s\n", ICAO_code.c_str());
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
			sel->Clear();
			sel->Insert(vector<ISelectable*>(tree_objs.begin(), tree_objs.end()));
//			wrl->CommitCommand();
			WED_DoConvertToForest(resolver, false);
//			wrl->StartCommand("Restart after Forest");
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
			for(auto fst : forests)
				fst->SetResource("lib/vegetation/trees/deciduous/maple_medium.for");
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
			if (false) // ToDo !!!!
			{
				LOG_MSG("SoftEdged pavemnts  at %s\n", ICAO_code.c_str());
			}
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
				LOG_MSG("Added AptGrass Edges at %s\n", ICAO_code.c_str());
			}
		}
*/		//
		// The "big xp12 gateway reset" - remove certain features unless the submission is "recent" as
		//  measureds by the scenery ID (i.e. a cutoff point in time after which ONLY Xp12 ready sceneries were accepted) 
		// or presence of certain, XP12 only art assets
		//
#if TYLER_MODE
		if ((*apt_itr)->GetSceneryID() < 94010 && terFX.empty() && pavFX.empty())
#else   // artists exporting to GW target at home. They want to see what happens AFTER their scenery is submitted., i.e. when its a "X-Plane 12 submission".
		// we likely want to do these deletions when IMPORTING from the GW or even on the GW itself - so this GUNK doesn't get reintroduced by ignorant artists
		if (false)
#endif
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
					WED_RecursiveDelete(things);
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
				set<WED_Thing*> things(grunge_objs.begin(), grunge_objs.end());
				WED_RecursiveDelete(things);
				LOG_MSG("Deleted %zd Grunges at %s\n", grunge_objs.size(), ICAO_code.c_str());
			}

			// nuke ALL exclusions at airports, but only for 2D stuff like Beaches, Roads, Polygons, Lines at Sea/Heliports
			vector<WED_ExclusionZone*> exclusions;
			CollectRecursive(*apt_itr, back_inserter(exclusions), IgnoreVisiblity, TakeAlways,
				WED_ExclusionZone::sClass, 2);
			if (exclusions.size())
			{
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
				LOG_MSG("I/XP12 Deleted %d Exclusions at %s\n", (int) exclusions.size() + reduced_ex, ICAO_code.c_str());
			}
			// nuke all per-airport flatten
			AptInfo_t apt_info;
			(*apt_itr)->Export(apt_info);
			auto it = std::find(apt_info.meta_data.begin(), apt_info.meta_data.end(), make_pair(string("flatten"), string("1")));
			if (it != apt_info.meta_data.end())
			{
				apt_info.meta_data.erase(it);
				(*apt_itr)->Import(apt_info, dummyPrintf, nullptr);
				LOG_MSG("I/XP12 Deleted Always Flatten at %s\n", ICAO_code.c_str());
			}
			// get the 3D meta tag right
			if (gExportTarget == wet_gateway || TYLER_MODE)
			{
				Enforce_MetaDataGuiLabel(*apt_itr);
			}
		}

		//-- If any pattern rumnways are ever using one-way only, disable pattern flying. 
		// As current ATC WILl ignore the one-way use and send you fly real closed patterns.
		vector<WED_ATCFlow*> flows;
		CollectRecursive(*apt_itr, back_inserter(flows));
		for (auto f : flows)
		{
			int pattern = f->GetPatternRunway();
			vector<WED_ATCRunwayUse*> rules;
			CollectRecursive(f, back_inserter(rules));
			bool arrivals(false);
			bool departures(false);
			for (auto r : rules)
			{
				if (r->GetRunway() == pattern) // take ANY kind of ops in a direction as 'useable in a pinch for pattern work'
				{
					if (r->HasArrivals())   arrivals = true;
					if (r->HasDepartures()) departures = true;
				}
			}
			if (!arrivals || !departures)
			{
				(*apt_itr)->AddMetaDataKey(META_KeyName(wed_AddMetaDataCircuits), "0");
				LOG_MSG("I/XP12 Disallowed patterns at %s\n", ICAO_code.c_str());
			}
			break;
		}
#endif
#if TYLER_MODE
		double percent_done = (double)distance(apts.begin(), apt_itr) / apts.size() * 100;
		printf("%0.0lf%% through heuristic at %s\n", percent_done, ICAO_code.c_str());

		auto t1 = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed = t1 - t2;
		LOG_MSG("Update %s took %lf sec\n", ICAO_code.c_str(), elapsed.count());
		t2 = t1;
//		if(distance(apts.begin(), apt_itr) == 15) break;  // for quick testing, only upgrade a few airports
#endif
	}
	wrl->CommitCommand();

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
	LOG_MSG("I/exp Done with upgrade heuristics on %d apts, took %lf sec\n", (int) apts.size(), elapsed.count());
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

	auto uMgr = resolver->GetUndoMgr();
	if (gExportTarget == wet_gateway)
	{
		uMgr->MarkUndo();
		DoHueristicAnalysisAndAutoUpgrade(resolver);
	}
#endif
	ILibrarian * l = WED_GetLibrarian(resolver);
	WED_Thing * w = WED_GetWorld(resolver);
	WED_Group * g = dynamic_cast<WED_Group*>(w);
	DebugAssert(g);
	set<WED_Thing *>	problem_children;

	string pack_base;
	l->LookupPath(pack_base);

	WED_ExportPackToPath(g, resolver, pack_base, problem_children);

#if !TYLER_MODE
	if (gExportTarget == wet_gateway)
	{
		if (uMgr->UndoToMark())
			DoUserAlert("Some of the upgrade heuristics applied during export could not be undone. Scenery was permanently altered by export.");
	}
#endif
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
