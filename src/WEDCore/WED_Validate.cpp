/* 
 * Copyright (c) 2013, Laminar Research.
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

#include "WED_Validate.h"
#include <iterator>

#include "WED_Globals.h"
#include "WED_Sign_Parser.h"
#include "WED_Runway.h"
#include "WED_Sealane.h"
#include "WED_Helipad.h"
#include "WED_Airport.h"
#include "WED_AirportSign.h"
#include "WED_ToolUtils.h"
#include "WED_FacadePlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_ObjPlacement.h"
#include "WED_StringPlacement.h"
#include "WED_LinePlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_FacadeNode.h"
#include "WED_RampPosition.h"
#include "WED_TaxiRoute.h"
#include "WED_TaxiRouteNode.h"
#include "WED_ATCFlow.h"
#include "WED_LibraryMgr.h"
#include "WED_AirportBoundary.h"
#include "WED_GISUtils.h"
#include "WED_Group.h"
#include "WED_ATCRunwayUse.h"
#include "WED_ATCWindRule.h"
#include "WED_EnumSystem.h"
#include "WED_Taxiway.h"
#include "WED_GroupCommands.h"
#include "IGIS.h"
#include <iomanip>
#include "WED_ValidateATCRunwayChecks.h"


#include "AptDefs.h"
#include "IResolver.h"
#include "ILibrarian.h"
#include "WED_LibraryMgr.h"
#include "WED_PackageMgr.h"

#include "CompGeomDefs2.h"
#include "CompGeomUtils.h"

#include "BitmapUtils.h"
#include "GISUtils.h"
#include "FileUtils.h"
#include "PlatformUtils.h"
#include "MathUtils.h"
#include "WED_ATCFrequency.h"

#define MAX_LON_SPAN_GATEWAY 0.2
#define MAX_LAT_SPAN_GATEWAY 0.2

// Until we get the taxi validation to create error lists, this 
// turns off the early exit when ATC nodes are messed up.
#define FIND_BAD_AIRPORTS 0

// Checks for zero length sides - can be turned off for grandfathered airports.
#define CHECK_ZERO_LENGTH 1

// This table is used to find the matching opposite direction for a given runway
// to detect head-on collisions.

static const int k_rwy_enums[73][2] = {
	{	atc_1,		atc_19		},
	{	atc_1L,		atc_19R		},
	{	atc_1C,		atc_19C		},
	{	atc_1R,		atc_19L		},

	{	atc_2,		atc_20		},
	{	atc_2L,		atc_20R		},
	{	atc_2C,		atc_20C		},
	{	atc_2R,		atc_20L		},

	{	atc_3,		atc_21		},
	{	atc_3L,		atc_21R		},
	{	atc_3C,		atc_21C		},
	{	atc_3R,		atc_21L		},

	{	atc_4,		atc_22		},
	{	atc_4L,		atc_22R		},
	{	atc_4C,		atc_22C		},
	{	atc_4R,		atc_22L		},

	{	atc_5,		atc_23		},
	{	atc_5L,		atc_23R		},
	{	atc_5C,		atc_23C		},
	{	atc_5R,		atc_23L		},

	{	atc_6,		atc_24		},
	{	atc_6L,		atc_24R		},
	{	atc_6C,		atc_24C		},
	{	atc_6R,		atc_24L		},

	{	atc_7,		atc_25		},
	{	atc_7L,		atc_25R		},
	{	atc_7C,		atc_25C		},
	{	atc_7R,		atc_25L		},

	{	atc_8,		atc_26		},
	{	atc_8L,		atc_26R		},
	{	atc_8C,		atc_26C		},
	{	atc_8R,		atc_26L		},

	{	atc_9,		atc_27		},
	{	atc_9L,		atc_27R		},
	{	atc_9C,		atc_27C		},
	{	atc_9R,		atc_27L		},

	{	atc_10,		atc_28		},
	{	atc_10L,		atc_28R		},
	{	atc_10C,		atc_28C		},
	{	atc_10R,		atc_28L		},

	{	atc_11,		atc_29		},
	{	atc_11L,		atc_29R		},
	{	atc_11C,		atc_29C		},
	{	atc_11R,		atc_29L		},

	{	atc_12,		atc_30		},
	{	atc_12L,		atc_30R		},
	{	atc_12C,		atc_30C		},
	{	atc_12R,		atc_30L		},

	{	atc_13,		atc_31		},
	{	atc_13L,		atc_31R		},
	{	atc_13C,		atc_31C		},
	{	atc_13R,		atc_31L		},

	{	atc_14,		atc_32		},
	{	atc_14L,		atc_32R		},
	{	atc_14C,		atc_32C		},
	{	atc_14R,		atc_32L		},

	{	atc_15,		atc_33		},
	{	atc_15L,		atc_33R		},
	{	atc_15C,		atc_33C		},
	{	atc_15R,		atc_33L		},

	{	atc_16,		atc_34		},
	{	atc_16L,		atc_34R		},
	{	atc_16C,		atc_34C		},
	{	atc_16R,		atc_34L		},

	{	atc_17,		atc_35		},
	{	atc_17L,		atc_35R		},
	{	atc_17C,		atc_35C		},
	{	atc_17R,		atc_35L		},

	{	atc_18,		atc_36		},
	{	atc_18L,		atc_36R		},
	{	atc_18C,		atc_36C		},
	{	atc_18R,		atc_36L		},

	{ 0, 0 },
};

static int get_opposite_rwy(int rwy_enum)
{
	DebugAssert(rwy_enum != atc_Runway_None);
	int p = 0;
	while(k_rwy_enums[p][0])
	{
		if(rwy_enum == k_rwy_enums[p][0])
			return k_rwy_enums[p][1];
		if(rwy_enum == k_rwy_enums[p][1])
			return k_rwy_enums[p][0];
		++p;
	}
	DebugAssert(!"Bad enum");
	return atc_Runway_None;
}

static bool cmp_frequency_type(WED_ATCFrequency* freq1, WED_ATCFrequency* freq2)
{
	AptATCFreq_t freq_info1;
	freq1->Export(freq_info1);
	
	AptATCFreq_t freq_info2;
	freq2->Export(freq_info2);
	
	return freq_info1.atc_type > freq_info2.atc_type;
}

static string format_freq(int f)
{
	int mhz = f / 100;
	int khz10 = f % 100;
	stringstream ss;
	ss << mhz << "." << std::setw(2) << std::setfill('0') << khz10;
	return ss.str();
}

vector<vector<WED_ATCFrequency*> > CollectAirportFrequencies(WED_Thing* who)
{
	vector<WED_ATCFrequency*> frequencies;
	CollectRecursive<back_insert_iterator<vector<WED_ATCFrequency*> > >(
		who,
		back_insert_iterator<vector<WED_ATCFrequency*> >(frequencies)
		);

	std::sort(frequencies.begin(),frequencies.end(), cmp_frequency_type);
	
	vector<vector<WED_ATCFrequency*> > sub_frequencies;

	vector<WED_ATCFrequency*>::iterator freq_itr = frequencies.begin();
	while(freq_itr != frequencies.end())
	{
		sub_frequencies.push_back(vector<WED_ATCFrequency*>());

		AptATCFreq_t freq_info;
		(*freq_itr)->Export(freq_info);

		int old_type = freq_info.atc_type;

		while(freq_itr != frequencies.end())
		{
			(*freq_itr)->Export(freq_info);
			
			if (freq_info.atc_type == old_type)
			{
				sub_frequencies.back().push_back(*freq_itr);
				++freq_itr;
				continue;
			}
			else
			{
				break;
			}
		}
	}
	return sub_frequencies;
}

// This digs out the resource for any WED_Thing that uses resources - it knows all of the types that
// use resources - used for library checks.
static bool GetThingResource(WED_Thing * who, string& r)
{
	WED_ObjPlacement * obj;
	WED_FacadePlacement * fac;
	WED_ForestPlacement * fst;
	WED_LinePlacement * lin;
	WED_StringPlacement * str;
	WED_PolygonPlacement * pol;
	
	#define CAST_WITH_CHECK(CLASS,VAR) \
	if(who->GetClass() == CLASS::sClass && (VAR = dynamic_cast<CLASS *>(who)) != NULL) { \
		VAR->GetResource(r); \
		return true; } 
	
	CAST_WITH_CHECK(WED_ObjPlacement,obj)
	CAST_WITH_CHECK(WED_FacadePlacement,fac)
	CAST_WITH_CHECK(WED_ForestPlacement,fst)
	CAST_WITH_CHECK(WED_LinePlacement,lin)
	CAST_WITH_CHECK(WED_StringPlacement,str)
	CAST_WITH_CHECK(WED_PolygonPlacement,pol)
	
	return false;
}

static bool IsThingResource(WED_Thing * who)
{
	string r;
	return GetThingResource(who,r);
}

// This template buidls an error list for a subset of objects that have the same name - one validation error is generated
// for each set of same-named objects.
template <typename T>
static bool CheckDuplicateNames(const T& container, validation_error_vector& msgs, WED_Airport * owner, const string& msg)
{
	typedef map<string, vector<typename T::value_type> > name_map_t;
	name_map_t name_index;
	for(typename T::const_iterator i = container.begin(); i != container.end(); ++i)
	{
		string n;
		(*i)->GetName(n);
		typename name_map_t::iterator ni = name_index.find(n);
		if(ni == name_index.end())
			ni = name_index.insert(typename name_map_t::value_type(n, typename name_map_t::mapped_type())).first;
			
		ni->second.push_back(*i);		
	}
	
	bool ret = false;
	for(typename name_map_t::iterator ii = name_index.begin(); ii != name_index.end(); ++ii)
	{
		if(ii->second.size() > 1)
		{
			ret = true;
			validation_error_t err;
			err.msg = msg;
			copy(ii->second.begin(),ii->second.end(),back_inserter(err.bad_objects));
			err.airport = owner;
			msgs.push_back(err);
		}
	}
	
	return ret;
}

static void ValidateOnePointSequence(WED_Thing* who, validation_error_vector& msgs, IGISPointSequence* ps, WED_Airport * apt)
{
	int nn = ps->GetNumSides();
	if(nn < 1)
	{
		msgs.push_back(validation_error_t("Linear feature needs at least two points.",dynamic_cast<WED_Thing *>(ps),apt));
	}
	
	for(int n = 0; n < nn; ++n)
	{
		Bezier2 b; Segment2 s;
		bool bez = ps->GetSide(gis_Geo,n,s,b);
		if(bez) {s.p1 = b.p1; s.p2 = b.p2; }
		
		if(s.p1 == s.p2)
		{
			WED_Thing * parent = who->GetParent();
			if(parent)
			{
				if (parent->GetClass() == WED_ForestPlacement::sClass ||
					parent->GetClass() == WED_FacadePlacement::sClass ||
					parent->GetClass() == WED_DrapedOrthophoto::sClass ||
					parent->GetClass() == WED_PolygonPlacement::sClass)
				{
				#if CHECK_ZERO_LENGTH
					msgs.push_back(validation_error_t(string("Zero length side on line or polygon, parent is a '") + parent->GetClass() + "'.", dynamic_cast<WED_Thing *>(ps), apt));
				#endif
				}
			}
		}
	}
}

static void ValidatePointSequencesRecursive(WED_Thing * who, validation_error_vector& msgs, WED_Airport * apt)
{
	// Don't validate hidden stuff - we won't export it!
	WED_Entity * ee = dynamic_cast<WED_Entity *>(who);
	if(ee && ee->GetHidden())
		return;

	IGISPointSequence * ps = dynamic_cast<IGISPointSequence *>(who);
	if(ps)
	{
		ValidateOnePointSequence(who,msgs,ps,apt);
	}
	int nn = who->CountChildren();
	for(int n = 0; n < nn; ++n)
	{
		WED_Thing * c = who->GetNthChild(n);
		if(c->GetClass() != WED_Airport::sClass)
			ValidatePointSequencesRecursive(c,msgs,apt);
	}
}


//------------------------------------------------------------------------------------------------------------------------------------
// DSF VALIDATIONS
//------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -

static void ValidateOneFacadePlacement(WED_Thing* who, validation_error_vector& msgs, WED_Airport * apt)
{
	/*--Facade Validate Rules--------------------------------------------------
		wet_xplane_900 rules
		  - Custom facade wall choices are only in X-Plane 10 and newer
		  - Curved facades are only supported in X-Plane 10 and newer
		Other rules
		  - Facades may not have holes in them
	 */

	WED_FacadePlacement * fac = dynamic_cast<WED_FacadePlacement*>(who);
	DebugAssert(who);
	if(gExportTarget == wet_xplane_900 && fac->HasCustomWalls())
	{
		msgs.push_back(validation_error_t("Custom facade wall choices are only supported in X-Plane 10 and newer.",who,apt));
	}
	
	if(fac->GetNumHoles() > 0)
	{
		msgs.push_back(validation_error_t("Facades may not have holes in them.",who,apt));
	}

	if(gExportTarget == wet_xplane_900 && WED_HasBezierPol(fac))
		msgs.push_back(validation_error_t("Curved facades are only supported in X-Plane 10 and newer.",who,apt));
}

