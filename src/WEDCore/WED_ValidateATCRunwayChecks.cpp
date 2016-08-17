#include "WED_ValidateATCRunwayChecks.h"

#include <algorithm>
#include <iterator>
#include <sstream>

#if DEV
#include <iostream>
//Visualizes some of the runway lines and the bounding box
#define DEBUG_VIS_LINES 0
#endif

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

static CoordTranslator2 translator;

//A special selection of info useful for these tests
struct RunwayInfo
{
	//All the information about the current runway future tests will want or need
	RunwayInfo(WED_Runway* runway)
		: runway_ptr(runway)
	{
		runway->GetName(runway_name);

		AptRunway_t apt_runway;
		runway_ptr->Export(apt_runway);

		runway_numbers[0] = runway_ptr->GetRunwayEnumsOneway().first;
		runway_numbers[1] = runway_ptr->GetRunwayEnumsOneway().second;

		runway_ops[0] = 0;
        runway_ops[1] = 0;

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
	WED_Runway* runway_ptr;

	//Name of current runway
	string runway_name;

	//Where [0] is north runway number and [1] is the south runway number
	//The int is an enum from ATCRunwayOneway
	int runway_numbers[2];

	//Where [0] is north ops and [1] is south ops
	int runway_ops[2];
	
	//The corners of the runway in lat/lon
	Polygon2 runway_corners_geo;
	
	//The corners of the runway in meters
	Polygon2 runway_corners_m;
	
	//The center line of the runway in lat/lon. p1 is source, p2 is target
	Segment2 runway_centerline_geo;

	//The center line of the runway in meters. p1 is source, p2 is target
	Segment2 runway_centerline_m;

	void ChangeRunwayOps(int runway_number, int operations)
	{
		if(runway_number <= atc_18R)
		{
			runway_ops[0] |= operations;
		}
		else
		{
			runway_ops[1] |= operations;
		}
	}

	bool IsHotForArrival(int runway_number) const
	{
		if(runway_number <= atc_18R)
		{
			return runway_ops[0] & 0x01;
		}
		else
		{
			return runway_ops[1] & 0x1;
		}
	}

	bool IsHotForDeparture(int runway_number) const
	{
		if(runway_number <= atc_18R)
		{
			return runway_ops[0] & 0x2;
		}
		else
		{
			return runway_ops[1] & 0x2;
		}
	}
};

struct TaxiRouteInfo
{
	TaxiRouteInfo(WED_TaxiRoute* taxiroute)
		: taxiroute_ptr(taxiroute),
		  node_0(static_cast<WED_TaxiRouteNode*>(taxiroute->GetNthSource(0))),
		  node_1(static_cast<WED_TaxiRouteNode*>(taxiroute->GetNthSource(1)))
	{
		AptRouteEdge_t apt_route;
		taxiroute->Export(apt_route);
		taxiroute_name = apt_route.name;
		hot_arrivals   = set<string>(apt_route.hot_arrive);
		hot_departures = set<string>(apt_route.hot_depart);

		Bezier2 bez;
		taxiroute->GetSide(gis_Geo, 0, taxiroute_segment_geo, bez);
		taxiroute_segment_m = Segment2(translator.Forward(taxiroute_segment_geo.p1),translator.Forward(taxiroute_segment_geo.p2));
	}
	
	//Pointer to the original WED_TaxiRoute in WED's datamodel
	WED_TaxiRoute* taxiroute_ptr;

	//Name of the taxiroute
	string taxiroute_name;

	//Segment2 representing the taxiroute in lat/lon
	Segment2 taxiroute_segment_geo;

	//Segment2 representing the taxiroute in meters
	Segment2 taxiroute_segment_m;

	set<string> hot_arrivals;
	set<string> hot_departures;

	//Source node of the taxiroute
	WED_TaxiRouteNode* node_0;

