#include "WED_ValidateATCRunwayChecks.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <queue>
#include <functional>

#if DEV
#include <iostream>
//Visualizes some of the runway lines and the bounding box
#define DEBUG_VIS_LINES 1
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

//Forward - m, Reverse - lat/lon
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
		: taxiroute_ptr(taxiroute)
	{
		AptRouteEdge_t apt_route;
		taxiroute->Export(apt_route);
		taxiroute_name = apt_route.name;
		hot_arrivals   = set<string>(apt_route.hot_arrive);
		hot_departures = set<string>(apt_route.hot_depart);

		//nodes
		for (int i = 0; i < taxiroute_ptr->CountSources(); i++)
		{
			WED_GISPoint* point = dynamic_cast<WED_GISPoint*>(taxiroute_ptr->GetNthSource(i));

			if (point != NULL)
			{
				nodes.push_back(point);
			}
		}

		DebugAssert(nodes.size() >= 2);
		Bezier2 bez;
		taxiroute->GetSide(gis_Geo, 0, taxiroute_segment_geo, bez);
		taxiroute_segment_m = Segment2(translator.Forward(taxiroute_segment_geo.p1),translator.Forward(taxiroute_segment_geo.p2));
	}
	
	//Pointer to the original WED_TaxiRoute in WED's data model
	WED_TaxiRoute* taxiroute_ptr;

	//Name of the taxiroute
	string taxiroute_name;

	//Segment2 representing the taxiroute in lat/lon
	Segment2 taxiroute_segment_geo;

	//Segment2 representing the taxiroute in meters
	Segment2 taxiroute_segment_m;

	set<string> hot_arrivals;
	set<string> hot_departures;

	//Source nodes of the taxiroute. Usually TaxiRouteNodes but sometimes something else
	vector<WED_GISPoint*> nodes;
};

typedef vector<WED_ATCRunwayUse*>  ATCRunwayUseVec_t;
typedef vector<WED_ATCFlow*>       FlowVec_t;
typedef vector<WED_Runway*>        RunwayVec_t;
typedef vector<WED_TaxiRoute*>     TaxiRouteVec_t;

//We're just using WED_GISPoint because old WED and airports
typedef vector<WED_GISPoint*>      TaxiRouteNodeVec_t;
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
		if((taxiroute_itr)->taxiroute_ptr->IsRunway())
		{
			string taxiroute_name = ENUM_Desc((taxiroute_itr)->taxiroute_ptr->GetRunway());
			if(runway_info.runway_name == taxiroute_name)
			{
				matching_taxiroutes.push_back(TaxiRouteInfo(*taxiroute_itr));
			}
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

//!!IMPORTANT: These methods return true if they pass without error, false if there was an error!!
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
		matching_taxiroute_nodes.push_back(dynamic_cast<WED_GISPoint*>(taxiroute_itr->taxiroute_ptr->GetNthSource(0)));
		matching_taxiroute_nodes.push_back(dynamic_cast<WED_GISPoint*>(taxiroute_itr->taxiroute_ptr->GetNthSource(1)));
	}

	Polygon2 runway_hit_box_m = runway_info.runway_corners_m;

	Point2& bottom_left  = runway_hit_box_m[0];
	Point2& top_left     = runway_hit_box_m[1];
	Point2& top_right    = runway_hit_box_m[2];
	Point2& bottom_right = runway_hit_box_m[3];

	Vector2 gis_line_direction(runway_info.runway_centerline_m.p1,runway_info.runway_centerline_m.p2);
	gis_line_direction.normalize();
	gis_line_direction *= 2.0; // allow runway taxi lines to go 2m beyond runway ends

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

//True for all hidden and all not WED_Entity
static bool is_hidden(const WED_Thing* node)
{
	const WED_Entity* ent = dynamic_cast<const WED_Entity*>(node);
	DebugAssert(ent != NULL);

	if (ent != NULL)
	{
		return static_cast<bool>(ent->GetHidden());
	}

	return true;
}