static void ValidateOneForestPlacement(WED_Thing* who, validation_error_vector& msgs, WED_Airport * apt)
{
	/*--Forest Placement Rules
		wet_xplane_900 rules
			- Line and point are only supported in X-Plane 10 and newer
		*/

	WED_ForestPlacement * fst = dynamic_cast<WED_ForestPlacement *>(who);
	DebugAssert(fst);
	if(gExportTarget == wet_xplane_900 && fst->GetFillMode() != dsf_fill_area)
		msgs.push_back(validation_error_t("Line and point forests are only supported in X-Plane 10 and newer.",who,apt));
}

static void ValidateDSFRecursive(WED_Thing * who, WED_LibraryMgr* library_mgr, validation_error_vector& msgs, WED_Airport * parent_apt)
{
	// Don't validate hidden stuff - we won't export it!
	WED_Entity * ee = dynamic_cast<WED_Entity *>(who);
	if(ee && ee->GetHidden())
		return;
		
	if(who->GetClass() == WED_FacadePlacement::sClass)
	{
		ValidateOneFacadePlacement(who, msgs, parent_apt);
	}
	
	if(who->GetClass() == WED_ForestPlacement::sClass)
	{
		ValidateOneForestPlacement(who, msgs, parent_apt);
	}
	
	if(gExportTarget == wet_gateway)
	{
		if(who->GetClass() != WED_Group::sClass)
		if(!parent_apt)
			msgs.push_back(validation_error_t("You cannot export airport overlays to the X-Plane Airport Gateway if overlay elements are outside airports in the hierarchy.",who,NULL));
	}

	//--Validate resources-----------------------------------------------------
	IHasResource* resource_containing_who = dynamic_cast<IHasResource*>(who);

	if(resource_containing_who != NULL)
	{
		string resource_str;
		resource_containing_who->GetResource(resource_str);

		//1. Is the resource entirely missing
		
		string path;
		if (GetSupportedType(resource_str.c_str()) != -1)
		{
			path = gPackageMgr->ComputePath(library_mgr->GetLocalPackage(), resource_str);
		}
		else
		{
			path = library_mgr->GetResourcePath(resource_str);
		}
		
		if(FILE_exists(path.c_str()) == false)
		{
			if(parent_apt != NULL)
			{
				msgs.push_back(validation_error_t(string(who->HumanReadableType()) + "'s resource " + resource_str + " cannot be found", who, parent_apt));
			}
		}
		
		//3. What happen if the user free types a real resource of the wrong type into the box?
		bool matches = false;
#define EXTENSION_DOES_MATCH(CLASS,EXT) (who->GetClass() == CLASS::sClass && resource_str.substr(resource_str.find_last_of(".")) == EXT) ? true : false;
		matches |= EXTENSION_DOES_MATCH(WED_DrapedOrthophoto, ".pol");
		matches |= EXTENSION_DOES_MATCH(WED_DrapedOrthophoto, FILE_get_file_extension(path)); //This may be a tautology
		matches |= EXTENSION_DOES_MATCH(WED_FacadePlacement,  ".fac");
		matches |= EXTENSION_DOES_MATCH(WED_ForestPlacement,  ".for");
		matches |= EXTENSION_DOES_MATCH(WED_LinePlacement,    ".lin");
		matches |= EXTENSION_DOES_MATCH(WED_ObjPlacement,     ".obj");
		matches |= EXTENSION_DOES_MATCH(WED_ObjPlacement,     ".agp");
		matches |= EXTENSION_DOES_MATCH(WED_PolygonPlacement, ".pol");
		matches |= EXTENSION_DOES_MATCH(WED_StringPlacement,  ".str");

		if(matches == false)
		{
			msgs.push_back(validation_error_t("Resource " + resource_str + " does not have the correct file type", who, parent_apt));
		}
	}
	//-----------------------------------------------------------------------//
	int nn = who->CountChildren();
	for (int n = 0; n < nn; ++n)
	{
		WED_Thing * c = who->GetNthChild(n);
		if(c->GetClass() != WED_Airport::sClass)
			ValidateDSFRecursive(c, library_mgr, msgs, parent_apt);
	}	
}