	//Target node of the taxiroute
	WED_TaxiRouteNode* node_1;
};

typedef vector<WED_ATCRunwayUse*>  ATCRunwayUseVec_t;
typedef vector<WED_ATCFlow*>       FlowVec_t;
typedef vector<WED_Runway*>        RunwayVec_t;
typedef vector<WED_TaxiRoute*>     TaxiRouteVec_t;
typedef vector<WED_TaxiRouteNode*> TaxiRouteNodeVec_t;
typedef vector<RunwayInfo>         RunwayInfoVec_t;
typedef vector<TaxiRouteInfo>      TaxiRouteInfoVec_t;

//Collects potentially active runways (or all of them if there are no flows) and checks that every 
static RunwayInfoVec_t CollectPotentiallyActiveRunways( const TaxiRouteInfoVec_t& all_taxiroutes,
														validation_error_vector& msgs,
														WED_Airport* apt)
{
	FlowVec_t flows;
	CollectRecursive(apt,back_inserter<FlowVec_t>(flows));

	RunwayVec_t all_runways;
	CollectRecursive(apt,back_inserter<RunwayVec_t>(all_runways));

	//Find all potentially active runways:
	//0 flows means treat all runways as potentially active
	//>1 means find all runways mentioned, ignoring duplicates
	RunwayVec_t potentially_active_runways;
	RunwayInfoVec_t runway_info_vec;
	if(flows.size() == 0)
	{
		potentially_active_runways = all_runways;
		runway_info_vec.insert(runway_info_vec.begin(),potentially_active_runways.begin(),potentially_active_runways.end());
		return runway_info_vec;
	}
	else
	{
		ATCRunwayUseVec_t use_rules;
		CollectRecursive(apt,back_inserter<ATCRunwayUseVec_t>(use_rules));

		//For all runways in the airport
		for(RunwayVec_t::const_iterator runway_itr = all_runways.begin(); runway_itr != all_runways.end(); ++runway_itr)
		{
			//Search through all runway uses, testing if the runway has a match, and 1 to n taxiroutes are associated with it
			for(ATCRunwayUseVec_t::const_iterator use_itr = use_rules.begin(); use_itr != use_rules.end(); ++use_itr)
			{
				AptRunwayRule_t runway_rule;
				(*use_itr)->Export(runway_rule);

				//Compare the name of the runway to the name of the 
				string runway_name;
				(*runway_itr)->GetName(runway_name);
				string runway_name_p1 = runway_name.substr(0,runway_name.find_first_of('/'));
				string runway_name_p2 = runway_name.substr(runway_name.find_first_of('/')+1);

				if( runway_rule.runway == runway_name_p1 ||
					runway_rule.runway == runway_name_p2)
				{
					bool found_matching_taxiroute = false;
					for(vector<TaxiRouteInfo>::const_iterator taxiroute_itr = all_taxiroutes.begin(); taxiroute_itr != all_taxiroutes.end(); ++taxiroute_itr)
					{
						string taxiroute_name = ENUM_Desc((taxiroute_itr)->taxiroute_ptr->GetRunway());
			
						string runway_name;
						(*runway_itr)->GetName(runway_name);
						if(runway_name == taxiroute_name)
						{
							runway_info_vec.push_back(*runway_itr);
							break; //exit all_taxiroutes loop
						}
					}
					break;//exit use_rules loop
				}
			}
		}
	}
	return runway_info_vec;
}

//Returns a vector of TaxiRouteInfos whose name matches the given runway
static TaxiRouteInfoVec_t FilterMatchingRunways( const RunwayInfo& runway_info,
												 const TaxiRouteInfoVec_t& all_taxiroutes)
{
	TaxiRouteInfoVec_t matching_taxiroutes;
	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin(); taxiroute_itr != all_taxiroutes.end(); ++taxiroute_itr)
	{
		string taxiroute_name = ENUM_Desc((taxiroute_itr)->taxiroute_ptr->GetRunway());
		if(runway_info.runway_name == taxiroute_name)
		{
			matching_taxiroutes.push_back(TaxiRouteInfo(*taxiroute_itr));
		}
	}