static set<WED_Thing*> get_all_visible_viewers(const WED_GISPoint* node)
{
	set<WED_Thing*> node_viewers;
	node->GetAllViewers(node_viewers);

	vector<WED_Thing*> node_viewers_vec(node_viewers.begin(), node_viewers.end());
	node_viewers_vec.erase(remove_if(node_viewers_vec.begin(), node_viewers_vec.end(), is_hidden),node_viewers_vec.end());

	return set<WED_Thing*>(node_viewers_vec.begin(),node_viewers_vec.end());
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
			string msg = "Taxi route segment for runway " + taxiroute_name + " is not on the center line";
			msgs.push_back(validation_error_t(msg,taxiroute_itr->taxiroute_ptr,apt));
		}
	}

	return msgs.size() - original_num_errors == 0 ? true : false;
}

static vector<TaxiRouteInfo> filter_viewers_by_is_runway(const WED_GISPoint* node, const string& runway_name)
{
	vector<TaxiRouteInfo> matching_routes;

	set<WED_Thing*> node_viewers = get_all_visible_viewers(node);

	for(set<WED_Thing*>::iterator itr = node_viewers.begin(); itr != node_viewers.end(); ++itr)
	{
		WED_TaxiRoute* test_route = dynamic_cast<WED_TaxiRoute*>(*itr);
		if(test_route != NULL)
		{
			TaxiRouteInfo taxiroute_info(test_route);
			if(taxiroute_info.taxiroute_ptr->IsRunway() && taxiroute_info.taxiroute_name == runway_name)
			{
				matching_routes.push_back(taxiroute_info);
			}
		}
	}

	return matching_routes;
}

//Checks that a Runway's taxi route has two nodes with a valence of one, and no nodes with a valence of > 2
static bool RunwaysTaxiRouteValencesCheck (const RunwayInfo& runway_info,
										   const TaxiRouteNodeVec_t& all_matching_nodes, //All nodes from taxiroutes matching the runway, these will come in sorted
										   WED_TaxiRoute*& out_start_taxiroute, //Out parameter, one of the ends of the taxiroute
										   validation_error_vector& msgs,
										   WED_Airport* apt)
{
	int original_num_errors = msgs.size();
	int num_valence_of_1 = 0; //Aka, the tips of the runway, should end up being 2
	out_start_taxiroute = NULL;

	//Valence allowed per runway enum = 2
	for(TaxiRouteNodeVec_t::const_iterator node_itr = all_matching_nodes.begin(); node_itr != all_matching_nodes.end(); ++node_itr)
	{
		int node_valence = std::count(all_matching_nodes.begin(), all_matching_nodes.end(), *node_itr);

		if(node_valence == 1)
		{
			if(num_valence_of_1 < 2)
			{
				if(out_start_taxiroute == NULL)
				{
					TaxiRouteInfoVec_t viewers = filter_viewers_by_is_runway(*node_itr,runway_info.runway_name);
					out_start_taxiroute = viewers.front().taxiroute_ptr;
				}
				++num_valence_of_1;
			}
			else
			{
				msgs.push_back(validation_error_t("Runway " + runway_info.runway_name + "'s taxi route is not continuous",*node_itr,apt));
			}
		}
		else if(node_valence >= 3)
		{
			ostringstream oss;
			oss << node_valence;

			string node_name;
			(*node_itr)->GetName(node_name);

			msgs.push_back(validation_error_t("Runway " + runway_info.runway_name + "'s taxi route is split " + oss.str() + " ways at taxi route node " + node_name, *node_itr,apt));
		}
	}

	if(num_valence_of_1 == 0 && all_matching_nodes.size() > 0)
	{
		msgs.push_back(validation_error_t("Runway " + runway_info.runway_name + "'s taxi route forms a loop",runway_info.runway_ptr,apt));
	}
	return msgs.size() - original_num_errors == 0 ? true : false;
}

static int get_node_valence(const WED_GISPoint* node)
{
	return get_all_visible_viewers(node).size();
}

