#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <queue>
#include <functional>
#include <iostream>

#define DBG_LIN_COLOR 1,0,1,1,0,1

#if DEV
#define DEBUG_VIS_LINES 1
#endif

#include "WED_Validate.h"
#include "WED_ValidateATCRunwayChecks.h"
#include "WED_EnumSystem.h"

#include "WED_Airport.h"
#include "WED_ATCFlow.h"
#include "WED_ATCRunwayUse.h"
#include "WED_PolygonPlacement.h"
#include "WED_Runway.h"
#include "WED_TaxiRoute.h"

#include "WED_ResourceMgr.h"
#include "WED_HierarchyUtils.h"
#include "CompGeomUtils.h"
#include "GISUtils.h"
#include "WED_PreviewLayer.h"

static CoordTranslator2 translator;

typedef vector<WED_ATCRunwayUse*>  ATCRunwayUseVec_t;
typedef vector<WED_ATCFlow*>       FlowVec_t;
typedef vector<WED_Runway*>        RunwayVec_t;
typedef vector<WED_TaxiRoute*>     TaxiRouteVec_t;

//We're just using WED_GISPoint because old WED and airports
typedef vector<WED_GISPoint*>      TaxiRouteNodeVec_t;
typedef vector<RunwayInfo>         RunwayInfoVec_t;
typedef vector<TaxiRouteInfo>      TaxiRouteInfoVec_t;