	return matching_taxiroutes;
}

static void AssaignRunwayUse( RunwayInfo& runway_info,
							  const ATCRunwayUseVec_t& all_use_rules)
{
	if(all_use_rules.empty() == true)
	{
		runway_info.runway_ops[0] = 0x01 | 0x02; //Mark for arrivals and departures
		runway_info.runway_ops[1] = 0x01 | 0x02;
		return;
	}

	//The case of "This airport has no flows" gets implicitly taken care of here by the fact that use_rules will be 0,
	//and the fact that we assaign this in 
	for(ATCRunwayUseVec_t::const_iterator use_itr = all_use_rules.begin(); use_itr != all_use_rules.end(); ++use_itr)
	{
		AptRunwayRule_t apt_runway_rule;
		(*use_itr)->Export(apt_runway_rule);
		
		if(runway_info.runway_numbers[0] == ENUM_LookupDesc(ATCRunwayOneway, apt_runway_rule.runway.c_str()))
		{
			runway_info.ChangeRunwayOps(runway_info.runway_numbers[0],apt_runway_rule.operations);
		}
		else if(runway_info.runway_numbers[1] == ENUM_LookupDesc(ATCRunwayOneway, apt_runway_rule.runway.c_str()))
		{
			runway_info.ChangeRunwayOps(runway_info.runway_numbers[1],apt_runway_rule.operations);
		}
	}
}

//--Centerline Checks----------------------------------------------------------
static bool AllTaxiRouteNodesInRunway( const RunwayInfo& runway_info,
									   const TaxiRouteInfoVec_t& matching_taxiroutes,
									   validation_error_vector& msgs,
									   WED_Airport* apt)
{
	int original_num_errors = msgs.size();
	
	TaxiRouteNodeVec_t matching_taxiroute_nodes;
	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = matching_taxiroutes.begin(); taxiroute_itr != matching_taxiroutes.end(); ++taxiroute_itr)
	{
		matching_taxiroute_nodes.push_back(static_cast<WED_TaxiRouteNode*>(taxiroute_itr->taxiroute_ptr->GetNthSource(0)));
		matching_taxiroute_nodes.push_back(static_cast<WED_TaxiRouteNode*>(taxiroute_itr->taxiroute_ptr->GetNthSource(1)));
	}

	Polygon2 runway_hit_box_m = runway_info.runway_corners_m;

	Point2& bottom_left  = runway_hit_box_m[0];
	Point2& top_left     = runway_hit_box_m[1];
	Point2& top_right    = runway_hit_box_m[2];
	Point2& bottom_right = runway_hit_box_m[3];

	Vector2 gis_line_direction(runway_info.runway_centerline_m.p1,runway_info.runway_centerline_m.p2);
	gis_line_direction.normalize();
	gis_line_direction *= 2.0;

	top_left  += gis_line_direction;
	top_right += gis_line_direction;
		
	bottom_left  -= gis_line_direction;
	bottom_right -= gis_line_direction;
	
	Polygon2 runway_hit_box_geo(4);
	runway_hit_box_geo[0] = translator.Reverse(bottom_left);
	runway_hit_box_geo[1] = translator.Reverse(top_left);
	runway_hit_box_geo[2] = translator.Reverse(top_right);
	runway_hit_box_geo[3] = translator.Reverse(bottom_right);

#if DEBUG_VIS_LINES
	debug_mesh_segment(runway_hit_box_geo.side(0), 0, 1, 0, 0, 0, 1); //left side
	debug_mesh_segment(runway_hit_box_geo.side(1), 0, 1, 0, 0, 0, 1); //top side
	debug_mesh_segment(runway_hit_box_geo.side(2), 0, 1, 0, 0, 0, 1); //right side
	debug_mesh_segment(runway_hit_box_geo.side(3), 0, 1, 0, 0, 0, 1); //bottom side
