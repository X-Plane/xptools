#include "WED_ValidateATCRunwayChecks.h"

#include <iterator>

#include "WED_EnumSystem.h"
#include "WED_Validate.h"

#include "WED_Airport.h"
#include "WED_ATCFlow.h"
#include "WED_ATCRunwayUse.h"
#include "WED_Runway.h"
#include "WED_TaxiRoute.h"
#include "WED_TaxiRouteNode.h"

#include "AptDefs.h"

#include "CompGeomDefs2.h"
#include "CompGeomUtils.h"
#include "GISUtils.h"

typedef vector<const WED_ATCRunwayUse*>  ATCRunwayUseVec_t;
typedef vector<const WED_ATCFlow*>       FlowVec_t;
typedef vector<const WED_Runway*>        RunwayVec_t;
typedef vector<const WED_TaxiRoute*>     TaxiRouteVec_t;
typedef vector<const WED_TaxiRouteNode*> TaxiRouteNodeVec_t;

static CoordTranslator2 translator;

//A special selection of info useful for these tests
struct RunwayInfo
{
	//All the information about the current runway future tests will want or need
	RunwayInfo(const WED_Runway* runway)
	{
		runway_ptr = runway;

		runway->GetName(runway_name);

		Point2 bounds[4];
		runway->GetCorners(gis_Geo,bounds);
		runway_corners_geo.push_back(bounds[0]);
		runway_corners_geo.push_back(bounds[1]);
		runway_corners_geo.push_back(bounds[2]);
		runway_corners_geo.push_back(bounds[3]);

		runway_corners_m.push_back(translator.Forward(bounds[0]));
		runway_corners_m.push_back(translator.Forward(bounds[1]));
		runway_corners_m.push_back(translator.Forward(bounds[2]));
		runway_corners_m.push_back(translator.Forward(bounds[3]));

		Point2 ends[2];
		runway->GetSource()->GetLocation(gis_Geo,ends[0]);
		runway->GetTarget()->GetLocation(gis_Geo,ends[1]);

		runway_centerline_geo = Segment2(ends[0], ends[1]);
		runway_centerline_m = Segment2(translator.Forward(ends[0]), translator.Forward(ends[1]));
	}

	//Pointer to the runway
	const WED_Runway* runway_ptr;

	//Name of current runway
	string runway_name;

	//The corners of the runway in lat/lon
	Polygon2 runway_corners_geo;
	
	//The corners of the runway in meters
	Polygon2 runway_corners_m;
	
	//The center line of the runway in lat/lon
	Segment2 runway_centerline_geo;

	//The center line of the runway in meters
	Segment2 runway_centerline_m;
};

//typedef vector<RunwayInfo>    RunwayInfoVec_t;

vector<RunwayInfo> CollectPotentiallyActiveRunways(const WED_Airport& apt)
{
	//Find all potentially active runways:
	//0 flows means treat all runways as potentially active
	//>1 means find all runways mentioned, ignoring duplicates

	FlowVec_t flows;

	CollectRecursive<back_insert_iterator<FlowVec_t>>(&apt,back_inserter<FlowVec_t>(flows));

	RunwayVec_t all_runways;

	CollectRecursive<back_insert_iterator<RunwayVec_t>>(&apt,back_inserter<RunwayVec_t>(all_runways));

	RunwayVec_t potentially_active_runways;
	if(flows.size() == 0)
	{
		potentially_active_runways = all_runways;
	}
	else
	{
		ATCRunwayUseVec_t use_rules;

		CollectRecursive<back_insert_iterator<ATCRunwayUseVec_t>>(&apt,back_inserter<ATCRunwayUseVec_t>(use_rules));

		for (RunwayVec_t::const_iterator runway_itr = all_runways.begin(); runway_itr != all_runways.end(); ++runway_itr)
		{
			for(ATCRunwayUseVec_t::const_iterator use_itr = use_rules.begin(); use_itr != use_rules.end(); ++use_itr)
			{
				AptRunwayRule_t runway_rule;
				(*use_itr)->Export(runway_rule);

				string runway_name;
				(*runway_itr)->GetName(runway_name);

				string runway_name_p1 = runway_name.substr(0,runway_name.find_first_of('/'));
				string runway_name_p2 = runway_name.substr(runway_name.find_first_of('/')+1);

				if( runway_rule.runway == runway_name_p1 ||
					runway_rule.runway == runway_name_p2)
				{
					potentially_active_runways.insert(potentially_active_runways.begin(), *runway_itr);
					break;
				}
			}
		}
	}

	vector<RunwayInfo> runway_info_vec;
	for(RunwayVec_t::const_iterator itr = potentially_active_runways.begin();
		itr != potentially_active_runways.end();
		++itr)
	{
		runway_info_vec.push_back(*itr);
	}

	return runway_info_vec;
}