//------------------------------------------------------------------------------------------------------------------------------------
// ATC VALIDATIONS
//------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -

static void ValidateAirportFrequencies(WED_Airport* who, validation_error_vector& msgs)
{
	//Collect all frequencies and group them by type into smaller vectors 
	vector<vector<WED_ATCFrequency*> > sub_freqs = CollectAirportFrequencies(who);

	vector<WED_ATCFrequency* > has_atc;

	bool has_tower = false;

	//Key=frequency like 12880 or 99913, Value is pointer to WED_ATCFrequency, part of the data model
	map<int, vector<WED_ATCFrequency*> > all_freqs;

	//For all groups see if each group has atleast one valid member (especially for Delivery, Ground, and Tower)
	for(vector<vector<WED_ATCFrequency*> >::iterator itr = sub_freqs.begin(); itr != sub_freqs.end(); ++itr)
	{
		bool found_one_valid = false;
		bool found_one_oob = false;		// found an out-of-band frequency for our use
		bool is_xplane_atc_related = false;
		//Contains values like "128.80" or "0.25" or "999.13"
		AptATCFreq_t freq_info;
		
		DebugAssert(!itr->empty());
		
		for(vector<WED_ATCFrequency*>::iterator freq = itr->begin(); freq != itr->end(); ++freq)
		{
			(*freq)->Export(freq_info);
			int mhz = freq_info.freq / 100;
			int last_digit = freq_info.freq % 10;
			string freq_str = format_freq(freq_info.freq);

			all_freqs[freq_info.freq].push_back(*freq);

			const int freq_type = ENUM_Import(ATCFrequency, freq_info.atc_type);
			is_xplane_atc_related = freq_type == atc_Delivery || freq_type == atc_Ground || freq_type == atc_Tower;
			
			if(freq_type == atc_Tower)
				has_tower = true;
			else if(is_xplane_atc_related)
				has_atc.push_back(*freq);

			if(mhz < 0 || mhz > 1000)
			{
				msgs.push_back(validation_error_t(string("Frequency ") + freq_str + " not between 0 and 1000 Mhz.",*freq,who));
				continue;
			}

			bool in_civilian_band = mhz >= 118 && mhz <= 136;

			//We only care about Delivery, Ground, and Tower frequencies
			if(is_xplane_atc_related)
			{
				if(in_civilian_band == false)
				{
					found_one_oob = true;
				}
				else
				{
					if(last_digit == 0 || last_digit == 2 || last_digit == 5 || last_digit == 7)
					{
						found_one_valid = true;
					}
					else
					{
						msgs.push_back(validation_error_t(string("The ATC frequency ") + freq_str + " is illegal. (Clearance Delivery, Ground, and Tower frequencies in the civilian band must be on 25 khz spacing.)",
							*freq, who));
					}
				}
			}
		}

		if(found_one_valid == false && is_xplane_atc_related)
		{		
			stringstream ss;
			ss  << "Could not find at least one valid ATC Frequency for group " << ENUM_Desc(ENUM_Import(ATCFrequency, freq_info.atc_type)) << ". "
			    << "Ensure all frequencies in this group end in 0, 2, 5, or 7.";				
			msgs.push_back(validation_error_t(ss.str(),*itr, who));
		}
	}
	
	for (map<int, vector<WED_ATCFrequency *> >::iterator f = all_freqs.begin(); f != all_freqs.end(); ++f)
	{
		if (f->second.size() > 1)
		{
			vector<WED_Thing*> problem_children;
			for (vector<WED_ATCFrequency*>::iterator itr = f->second.begin(); itr != f->second.end(); ++itr)
			{
				AptATCFreq_t apt_atc_freq;
				(*itr)->Export(apt_atc_freq);
				const int freq_type = ENUM_Import(ATCFrequency,apt_atc_freq.atc_type);
				if (freq_type == atc_AWOS     ||
					freq_type == atc_Delivery ||
					freq_type == atc_Ground   ||
					freq_type == atc_Tower)
				{
					problem_children.push_back(*itr);
				}
			}

			if (problem_children.size() > 1)
			{
				msgs.push_back(validation_error_t(string("The frequency ") + format_freq(f->first) + " is used more than once at this airport.", problem_children, who));
			}
		}
	}

	if(!has_atc.empty() && !has_tower)
	{
		msgs.push_back(validation_error_t("This airport has ground or delivery but no tower.  Add a control tower frequency or remove ground/delivery.", has_atc, who));
	}
	
}