#endif
	for(TaxiRouteNodeVec_t::const_iterator node_itr = matching_taxiroute_nodes.begin(); node_itr != matching_taxiroute_nodes.end(); ++node_itr)
	{
		Point2 node_location;
		(*node_itr)->GetLocation(gis_Geo,node_location);
		if(runway_hit_box_geo.inside(node_location) == false)
		{
			string node_name;
			(*node_itr)->GetName(node_name);
			string msg = "Taxiroute node " + node_name + " is out of runway " + runway_info.runway_name + "'s bounds";
			msgs.push_back(validation_error_t(msg,*node_itr,apt));
		}
	}

	return msgs.size() - original_num_errors == 0 ? true : false;
}

static bool TaxiRouteCenterlineCheck( const RunwayInfo& runway_info,
			 						  const TaxiRouteInfoVec_t& matching_taxiroutes,
			 						  validation_error_vector& msgs,
			 						  WED_Airport* apt)
{
	int original_num_errors = msgs.size();
	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = matching_taxiroutes.begin(); taxiroute_itr != matching_taxiroutes.end(); ++taxiroute_itr)
	{
		double METERS_TO_CENTER_THRESHOLD = 5.0;
		double p1_to_center_dist = sqrt(runway_info.runway_centerline_m.squared_distance_supporting_line(taxiroute_itr->taxiroute_segment_m.p1));
		double p2_to_center_dist = sqrt(runway_info.runway_centerline_m.squared_distance_supporting_line(taxiroute_itr->taxiroute_segment_m.p2));

		if( p1_to_center_dist > METERS_TO_CENTER_THRESHOLD ||
			p2_to_center_dist > METERS_TO_CENTER_THRESHOLD)
		{
			string taxiroute_name = ENUM_Desc((taxiroute_itr)->taxiroute_ptr->GetRunway());
			string msg = "Taxi route segement for runway " + taxiroute_name + " is not on the center line";
			msgs.push_back(validation_error_t(msg,taxiroute_itr->taxiroute_ptr,apt));
		}
	}

	return msgs.size() - original_num_errors == 0 ? true : false;
}

static bool TaxiRouteSplitPathCheck( const RunwayInfo& runway_info,
									 const TaxiRouteInfoVec_t& all_taxiroutes,
									 validation_error_vector& msgs,
									 WED_Airport* apt)
{
	int original_num_errors = msgs.size();
	TaxiRouteNodeVec_t all_nodes;
	
	for (int i = 0; i < all_taxiroutes.size(); ++i)
	{
		all_nodes.push_back(all_taxiroutes[i].node_0);
		all_nodes.push_back(all_taxiroutes[i].node_1);
	}

	//Since we are storing pointers we can sort them numerically and see if any of them appear 3 or more times
	sort(all_nodes.begin(),all_nodes.end());

	//Early exit
	if(all_nodes.size() % 2 != 0 || all_nodes.size() <=2)
	{
		return true;
	}

	for (int i = 0; i < all_nodes.size() - 3 - 1; i++)
	{
		WED_TaxiRouteNode* node_1 = all_nodes[i + 0];
		WED_TaxiRouteNode* node_2 = all_nodes[i + 1];
		WED_TaxiRouteNode* node_3 = all_nodes[i + 2];

		int duplicate_count = 0;
		
		duplicate_count = node_1 == node_2 ? duplicate_count + 1 : duplicate_count;
		duplicate_count = node_2 == node_3 ? duplicate_count + 1 : duplicate_count;
		duplicate_count = node_3 == node_1 ? duplicate_count + 1 : duplicate_count;
		
		if(duplicate_count == 3)
		{
			//We may have a duplicate but is it one we care about?
			
			set<WED_Thing*> node_viewers;
			(node_1)->GetAllViewers(node_viewers);
			
			int relavent_runway_taxiroutes = 0;
			for (set<WED_Thing*>::iterator viewer_itr = node_viewers.begin();
				viewer_itr != node_viewers.end();
				viewer_itr++)
			{
				WED_TaxiRoute* taxiroute = dynamic_cast<WED_TaxiRoute*>(*viewer_itr);
				if(taxiroute)
				{
					string taxiroute_name;
					taxiroute_name = ENUM_Desc(taxiroute->GetRunway());
					if(runway_info.runway_name == taxiroute_name)
					{
						++relavent_runway_taxiroutes;
					}
				}
			}

			if(relavent_runway_taxiroutes >= 3)
			{
				string node_name;
				(node_1)->GetName(node_name);
				string msg = "Taxi route node " + node_name + " is used three or more times in a runway's taxiroute";
				msgs.push_back(validation_error_t(msg,node_1,apt));
			}
		}
	}

	return msgs.size() - original_num_errors == 0 ? true : false;
}