static void DoRunwayHasMatchingTaxiRoute( const RunwayInfo& runway_info,
										  const TaxiRouteVec_t& all_taxiroutes,
										  string* msg,
										  const WED_Thing*& problem_thing)
{
	for(TaxiRouteVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin();
		taxiroute_itr != all_taxiroutes.end();
		++taxiroute_itr)
	{
		Segment2 taxiroute_segment_geo;
		Bezier2 bez;
		(*taxiroute_itr)->GetSide(gis_Geo, 0, taxiroute_segment_geo, bez);
#if DEV
		debug_mesh_segment(taxiroute_segment_geo,0,1,0,0,1,0);
#endif
		if( runway_info.runway_corners_geo.inside(taxiroute_segment_geo.p1) == true &&
			runway_info.runway_corners_geo.inside(taxiroute_segment_geo.p2))
		{
			continue;
		}
		else
		{
			int intersected_sides = 0;

			Segment2 left_side = runway_info.runway_corners_geo.side(2);
			debug_mesh_segment(left_side,0,0,1,0,0,1);

			Segment2 top_side = runway_info.runway_corners_geo.side(3);
			debug_mesh_segment(top_side,0,0,1,0,0,1);

			Segment2 right_side = runway_info.runway_corners_geo.side(0);
			debug_mesh_segment(right_side,0,0,1,0,0,1);

			Segment2 bottom_side = runway_info.runway_corners_geo.side(1);
			debug_mesh_segment(bottom_side,0,0,1,0,0,1);

			Point2 p;

			intersected_sides = taxiroute_segment_geo.intersect(left_side,   p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_segment_geo.intersect(top_side,    p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_segment_geo.intersect(right_side,  p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_segment_geo.intersect(bottom_side, p) ? intersected_sides + 1 : intersected_sides;

			if(intersected_sides >= 2)
			{
				string taxiroute_name;
				(*taxiroute_itr)->GetName(taxiroute_name);

				*msg = string("Error: Taxiroute segment " + taxiroute_name) + " crosses runway " + runway_info.runway_name + " completely";
				problem_thing =  static_cast<const WED_Thing*>(*taxiroute_itr);
			}
		}
	}
}

static void DoTaxiRouteRunwayTraversalCheck( const RunwayInfo& runway_info,
											 const TaxiRouteVec_t& all_taxiroutes,
											 string* msg,
											 const WED_Thing*& problem_thing)
{
	for(TaxiRouteVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin();
		taxiroute_itr != all_taxiroutes.end();
		++taxiroute_itr)
	{
		Segment2 taxiroute_segment_geo;
		Bezier2 bez;
		(*taxiroute_itr)->GetSide(gis_Geo, 0, taxiroute_segment_geo, bez);
#if DEV
		debug_mesh_segment(taxiroute_segment_geo,0,1,0,0,1,0);
#endif
		if( runway_info.runway_corners_geo.inside(taxiroute_segment_geo.p1) == true &&
			runway_info.runway_corners_geo.inside(taxiroute_segment_geo.p2))
		{
			continue;
		}
		else
		{
			int intersected_sides = 0;

			Segment2 left_side = runway_info.runway_corners_geo.side(2);
			debug_mesh_segment(left_side,0,0,1,0,0,1);

			Segment2 top_side = runway_info.runway_corners_geo.side(3);
			debug_mesh_segment(top_side,0,0,1,0,0,1);

			Segment2 right_side = runway_info.runway_corners_geo.side(0);
			debug_mesh_segment(right_side,0,0,1,0,0,1);

			Segment2 bottom_side = runway_info.runway_corners_geo.side(1);
			debug_mesh_segment(bottom_side,0,0,1,0,0,1);
				
			Point2 p;

			intersected_sides = taxiroute_segment_geo.intersect(left_side,   p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_segment_geo.intersect(top_side,    p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_segment_geo.intersect(right_side,  p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_segment_geo.intersect(bottom_side, p) ? intersected_sides + 1 : intersected_sides;

			if(intersected_sides >= 2)
			{
				string taxiroute_name;
				(*taxiroute_itr)->GetName(taxiroute_name);

				*msg = string("Error: Taxiroute segment " + taxiroute_name) + " crosses runway " + runway_info.runway_name + " completely";
				problem_thing =  static_cast<const WED_Thing*>(*taxiroute_itr);
			}
		}
	}
}

static void DoTaxiRouteCenterlineCheck( const RunwayInfo& runway_info,
									    const TaxiRouteVec_t& all_taxiroutes,
									    string* msg,
									    const WED_Thing*& problem_thing)
{
	TaxiRouteVec_t matching_taxiroutes;
	string taxiroute_name = "";
	for(TaxiRouteVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin(); taxiroute_itr != all_taxiroutes.end(); ++taxiroute_itr)
	{
		taxiroute_name = ENUM_Desc((*taxiroute_itr)->GetRunway());
		if(runway_info.runway_name == taxiroute_name)
		{
			matching_taxiroutes.push_back(*taxiroute_itr);
		}
	}

	for(TaxiRouteVec_t::const_iterator taxiroute_itr = matching_taxiroutes.begin(); taxiroute_itr != matching_taxiroutes.end(); ++taxiroute_itr)
	{
		//m for meters
		Segment2 taxiroute_segment_m;

		//Enter new scope purely for organizational purposes
		{
			Segment2 taxiroute_segment_geo;
			Bezier2 bez;
			(*taxiroute_itr)->GetSide(gis_Geo, 0, taxiroute_segment_geo, bez);
			taxiroute_segment_m = Segment2(translator.Forward(taxiroute_segment_geo.p1),translator.Forward(taxiroute_segment_geo.p2));
		}

		double METERS_TO_CENTER_THRESHOLD = 5.0;
		double p1_to_center_dist = sqrt(runway_info.runway_centerline_m.squared_distance_supporting_line(taxiroute_segment_m.p1));
		double p2_to_center_dist = sqrt(runway_info.runway_centerline_m.squared_distance_supporting_line(taxiroute_segment_m.p2));

		if( p1_to_center_dist > METERS_TO_CENTER_THRESHOLD ||
			p2_to_center_dist > METERS_TO_CENTER_THRESHOLD)
		{
			taxiroute_name = ENUM_Desc((*taxiroute_itr)->GetRunway());
			*msg = "Taxi route segement for runway " + taxiroute_name + " is not on the center line";
			problem_thing = (*taxiroute_itr);
		}
	}
}

static void DoAllTaxiRouteNodesInRunway( const RunwayInfo& runway_info,
										 const TaxiRouteVec_t& all_taxiroutes,
										 string* msg,
										 const WED_Thing*& problem_thing)
{
	string taxiroute_name = "";
	TaxiRouteNodeVec_t matching_taxiroute_nodes;
	for(TaxiRouteVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin(); taxiroute_itr != all_taxiroutes.end(); ++taxiroute_itr)
	{
		taxiroute_name = ENUM_Desc((*taxiroute_itr)->GetRunway());
		if(runway_info.runway_name == taxiroute_name)
		{
			matching_taxiroute_nodes.push_back(static_cast<const WED_TaxiRouteNode*>((*taxiroute_itr)->GetNthSource(0)));
			matching_taxiroute_nodes.push_back(static_cast<const WED_TaxiRouteNode*>((*taxiroute_itr)->GetNthSource(1)));
		}
	}

	for(vector<const WED_TaxiRouteNode*>::const_iterator node_itr = matching_taxiroute_nodes.begin(); node_itr != matching_taxiroute_nodes.end(); ++node_itr)
	{
		Point2 node_location;
		(*node_itr)->GetLocation(gis_Geo,node_location);
		if(runway_info.runway_corners_geo.inside(node_location) == false)
		{
			string node_name;
			(*node_itr)->GetName(node_name);
			*msg = "Taxiroute node " + node_name + " is out of runway " + runway_info.runway_name + "'s bounds";
			problem_thing = *node_itr;
		}
	}
}

void DoATCRunwayChecks(const WED_Airport& apt, string* msg, const WED_Thing*& problem_thing)
{
	Bbox2 box;
	apt.GetBounds(gis_Geo, box);
	CreateTranslatorForBounds(box,translator);
	
	vector<RunwayInfo> potentially_active_runways = CollectPotentiallyActiveRunways(apt);
	TaxiRouteVec_t all_taxiroutes;
	CollectRecursive<back_insert_iterator<TaxiRouteVec_t>>(static_cast<const WED_Thing*>(&apt),back_inserter<TaxiRouteVec_t>(all_taxiroutes));
	
	//Pre-check
	//- Does this active runway even have any taxi routes associated with it?
	for(vector<RunwayInfo>::const_iterator runway_info_itr = potentially_active_runways.begin();
		runway_info_itr != potentially_active_runways.end();
		++runway_info_itr)
	{
#if DEV
		debug_mesh_polygon((*runway_info_itr).runway_corners_geo,1,0,0);
		debug_mesh_segment((*runway_info_itr).runway_centerline_geo,1,0,0,1,0,0);
#endif

		DoRunwayHasMatchingTaxiRoute(    *runway_info_itr, all_taxiroutes, msg, problem_thing);
		DoAllTaxiRouteNodesInRunway(     *runway_info_itr, all_taxiroutes, msg, problem_thing);
		DoTaxiRouteRunwayTraversalCheck( *runway_info_itr, all_taxiroutes, msg, problem_thing);
		DoTaxiRouteCenterlineCheck(      *runway_info_itr, all_taxiroutes, msg, problem_thing);
	}
	return;
}