static void ValidateOneATCRunwayUse(WED_ATCRunwayUse* use, validation_error_vector& msgs, WED_Airport * apt)
{
	AptRunwayRule_t urule;
	use->Export(urule);
	if(urule.operations == 0)
		msgs.push_back(validation_error_t("ATC runway use must support at least one operation type.",use, apt));
	else if(urule.equipment == 0)
		msgs.push_back(validation_error_t("ATC runway use must support at least one equipment type.", use, apt));
}

struct TaxiRouteInfo2
{
	TaxiRouteInfo2(WED_TaxiRoute* taxiroute, const CoordTranslator2 translator)
		: taxiroute_ptr(taxiroute),
		node_0(static_cast<WED_GISPoint*>(taxiroute->GetNthSource(0))),
		node_1(static_cast<WED_GISPoint*>(taxiroute->GetNthSource(1)))
	{
		AptRouteEdge_t apt_route;
		taxiroute->Export(apt_route);
		taxiroute_name = apt_route.name;

		Bezier2 bez;
		taxiroute->GetSide(gis_Geo, 0, taxiroute_segment_geo, bez);
		taxiroute_segment_m = Segment2(translator.Forward(taxiroute_segment_geo.p1), translator.Forward(taxiroute_segment_geo.p2));

		nodes_m[0] = Point2();
		nodes_m[1] = Point2();

		node_0->GetLocation(gis_Geo, nodes_m[0]);
		nodes_m[0] = translator.Forward(nodes_m[0]);

		node_1->GetLocation(gis_Geo, nodes_m[1]);
		nodes_m[1] = translator.Forward(nodes_m[1]);
	}

	//Pointer to the original WED_TaxiRoute in WED's data model
	WED_TaxiRoute* taxiroute_ptr;

	//Name of the taxiroute
	string taxiroute_name;

	//Segment2 representing the taxiroute in lat/lon
	Segment2 taxiroute_segment_geo;

	//Segment2 representing the taxiroute in meters
	Segment2 taxiroute_segment_m;

	//Source node of the taxiroute
	WED_GISPoint* node_0;

	//Target node of the taxiroute
	WED_GISPoint* node_1;

	//0 is node 0,
	//1 is node 1
	Point2 nodes_m[2];
};

static void TJunctionTest(vector<WED_TaxiRoute*> all_taxiroutes, validation_error_vector& msgs, WED_Airport * apt)
{
	static CoordTranslator2 translator;
	Bbox2 box;
	apt->GetBounds(gis_Geo, box);
	CreateTranslatorForBounds(box,translator);

	/*For each edge A
		for each OTHER edge B
		
		If A and B intersect, do not mark them as a T - the intersection test will pick this up and we don't want to have double errors on a single user problem.
		else If A and B share a common vertex, do not mark them as a T junction - this is legal. (Do this by comparing the Point2, not the IGISPoint *.If A and B have separate exactly on top of each other nodes, the duplicate nodes check will find this, and again we don't want to squawk twice on one user error.

			for each end B(B src and B dst)
				if the distance between A and the end node you are testing is < M meters
					validation failure - that node is too close to a taxiway route but isn't joined.
	*/

	for (vector<WED_TaxiRoute*>::iterator edge_a_itr = all_taxiroutes.begin(); edge_a_itr != all_taxiroutes.end(); ++edge_a_itr)
	{
		for (vector<WED_TaxiRoute*>::iterator edge_b_itr = all_taxiroutes.begin(); edge_b_itr != all_taxiroutes.end(); ++edge_b_itr)
		{
			//Skip over the same ones
			if (edge_a_itr == edge_b_itr)
			{
				continue;
			}

			TaxiRouteInfo2 edge_a(*edge_a_itr,translator);
			TaxiRouteInfo2 edge_b(*edge_b_itr,translator);

			//tmp doesn't matter to us
			Point2 tmp;
			if (edge_a.taxiroute_segment_m.intersect(edge_b.taxiroute_segment_m,tmp) == true)
			{
				//An intersection is different from a T junction
				continue;
			}
			
			bool found_duplicate = false;
			for (int i = 0; i < 2 && found_duplicate == false; i++)
			{
				for (int j = 0; j < 2 && found_duplicate == false; j++)
				{
					if (edge_a.nodes_m[i] == edge_b.nodes_m[j])
					{
						//This is a duplicate of the doubled up vertex test
						found_duplicate = true;
					}
				}
			}

			if (found_duplicate == true)
			{
				//Try another one
				continue;
			}

			const double TJUNCTION_THRESHOLD = 5.00;
			for (int i = 0; i < 2; i++)
			{
				double dist_a_to_b_node = sqrt(edge_a.taxiroute_segment_m.squared_distance(edge_b.nodes_m[i]));

				if (dist_a_to_b_node < TJUNCTION_THRESHOLD)
				{
					vector<WED_Thing*> problem_children;
					problem_children.push_back(*edge_a_itr);
					
					string problem_node_name;

					if (i == 0)//src
					{
						problem_children.push_back((edge_b.node_0));
						edge_b.node_0->GetName(problem_node_name);
					}
					else if(i == 1)
					{
						problem_children.push_back((edge_b.node_1));
						edge_b.node_1->GetName(problem_node_name);
					}

					msgs.push_back(validation_error_t("Taxi route " + edge_a.taxiroute_name + " and " + problem_node_name + " form a T junction, without being joined", problem_children, apt));
				}
			}
		}
	}
}