static bool TaxiRouteParallelCheck( const RunwayInfo& runway_info,
									 const TaxiRouteInfoVec_t& matching_routes,
									 validation_error_vector& msgs,
									 WED_Airport* apt)
{
	int original_num_errors = msgs.size();

	//The first matching taxiroute chooses the direction, the rest must match
	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = matching_routes.begin();
		taxiroute_itr != matching_routes.end();
		++taxiroute_itr)
	{
		Vector2 runway_centerline_vec = Vector2(runway_info.runway_centerline_m.p1,runway_info.runway_centerline_m.p2);
		Vector2 taxiroute_vec = Vector2(taxiroute_itr->taxiroute_segment_m.p1,taxiroute_itr->taxiroute_segment_m.p2);
		runway_centerline_vec.normalize();
		taxiroute_vec.normalize();

		double dot_product = abs(runway_centerline_vec.dot(taxiroute_vec));
		double ANGLE_THRESHOLD = 0.995;
		if(dot_product < ANGLE_THRESHOLD)
		{
			string msg = "Taxi route segement " + taxiroute_itr->taxiroute_name + " is not parallel to the runway's center line.";
			msgs.push_back(validation_error_t(msg,taxiroute_itr->taxiroute_ptr,apt));
		}
	}

	return msgs.size() - original_num_errors == 0 ? true : false;
}

static bool RunwayHasCorrectCoverage( const RunwayInfo& runway_info,
									const TaxiRouteInfoVec_t& matching_taxiroutes,
									validation_error_vector& msgs,
									WED_Airport* apt)
{
	int original_num_errors = msgs.size();
	if(matching_taxiroutes.size() == 0)
	{
		return true;
	}

	double total_length_m = 0.0;
	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = matching_taxiroutes.begin(); taxiroute_itr != matching_taxiroutes.end(); ++taxiroute_itr)
	{
		//Add up all the lengths of the runway, see if it is within the threshold of how much coverage there must be
		total_length_m += sqrt(taxiroute_itr->taxiroute_segment_m.squared_length());
	}

	AptRunway_t apt_runway;
	runway_info.runway_ptr->Export(apt_runway);
	double COVERAGE_THRESHOLD = runway_info.runway_ptr->GetLength();
	if((apt_runway.width_mtr  * 2) < COVERAGE_THRESHOLD)
	{
		COVERAGE_THRESHOLD -= apt_runway.width_mtr * 2;
	}

	if(total_length_m < COVERAGE_THRESHOLD)
	{
		string msg = "Runway " + runway_info.runway_name + " is not sufficiently covered with taxi routes.";
		msgs.push_back(validation_error_t(msg,runway_info.runway_ptr,apt));
		return false;
	}

	double OVERSHOOT_THRESHOLD = 2.0 * 2; //You get two meteres of room for being off the runway
	if(total_length_m > (runway_info.runway_ptr->GetLength() + OVERSHOOT_THRESHOLD))
	{
		string msg = "Taxi route " + matching_taxiroutes.begin()->taxiroute_name + "'s ends over extend past the edges of runway " + runway_info.runway_name;
		msgs.push_back(validation_error_t(msg,runway_info.runway_ptr,apt));
		return false;
	}
	return msgs.size() - original_num_errors == 0 ? true : false;
}
//-----------------------------------------------------------------------------