static WED_GISPoint* get_next_node(const WED_GISPoint* current_node,
							const TaxiRouteInfo& next_taxiroute)
{
	WED_GISPoint* next = NULL;
	if(next_taxiroute.nodes[0] == current_node)
	{
		next = next_taxiroute.nodes[1]; //Going backwards choose next-1 | 0--next--1-->0--current--1-->
	}
	else
	{
		next = next_taxiroute.nodes[0]; //Going forwards, choose next-0 | 0--current--1-->0--next--1-->
	}

	if(next == NULL)
	{
		return NULL;
	}
	else if(get_node_valence(next) == 1)
	{
		return NULL; //We don't want to travel there next, its time to end
	}
	//Will we have somewhere to go next?
	else if(filter_viewers_by_is_runway(next, next_taxiroute.taxiroute_name).size() == 0)
	{
		return NULL;
	}
	else
	{
		return next;
	}
}

static WED_TaxiRoute* get_next_taxiroute(const WED_GISPoint* current_node,
										 const TaxiRouteInfo& current_taxiroute)
{
	TaxiRouteInfoVec_t viewers = filter_viewers_by_is_runway(current_node, current_taxiroute.taxiroute_name);//The taxiroute name should equal to the runway name
	DebugAssert(viewers.size() == 1 || viewers.size() == 2);
	
	if(viewers.size() == 2)
	{
		return current_taxiroute.taxiroute_ptr == viewers[0].taxiroute_ptr ? viewers[1].taxiroute_ptr : viewers[0].taxiroute_ptr;
	}
	else
	{
		return current_taxiroute.taxiroute_ptr == viewers[0].taxiroute_ptr ? NULL : viewers[0].taxiroute_ptr;
	}
}

//returns pair<is_target_of_current,is_target_of_next>
static pair<bool,bool> get_taxiroute_relationship(const WED_GISPoint* current_node,
												  const TaxiRouteInfo& current_taxiroute,
												  const TaxiRouteInfo& next_taxiroute)
{
	//pair<is_target_of_current,is_target_of_next>
	std::pair<bool,bool> taxiroute_relationship(false,false);
	
	//See relationship chart in TaxiRouteSquishedZCheck
	if(current_taxiroute.nodes[1] == current_node)
	{
		taxiroute_relationship.first = true;
	}
	else
	{
		taxiroute_relationship.first = false;
	}

	if(next_taxiroute.nodes[1] == current_node)
	{
		taxiroute_relationship.second = true;
	}
	else
	{
		taxiroute_relationship.second = false;
	}

	return taxiroute_relationship;
}