static void ValidateOneATCFlow(WED_ATCFlow * flow, validation_error_vector& msgs, set<int>& legal_rwy_oneway, WED_Airport * apt)
{
	string name;
	flow->GetName(name);
	AptFlow_t exp;
	flow->Export(exp);
	if(exp.icao.empty())
		msgs.push_back(validation_error_t(string("ATC Flow '") + name + "' has a blank ICAO code for its visibility METAR source.",flow, apt));

	if(name.empty())
		msgs.push_back(validation_error_t("An ATC flow has a blank name.  You must name every flow.",flow, apt));

	vector<WED_ATCWindRule*>	wind;
	vector<WED_ATCRunwayUse*>	ruse;
	
	CollectRecursive(flow, back_inserter(wind));
	CollectRecursive(flow, back_inserter(ruse));

	if(ruse.empty())
		msgs.push_back(validation_error_t("You have an airport flow with no runway use rules.  You need at least oneway use rule to create an active runway.",flow, apt));

	if(legal_rwy_oneway.count(flow->GetPatternRunway()) == 0)
		msgs.push_back(validation_error_t(string("The pattern runway ") + string(ENUM_Desc(flow->GetPatternRunway())) + " is illegal for the ATC flow '" + name + "' because it is not a runway at this airport.",flow, apt));

	for(vector<WED_ATCWindRule*>::iterator w = wind.begin(); w != wind.end(); ++w)
	{
		WED_ATCWindRule * wrule = *w;
		AptWindRule_t exp;
		wrule->Export(exp);
		if(exp.icao.empty())
			msgs.push_back(validation_error_t(string("ATC wind rule '") + name + "' has a blank ICAO code for its METAR source.", wrule, apt));
	}
	
	#if !GATEWAY_IMPORT_FEATURES

	map<int,vector<WED_ATCRunwayUse*> >		arrival_rwys;
	map<int,vector<WED_ATCRunwayUse*> >		departure_rwys;

	for(vector<WED_ATCRunwayUse*>::iterator r = ruse.begin(); r != ruse.end(); ++r)
	{
		WED_ATCRunwayUse * use = *r;
		ValidateOneATCRunwayUse(use,msgs,apt);
		int rwy = use->GetRunway();
		if(rwy == atc_Runway_None)
		{
			msgs.push_back(validation_error_t("Runway use has no runway selected.",use,apt));
		} 
		else
		{
			if(use->HasArrivals())
			{
				if(arrival_rwys.count(get_opposite_rwy(rwy)))
				{
					msgs.push_back(validation_error_t("Airport flow has opposite direction arrivals.", arrival_rwys[get_opposite_rwy(rwy)], apt));
					msgs.back().bad_objects.push_back(use);
				}
				arrival_rwys[rwy].push_back(use);
			}
			if(use->HasDepartures())
			{
				if(departure_rwys.count(get_opposite_rwy(rwy)))
				{
					msgs.push_back(validation_error_t("Airport flow has opposite direction departures.", departure_rwys[get_opposite_rwy(rwy)], apt));
					msgs.back().bad_objects.push_back(use);
				}
				departure_rwys[rwy].push_back(use);				
			}
		}
	}
	#endif
}

static void ValidateATC(WED_Airport* apt, validation_error_vector& msgs, set<int>& legal_rwy_oneway, set<int>& legal_rwy_twoway)
{
	vector<WED_ATCFlow *>		flows;
	vector<WED_TaxiRoute *>	taxi_routes;
	
	CollectRecursive(apt,back_inserter(flows));
	CollectRecursive(apt,back_inserter(taxi_routes));
	
	if(gExportTarget == wet_xplane_900)
	{
		if(!flows.empty())
			msgs.push_back(validation_error_t("ATC flows are not legal in X-Plane 9.", flows, apt));
		if(!taxi_routes.empty())
			msgs.push_back(validation_error_t("ATC Taxi Routes are not legal in X-Plane 9.", flows, apt));
		return;
	}
	
	if(CheckDuplicateNames(flows, msgs, apt, "Two or more airport flows have the same name."))
	{
		return;
	}
	
	for(vector<WED_ATCFlow *>::iterator f = flows.begin(); f != flows.end(); ++f)
	{
		ValidateOneATCFlow(*f, msgs, legal_rwy_oneway, apt);
	}
	
	for(vector<WED_TaxiRoute *>::iterator t = taxi_routes.begin(); t != taxi_routes.end(); ++t)
	{
		WED_TaxiRoute * taxi = *t;
		string name;
		taxi->GetName(name);
		// See bug http://dev.x-plane.com/bugbase/view.php?id=602 - blank names are okay!
		//			if (name.empty() && !taxi->IsRunway())
		//			{
		//				msg = "This taxi route has no name.  All taxi routes must have a name so that ATC can give taxi instructions.";
		//			}

		if(taxi->HasInvalidHotZones(legal_rwy_oneway))
		{
			msgs.push_back(validation_error_t(string("The taxi route '") + name + "' has hot zones for runways not present at its airport.",
											taxi, apt));
		}

		if(taxi->IsRunway())
			if(legal_rwy_twoway.count(taxi->GetRunway()) == 0)
			{
				msgs.push_back(validation_error_t(string("The taxi route '") + name + "' is set to a runway not present at the airport.", taxi, apt));
			}

		Point2	start, end;
		taxi->GetNthPoint(0)->GetLocation(gis_Geo, start);
		taxi->GetNthPoint(1)->GetLocation(gis_Geo, end);
		if(start == end)
		{
#if CHECK_ZERO_LENGTH	
			msgs.push_back(validation_error_t(string("The taxi route '") + name + "' is zero length.",taxi,apt));
#endif
		}
	}

	TJunctionTest(taxi_routes, msgs, apt);
}

//------------------------------------------------------------------------------------------------------------------------------------
// AIRPORT VALIDATIONS
//------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -

static void ValidateOneAirportBoundary(WED_AirportBoundary* bnd, validation_error_vector& msgs, WED_Airport * apt)
{
	if(WED_HasBezierPol(bnd))
		msgs.push_back(validation_error_t("Do not use bezier curves in airport boundaries.",bnd,apt));
}

static void ValidateOneRampPosition(WED_RampPosition* ramp, validation_error_vector& msgs, WED_Airport * apt)
{
	AptGate_t	g;
	ramp->Export(g);

	if(gExportTarget == wet_xplane_900)
		if(g.equipment != 0)
			if(g.type != atc_ramp_misc || g.equipment != atc_traffic_all)
				msgs.push_back(validation_error_t("Ramp starts with specific traffic and types are only suported in X-Plane 10 and newer.",ramp,apt));

	if(g.equipment == 0)
		msgs.push_back(validation_error_t("Ramp starts must have at least one valid type of equipment selected.",ramp,apt));

	if(gExportTarget == wet_xplane_1050)
	{
		if(g.type == atc_ramp_misc || g.type == atc_ramp_hangar)
		{
			if(!g.airlines.empty() || g.ramp_op_type != ramp_operation_none)
			{
				msgs.push_back(validation_error_t("Ramp operation types and airlines are only allowed at real ramp types, e.g. gates and tie-downs, not misc and hangars.",ramp,apt));
			}
		}
	
		//Our flag to keep going until we find an error
		bool found_err = false;
		if(g.airlines == "" && !found_err)
		{
			//Error:"not really an error, we're just done here"
			found_err = true;
		}
		else if(g.airlines.length() < 3 && !found_err)
		{
			msgs.push_back(validation_error_t(string("Ramp start airlines string ") + g.airlines + " is not a group of three letters.",ramp,apt));
			found_err = true;
		}

		//The number of spaces 
		int num_spaces = 0;

		if(!found_err)
			for(string::iterator itr = g.airlines.begin(); itr != g.airlines.end(); itr++)
			{
				char c = *itr;
				if(c == ' ')
				{
					num_spaces++;
				}
				else 
				{
					if(c < 'a' || c > 'z')
					{
						msgs.push_back(validation_error_t(string("Ramp start airlines string ") + g.airlines + " contains non-lowercase letters.",ramp,apt));
						found_err = true;
					}
				}
			}

		//The length of the string
		int wo_spaces_len = (g.airlines.length() - num_spaces);
		if(wo_spaces_len % 3 != 0 && !found_err)
		{
			msgs.push_back(validation_error_t(string("Ramp start airlines string ") + g.airlines + " is not in groups of three letters.",ramp,apt));;
			found_err = true;
		}

		//ABC, num_spaces = 0 = ("ABC".length()/3) - 1
		//ABC DEF GHI, num_spaces = 2 = "ABCDEFGHI".length()/3 - 1
		//ABC DEF GHI JKL MNO PQR, num_spaces = 5 = "...".length()/3 - 1 
		if(num_spaces != (wo_spaces_len/3) - 1 && !found_err)
		{
			msgs.push_back(validation_error_t(string("Ramp start airlines string ") + g.airlines + " is not spaced correctly.",ramp,apt));
			found_err = true;
		}
	}
}