//--Hot zone checks------------------------------------------------------------

static void FindIfMarked( const int runway_number,        //enum from ATCRunwayOneway
						  const TaxiRouteInfo& taxiroute, //The taxiroute to check if it is marked properly
						  const set<string>& hot_set,     //The set of hot_listed_runways
						  const string& op_type,          //String for the description
						  validation_error_vector& msgs,  //WED_Valiation messages
						  WED_Airport* apt) //Airport to pass into msgs
{
	
	bool found_marked = false;
	for(set<string>::const_iterator hot_set_itr = hot_set.begin();
		hot_set_itr != hot_set.end();
		++hot_set_itr)
	{
		if(runway_number == ENUM_LookupDesc(ATCRunwayOneway,(*hot_set_itr).c_str()))
		{
			found_marked = true;
		}
	}

	if(found_marked == false)
	{
		msgs.push_back(validation_error_t(
			
			string("Taxi route " + taxiroute.taxiroute_name + " is too close to runway ") + 
			string(ENUM_Desc(runway_number)) + 
			string(" and now must be marked active for runway ") +
			string(ENUM_Desc(runway_number) + 
			string(" " + op_type)),
			
			taxiroute.taxiroute_ptr,
			apt));
	}
}

static TaxiRouteInfoVec_t GetTaxiRoutesFromViewers(WED_TaxiRouteNode* node)
{
	set<WED_Thing*> node_viewers;
	node->GetAllViewers(node_viewers);

	TaxiRouteInfoVec_t matching_taxiroutes;
	for(set<WED_Thing*>::iterator node_viewer_itr = node_viewers.begin(); node_viewer_itr != node_viewers.end(); ++node_viewer_itr)
	{
		WED_TaxiRoute* taxiroute = dynamic_cast<WED_TaxiRoute*>(*node_viewer_itr);
		if(taxiroute != NULL)
		{
			matching_taxiroutes.push_back(taxiroute);
		}
	}

	return matching_taxiroutes;
}

