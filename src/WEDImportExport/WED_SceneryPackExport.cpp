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
#include "WED_UIDefs.h"
#include "WED_Validate.h"

#include <iostream>

void	WED_ExportPackToPath(WED_Thing * root, IResolver * resolver, const string& in_path, set<WED_Thing *>& problem_children)
{
//	int result = DSF_Export(root, resolver, in_path,problem_children);
//	if (result == -1)
	{
//		return;
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
#include "WED_RampPosition.h"
#include "WED_Runway.h"
#include "WED_TruckDestination.h"
#include "WED_TruckParkingLocation.h"
#include "WED_ObjPlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_MetaDataKeys.h"
#include "WED_MetaDataDefaults.h"
#include "WED_Menus.h"
#include "GISUtils.h"
#include <chrono>

vector<vector<string> > translations = {
{"ABW",  "ARUBA",},			{"AFG",  "AFGHANISTAN",},	{"AGO",  "ANGOLA",},		{"AIA",  "ANGUILLA",},
{"ALA",  "≈LAND",},			{"ALB",  "ALBANIA",},		{"AND",  "ANDORRA",},		{"ARE",  "EMIRATES",},
{"ARG",  "ARGENTINA",},		{"ARM",  "ARMENIA",},		{"ASM",  "SAMOA",},			{"ATA",  "ANTARCTIC",},
{"ATF",  "FRENCH SOUTHERN",},			{"ATG",  "ANTIGUA", "BARBUDA",},			{"AUS",  "AUSTRALIA",},
{"AUT",  "AUSTRIA",},		{"AZE",  "AZERBAIJAN",},	{"BDI",  "BURUNDI",},		{"BEL",  "BELGIUM",},
{"BEN",  "BENIN",},			{"BES",  "BONAIRE", "SABA",},							{"BFA",  "BURKINA",},
{"BGD",  "BANGLADESH",},	{"BGR",  "BULGARIA",},		{"BHR",  "BAHRAIN",},		{"BHS",  "BAHAMAS",},
{"BIH",  "BOSNIA",},		{"BLM",  "BARTHÈLEMY", "BARTHÈLEMY",},					{"BLR",  "BELARUS",},
{"BLZ",  "BELIZE",},		{"BMU",  "BERMUDA",},		{"BOL",  "BOLIVIA",},		{"BRA",  "BRAZIL",},
{"BRB",  "BARBADOS",},		{"BRN",  "BRUNEI",},		{"BTN",  "BHUTAN",},		{"BVT",  "BOUVET",},
{"BWA",  "BOTSWANA",},		{"CAF",  "CENTRAL AFRICAN",},							{"CAN",  "CANADA",},
{"CCK",  "COCOS",},			{"CHE",  "SWITZERLAND", "SWISS",},						{"CHL",  "CHILE",},
{"CHN",  "CHINA",},			{"CIV",  "IVOIRE", "IVORY"},{"CMR",  "CAMEROON",},		{"COD",  "CONGO DEMO",},
{"COG",  "CONGO",},			{"COK",  "COOK",},			{"COL",  "COLOMBIA",},		{"COM",  "COMOROS",},
{"CPV",  "CABO VERDE",},	{"CRI",  "COSTA RICA",},	{"CUB",  "CUBA",},			{"CUW",  "CURAÁAO",	"CURACAO"},
{"CXR",  "CHRISTMAS",},		{"CYM",  "CAYMAN",},		{"CYP",  "CYPRUS",},		{"CZE",  "CZECHIA",},
{"DEU",  "GERMANY",},		{"DJI",  "DJIBOUTI",},		{"DMA",  "DOMINICA ",},		{"DNK",  "DENMARK",},
{"DOM",  "DOMINICAN R",},								{"DZA",  "ALGERIA",},		{"ECU",  "ECUADOR",},
{"EGY",  "EGYPT",},			{"ERI",  "ERITREA",},		{"ESH",  "SAHARA",},		{"ESP",  "SPAIN",},
{"EST",  "ESTONIA",},		{"ETH",  "ETHIOPIA",},		{"FIN",  "FINLAND",},		{"FJI",  "FIJI",},
{"FLK",  "FALKLAND",},		{"FRA",  "FRANCE",},		{"FRO",  "FAROE",},			{"FSM",  "MICRONESIA",},
{"GAB",  "GABON",},			{"GBR",  "UNITED KINGDOM", "BRITAIN",},					{"GEO",  "GEORGIA",},
{"GGY",  "GUERNSEY",},		{"GHA",  "GHANA",},			{"GIB",  "GIBRALTAR",},		{"GIN",  "GUINEA",},
{"GLP",  "GUADELOUPE",},	{"GMB",  "GAMBIA",},		{"GNB",  "GUINEA-BISSAU",},	{"GNQ",  "EQUATORIAL GUINEA",},
{"GRC",  "GREECE",},		{"GRD",  "GRENADA",},		{"GRL",  "GREENLAND",},		{"GTM",  "GUATEMALA",},
{"GUF",  "FRENCH GUIANA",},	{"GUM",  "GUAM",},			{"GUY",  "GUYANA",},		{"HKG",  "HONG KONG", "HONGKONG",},
{"HMD",  "HEARD ISLAND",},	{"HND",  "HONDURAS",},		{"HRV",  "CROATIA",},		{"HTI",  "HAITI",},
{"HUN",  "HUNGARY",},		{"IDN",  "INDONESIA",},		{"IMN",  "ISLE OF MAN",},	{"IND",  "INDIA",},
{"IOT",  "BRITISH INDIAN",},{"IRL",  "IRELAND",},		{"IRN",  "IRAN",},			{"IRQ",  "IRAQ",},
{"ISL",  "ICELAND",},		{"ISR",  "ISRAEL",},		{"ITA",  "ITALY",},			{"JAM",  "JAMAICA",},
{"JEY",  "JERSEY",},		{"JOR",  "JORDAN",},		{"JPN",  "JAPAN",},			{"KAZ",  "KAZAKHSTAN",},
{"KEN",  "KENYA",},			{"KGZ",  "KYRGYZSTAN",},	{"KHM",  "CAMBODIA",},		{"KIR",  "KIRIBATI",},
{"KNA",  "KITTS",},			{"KOR",  "SOUTH KOREA",},	{"KWT",  "KUWAIT",},		{"LAO",  "LAO",},
{"LBN",  "LEBANON",},		{"LBR",  "LIBERIA",},		{"LBY",  "LIBYA",},			{"LCA",  "LUCIA",},
{"LIE",  "LIECHTENSTEIN",},	{"LKA",  "LANKA",},			{"LSO",  "LESOTHO",},		{"LTU",  "LITHUANIA",},
{"LUX",  "LUXEMBOURG",},	{"LVA",  "LATVIA",},		{"MAC",  "MACAO",},			{"MAF",  "MARTIN",},
{"MAR",  "MOROCCO",},		{"MCO",  "MONACO",},		{"MDA",  "MOLDOVA",},		{"MDG",  "MADAGASCAR",},
{"MDV",  "MALDIVES",},		{"MEX",  "MEXICO",},		{"MHL",  "MARSHALL",},		{"MKD",  "MACEDONIA",},
{"MLI",  "MALI",},			{"MLT",  "MALTA",},			{"MMR",  "MYANMAR",},		{"MNE",  "MONTENEGRO",},
{"MNG",  "MONGOLIA",},		{"MNP",  "MARIANA",},		{"MOZ",  "MOZAMBIQUE",},	{"MRT",  "MAURITANIA",},
{"MSR",  "MONTSERRAT",},	{"MTQ",  "MARTINIQUE",},	{"MUS",  "MAURITIUS",},		{"MWI",  "MALAWI",},
{"MYS",  "MALAYSIA",},		{"MYT",  "MAYOTTE",},		{"NAM",  "NAMIBIA",},		{"NCL",  "CALEDONIA",},
{"NER",  "NIGER",},			{"NFK",  "NORFOLK",},		{"NGA",  "NIGERIA",},		{"NIC",  "NICARAGUA",},
{"NIU",  "NIUE",},			{"NLD",  "NETHERLANDS",},	{"NOR",  "NORWAY",},		{"NPL",  "NEPAL",},
{"NRU",  "NAURU",},			{"NZL",  "ZEALAND",},		{"OMN",  "OMAN ",},			{"PAK",  "PAKISTAN",},
{"PAN",  "PANAMA",},		{"PCN",  "PITCAIRN",},		{"PER",  "PERU",},			{"PHL",  "PHILIPPINES",},
{"PLW",  "PALAU",},			{"PNG",  "PAPUA", "NEW GUINEA",},						{"POL",  "POLAND",},
{"PRI",  "PUERTO RICO",},	{"PRK",  "NORTH KOREA",},	{"PRT",  "PORTUGAL",},		{"PRY",  "PARAGUAY",},
{"PSE",  "PALESTINE",},		{"PYF",  "POLYNESIA",},		{"QAT",  "QATAR",},			{"REU",  "RÈUNION",},
{"ROU",  "ROMANIA",},		{"RUS",  "RUSSIA",},		{"RWA",  "RWANDA",},
{"SAU",  "SAUDI ARABIA",},	{"SDN",  "SUDAN",},			{"SEN",  "SENEGAL",},		{"SGP",  "SINGAPORE",},
{"SGS",  "H GEORGIA", "SANDWICH",},						{"SHN",  "HELENA", "ASCENSION", "TRISTAN",},
{"SJM",  "SVALBARD",},		{"SLB",  "SOLOMON",},		{"SLE",  "SIERRA LEONE",},	{"SLV",  "SALVADOR",},
{"SMR",  "SAN MARINO",},	{"SOM",  "SOMALIA",},		{"SPM",  "PIERRE", "MIQUELON",},
{"SRB",  "SERBIA",},		{"SSD",  "SUDAN",},			{"STP",  "TOME", "PRINCIPE",},
{"SUR",  "SURINAME",},		{"SVK",  "SLOVAKIA",},		{"SVN",  "SLOVENIA",},		{"SWE",  "SWEDEN",},
{"SWZ",  "ESWATINI",},		{"SXM",  "MAARTEN",},		{"SYC",  "SEYCHELLES",},	{"SYR",  "SYRIA",},
{"TCA",  "TURKS", "CAICOS",},							{"TCD",  "CHAD",},			{"TGO",  "TOGO",},
{"THA",  "THAILAND",},		{"TJK",  "TAJIKISTAN",},	{"TKL",  "TOKELAU",},		{"TKM",  "TURKMENISTAN",},
{"TLS",  "TIMOR",},			{"TON",  "TONGA",},			{"TTO",  "TRINIDAD", "TOBAGO",},
{"TUN",  "TUNISIA",},		{"TUR",  "TURKEY",},		{"TUV",  "TUVALU",},		{"TWN",  "TAIWAN",},
{"TZA",  "TANZANIA",},		{"UGA",  "UGANDA",},		{"UKR",  "UKRAINE",},		{"UMI",  "UNITED STATES MINOR",},
{"URY",  "URUGUAY",},		{"USA",  "UNITED STATES", "U.S.A.",},
{"UZB",  "UZBEKISTAN",},	{"VAT",  "HOLY SEE",},				{"VCT",  "VINCENT", "GRENADINES",},
{"VEN",  "VENEZUELA",},		{"VGB",  "VIRGIN ISLANDS (B",},		{"VIR",  "VIRGIN ISLANDS (U",},
{"VNM",  "VIETNAM", "VIET NAM",},						{"VUT",  "VANUATU",},		{"WLF",  "WALLIS", "FUTUNA",},
{"WSM",  "SAMOA",},			{"YEM",  "YEMEN",},			{"ZAF",  "SOUTH AFRICA", "SOUTH-AFRICA",},
{"ZMB",  "ZAMBIA",},		{"ZWE",  "ZIMBABWE",}, };

static void	DoHueristicAnalysisAndAutoUpgrade(IResolver* resolver)
{
	LOG_MSG("## Tyler mode ## Starting upgrade heuristics\n");
	WED_Thing * wrl = WED_GetWorld(resolver);
	vector<WED_Airport*> apts;
	CollectRecursiveNoNesting(wrl, back_inserter(apts),WED_Airport::sClass);    // ATTENTION: all code here assumes 'normal' hierachies and no hidden items,
																				//	i.e. apts 1 level down, groups next, then items in them at 2 levels down.
	WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);                      // Speeds up recursive collecting, avoids recursing too deep.
	ISelection * sel = WED_GetSelect(resolver);
	
//	const bdy[] = {-130,49, -40,49, -80,24, -97,25, -107,31, -111,31, -115,33, -140,32};
//	Polygon2 FAA_bounds;
//	for (int i = 0; i < 16; i += 2)
//		FAA_bounds.push_back(Point2(bdy[i], bdy[i + 1]);

	int deleted_illicit_icao = 0;
	int added_local_code = 0;
	int grass_statistics[4] = { 0 };

	auto t0 = chrono::high_resolution_clock::now();
	
	for (auto apt_itr = apts.begin(); apt_itr != apts.end(); ++apt_itr)
	{
		auto t2 = chrono::high_resolution_clock::now();
		string ICAO_code;

		//-- Erase implausible ICAO and now undesired closed tags (the [X] in the name is now official) ----
		if ((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataICAO))
		{
			ICAO_code = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataICAO);

			bool illicit = ICAO_code.size() != 4 || toupper(ICAO_code[0]) < 'A' || toupper(ICAO_code[0]) >= 'Z';
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
					added_local_code++;
				}
			}
		}

		//-- upgrade Country Metadata -------------
		string country;
		bool has_iso(false);

		if ((*apt_itr)->ContainsMetaDataKey(wed_AddMetaDataCountry))
		{
			country = (*apt_itr)->GetMetaDataValue(wed_AddMetaDataCountry);

			bool has_iso = country.size() >= 3;
			for (int i = 0; i < 3 && has_iso; i++)
				has_iso &= (bool)isalpha(country[i]);
			if (country.size() > 3)
				has_iso &= country[3] == ' ';

			if (has_iso)
			{
				has_iso = false;
				for (auto& iso : iso3166_codes)
					if (strncmp(iso.front(), country.c_str(), 3) == 0)
					{
						has_iso = true;
						break;
					}
			}
		}
		if (!has_iso)
		{
			if (country == "US") country = "United States";
			else if (country == "CA") country = "Canada";
			else if (country == "FR") country = "France";
			else if (country == "DE") country = "Germany";
			else if (country == "RK") country = "South Korea";

			int matches = 0;
			string code3;
			if(country.size())
			{
				string country_l(country);
				std::transform(country_l.begin(), country_l.end(), country_l.begin(), [](unsigned char c) { return std::toupper(c); });
				country_l += " ";

				for (auto& list : translations)
					for (auto cty = list.begin() + 1; cty < list.end(); cty++)
						if (country_l.find(*cty) != string::npos)
						{
							code3 = list.front() + " ";
							matches++;
							break;
						}
			}
			if (matches == 0)
			{
				if (!ICAO_code.size())        // trust LR assigned airport ID's to have meaninful region prefix
				{
					string s;
					(*apt_itr)->GetName(s);
					if (s[0] == 'X' && s.size() > 4)
						ICAO_code = s.substr(1);
				}
				if (ICAO_code.size())         // trust explicit set and plausibility checked ICAO meta adata
				{
					ICAO_code = ICAO_code.substr(0, 2);
					if (ICAO_code[0] == 'K')			code3 = "USA ";
					else if (ICAO_code[0] == 'C')		code3 = "CAN ";
					else if (ICAO_code[0] == 'E')
					{
						if (ICAO_code[1] == 'D')		code3 = "DEU ";
						else if (ICAO_code[1] == 'G')	code3 = "GBR ";
						else if (ICAO_code[1] == 'S')	code3 = "SWE ";
						else if (ICAO_code[1] == 'N')	code3 = "NOR ";
						else if (ICAO_code[1] == 'K')	code3 = "DAN ";
						else if (ICAO_code[1] == 'P')	code3 = "POL ";
					}
					else if (ICAO_code[0] == 'L')
					{
						if (ICAO_code[1] == 'F')		code3 = "FRA ";
						else if (ICAO_code[1] == 'E')	code3 = "ESP ";
						else if (ICAO_code[1] == 'O')	code3 = "AUT ";
						else if (ICAO_code[1] == 'I')	code3 = "ITA ";
						else if (ICAO_code[1] == 'S')	code3 = "CHE ";
						else if (ICAO_code[1] == 'K')	code3 = "SWI ";
						else if (ICAO_code[1] == 'C')	code3 = "CYP ";
						else if (ICAO_code[1] == 'P')	code3 = "PRT ";
						else if (ICAO_code[1] == 'T')	code3 = "TUR ";
					}
					else if (ICAO_code[0] == 'Y')		code3 = "AUS ";
					else if (ICAO_code[0] == 'Z' && ICAO_code[1] != 'K' && ICAO_code[1] != 'M')
					{
														code3 = "CHN ";
						if (country.empty()) country = "China";
					}
					else if (ICAO_code == "FA")			code3 = "ZAF ";
					else if (ICAO_code == "GV")			code3 = "CPV ";
					else if (ICAO_code == "MG")			code3 = "GTM ";
					else if (ICAO_code == "SA")			code3 = "ARG ";
					else if (ICAO_code == "SB" || ICAO_code == "SD" || ICAO_code == "SI" || ICAO_code == "SJ" || ICAO_code == "SN"
						  || ICAO_code == "SW")			code3 = "BRA ";
					else if (ICAO_code == "SP")			code3 = "PER ";
					else if (ICAO_code == "VY")			code3 = "MMR ";
					else if (ICAO_code[0] == 'U' && ICAO_code[1] > 'D'	&& ICAO_code[1] != 'G' && ICAO_code[1] != 'K' && ICAO_code[1] != 'M'
						  && ICAO_code[1] != 'T')		code3 = "RUS ";
					else if (ICAO_code == "WA" || ICAO_code == "WI" || ICAO_code == "WQ" || ICAO_code == "WR") code3 = "IDN ";

					if(code3.empty())
						LOG_MSG("'%s' failed to resolve by ICAO\n", ICAO_code.c_str());
				}
			}
			if (matches > 1)
				LOG_MSG("'%s' matches %dx using %s\n", country.c_str(), matches, code3.c_str());

			wrl->StartCommand("Add country code");
			(*apt_itr)->AddMetaDataKey(META_KeyName(wed_AddMetaDataCountry), code3 + country);
			wrl->CommitCommand();
		}

		(*apt_itr)->GetName(ICAO_code);