static void ValidateOneRunwayOrSealane(WED_Thing* who, validation_error_vector& msgs, WED_Airport * apt)
{
	/*--Runway/Sealane Validation Rules----------------------------------------
		Duplicate Runway/Sealane Name
		Low-end Name rules
		  - Empty Low-end Name
		  - Illegal suffix for low-end runway
		  - Illegal characters in its low-end name
		  - Illegal low-end number, must be between 1 and 36
		High-end Name rules
		  - Empty High-end Name
		  - Illegal suffix for high-end runway
		  - Illegal characters in its high-end name
		  - Illegal high-end number, must be between 19 and 36
		Other Suffix rules
		  - Mismatched suffixes, L vs R etc
		  - Suffix on only one end
		Runway Numbers rules
		  - Low number must be first
		  - High end is not the reciprocal of the low end
		Runway Other rules
		  - Width must be atleast 1 meter wide
		  - Water us bit a valid surface type (runway only)
		  - Overlapping displaced thresholds
		  - Illegal surface roughness, shuold be between 0 and 1, inclusive
		  - Has end outside world map
		  - Needs to be reversed to match its name
		  - Misaligned with its runway name
	 */

	string name, n1, n2;
	who->GetName(name);
	
	string::size_type p = name.find("/");
	if (p != name.npos)
	{
		n1 = name.substr(0,p);
		n2 = name.substr(p+1);
	} else
		n1 = name;

	int suf1 = 0, suf2 = 0;
	int	num1 = -1, num2 = -1;

	if (n1.empty())	
		msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an empty low-end name.",who,apt));
	else {
		int suffix = n1[n1.length()-1];
		if (suffix < '0' || suffix > '9')
		{
			if (suffix == 'L' || suffix == 'R' || suffix == 'C' || suffix == 'S') suf1 = suffix;
			else msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an illegal suffix for the low-end runway.",who,apt));
			n1.erase(n1.length()-1);
		}

		int i;
		for (i = 0; i < n1.length(); ++i)
		if (n1[i] < '0' || n1[i] > '9')
		{
			msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an illegal characters in its low-end name.",who,apt));
			break;
		}
		if (i == n1.length())
		{
			num1 = atoi(n1.c_str());
		}
		if (num1 < 1 || num1 > 36)
		{
			msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an illegal low-end number, which must be between 1 and 36.",who,apt));
			num1 = -1;
		}
	}

	if (p != name.npos)
	{
		if (n2.empty())	msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an empty high-end name.",who,apt));
		else {
			int suffix = n2[n2.length()-1];
			if (suffix < '0' || suffix > '9')
			{
				if (suffix == 'L' || suffix == 'R' || suffix == 'C' || suffix == 'S') suf2 = suffix;
				else msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an illegal suffix for the high-end runway.",who,apt));
				n2.erase(n2.length()-1);
			}

			int i;
			for (i = 0; i < n2.length(); ++i)
			if (n2[i] < '0' || n2[i] > '9')
			{
				msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an illegal characters in its high-end name.",who,apt));
				break;
			}
			if (i == n2.length())
			{
				num2 = atoi(n2.c_str());
			}
			if (num2 < 19 || num2 > 36)
			{
				msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an illegal high-end number, which must be between 19 and 36.",who,apt));
				num2 = -1;
			}
		}
	}

	if (suf1 != 0 && suf2 != 0)
	{
		if ((suf1 == 'L' && suf2 != 'R') ||
			(suf1 == 'R' && suf2 != 'L') ||
			(suf1 == 'C' && suf2 != 'C') ||
			(suf1 == 'S' && suf2 != 'S'))
				msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has mismatched suffixes - check L vs R, etc.",who,apt));
	}
	else if((suf1 == 0) != (suf2 == 0))
	{
		msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has a suffix on only one end.",who,apt));
	}
	if (num1 != -1 && num2 != -1)
	{
		if (num2 < num1)
			msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has mismatched runway numbers - the low number must be first.'",who,apt));
		else if (num2 != num1 + 18)
			msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has mismatched runway numbers - high end is not the reciprocal of the low-end.",who,apt));
	}

	{
		WED_GISLine_Width * lw = dynamic_cast<WED_GISLine_Width *>(who);
		Assert(lw);			
		if (lw->GetWidth() < 1.0) msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' must be at least one meter wide.",who,apt));

		WED_Runway * rwy = dynamic_cast<WED_Runway *>(who);
		if (rwy)
		{
			if(rwy->GetSurface() == surf_Water && gExportTarget == wet_gateway)
			{
				msgs.push_back(validation_error_t("Water is not a valid surface type for runways",who,apt));
			}

			if (rwy->GetDisp1() + rwy->GetDisp2() > rwy->GetLength()) msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has overlapping displaced thresholds.",who,apt));
			
			#if !GATEWAY_IMPORT_FEATURES
				if(rwy->GetRoughness() < 0.0 || rwy->GetRoughness() > 1.0) msgs.push_back(validation_error_t(string("The runway '") + name + "' has an illegal surface roughness. It should be in the range 0 to 1.",who,apt));
			#endif
			
		}
		
		Point2 ends[2];
		lw->GetNthPoint(0)->GetLocation(gis_Geo,ends[0]);
		lw->GetNthPoint(1)->GetLocation(gis_Geo,ends[1]);
		Bbox2	runway_extent(ends[0],ends[1]);
		if (runway_extent.xmin() < -180.0 ||
			runway_extent.xmax() >  180.0 ||
			runway_extent.ymin() <  -90.0 ||
			runway_extent.ymax() >   90.0)
		{
			msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' has an end outside the World Map.",who,apt));
		}	
		else
		{
			#if !GATEWAY_IMPORT_FEATURES
				double heading, len;
				Point2 ctr;
				Quad_2to1(ends, ctr, heading, len);
				double approx_heading = num1 * 10.0;
				double heading_delta = fabs(dobwrap(approx_heading - heading, -180.0, 180.0));
				if(heading_delta > 135.0)
					msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' needs to be reversed to match its name.",who,apt));
				else if(heading_delta > 45.0)
					msgs.push_back(validation_error_t(string("The runway/sealane '") + name + "' is misaligned with its runway name.",who,apt));
			#endif
		}
	}
}