static bool TaxiRouteSquishedZCheck( const RunwayInfo& runway_info,
									 const TaxiRouteInfo& start_taxiroute,//One of the ends of this chain of taxi routes
									 validation_error_vector& msgs,
									 WED_Airport* apt)
{
	//We know all the nodes are within threshold of the center and within bounds, the segments are parallel enough,
	//the route is a complete chain with no 3+-way splits. Now: do any of the segments make a complete 180 unexpectedly?
	WED_GISPoint* current_node = NULL;
	TaxiRouteInfo current_taxiroute(start_taxiroute.taxiroute_ptr);

	if(get_node_valence(start_taxiroute.nodes[0]) == 2)
	{
		current_node = start_taxiroute.nodes[0]; //We'll be moving backwards through the route, <----0--------1-->
	}
	else
	{
		current_node = start_taxiroute.nodes[1];//0---->1---->
	}

	//while we have not run out of nodes to traverse
	while(current_node != NULL)
	{
		WED_TaxiRoute* next_route = get_next_taxiroute(current_node,current_taxiroute);
		if(next_route == NULL)
		{
			break;
		}

		TaxiRouteInfo next_taxiroute(next_route);

		pair<bool,bool> relationship = get_taxiroute_relationship(current_node,current_taxiroute,next_taxiroute);
		WED_GISPoint* next_node = get_next_node(current_node,next_taxiroute);

		Vector2 runway_vec = Vector2(runway_info.runway_centerline_m.p1,runway_info.runway_centerline_m.p2);
		runway_vec.normalize();

		Point2 current_p1;
		Point2 current_p2;

		current_taxiroute.nodes[0]->GetLocation(gis_Geo, current_p1);
		current_p1 = translator.Forward(current_p1);
		
		current_taxiroute.nodes[1]->GetLocation(gis_Geo, current_p2);
		current_p2 = translator.Forward(current_p2);

		Vector2 taxiroute_vec_1 = Vector2(current_p1, current_p2);
		taxiroute_vec_1.normalize();
		double dot_runway_route_1 = runway_vec.dot(taxiroute_vec_1);

		Point2 next_p1;
		Point2 next_p2;

		next_taxiroute.nodes[0]->GetLocation(gis_Geo, next_p1);
		next_p1 = translator.Forward(next_p1);

		next_taxiroute.nodes[1]->GetLocation(gis_Geo, next_p2);
		next_p2 = translator.Forward(next_p2);

		Vector2 taxiroute_vec_2 = Vector2(next_p1, next_p2);
		taxiroute_vec_2.normalize();
		double dot_runway_route_2 = runway_vec.dot(taxiroute_vec_2);

		// Given a runway [<--------------------] where this side is this source
        //
		//    r_1        | o is target        | o is source
		//r_2            |--------------------------------------  |a|b
		//o is target    |[--2-->o<--1--] = - |[--2-->o--1-->] = +|-+-
		//o is source    |[<--2--o<--1--] = + |[<--2--o--1-->] = -|c|d
		//
		//The o represents the common, current, node
		//Because by this point we've passed the ParallelCheck all we need to do is check positive or negative

		int expected_sign = 0;

		bool first_is_target  = relationship.first;
		bool second_is_target = relationship.second;

		if((first_is_target == true  && second_is_target == true) ||
		   (first_is_target == false && second_is_target == false))
		{
			expected_sign = -1; //a and d
		}
		else
		{
			expected_sign = 1;//b and c
		}

		//Take the dot product between the two runway_routes
		double dot_product = taxiroute_vec_1.dot(taxiroute_vec_2);

		int real_sign = dot_product > 0 ? 1 : -1;
		if( real_sign != expected_sign)
		{
			TaxiRouteVec_t problem_children;
			problem_children.push_back(current_taxiroute.taxiroute_ptr);
			problem_children.push_back(next_taxiroute.taxiroute_ptr);

			//This copying is just to save to save screen space
			TaxiRouteInfoVec_t route_info_vec(problem_children.begin(),problem_children.end());

			msgs.push_back(validation_error_t("Taxi routes " + route_info_vec[0].taxiroute_name + " and " + route_info_vec[1].taxiroute_name + " have too extreme an angle between them",
											  problem_children,
											  apt));
			return false; //The next test will just be redundent so we'll end here
		}

		current_taxiroute = next_taxiroute;
		current_node = next_node;
	}

	return true;
}