//Returns polygon in lat/lon
static Polygon2 MakeHotZoneHitBox( const RunwayInfo& runway_info,
								   int runway_number) //The relavent runway info's use
{
	if( 
		(runway_info.IsHotForArrival(runway_number) == false &&
		runway_info.IsHotForDeparture(runway_number) == false)
		||
		runway_number == atc_Runway_None
	  )
	{
		return Polygon2(0);
	}

	const double HITZONE_WIDTH_THRESHOLD_M   = 10.00;
	const double HITZONE_OVERFLY_THRESHOLD_M = 1500.00;

	Polygon2 runway_hit_box_m = runway_info.runway_corners_m;

	Point2& bottom_left  = runway_hit_box_m[0];
	Point2& top_left     = runway_hit_box_m[1];
	Point2& top_right    = runway_hit_box_m[2];
	Point2& bottom_right = runway_hit_box_m[3];

	/*
	           top
	                 ^
	                 |
	                 vertical_extension_vec
	            1    |
	        1_______2!
		   |         |
		   |         |
		0  |         | 3
		   |         |
		   0---------3---side_extension_vec--->
		        0
		      bottom
	*/

	Vector2 gis_line_direction(runway_info.runway_centerline_m.p1,runway_info.runway_centerline_m.p2);
	gis_line_direction.normalize();
	gis_line_direction *= HITZONE_OVERFLY_THRESHOLD_M;

	if(runway_number <= atc_18R)
	{
		if(runway_info.IsHotForArrival(runway_number) == true)
		{
			//arrival_side is bottom_side;
			bottom_left  -= gis_line_direction;
			bottom_right -= gis_line_direction;
		}
		
		if(runway_info.IsHotForDeparture(runway_number) == true)
		{
			//departure_side is top_side;
			top_left  += gis_line_direction;
			top_right += gis_line_direction;
		}
	}
	else
	{
		if(runway_info.IsHotForArrival(runway_number) == true)
		{
			//arrival_side is top_side;
			top_left  += gis_line_direction;
			top_right += gis_line_direction;
		}
		
		if(runway_info.IsHotForDeparture(runway_number) == true)
		{
			//departure_side is bottom_side;
			bottom_left  -= gis_line_direction;
			bottom_right -= gis_line_direction;
		}
	}

	Polygon2 runway_hit_box_geo(4);
	runway_hit_box_geo[0] = translator.Reverse(bottom_left);
	runway_hit_box_geo[1] = translator.Reverse(top_left);
	runway_hit_box_geo[2] = translator.Reverse(top_right);
	runway_hit_box_geo[3] = translator.Reverse(bottom_right);
#if DEBUG_VIS_LINES
	//Extend out fully for testing purposes
	//bottom_left  -= gis_line_direction;
	//bottom_right -= gis_line_direction;
	//
	//departure_side is top_side;
	//top_left  += gis_line_direction;
	//top_right += gis_line_direction;

	//debug_mesh_line(translator.Reverse(bottom_left),  translator.Reverse(top_left),     1, 0, 0, 1, 0, 0); //left side
	//debug_mesh_line(translator.Reverse(top_left),     translator.Reverse(top_right),    0, 1, 0, 0, 1, 0); //top side
	//debug_mesh_line(translator.Reverse(top_right),    translator.Reverse(bottom_right), 0, 0, 1, 0, 0, 1); //right side
	//debug_mesh_line(translator.Reverse(bottom_right), translator.Reverse(bottom_left),  1, 1, 1, 1, 1, 1); //bottom side
	debug_mesh_segment(runway_hit_box_geo.side(0), 1, 0, 0, 1, 0, 0); //left side
	debug_mesh_segment(runway_hit_box_geo.side(1), 0, 1, 0, 0, 1, 0); //top side
	debug_mesh_segment(runway_hit_box_geo.side(2), 0, 0, 1, 0, 0, 1); //right side
	debug_mesh_segment(runway_hit_box_geo.side(3), 1, 1, 1, 1, 1, 1); //bottom side
#endif
	return runway_hit_box_geo;
}

static bool DoHotZoneChecks( const RunwayInfo& runway_info,
							 const TaxiRouteInfoVec_t& all_taxiroutes,
							 validation_error_vector& msgs,
							 WED_Airport* apt)
{
	int original_num_errors = msgs.size();
	TaxiRouteNodeVec_t all_nodes;
	for (int i = 0; i < all_taxiroutes.size(); ++i)
	{
		all_nodes.push_back(all_taxiroutes[i].node_0);
		all_nodes.push_back(all_taxiroutes[i].node_1);
	}
	
	//runway side 0 is the lower side, 1 is the higher side
	for (int runway_side = 0; runway_side < 2; ++runway_side)
	{
		//Make the hitbox baed on the runway and which side (low/high) you're currently on
		Polygon2 hit_box = MakeHotZoneHitBox(runway_info, runway_info.runway_numbers[runway_side]);

		//For every node in the airport,
		//- get its associated taxiroutes,
		//- see if they intersect with our box
		//- and if so, test if they are properly marked
		for(TaxiRouteNodeVec_t::iterator node_itr = all_nodes.begin();
			node_itr != all_nodes.end() && hit_box.size() > 0;
			++node_itr)
		{
			TaxiRouteInfoVec_t taxiroutes = GetTaxiRoutesFromViewers(*node_itr);
			for(TaxiRouteInfoVec_t::iterator taxiroute_itr = taxiroutes.begin(); taxiroute_itr != taxiroutes.end(); ++taxiroute_itr)
			{
				//Run two tests, intersection with the side, and if that fails, point inside the polygon
				bool did_hit = hit_box.intersects((*taxiroute_itr).taxiroute_segment_geo);
				if(did_hit == false)
				{
					Point2 p;
					(*node_itr)->GetLocation(gis_Geo,p);
					did_hit = hit_box.inside(p);
				}

				if(did_hit)
				{
					int runway_number = runway_info.runway_numbers[runway_side];

					if(runway_info.IsHotForArrival(runway_number) == true)
					{
						FindIfMarked(runway_number, *taxiroute_itr, (*taxiroute_itr).hot_arrivals, "arrivals", msgs, apt);
					}

					if(runway_info.IsHotForDeparture(runway_number) == true)
					{
						FindIfMarked(runway_number, *taxiroute_itr, (*taxiroute_itr).hot_departures, "departures", msgs, apt);
					}
				}
			}
		}
	}
	return msgs.size() - original_num_errors == 0 ? true : false;
}