static void ValidateOneHelipad(WED_Helipad* who, validation_error_vector& msgs, WED_Airport * apt)
{
	/*--Helipad Validation Rules-----------------------------------------------
		Helipad Name rules
		  - Name already used
		  - The selected helipad has no name
		  - Name does not start with letter H
		  - Name is longer than 3 characters
		  - Contains illegal characters, must be in the form of H<number>
		Helipad Width rules
		  - Helipad is less than one meter wide
		  - Helipad is less than one meter long
	 */
	string name, n1;
	who->GetName(name);
	
	n1 = name;
	if (n1.empty())
	{
		msgs.push_back(validation_error_t("The selected helipad has no name.",who,apt));
	}
	else
	{
		if (n1[0] != 'H')
		{
			msgs.push_back(validation_error_t(string("The helipad '") + name + "' does not start with the letter H.",who,apt));
		}
		else
		{
			if(n1.length() > 3)
			{
				msgs.push_back(validation_error_t(string("The helipad '") + name + "' is longer than the maximum 3 characters.",who,apt));
			}

			n1.erase(0,1);
			for (int i = 0; i < n1.length(); ++i)
			{
				if (n1[i] < '0' || n1[i] > '9')
				{
					msgs.push_back(validation_error_t(string("The helipad '") + name + "' conntains illegal characters in its name.  It must be in the form H<number>.",who,apt));
					break;
				}
			}
		}
	}

	{
		WED_Helipad * heli = dynamic_cast<WED_Helipad *>(who);
		if (heli->GetWidth() < 1.0) msgs.push_back(validation_error_t(string("The helipad '") + name + "' is less than one meter wide.",who,apt));
		if (heli->GetLength() < 1.0) msgs.push_back(validation_error_t(string("The helipad '") + name + "' is less than one meter long.",who,apt));
	}
}

static void ValidateOneTaxiSign(WED_AirportSign* airSign, validation_error_vector& msgs, WED_Airport * apt)
{
	/*--Taxi Sign Validation Rules---------------------------------------------
		See Taxi Sign spec and parser for detailed validation rules
	 */

	string signName;
	airSign->GetName(signName);

	//Create the necessary parts for a parsing operation
	parser_in_info in(signName);
	parser_out_info out;

	ParserTaxiSign(in,out);
	if(out.errors.size() > 0)
	{
		int MAX_ERRORS = 12;//TODO - Is this good?
		string m;
		for (int i = 0; i < MAX_ERRORS && i < out.errors.size(); i++)
		{
			m += out.errors[i].msg;
			m += '\n';
		}
		msgs.push_back(validation_error_t(m,airSign,apt));
	}
}

static void ValidateOneTaxiway(WED_Taxiway* twy, validation_error_vector& msgs, WED_Airport * apt)
{
	/*--Taxiway Validation Rules-----------------------------------------------
		Water is not a valide surface type for taxiways
		Outer boundry of taxiway is not at least 3 sided
		Hole of taxiway is not at least 3 sided
	 */

	if(twy->GetSurface() == surf_Water && gExportTarget == wet_gateway)
	{
		msgs.push_back(validation_error_t("Water is not a valid surface type for taxiways",twy,apt));
	}

	IGISPointSequence * ps;
	ps = twy->GetOuterRing();
	if(!ps->IsClosed() || ps->GetNumSides() < 3)
	{
		msgs.push_back(validation_error_t("Outer boundary of taxiway is not at least 3 sided.",twy,apt));
	}
	else
	{
		for(int h = 0; h < twy->GetNumHoles(); ++h)
		{
			ps = twy->GetNthHole(h);
			if(!ps->IsClosed() || ps->GetNumSides() < 3)
			{
				// Ben says: two-point holes are INSANELY hard to find.  So we do the rare thing and intentionally
				// hilite the hole so that the user can nuke it.				
				WED_Thing * h = dynamic_cast<WED_Thing *>(ps);
				{
					msgs.push_back(validation_error_t("Hole of taxiway is not at least 3 sided.",h ? h : (WED_Thing *) twy, apt));
				}
			}
		}
	}

}

//------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------