//Collects 'potentially active' runways. 
// - any runway that is referenced in at least one flow AND there is at least one runway segement taxi route on it
// - if no flows are defined, all runways are considered active
// - if no taxiway vector is passed, being mentioned in a flow is sufficient to consider it active
static RunwayInfoVec_t CollectPotentiallyActiveRunways( const TaxiRouteInfoVec_t& all_taxiroutes,
														const RunwayInfoVec_t& all_runways_info,
														validation_error_vector& msgs,
														WED_Airport* apt)
{
	FlowVec_t flows;
	CollectRecursive(apt,back_inserter<FlowVec_t>(flows),WED_ATCFlow::sClass);

	//Find all potentially active runways:
	//0 flows means treat all runways as potentially active
	//>1 means find all runways mentioned, ignoring duplicates
	RunwayVec_t potentially_active_runways;
	RunwayInfoVec_t runway_info_vec;
	
	if(flows.size() == 0)
	{
		runway_info_vec = all_runways_info;
	}
	else
	{
		ATCRunwayUseVec_t use_rules;
		CollectRecursive(apt,back_inserter<ATCRunwayUseVec_t>(use_rules),WED_ATCRunwayUse::sClass);

		//For all runways in the airport
		for(auto runway_itr : all_runways_info)
		{
			//Search through all runway uses, testing if the runway has a match AND at least one taxiroutes associated with it
			for(ATCRunwayUseVec_t::const_iterator use_itr = use_rules.begin(); use_itr != use_rules.end(); ++use_itr)
			{
				AptRunwayRule_t runway_rule;
				(*use_itr)->Export(runway_rule);

				//Compare the name of the runway mentioned by the taxiway to the runway
				string runway_name_p1 = runway_itr.name.substr(0,runway_itr.name.find_first_of('/'));
				string runway_name_p2 = runway_itr.name.substr(runway_itr.name.find_first_of('/')+1);

				if( runway_rule.runway == runway_name_p1 ||
					runway_rule.runway == runway_name_p2)
				{
					if (all_taxiroutes.empty())
					{
						// if no taxiroutes specified, being mentioned in a flow is sufficient to be considered active
						runway_info_vec.push_back(runway_itr);
					}
					else
					{
						// check that there is at least one taxi route associated with it
						for(vector<TaxiRouteInfo>::const_iterator taxiroute_itr = all_taxiroutes.begin(); taxiroute_itr != all_taxiroutes.end(); ++taxiroute_itr)
						{
							string taxiroute_name = ENUM_Desc((taxiroute_itr)->taxiroute_ptr->GetRunway());

							if(runway_itr.name == taxiroute_name || ( taxiroute_name[0] = '0' && runway_itr.name == taxiroute_name.substr(1) ))
							{
								runway_info_vec.push_back(runway_itr);
								break; //exit all_taxiroutes loop
							}
						}
					}
					break; //exit use_rules loop
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
	for(auto taxiroute_itr : all_taxiroutes)
	{
		if(taxiroute_itr.taxiroute_ptr->IsRunway())
		{
			string taxiroute_name = ENUM_Desc(taxiroute_itr.taxiroute_ptr->GetRunway());
			if(runway_info.name == taxiroute_name)
			{
				matching_taxiroutes.push_back(taxiroute_itr);
			}
		}
	}

	return matching_taxiroutes;
}

static void AssignRunwayUse( RunwayInfo& runway_info,
							  const ATCRunwayUseVec_t& all_use_rules)
{
	if(all_use_rules.empty() == true)
	{
		runway_info.runway_ops[0] = 0x01 | 0x02; //Mark for arrivals and departures
		runway_info.runway_ops[1] = 0x01 | 0x02;
		return;
	}

	//The case of "This airport has no flows" gets implicitly taken care of here by the fact that use_rules will be 0,
	//and the fact that we assign this in ???
	
	for(ATCRunwayUseVec_t::const_iterator use_itr = all_use_rules.begin(); use_itr != all_use_rules.end(); ++use_itr)
	{
		AptRunwayRule_t apt_runway_rule;
		(*use_itr)->Export(apt_runway_rule);
		
		if(runway_info.runway_numbers[0] == ENUM_LookupDesc(ATCRunwayOneway, apt_runway_rule.runway.c_str()))
		{
			runway_info.runway_ops[0] |= apt_runway_rule.operations;
		}
		else if(runway_info.runway_numbers[1] == ENUM_LookupDesc(ATCRunwayOneway, apt_runway_rule.runway.c_str()))
		{
			runway_info.runway_ops[1] |= apt_runway_rule.operations;
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
	for(auto itr : matching_taxiroutes)
	{
		matching_taxiroute_nodes.push_back(dynamic_cast<WED_GISPoint*>(itr.taxiroute_ptr->GetNthSource(0)));
		matching_taxiroute_nodes.push_back(dynamic_cast<WED_GISPoint*>(itr.taxiroute_ptr->GetNthSource(1)));
	}

	Polygon2 runway_hit_box(runway_info.corners_geo);
	
	Vector2 dir_ext = runway_info.dir_vec_1m *2.0;       // extend runway in longitudinal direction by 2m

	runway_hit_box[0] -= dir_ext;
	runway_hit_box[1] += dir_ext;
	runway_hit_box[2] += dir_ext;
	runway_hit_box[3] -= dir_ext;

	for(auto node_itr : matching_taxiroute_nodes)
	{
		Point2 node_location;
		node_itr->GetLocation(gis_Geo,node_location);
		if(runway_hit_box.inside(node_location) == false)
		{
			string node_name;
			node_itr->GetName(node_name);
			string msg = "Taxiroute node " + node_name + " is out of runway " + runway_info.name + "'s bounds";
			msgs.push_back(validation_error_t(msg, err_atcrwy_taxi_route_node_out_of_bounds, node_itr,apt));
		}
	}

#if DEBUG_VIS_LINES
#if DEBUG_VIS_LINES < 2
    if (msgs.size() - original_num_errors != 0)
#endif
    {
        debug_mesh_segment(runway_hit_box.side(0), DBG_LIN_COLOR); //left side
        debug_mesh_segment(runway_hit_box.side(1), DBG_LIN_COLOR); //top side
        debug_mesh_segment(runway_hit_box.side(2), DBG_LIN_COLOR); //right side
        debug_mesh_segment(runway_hit_box.side(3), DBG_LIN_COLOR); //bottom side
	}
#endif
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
		double p1_to_center_dist = sqrt(runway_info.centerline_m.squared_distance_supporting_line(taxiroute_itr->segment_m.p1));
		double p2_to_center_dist = sqrt(runway_info.centerline_m.squared_distance_supporting_line(taxiroute_itr->segment_m.p2));

		if( p1_to_center_dist > METERS_TO_CENTER_THRESHOLD ||
			p2_to_center_dist > METERS_TO_CENTER_THRESHOLD)
		{
			string taxiroute_name = ENUM_Desc((taxiroute_itr)->taxiroute_ptr->GetRunway());
			string msg = "Taxi route segment for runway " + taxiroute_name + " is not on the center line";
			msgs.push_back(validation_error_t(msg, err_atcrwy_centerline_taxiroute_segment_off_center, taxiroute_itr->taxiroute_ptr,apt));
		}
	}

	return msgs.size() - original_num_errors == 0 ? true : false;
}

static vector<TaxiRouteInfo> filter_viewers_by_is_runway(const WED_GISPoint* node, const string& runway_name)
{
	vector<TaxiRouteInfo> matching_routes;

	set<WED_Thing*> node_viewers = get_all_visible_viewers(node);

	for(auto itr : node_viewers)
	{
		WED_TaxiRoute * test_route = dynamic_cast<WED_TaxiRoute*>(itr);
		if(test_route != NULL)
		{
			TaxiRouteInfo taxiroute_info(test_route,translator);
			if(taxiroute_info.taxiroute_ptr->IsRunway() && taxiroute_info.name == runway_name)
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
					TaxiRouteInfoVec_t viewers = filter_viewers_by_is_runway(*node_itr,runway_info.name);
					out_start_taxiroute = viewers.front().taxiroute_ptr;
				}
				++num_valence_of_1;
			}
			else
			{
				msgs.push_back(validation_error_t("Runway " + runway_info.name + "'s taxi route is not continuous", err_atcrwy_connectivity_not_continous, *node_itr,apt));
			}
		}
		else if(node_valence >= 3)
		{
			string node_name;
			(*node_itr)->GetName(node_name);

			msgs.push_back(validation_error_t("Runway " + runway_info.name + "'s taxi route is split " + to_string(node_valence)+ " ways at taxi route node " + node_name, err_atcrwy_connectivity_n_split, *node_itr,apt));
		}
	}

	if(num_valence_of_1 == 0 && all_matching_nodes.size() > 0)
	{
		msgs.push_back(validation_error_t("Runway " + runway_info.name + "'s taxi route forms a loop", err_atcrwy_connectivity_forms_loop, runway_info.runway_ptr,apt));
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
	else if(filter_viewers_by_is_runway(next, next_taxiroute.name).size() == 0)
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
	TaxiRouteInfoVec_t viewers = filter_viewers_by_is_runway(current_node, current_taxiroute.name);//The taxiroute name should equal to the runway name
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
	//the route is a complete chain with no 3+-way splits. Now: do any of the segments make TWO complete 180 unexpectedly?
	WED_GISPoint* current_node = NULL;
	TaxiRouteInfo current_taxiroute(start_taxiroute.taxiroute_ptr,translator);

	{
//	if(get_node_valence(start_taxiroute.nodes[0]) == 2) // that is no good criteria at all: many, if not most, runways have exactly one
                                                        // taxiroute connected to their ends. So we may start out in the wrong direction ...
                                                        // leaving that runway route unchecked.
	set<WED_Thing *> viewers;
	start_taxiroute.nodes[0]->GetAllViewers(viewers);
	int runway_tags(0);
	
	for(auto v : viewers)
	{
		auto e = dynamic_cast<WED_TaxiRoute*>(v);
		if(e && e->IsRunway()) runway_tags++;
	}
	
	if(runway_tags > 1)
	{
		current_node = start_taxiroute.nodes[0]; //We'll be moving backwards through the route, <----0--------1-->
	}
	else
	{
		current_node = start_taxiroute.nodes[1];//0---->1---->
	}
	
	}

	//while we have not run out of nodes to traverse
	while(current_node != NULL)
	{
//printf("squishedZ iter\n");
		WED_TaxiRoute* next_route = get_next_taxiroute(current_node,current_taxiroute);
		if(next_route == NULL)
		{
			break;
		}

		TaxiRouteInfo next_taxiroute(next_route,translator);

		pair<bool,bool> relationship = get_taxiroute_relationship(current_node,current_taxiroute,next_taxiroute);
		WED_GISPoint* next_node = get_next_node(current_node,next_taxiroute);

		Vector2 taxiroute_vec_1 = Vector2(current_taxiroute.segment_m.p1, current_taxiroute.segment_m.p2);
		taxiroute_vec_1.normalize();
		double dot_runway_route_1 = runway_info.dir_1m.dot(taxiroute_vec_1);

		Vector2 taxiroute_vec_2 = Vector2(next_taxiroute.segment_m.p1, next_taxiroute.segment_m.p2);
		taxiroute_vec_2.normalize();
		double dot_runway_route_2 = runway_info.dir_1m.dot(taxiroute_vec_2);

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

			msgs.push_back(validation_error_t( "Taxi routes " + current_taxiroute.name + " and " + next_taxiroute.name + " are making a turn that is too tight for aircraft to follow.",
											   err_atcrwy_centerline_too_sharp_turn,
											   problem_children,
											   apt));

			return false; //The next test will just be redundant so we'll end here
		}

		current_taxiroute = next_taxiroute;
		current_node = next_node;
	}

	return true;
}

static bool FullyConnectedNetworkCheck( const TaxiRouteVec_t& all_taxiroutes,      // All the taxiroutes in the airport, for EnsureRunwayTaxirouteValences
										validation_error_vector& msgs,
										WED_Airport * apt)
{
	int original_num_errors = msgs.size();

	set<WED_Thing *> to_visit, all_routes;
	vector<set<WED_Thing *> > networks;
	
	for(auto tr : all_taxiroutes)
		all_routes.insert(tr);
		
	while(!all_routes.empty())
	{	
		to_visit.clear();
		networks.push_back(to_visit);
		to_visit.insert(*all_routes.begin());

		while(!to_visit.empty())
		{
			WED_Thing * i = *to_visit.begin();
			to_visit.erase(to_visit.begin());
			networks.back().insert(i);
			all_routes.erase(i);

			int ns = i->CountSources();
			for(int s = 0; s < ns; ++s)
			{
				WED_Thing * src = i->GetNthSource(s);
				set<WED_Thing *>	viewers;
				src->GetAllViewers(viewers);
				for(auto v : viewers)
					if(all_routes.count(v))	to_visit.insert(v);
			}
		}
	}
	
	int largest_nw(0);
	set<WED_Thing *> * largest_nw_set;
	
	for(auto& nw : networks)
	{
		if(nw.size() > largest_nw)
		{
			largest_nw_set = &nw;
			largest_nw = nw.size();
		}
	}
	
	for(auto& nw : networks)
	{
		if(&nw != largest_nw_set)
		{
			string msg;
			(*nw.begin())->GetName(msg);
			if(nw.size() == 1)
				msg = "Taxi Edge " + msg;
			else
				msg = "A set of " + to_string(nw.size()) + " Taxi Edges";
			msg += " is not connected to the remainder of the taxi network.";
			msgs.push_back(validation_error_t(msg, err_atc_taxi_routes_not_connected, nw ,apt));
		}
	}
	return msgs.size() - original_num_errors == 0 ? true : false;
}


//All checks that require knowledge of taxiroute connectivity checks
static bool DoTaxiRouteConnectivityChecks( const RunwayInfo& runway_info,
										   const TaxiRouteInfoVec_t& all_taxiroutes,      // All the taxiroutes in the airport, for EnsureRunwayTaxirouteValences
										   const TaxiRouteInfoVec_t& matching_taxiroutes, // Only the taxiroutes which match the runway in runway_info
										   validation_error_vector& msgs,
										   WED_Airport * apt)
{
	int original_num_errors = msgs.size();
	
	TaxiRouteNodeVec_t matching_nodes;
	for (auto itr : matching_taxiroutes)
	{
		matching_nodes.push_back(itr.nodes[0]);
		matching_nodes.push_back(itr.nodes[1]);
	}

	sort(matching_nodes.begin(),matching_nodes.end());

	WED_TaxiRoute* out_start_taxiroute = NULL;
	if(RunwaysTaxiRouteValencesCheck(runway_info, matching_nodes, out_start_taxiroute, msgs, apt))
	{
		//The algorithm requires there to be atleast 2 taxiroutes
		if(all_taxiroutes.size() >= 2 && out_start_taxiroute != NULL)
		{
			TaxiRouteSquishedZCheck(runway_info, TaxiRouteInfo(out_start_taxiroute,translator), msgs, apt);
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
		Vector2 taxiroute_vec = Vector2(taxiroute_itr->segment_m.p1,taxiroute_itr->segment_m.p2);
		taxiroute_vec.normalize();

		double dot_product = fabs(runway_info.dir_1m.dot(taxiroute_vec));
		double ANGLE_THRESHOLD = 0.995;
		if(dot_product < ANGLE_THRESHOLD)
		{
			string msg = "Taxi route segment " + taxiroute_itr->name + " is not parallel to the runway's " + runway_info.name + "'s center line.";
			msgs.push_back(validation_error_t(msg, err_atcrwy_centerline_not_parallel_centerline, taxiroute_itr->taxiroute_ptr,apt));
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
			Point2 node;
			(*point_itr)->GetLocation(gis_Geo, node);

			//We don't want to collect the runway nodes, which are inside and are WED_GISPoints, but never something we care about
			if (runway_info.corners_geo.inside(node) == true)
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
				msgs.push_back(validation_error_t("Route node " + node_name + " is within the runway's bounds but is not connected to the runway's taxiroute", err_atcrwy_taxi_route_node_within_bounds_but_not_connected, *itr, apt));
			}
		}
	}

	double length_accumulator = 0.0;
	
	TaxiRouteInfoVec_t runway_routes_info_vec;
	for (set<WED_TaxiRoute*>::const_iterator taxiroute_itr = runway_routes.begin(); taxiroute_itr != runway_routes.end(); ++taxiroute_itr)
			runway_routes_info_vec.push_back(TaxiRouteInfo(*taxiroute_itr,translator));
	
	for (TaxiRouteInfoVec_t::const_iterator taxiroute_itr = runway_routes_info_vec.begin(); taxiroute_itr != runway_routes_info_vec.end(); ++taxiroute_itr)
	{
		length_accumulator += sqrt(taxiroute_itr->segment_m.squared_length());
	}

	//How much gap on a side there could be
	double COVERAGE_THRESHOLD = runway_info.runway_ptr->GetLength() * 0.25;

	if((runway_info.runway_ptr->GetWidth() * 4.0) > COVERAGE_THRESHOLD)
	{
		COVERAGE_THRESHOLD = runway_info.runway_ptr->GetWidth() * 4.0;
	}

	//Plus 5 meters in slop zone
	COVERAGE_THRESHOLD += 5;
	
	if (length_accumulator < COVERAGE_THRESHOLD)
	{
#if DEV
		//We have to figure out which side(s) to report as too short
		ostringstream oss;
		oss.precision(2);
		oss << std::fixed << COVERAGE_THRESHOLD - length_accumulator;
#endif
		string msg = "Taxi route for runway " + runway_info.name + " does not span enough runway";
		msgs.push_back(validation_error_t(msg, err_atcrwy_taxi_route_does_not_span_enough_rwy,  runway_info.runway_ptr,apt));
		return false;
	}

	return msgs.size() - original_num_errors == 0 ? true : false;
}
//-----------------------------------------------------------------------------

//--Hot zone checks------------------------------------------------------------

static bool FindIfMarked( const int runway_number,        //enum from ATCRunwayOneway
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
		stringstream ss;
		ss  << "Taxi route " 
			<< taxiroute.name 
			<< " is too close to runway " 
			<< ENUM_Desc(runway_number)
			<< " and now must be marked active for runway "
			<< ENUM_Desc(runway_number)
			<< " " 
			<< op_type;

		msgs.push_back(validation_error_t(
			ss.str(),
			err_atcrwy_hotzone_taxi_route_too_close,
			taxiroute.taxiroute_ptr,
			apt));
	}
	return !found_marked;
}

static TaxiRouteInfoVec_t GetTaxiRoutesFromViewers(const WED_GISPoint* node)
{
	set<WED_Thing*> node_viewers = get_all_visible_viewers(node);

	TaxiRouteInfoVec_t matching_taxiroutes;
	for(set<WED_Thing*>::iterator node_viewer_itr = node_viewers.begin(); node_viewer_itr != node_viewers.end(); ++node_viewer_itr)
	{
		WED_TaxiRoute* taxiroute = dynamic_cast<WED_TaxiRoute*>(*node_viewer_itr);
		if(taxiroute != NULL)
		if(taxiroute->AllowAircraft())
		{
			matching_taxiroutes.push_back(TaxiRouteInfo(taxiroute,translator));
		}
	}

	return matching_taxiroutes;
}

//Returns polygon in lat/lon
static Polygon2 MakeHotZoneHitBox( const RunwayInfo& runway_info, // The relavent runway info
								   int runway_number,             // The runway number we're doing
								   bool make_arrival)             // add arrival protection area, but *only if* runway is used for arrivals
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

	double HITZONE_WIDTH_THRESHOLD_M; // Width of Runway Protection Zone beyond runway width, each side
	
	//Unfortunatly due to the messy real world we must have unrealistically low thresholds to avoid edge case after edge case
	//for every airport that doesn't play by the rules, was grandfathered in, or was built on Mt. Doom and needs to avoid the volcanic dust clouds. You know, the usual.
	double HITZONE_OVERFLY_THRESHOLD_M = 100.00;

	if (runway_info.runway_ptr->GetLength() < 1500.0)
	{
		HITZONE_WIDTH_THRESHOLD_M = 10.0; // good guess, FAA advisory 150' from CL is impractically wide for many small airfields
	}
	else
	{
		HITZONE_WIDTH_THRESHOLD_M = 30.0; // good guess, again, FAA is too wishfull
	}

	Polygon2 runway_hit_box(runway_info.corners_geo);
	/*         top   ^
	                 |
	                 runway_info.dir_vec_1m
	            1    |
	        1_______2!----> runway_info.width_vec_1m
		   |    |    |
		0  |    |    | 2
		   |    |    |
		   0---------3
		        3
		      bottom 
	*/
	Vector2 width_ext = runway_info.width_vec_1m * HITZONE_WIDTH_THRESHOLD_M;
	
	runway_hit_box[0] -= width_ext;
	runway_hit_box[1] -= width_ext;
	runway_hit_box[2] += width_ext;
	runway_hit_box[3] += width_ext;

	Vector2 len_ext;

	if(runway_number <= atc_18R)
	{
		if(runway_info.IsHotForArrival(runway_number) == true && make_arrival == true)
		{
			HITZONE_OVERFLY_THRESHOLD_M = max(HITZONE_OVERFLY_THRESHOLD_M - runway_info.runway_ptr->GetDisp1(), 0.0);
			//arrival_side is bottom_side;
			runway_hit_box[0] -= runway_info.dir_vec_1m * HITZONE_OVERFLY_THRESHOLD_M;
			runway_hit_box[3] -= runway_info.dir_vec_1m * HITZONE_OVERFLY_THRESHOLD_M;
		}

		if(runway_info.IsHotForDeparture(runway_number) == true && make_arrival == false)
		{
			//departure_side is top_side;
			runway_hit_box[1] += runway_info.dir_vec_1m * HITZONE_OVERFLY_THRESHOLD_M;
			runway_hit_box[2] += runway_info.dir_vec_1m * HITZONE_OVERFLY_THRESHOLD_M;
		}
	}
	else
	{
		if(runway_info.IsHotForArrival(runway_number) == true  && make_arrival == true)
		{
			HITZONE_OVERFLY_THRESHOLD_M = max(HITZONE_OVERFLY_THRESHOLD_M - runway_info.runway_ptr->GetDisp2(), 0.0);
			//arrival_side is top_side;
			runway_hit_box[1] += runway_info.dir_vec_1m * HITZONE_OVERFLY_THRESHOLD_M;
			runway_hit_box[2] += runway_info.dir_vec_1m * HITZONE_OVERFLY_THRESHOLD_M;
		}

		if(runway_info.IsHotForDeparture(runway_number) == true  && make_arrival == false)
		{
			//departure_side is bottom_side;
			runway_hit_box[0] -= runway_info.dir_vec_1m * HITZONE_OVERFLY_THRESHOLD_M;
			runway_hit_box[3] -= runway_info.dir_vec_1m * HITZONE_OVERFLY_THRESHOLD_M;
		}
	}

	return runway_hit_box;
}

static bool DoHotZoneChecks( const RunwayInfo& runway_info,
							 const TaxiRouteInfoVec_t& all_taxiroutes,
							 validation_error_vector& msgs,
							 WED_Airport* apt)
{
	int original_num_errors = msgs.size();
	for (int runway_side = 0; runway_side < 2; ++runway_side)
	{
		int runway_number = runway_info.runway_numbers[runway_side];
	
		for (int make_arrival = 0; make_arrival < 2; make_arrival++)
		{
			//Make the hitbox baed on the runway and which side (low/high) you're currently on and if you need to be making arrival or departure
			Polygon2 hit_box = MakeHotZoneHitBox(runway_info, runway_number, (bool)make_arrival);
			
			if(hit_box.empty()) continue;
				
			for(auto taxiroute_itr : all_taxiroutes)
			{
				if(!taxiroute_itr.taxiroute_ptr->AllowAircraft()) continue;

				//Run two tests, intersection with the side, and if that fails, point inside the polygon
				if(hit_box.intersects(taxiroute_itr.segment_geo) || hit_box.inside(taxiroute_itr.segment_geo.p1))
				{
					if(runway_info.IsHotForArrival(runway_number) == true && static_cast<bool>(make_arrival) == true)
					{
						bool hitbox_error = FindIfMarked(runway_number, taxiroute_itr, taxiroute_itr.hot_arrivals, "arrivals", msgs, apt);
#if DEBUG_VIS_LINES
#if DEBUG_VIS_LINES < 2
						if (hitbox_error)
#endif
						{
							debug_mesh_segment(hit_box.side(0), DBG_LIN_COLOR); //left side
							debug_mesh_segment(hit_box.side(1), DBG_LIN_COLOR); //top side
							debug_mesh_segment(hit_box.side(2), DBG_LIN_COLOR); //right side
							debug_mesh_segment(hit_box.side(3), DBG_LIN_COLOR); //bottom side
						}
#endif
					}

					if(runway_info.IsHotForDeparture(runway_number) == true && static_cast<bool>(make_arrival) == false)
					{
						bool hitbox_error = FindIfMarked(runway_number, taxiroute_itr, taxiroute_itr.hot_departures, "departures", msgs, apt);
#if DEBUG_VIS_LINES
#if DEBUG_VIS_LINES < 2
						if (hitbox_error)
#endif
						{
							debug_mesh_segment(hit_box.side(0), DBG_LIN_COLOR); //left side
							debug_mesh_segment(hit_box.side(1), DBG_LIN_COLOR); //top side
							debug_mesh_segment(hit_box.side(2), DBG_LIN_COLOR); //right side
							debug_mesh_segment(hit_box.side(3), DBG_LIN_COLOR); //bottom side
						}
#endif
					}
				}
			}
		}
	}
	return msgs.size() - original_num_errors == 0 ? true : false;
}

// flag all ground traffic routes that cross a runways hitbox

static void AnyTruckRouteNearRunway( const RunwayInfo& runway_info,
							 const TaxiRouteInfoVec_t& all_taxiroutes,
							 validation_error_vector& msgs,
							 WED_Airport* apt)
{
	int original_num_errors = msgs.size();

	Polygon2 runway_hit_box(runway_info.corners_geo);
	
	Vector2 side_ext = runway_info.width_vec_1m * 15.0;  // Require 10m side clearance - some 15m to the *cenerline* of that road
	Vector2 len_ext  = runway_info.dir_vec_1m   * 30.0;  // required distance of ground traffic routes from runway ends in meters
	if(runway_info.runway_ptr->GetLength() > 1500.0 ) len_ext *= 2.0; // Require more off-end clearance for full-size runways
	runway_hit_box[0] -= len_ext + side_ext;
	runway_hit_box[1] += len_ext - side_ext;
	runway_hit_box[2] += len_ext + side_ext;
	runway_hit_box[3] -= len_ext - side_ext;
	
	for(TaxiRouteInfoVec_t::const_iterator route_itr = all_taxiroutes.begin(); route_itr != all_taxiroutes.end(); ++route_itr)
	{
		if(!route_itr->taxiroute_ptr->AllowTrucks()) continue;

		if (runway_hit_box.intersects(route_itr->segment_geo) == true)
		{
			string msg = "Truck Route " + route_itr->name + " intersects with runway " + runway_info.name;
			msgs.push_back(validation_error_t(msg, err_atcrwy_truck_route_too_close_to_runway, route_itr->taxiroute_ptr, apt));
		}
		else
		{
			for (int i = 0; i < 2; ++i)
			{
				WED_GISPoint *node = dynamic_cast<WED_GISPoint*>(route_itr->taxiroute_ptr->GetNthSource(i));
				Point2 node_location;
				node->GetLocation(gis_Geo, node_location);

				if (runway_hit_box.inside(node_location))
				{
					string node_name;
					node->GetName(node_name);
					string msg = "Truck Route node " + node_name + " is too close to runway " + runway_info.name;
					msgs.push_back(validation_error_t(msg, err_atcrwy_truck_route_too_close_to_runway, node, apt));
				}
			}
		}
	}

#if DEBUG_VIS_LINES
#if DEBUG_VIS_LINES < 2
    if (msgs.size() - original_num_errors != 0)
#endif
    {
        debug_mesh_segment(runway_hit_box.side(0), DBG_LIN_COLOR); //left side
        debug_mesh_segment(runway_hit_box.side(1), DBG_LIN_COLOR); //top side
        debug_mesh_segment(runway_hit_box.side(2), DBG_LIN_COLOR); //right side
        debug_mesh_segment(runway_hit_box.side(3), DBG_LIN_COLOR); //bottom side
	}
#endif
}


static void	AnyPolgonsOnRunway( const RunwayInfo& runway_info,	 const vector<WED_PolygonPlacement *>& all_polygons,
							 validation_error_vector& msgs, WED_Airport* apt, WED_ResourceMgr * rmgr)
{

	Polygon2 runway_hit_box(runway_info.corners_geo);

	Vector2 side_ext = runway_info.width_vec_1m * -1.0;  // Allow any polygon to overlap runway by 1m on all sides
	Vector2 len_ext  = runway_info.dir_vec_1m   * -1.0;

	runway_hit_box[0] -= len_ext + side_ext;
	runway_hit_box[1] += len_ext - side_ext;
	runway_hit_box[2] += len_ext + side_ext;
	runway_hit_box[3] -= len_ext - side_ext;

	Bbox2	runway_bounds;
	runway_info.runway_ptr->GetBounds(gis_Geo,runway_bounds);
	
	for(auto pp : all_polygons)
	{
		if(pp->Cull(runway_bounds))
		{
			string vpath;
			const pol_info_t * pol_info;
			
			int lg = group_TaxiwaysBegin;
			pp->GetResource(vpath);
			if(!vpath.empty() && rmgr->GetPol(vpath,pol_info) && !pol_info->group.empty())
				lg = layer_group_for_string(pol_info->group.c_str(),pol_info->group_offset, lg);
				
			if(lg <= group_RunwaysEnd ) break;  // don't worry about polygons drawn underneath the runway
			
			bool isOnRunway = pp->Overlaps(gis_Geo, runway_hit_box);

			if (isOnRunway)
			{
				string msg ;
				pp->GetName(msg);
				msg = "The gateway discourages user created runway markings. DrapedPolygon '" + msg + "' intersects with runway " + runway_info.name;
				msgs.push_back(validation_error_t(msg, warn_atcrwy_marking, pp, apt));
			}
		}
	}
}

static bool is_ground_traffic_route(WED_Thing * r)
{
	return static_cast<WED_TaxiRoute*>(r)->AllowTrucks();
}

static bool is_aircraft_taxi_route(WED_Thing * r)
{
	return static_cast<WED_TaxiRoute*>(r)->AllowAircraft();
}

//-----------------------------------------------------------------------------

void WED_DoATCRunwayChecks(WED_Airport& apt, validation_error_vector& msgs, WED_ResourceMgr * res_mgr)
{
	Bbox2 box;
	apt.GetBounds(gis_Geo, box);
	CreateTranslatorForBounds(box,translator); // equivalent to MapZoomerNew: Pre-calculates cos(lat) to covert LL->Meter with linear algebra, only
	
	TaxiRouteVec_t all_taxiroutes_plain;
	CollectRecursive(&apt,back_inserter<TaxiRouteVec_t>(all_taxiroutes_plain)); // that includes GT routes
	
	TaxiRouteInfoVec_t	all_taxiroutes_info;
	TaxiRouteVec_t 		all_aircraftroutes_plain;
	TaxiRouteInfoVec_t	all_aircraftroutes;
	TaxiRouteInfoVec_t	all_truckroutes;
	
	all_taxiroutes_info.reserve(all_taxiroutes_plain.size());
	
	for(auto itr : all_taxiroutes_plain)
	{
		TaxiRouteInfo tr_info(itr,translator);
		all_taxiroutes_info.push_back(tr_info);
		if(tr_info.is_aircraft_route)
		{
			all_aircraftroutes.push_back(tr_info);
			all_aircraftroutes_plain.push_back(itr);
		}
		else
			all_truckroutes.push_back(tr_info);
	}
			
	RunwayVec_t all_runways;
	CollectRecursive(&apt,back_inserter<RunwayVec_t>(all_runways),WED_Runway::sClass);
	
	RunwayInfoVec_t all_runways_info;
	for(auto itr : all_runways)
		all_runways_info.push_back(RunwayInfo(itr,translator));

	if(!all_taxiroutes_info.empty())
	{
		RunwayInfoVec_t potentially_active_runways = CollectPotentiallyActiveRunways(all_aircraftroutes, all_runways_info, msgs, &apt);
		ATCRunwayUseVec_t all_use_rules;
		CollectRecursive(&apt,back_inserter<ATCRunwayUseVec_t>(all_use_rules), WED_ATCRunwayUse::sClass);
		
		FullyConnectedNetworkCheck(all_aircraftroutes_plain, msgs, &apt);
		
		for(auto runway_info_itr : potentially_active_runways)
		{
			int original_num_errors = msgs.size();
			TaxiRouteInfoVec_t matching_taxiroutes = FilterMatchingRunways(runway_info_itr, all_aircraftroutes);

			if (!matching_taxiroutes.empty())
			{
				if (AllTaxiRouteNodesInRunway(runway_info_itr, matching_taxiroutes, msgs, &apt))
				{
					if (TaxiRouteParallelCheck(runway_info_itr, matching_taxiroutes, msgs, &apt))
					{
						if (TaxiRouteCenterlineCheck(runway_info_itr, matching_taxiroutes, msgs, &apt))
						{
							if (DoTaxiRouteConnectivityChecks(runway_info_itr, all_aircraftroutes, matching_taxiroutes, msgs, &apt))
							{
								if (RunwayHasCorrectCoverage(runway_info_itr, matching_taxiroutes, msgs, &apt))
								{
									//Add additional checks as needed here
								}
							}
						}
					}
				}
			}
	#if DEBUG_VIS_LINES
	#if DEBUG_VIS_LINES < 2
			if (msgs.size() - original_num_errors != 0)
	#endif
			{
				debug_mesh_polygon((runway_info_itr).corners_geo,1,0,1);
				debug_mesh_segment((runway_info_itr).centerline_geo,DBG_LIN_COLOR);
			}
	#endif
			AssignRunwayUse(runway_info_itr, all_use_rules);
			bool passes_hotzone_checks = DoHotZoneChecks(runway_info_itr, all_taxiroutes_info, msgs, &apt);
			//Nothing to do here yet until we have more checks after this
		}
	}

	vector<WED_PolygonPlacement *> all_polys;
	if(gExportTarget == wet_gateway)
		CollectRecursive(&apt,back_inserter(all_polys), WED_PolygonPlacement::sClass);
		
	if(!all_polys.empty())
	{
		for(auto runway_info_itr : all_runways_info)
			AnyPolgonsOnRunway(runway_info_itr, all_polys, msgs, &apt, res_mgr);
	}

	if(!all_truckroutes.empty())
	{
		for(auto runway_info_itr : all_runways_info)
			AnyTruckRouteNearRunway(runway_info_itr, all_truckroutes, msgs, &apt);
	}
	
}