#if 0
		//-- upgrade Ramp Positions with XP10.45 data to get parked A/C -------------
		vector<WED_RampPosition*> ramp_positions;
		CollectRecursive(*apt_itr, back_inserter(ramp_positions), IgnoreVisiblity, TakeAlways, WED_RampPosition::sClass, 2);

		int non_empty_airlines_strs = 0;
		int non_op_none_ramp_starts = 0;
		for (vector<WED_RampPosition*>::iterator ramp_itr = ramp_positions.begin(); ramp_itr != ramp_positions.end(); ++ramp_itr)
		{
			if ((*ramp_itr)->GetAirlines() != "")
				++non_empty_airlines_strs;

			if ((*ramp_itr)->GetRampOperationType() != ramp_operation_None)
				++non_op_none_ramp_starts;
		}

		if (non_empty_airlines_strs == 0 || non_op_none_ramp_starts == 0)
		{
			wrl->StartCommand("Upgrade Ramp Positions");
			if (wed_upgrade_one_airport(*apt_itr, rmgr, sel))
				wrl->CommitCommand();
			else
				wrl->AbortCommand();
		}
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

		if ((*apt_itr)->GetAirportType() == 1)
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
		vector<WED_Group*> grps;
		CollectRecursive(*apt_itr, back_inserter(grps), IgnoreVisiblity, [](WED_Thing* t)->bool 
			{
				string res;
				t->GetName(res);
				return res == "Terrain FX";
			},
			WED_Group::sClass, 1);
		if(grps.empty())
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
		// nuke all large terrain polygons at mid-lattitudes
		if(apt_box.p1.y() < 73.0 && apt_box.p1.y() > -60.0)
		{
			vector<WED_PolygonPlacement*> terrain_polys;
			CollectRecursive(*apt_itr, back_inserter(terrain_polys), IgnoreVisiblity, [](WED_Thing* t)->bool 
				{
					string res;
					static_cast<WED_PolygonPlacement*>(t)->GetResource(res);
					return res.compare(0, strlen("lib/g10/terrain10/"), "lib/g10/terrain10/") == 0;
				},
				WED_PolygonPlacement::sClass, 2);
			if (terrain_polys.size())
			{
				set<WED_Thing*> things;
				for (auto p : terrain_polys)
				{
					Bbox2 bounds;
					p->GetBounds(gis_Geo, bounds);
					if(LonLatDistMeters(bounds.bottom_left(), bounds.top_right()) > 20.0)      // passes at least 10m lettering drawn with snow texture
						CollectRecursive(p, inserter(things, things.end()), IgnoreVisiblity, TakeAlways);
				}
				wrl->StartCommand("Delete Terrain Polys");
				WED_RecursiveDelete(things);
				wrl->CommitCommand();
				LOG_MSG("Deleted %ld terrain polys at %s\n", terrain_polys.size(), ICAO_code.c_str());
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
			LOG_MSG("Deleted %d Grunges at %s\n", grunge_objs.size(), ICAO_code.c_str());
		}
		// nuke all "Grunge" draped objects
#endif
		double percent_done = (double)distance(apts.begin(), apt_itr) / apts.size() * 100;
		printf("%0.0f%% through heuristic at %s\n", percent_done, ICAO_code.c_str());
#endif
		auto t1 = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed = t1 - t2;
		LOG_MSG("Update %s took %lf sec\n", ICAO_code.c_str(), elapsed.count());
		t2 = t1;
//		if(distance(apts.begin(), apt_itr) == 15) break;
	}
#if TYLER_MODE == 11
	// Remove all remaining new XP12 stuff - so this needs to be run in an XP11 installation. 
	// Or items be copied to be local items in the Global Airports Scenery.
	WED_DoSelectMissingObjects(resolver);
	WED_DoClear(resolver);
#endif		
	LOG_MSG("Deleted %d illicit ICAO meta tags\n", deleted_illicit_icao);
	LOG_MSG("Added %d local code metas to prevent Airport_ID getting taken for ICAO\n", added_local_code);
	LOG_MSG("Mowed %d polys %d lines %d spots %d patches\n", grass_statistics[0], grass_statistics[1],grass_statistics[2],grass_statistics[3]);

	auto t1 = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = t1 - t0;
	LOG_MSG("## Tyler mode ## Done with upgrade heuristics, took %lf sec\n", elapsed.count());
	LOG_FLUSH();
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