static void ValidateOneAirport(WED_Airport* apt, validation_error_vector& msgs, WED_LibraryMgr* lib_mgr)
{
	/*--Validate Airport Rules-------------------------------------------------
		Airport Name rules
		  - Empty ICAO code
		  - ICAO used twice in airport
		  - Duplicate runway names
		  - Duplicate helipad names
		  - No runways or helipads or sealanes at all
		  - Gateway: illegal use of third party library resources
	 */
	
	vector<WED_Runway *>		runways;
	vector<WED_Helipad *>		helipads;
	vector<WED_Sealane *>		sealanes;
	vector<WED_AirportSign *>	signs;
	vector<WED_Taxiway *>		taxiways;
	vector<WED_RampPosition*>	ramps;
	vector<WED_Thing *>		runway_or_sealane;
	
	string name, icao;
	apt->GetName(name);
	apt->GetICAO(icao);
	
	if(name.empty())
		msgs.push_back(validation_error_t("An airport contains no name.",apt,apt));
	else if(icao.empty())
		msgs.push_back(validation_error_t(string("The airport '") + name + "' has an empty ICAO code.",apt,apt));
	
	set<int>		legal_rwy_oneway;
	set<int>		legal_rwy_twoway;

	CollectRecursive(apt, back_inserter(runways));
	CollectRecursive(apt, back_inserter(helipads));
	CollectRecursive(apt, back_inserter(sealanes));
	CollectRecursive(apt, back_inserter(signs));
	CollectRecursive(apt, back_inserter(taxiways));
	CollectRecursive(apt, back_inserter(ramps));
	
	copy(runways.begin(), runways.end(), back_inserter(runway_or_sealane));
	copy(sealanes.begin(), sealanes.end(), back_inserter(runway_or_sealane));
	
	if(CheckDuplicateNames(helipads,msgs,apt,"A helipad name is used more than once."))
	{
		return;
	}
	
	if(CheckDuplicateNames(runway_or_sealane,msgs,apt,"A runway or sealane name is used more than once."))
	{
		return;
	}

	WED_GetAllRunwaysOneway(apt,legal_rwy_oneway);
	WED_GetAllRunwaysTwoway(apt,legal_rwy_twoway);

	if(runways.empty() && helipads.empty() && sealanes.empty())
		msgs.push_back(validation_error_t(string("The airport '") + name + "' contains no runways, sea lanes, or helipads.",apt,apt));
	
	#if !GATEWAY_IMPORT_FEATURES
	WED_DoATCRunwayChecks(*apt, msgs);
	#endif
	
	ValidateATC(apt, msgs, legal_rwy_oneway, legal_rwy_twoway);
	
	ValidateAirportFrequencies(apt,msgs);
	
	
	for(vector<WED_AirportSign *>::iterator s = signs.begin(); s != signs.end(); ++s)
	{
		ValidateOneTaxiSign(*s, msgs,apt);
	}

	for(vector<WED_Taxiway *>::iterator t = taxiways.begin(); t != taxiways.end(); ++t)
	{
		ValidateOneTaxiway(*t,msgs,apt);
	}

	for(vector<WED_Thing *>::iterator r = runway_or_sealane.begin(); r != runway_or_sealane.end(); ++r)
	{
		ValidateOneRunwayOrSealane(*r, msgs,apt);
	}
	
	for(vector<WED_Helipad *>::iterator h = helipads.begin(); h != helipads.end(); ++h)
	{
		ValidateOneHelipad(*h, msgs,apt);
	}
	
	for(vector<WED_RampPosition *>::iterator r = ramps.begin(); r != ramps.end(); ++r)
	{
		ValidateOneRampPosition(*r,msgs,apt);
	}
	
	if(gExportTarget == wet_gateway)
	{
		Bbox2 bounds;
		apt->GetBounds(gis_Geo, bounds);
		if(bounds.xspan() > MAX_LON_SPAN_GATEWAY ||
				bounds.yspan() > MAX_LAT_SPAN_GATEWAY)
		{
			msgs.push_back(validation_error_t("This airport is too big.  Perhaps a random part of the airport has been dragged to another part of the world?",apt,apt));
		}

#if !GATEWAY_IMPORT_FEATURES
		vector<WED_AirportBoundary *>	boundaries;
		CollectRecursive(apt, back_inserter(boundaries));
		for(vector<WED_AirportBoundary *>::iterator b = boundaries.begin(); b != boundaries.end(); ++b)
		{
			ValidateOneAirportBoundary(*b, msgs,apt);
		}
#endif
		
		vector<WED_DrapedOrthophoto *> orthos;
		CollectRecursive(apt, back_inserter(orthos));
		if(!orthos.empty())
			msgs.push_back(validation_error_t("Draped orthophotos are not allowed in the global airport database.",orthos,apt));

		
		vector<WED_Thing *>	res_users;
		CollectRecursive(apt, back_inserter(res_users), IsThingResource);
		for(vector<WED_Thing *>::iterator ru = res_users.begin(); ru != res_users.end(); ++ru)
		{
			string res;
			if(GetThingResource(*ru,res))
			{
				if(!lib_mgr->IsResourceDefault(res))
					msgs.push_back(validation_error_t(string("The library path '") + res + "' is not part of X-Plane's default installation and cannot be submitted to the global airport database.",
						*ru, apt));
				#if !GATEWAY_IMPORT_FEATURES
				if(lib_mgr->IsResourceDeprecatedOrPrivate(res))
					msgs.push_back(validation_error_t(string("The library path '") + res + "' is a deprecated or private X-Plane resource and cannot be used in global airports.",
						*ru, apt));
				#endif
			}
		}
	}

	ValidatePointSequencesRecursive(apt, msgs,apt);

	ValidateDSFRecursive(apt, lib_mgr, msgs, apt);	
}


bool	WED_ValidateApt(IResolver * resolver, WED_Thing * wrl)
{
#if FIND_BAD_AIRPORTS
	string exp_target_str;
	switch(gExportTarget)
	{
		case wet_xplane_900:
			exp_target_str = "wet_xplane_900";
			break;
		case wet_xplane_1000:
			exp_target_str = "wet_xplane_1000";
			break;
		case wet_xplane_1021:
			exp_target_str = "wet_xplane_1021";
			break;
		case wet_xplane_1050:
			exp_target_str = "wet_xplane_1050";
			break;
		case wet_gateway:
			exp_target_str = "wet_gateway";
			break;
		default: 
			AssertPrintf("Export target %s is unknown", exp_target_str.c_str());
			break;
	}
	
	printf("Export Target: %s\n", exp_target_str.c_str());
#endif

#if !GATEWAY_IMPORT_FEATURES
	string msg = "";
	if(WED_DoSelectZeroLength(resolver))
	{
		msg = "Your airport contains zero-length ATC routing lines. These should be deleted.";
#if !FIND_BAD_AIRPORTS
		DoUserAlert(msg.c_str());
		return false;
#else
		printf("%s\n", msg.c_str());
#endif
	}

	if(WED_DoSelectDoubles(resolver))
	{
		msg = "Your airport contains doubled ATC routing nodes. These should be merged.";
#if !FIND_BAD_AIRPORTS
		DoUserAlert(msg.c_str());
		return false;
#else
		printf("%s\n", msg.c_str());
#endif
	}

	if(WED_DoSelectCrossing(resolver))
	{
		msg = "Your airport contains crossing ATC routing lines with no node at the crossing point.  Split the lines and join the nodes.";
#if !FIND_BAD_AIRPORTS
		DoUserAlert(msg.c_str());
		return false;
#else
		printf("%s\n", msg.c_str());
#endif
	}
#endif

	validation_error_vector		msgs;

	if(wrl == NULL) wrl = WED_GetWorld(resolver);

	ISelection * sel = WED_GetSelect(resolver);
	
	WED_LibraryMgr * lib_mgr = 	WED_GetLibraryMgr(resolver);
	
	vector<WED_Airport *> apts;
	CollectRecursiveNoNesting(wrl, back_inserter(apts));
	CheckDuplicateNames(apts,msgs,NULL,"Duplicate airport name.");
	
	for(vector<WED_Airport *>::iterator a = apts.begin(); a != apts.end(); ++a)
	{
		ValidateOneAirport(*a, msgs, lib_mgr);
	}
	
	// These are programmed to NOT iterate up INTO airports.  But you can START them at an airport.
	// So...IF wrl (which MIGHT be the world or MIGHt be a selection or might be an airport) turns out to
	// be an airport, we hvae to tell it "this is our credited airport."  Dynamic cast gives us the airport
	// or null for 'free' stuff.
	ValidatePointSequencesRecursive(wrl, msgs,dynamic_cast<WED_Airport *>(wrl));
	ValidateDSFRecursive(wrl, lib_mgr, msgs, dynamic_cast<WED_Airport *>(wrl));
	
	for(validation_error_vector::iterator v = msgs.begin(); v != msgs.end(); ++v)
	{
		string aname;
		if(v->airport)
			v->airport->GetICAO(aname);
		printf("%s: %s\n", aname.c_str(), v->msg.c_str());
	}
	
	if(!msgs.empty())
	{
		DoUserAlert(msgs.front().msg.c_str());

		wrl->StartOperation("Select Invalid");
		sel->Clear();
		for(vector<WED_Thing *>::iterator b = msgs.front().bad_objects.begin(); b != msgs.front().bad_objects.end(); ++b)
			sel->Insert(*b);
		wrl->CommitOperation();
		return false;
	}
	return msgs.empty();
}