//-----------------------------------------------------------------------------
void WED_DoATCRunwayChecks(WED_Airport& apt, validation_error_vector& msgs)
{
#if DEBUG_VIS_LINES && 1
	//Clear the previously drawn lines before every validation
	gMeshPoints.clear();
	gMeshLines.clear();
	gMeshPolygons.clear();
#endif

	Bbox2 box;
	apt.GetBounds(gis_Geo, box);
	CreateTranslatorForBounds(box,translator);
	
	ATCRunwayUseVec_t all_use_rules;
	CollectRecursive(&apt,back_inserter<ATCRunwayUseVec_t>(all_use_rules));

	TaxiRouteVec_t all_taxiroutes_plain;
	CollectRecursive(&apt,back_inserter<TaxiRouteVec_t>(all_taxiroutes_plain));
	
	//All taxiroutes within the airport that are visible
	TaxiRouteInfoVec_t all_taxiroutes = TaxiRouteInfoVec_t(all_taxiroutes_plain.begin(),all_taxiroutes_plain.end());
	
	//All runways which match the following criteria
	//- The runway is mentioned in an ATC Flow
	//- The runway has a taxiroute associated with it
	RunwayInfoVec_t potentially_active_runways = CollectPotentiallyActiveRunways(all_taxiroutes, msgs, &apt);

	//Pre-check
	//- Does this active runway even have any taxi routes associated with it?
	for(RunwayInfoVec_t::iterator runway_info_itr = potentially_active_runways.begin();
		runway_info_itr != potentially_active_runways.end();
		++runway_info_itr)
	{
#if DEBUG_VIS_LINES
		debug_mesh_polygon((*runway_info_itr).runway_corners_geo,1,0,0);
		debug_mesh_segment((*runway_info_itr).runway_centerline_geo,1,0,0,1,0,0);
#endif
		TaxiRouteInfoVec_t matching_taxiroutes = FilterMatchingRunways(*runway_info_itr, all_taxiroutes);

		bool passes_centerline_checks = false;
		if(TaxiRouteSplitPathCheck(*runway_info_itr, all_taxiroutes, msgs, &apt))
		{
			if(TaxiRouteCenterlineCheck(*runway_info_itr, matching_taxiroutes, msgs, &apt))
			{
				if(AllTaxiRouteNodesInRunway(*runway_info_itr, matching_taxiroutes, msgs, &apt))
				{
					if(RunwayHasCorrectCoverage(*runway_info_itr, matching_taxiroutes, msgs, &apt))
					{
						if(TaxiRouteParallelCheck(*runway_info_itr, matching_taxiroutes, msgs, &apt))
						{
							passes_centerline_checks = true;
						}
					}
				}
			}
		}

		if(passes_centerline_checks == false)
		{
			return;
		}

		AssaignRunwayUse(*runway_info_itr, all_use_rules);
		bool passes_hotzone_checks = DoHotZoneChecks(*runway_info_itr, all_taxiroutes, msgs, &apt);
		//Nothing to do here yet until we have more checks after this
	}
	return;
}