//All checks that require knowledge of taxiroute connectivity checks
static bool DoTaxiRouteConnectivityChecks( const RunwayInfo& runway_info,
										   const TaxiRouteInfoVec_t& all_taxiroutes, //All the taxiroutes in the airport, for EnsureRunwayTaxirouteValences
										   const TaxiRouteInfoVec_t& matching_taxiroutes, //Only the taxiroutes which match the runway in runway_info
										   validation_error_vector& msgs,
										   WED_Airport* apt)
{
	int original_num_errors = msgs.size();
	
	TaxiRouteNodeVec_t matching_nodes;
	for (int i = 0; i < matching_taxiroutes.size(); ++i)
	{
		matching_nodes.push_back(matching_taxiroutes[i].nodes[0]);
		matching_nodes.push_back(matching_taxiroutes[i].nodes[1]);
	}

	sort(matching_nodes.begin(),matching_nodes.end());

	WED_TaxiRoute* out_start_taxiroute = NULL;
	if(RunwaysTaxiRouteValencesCheck(runway_info, matching_nodes, out_start_taxiroute, msgs, apt))
	{
		bool has_squished_z = false;

		//The algorithm requires there to be atleast 2 taxiroutes
		if(all_taxiroutes.size() >= 2 && out_start_taxiroute != NULL)
		{
			TaxiRouteSquishedZCheck(runway_info, out_start_taxiroute, msgs, apt);
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

		double dot_product = fabs(runway_centerline_vec.dot(taxiroute_vec));
		double ANGLE_THRESHOLD = 0.995;
		if(dot_product < ANGLE_THRESHOLD)
		{
			string msg = "Taxi route segment " + taxiroute_itr->taxiroute_name + " is not parallel to the runway's center line.";
			msgs.push_back(validation_error_t(msg,taxiroute_itr->taxiroute_ptr,apt));
		}
	}

	return msgs.size() - original_num_errors == 0 ? true : false;
}

static bool RunwayHasCorrectCoverage( const RunwayInfo& runway_info,
									  const TaxiRouteInfoVec_t& all_taxiroutes,
									  validation_error_vector& msgs,
									  WED_Airport* apt)
{
	int original_num_errors = msgs.size();

	vector<WED_GISPoint*> on_pavement_nodes;

	//First pass, remove all points that are outside of the runway
	for (TaxiRouteInfoVec_t::const_iterator itr = all_taxiroutes.begin(); itr != all_taxiroutes.end(); ++itr)
	{
		for (vector<WED_GISPoint*>::const_iterator point_itr = itr->nodes.begin(); point_itr != itr->nodes.end(); ++point_itr)
		{
			Point2 node_geo;
			(*point_itr)->GetLocation(gis_Geo, node_geo);

			Point2 node_m = translator.Forward(node_geo);

			//We don't want to collect the runway nodes, which are inside and are WED_GISPoints, but never something we care about
			if (runway_info.runway_corners_m.inside(node_m) == true)
			{
				on_pavement_nodes.push_back(*point_itr);
			}
		}
	}

	//A set of relavent runway_routes, to accumulate their lengths later on
	set<WED_TaxiRoute*> runway_routes;

	//Second pass, does each node have a runway taxi route attached? Also, collect said matching taxiroutes
	for (TaxiRouteNodeVec_t::iterator itr = on_pavement_nodes.begin(); itr != on_pavement_nodes.end(); ++itr)
	{
		int number_of_connected_routes = 0;

		//Filter by visible viewers that are of WED_TaxiRoute* type and whose runway is the same as the runway we're testing
		set<WED_Thing*> thing_viewers = get_all_visible_viewers(*itr);
		for (set<WED_Thing*>::iterator thing_itr = thing_viewers.begin(); thing_itr != thing_viewers.end(); ++thing_itr)
		{
			WED_TaxiRoute* taxiroute = dynamic_cast<WED_TaxiRoute*>(*thing_itr);

			if (taxiroute != NULL && taxiroute->GetRunway() == runway_info.runway_ptr->GetRunwayEnumsTwoway())
			{
				runway_routes.insert(taxiroute);
				++number_of_connected_routes;
			}
		}

		if (number_of_connected_routes == 0)
		{
			//Last minute check if this is a dead end
			if (get_node_valence(*itr) == 1)
			{
				string node_name;
				(*itr)->GetName(node_name);
				msgs.push_back(validation_error_t("Route node " + node_name + " is within the runway's bounds but is not connected to the runway's taxiroute", *itr, apt));
			}
		}
	}

	double length_accumulator = 0.0;
	TaxiRouteInfoVec_t runway_routes_info_vec(runway_routes.begin(),runway_routes.end());
	for (TaxiRouteInfoVec_t::const_iterator taxiroute_itr = runway_routes_info_vec.begin(); taxiroute_itr != runway_routes_info_vec.end(); ++taxiroute_itr)
	{
		length_accumulator += sqrt(taxiroute_itr->taxiroute_segment_m.squared_length());
	}

	//How much gap on a side there could be
	double COVERAGE_THRESHOLD = runway_info.runway_ptr->GetLength() * 0.25;

	AptRunway_t apt_runway;
	runway_info.runway_ptr->Export(apt_runway);
	//Clamp extremely small runways to the length
	if((apt_runway.width_mtr  * 4) > COVERAGE_THRESHOLD)
	{
		COVERAGE_THRESHOLD = (apt_runway.width_mtr * 4);
	}

	//Plus 5 meters in slop zone
	COVERAGE_THRESHOLD += 5;
	
	if (length_accumulator < COVERAGE_THRESHOLD)
	{
#if DEV
		//We have to figure out which side(s) to report as too short
		double amount_not_covered = 0.0;
		
		ostringstream oss;
		oss.precision(2);
		oss << std::fixed << COVERAGE_THRESHOLD - length_accumulator;
#endif
		string msg = "Taxi route for runway " + runway_info.runway_name + " does not span enough runway";
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

static TaxiRouteInfoVec_t GetTaxiRoutesFromViewers(const WED_GISPoint* node)
{
	set<WED_Thing*> node_viewers = get_all_visible_viewers(node);

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
static Polygon2 MakeHotZoneHitBox( const RunwayInfo& runway_info, //The relavent runway info's use
								   int runway_number, //The runway runway number we're doing
								   bool make_arrival) //Makes sure we're only making the correct side of the box
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

	double HITZONE_WIDTH_THRESHOLD_M;         // Width of Runway Protection Zone beyond runway width, each side
	double HITZONE_OVERFLY_THRESHOLD_M;      // Length of RPZ beyond runway end (departure end) or runway threshold (arrival end)
	if (runway_info.runway_ptr->GetLength() < 1500.0)
	{
		HITZONE_OVERFLY_THRESHOLD_M = 305.0;    // per FAA advisory circular 150-5300-13A, size A/B aircraft runways: 1000'
		HITZONE_WIDTH_THRESHOLD_M = 10.0;       // good guess, FAA advisory 150' from CL is impractically wide for many small airfields
	}
	else
	{
		HITZONE_OVERFLY_THRESHOLD_M = 518.0;    // per FAA, size C/D/E aircraft runways: 1700'
		HITZONE_WIDTH_THRESHOLD_M = 30.0;       // good guess, again, FAA is too wishfull
	}

	Polygon2 runway_hit_box_m = runway_info.runway_corners_m;

	Point2& bottom_left  = runway_hit_box_m[0];
	Point2& top_left     = runway_hit_box_m[1];
	Point2& top_right    = runway_hit_box_m[2];
	Point2& bottom_right = runway_hit_box_m[3];

	/*
	           top
	                 ^
	                 |
	                 gis_line_direction
	            1    |
	        1_______2!
		   |    |    |
		   |    |    |
		0  |    |    | 3
		   |    |    |
		   |    |    |
		   0---------3
		        0
		      bottom
	*/

	Vector2 gis_line_direction(runway_info.runway_centerline_m.p1,runway_info.runway_centerline_m.p2);
	gis_line_direction.normalize();

	Vector2 side_extension_vec = gis_line_direction.perpendicular_cw() * HITZONE_WIDTH_THRESHOLD_M; // Make RPZ wider than runway
	bottom_left -= side_extension_vec;
	bottom_right += side_extension_vec;
	top_left -= side_extension_vec;
	top_right += side_extension_vec;

	if(runway_number <= atc_18R)
	{
		if(runway_info.IsHotForArrival(runway_number) == true && make_arrival == true)
		{
			HITZONE_OVERFLY_THRESHOLD_M = max(HITZONE_OVERFLY_THRESHOLD_M - runway_info.runway_ptr->GetDisp1(), 0.0);
			gis_line_direction *= HITZONE_OVERFLY_THRESHOLD_M;
			//arrival_side is bottom_side;
			bottom_left  -= gis_line_direction;
			bottom_right -= gis_line_direction;
		}
		
		if(runway_info.IsHotForDeparture(runway_number) == true && make_arrival == false)
		{
			gis_line_direction *= HITZONE_OVERFLY_THRESHOLD_M;
			//departure_side is top_side;
			top_left  += gis_line_direction;
			top_right += gis_line_direction;
		}
	}
	else
	{
		if(runway_info.IsHotForArrival(runway_number) == true  && make_arrival == true)
		{
			HITZONE_OVERFLY_THRESHOLD_M = max(HITZONE_OVERFLY_THRESHOLD_M - runway_info.runway_ptr->GetDisp2(), 0.0);
			gis_line_direction *= HITZONE_OVERFLY_THRESHOLD_M;
			//arrival_side is top_side;
			top_left  += gis_line_direction;
			top_right += gis_line_direction;
		}
		
		if(runway_info.IsHotForDeparture(runway_number) == true  && make_arrival == false)
		{
			gis_line_direction *= HITZONE_OVERFLY_THRESHOLD_M;
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
	debug_mesh_segment(runway_hit_box_geo.side(0), 1, 0, 0, make_arrival, 0, 0); //left side
	debug_mesh_segment(runway_hit_box_geo.side(1), 0, 1, 0, 0, make_arrival, 0); //top side
	debug_mesh_segment(runway_hit_box_geo.side(2), 0, 0, 1, 0, 0, make_arrival); //right side
	debug_mesh_segment(runway_hit_box_geo.side(3), 1, 1, 1, make_arrival, make_arrival, make_arrival); //bottom side
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
		copy(all_taxiroutes[i].nodes.begin(), all_taxiroutes[i].nodes.end(), back_inserter(all_nodes));
	}
	
	//runway side 0 is the lower side, 1 is the higher side
	for (int runway_side = 0; runway_side < 2; ++runway_side)
	{
		//For every node in the airport,
		//- get its associated taxiroutes,
		//- see if they intersect with our box
		//- and if so, test if they are properly marked
		for(TaxiRouteNodeVec_t::iterator node_itr = all_nodes.begin();
			node_itr != all_nodes.end();
			++node_itr)
		{
			TaxiRouteInfoVec_t taxiroutes = GetTaxiRoutesFromViewers(*node_itr);
			for(TaxiRouteInfoVec_t::iterator taxiroute_itr = taxiroutes.begin(); taxiroute_itr != taxiroutes.end(); ++taxiroute_itr)
			{
				//First make the departures box, second make the arrivals box
				for (int make_arrival = 0; make_arrival < 2; make_arrival++)
				{
					int runway_number = runway_info.runway_numbers[runway_side];

					//Make the hitbox baed on the runway and which side (low/high) you're currently on and if you need to be making arrival or departure
					Polygon2 hit_box = MakeHotZoneHitBox(runway_info, runway_number, (bool)make_arrival);
					if(hit_box.empty() == true)
					{
						continue;
					}

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
						if(runway_info.IsHotForArrival(runway_number) == true && static_cast<bool>(make_arrival) == true)
						{
							FindIfMarked(runway_number, *taxiroute_itr, (*taxiroute_itr).hot_arrivals, "arrivals", msgs, apt);
						}

						if(runway_info.IsHotForDeparture(runway_number) == true && static_cast<bool>(make_arrival) == false)
						{
							FindIfMarked(runway_number, *taxiroute_itr, (*taxiroute_itr).hot_departures, "departures", msgs, apt);
						}
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
	
	if(all_taxiroutes_plain.empty() == true)
	{
		return; //Nothing to do here
	}

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

		if (matching_taxiroutes.empty() == false)
		{
			if (AllTaxiRouteNodesInRunway(*runway_info_itr, matching_taxiroutes, msgs, &apt))
			{
				if (TaxiRouteParallelCheck(*runway_info_itr, matching_taxiroutes, msgs, &apt))
				{
					if (TaxiRouteCenterlineCheck(*runway_info_itr, matching_taxiroutes, msgs, &apt))
					{
						if (DoTaxiRouteConnectivityChecks(*runway_info_itr, all_taxiroutes, matching_taxiroutes, msgs, &apt))
						{
							if (RunwayHasCorrectCoverage(*runway_info_itr, all_taxiroutes, msgs, &apt))
							{
								//Add additional checks as needed here
							}
						}
					}
				}
			}
		}

		AssaignRunwayUse(*runway_info_itr, all_use_rules);
		bool passes_hotzone_checks = DoHotZoneChecks(*runway_info_itr, all_taxiroutes, msgs, &apt);
		//Nothing to do here yet until we have more checks after this
	}
	return;
}
