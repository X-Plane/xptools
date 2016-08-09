#include "WED_ValidateATCRunwayChecks.h"

#include <iterator>

#if DEV
#include <iostream>
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

struct TaxiRouteInfo
{
	TaxiRouteInfo(const WED_TaxiRoute* taxiroute)
	{
		taxiroute_ptr = taxiroute;
		
		taxiroute_segment_geo;
		Bezier2 bez;
		taxiroute->GetSide(gis_Geo, 0, taxiroute_segment_geo, bez);
		taxiroute_segment_m = Segment2(translator.Forward(taxiroute_segment_geo.p1),translator.Forward(taxiroute_segment_geo.p2));
		node_0 = static_cast<const WED_TaxiRouteNode*>(taxiroute->GetNthSource(0));
		node_1 = static_cast<const WED_TaxiRouteNode*>(taxiroute->GetNthSource(1));
	}
	
	//Pointer to the original WED_TaxiRoute in WED's datamodel
	const WED_TaxiRoute* taxiroute_ptr;

	//Name of the taxiroute
	string taxiroute_name;

	//Segment2 representing the taxiroute in lat/lon
	Segment2 taxiroute_segment_geo;

	//Segment2 representing the taxiroute in meters
	Segment2 taxiroute_segment_m;

	//Source node of the taxiroute
	const WED_TaxiRouteNode* node_0;

	//Target node of the taxiroute
	const WED_TaxiRouteNode* node_1;
};

typedef vector<const WED_ATCRunwayUse*>  ATCRunwayUseVec_t;
typedef vector<const WED_ATCFlow*>       FlowVec_t;
typedef vector<const WED_Runway*>        RunwayVec_t;
typedef vector<const WED_TaxiRoute*>     TaxiRouteVec_t;
typedef vector<const WED_TaxiRouteNode*> TaxiRouteNodeVec_t;
typedef vector<const TaxiRouteInfo>      TaxiRouteInfoVec_t;

static vector<const RunwayInfo> CollectPotentiallyActiveRunways( const WED_Airport& apt,
																 const TaxiRouteInfoVec_t all_taxiroutes)
{
	FlowVec_t flows;
	CollectRecursive<back_insert_iterator<FlowVec_t>>(&apt,back_inserter<FlowVec_t>(flows));

	RunwayVec_t all_runways;
	CollectRecursive<back_insert_iterator<RunwayVec_t>>(&apt,back_inserter<RunwayVec_t>(all_runways));

	//Find all potentially active runways:
	//0 flows means treat all runways as potentially active
	//>1 means find all runways mentioned, ignoring duplicates
	RunwayVec_t potentially_active_runways;
	if(flows.size() == 0)
	{
		potentially_active_runways = all_runways;
	}
	else
	{
		ATCRunwayUseVec_t use_rules;
		CollectRecursive<back_insert_iterator<ATCRunwayUseVec_t>>(&apt,back_inserter<ATCRunwayUseVec_t>(use_rules));

		//For all runways in the airport
		for (RunwayVec_t::const_iterator runway_itr = all_runways.begin(); runway_itr != all_runways.end(); ++runway_itr)
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
					potentially_active_runways.insert(potentially_active_runways.begin(), *runway_itr);
					break;
				}
			}
		}
	}

	vector<const RunwayInfo> runway_info_vec;
	//Filter out any runway that has no taxiroutes associated with it
	for (RunwayVec_t::const_iterator runway_itr = potentially_active_runways.begin(); runway_itr != potentially_active_runways.end(); ++runway_itr)
	{
		for(vector<const TaxiRouteInfo>::const_iterator taxiroute_itr = all_taxiroutes.begin(); taxiroute_itr != all_taxiroutes.end(); ++taxiroute_itr)
		{
			string taxiroute_name = ENUM_Desc((taxiroute_itr)->taxiroute_ptr->GetRunway());
			
			string runway_name;
			(*runway_itr)->GetName(runway_name);
			if(runway_name == taxiroute_name)
			{
				runway_info_vec.push_back(*runway_itr);
				//break now so we only add this once
				break;
			}
		}
	}

	return runway_info_vec;
}

static bool IsTaxiRouteCloseToRunwayEdge( const RunwayInfo& runway_info,
										  const TaxiRouteInfo& taxiroute_info)
{
	//For all runways
		//For runway side Left and Right
			//for all taxiroutes
				//if taxiroute's node_0 or node_1 is within X meters of the runway
					//case on runTo do this we make a box off of the runway runway width and X meters long. If either node_0 or node_1 is inside it, we need to be hot if we need to be hot, a "hit_box"

	//We add on an addtional threshold of 
	int intersected_sides = 0;

	Segment2 left_side = runway_info.runway_corners_m.side(2);
	//debug_mesh_segment(left_side,0,0,1,0,0,1);

	Segment2 top_side = runway_info.runway_corners_m.side(3);
	//debug_mesh_segment(top_side,0,0,1,0,0,1);

	Segment2 right_side = runway_info.runway_corners_m.side(0);
	//debug_mesh_segment(right_side,0,0,1,0,0,1);

	Segment2 bottom_side = runway_info.runway_corners_m.side(1);
	//debug_mesh_segment(bottom_side,0,0,1,0,0,1);

	Point2 node_0_geo;
	Point2 node_1_geo;

	taxiroute_info.node_0->GetLocation(gis_Geo, node_0_geo);
	taxiroute_info.node_0->GetLocation(gis_Geo, node_1_geo);

	double left_distance_0 = left_side.squared_distance(node_0_geo);
	double top_distance_0 = top_side.squared_distance(node_0_geo);
	double right_distance_0 = right_side.squared_distance(node_0_geo);
	double bottom_distance_0 = bottom_side.squared_distance(node_0_geo);
}

static bool IsTaxiRouteHotForArrival( const RunwayInfo& runway_info,
									  const TaxiRouteInfo& taxiroute)
{
	if(taxiroute.taxiroute_ptr->HasHotArrival())
	{
		//	runway_info.runway_ptr->
	}
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

//--Centerline Checks----------------------------------------------------------
static bool AllTaxiRouteNodesInRunway( const RunwayInfo& runway_info,
									   const TaxiRouteInfoVec_t& matching_taxiroutes,
									   string* msg,
									   const WED_Thing*& problem_thing)
{
	string taxiroute_name = "";
	TaxiRouteNodeVec_t matching_taxiroute_nodes;
	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = matching_taxiroutes.begin(); taxiroute_itr != matching_taxiroutes.end(); ++taxiroute_itr)
	{
		matching_taxiroute_nodes.push_back(static_cast<const WED_TaxiRouteNode*>(taxiroute_itr->taxiroute_ptr->GetNthSource(0)));
		matching_taxiroute_nodes.push_back(static_cast<const WED_TaxiRouteNode*>(taxiroute_itr->taxiroute_ptr->GetNthSource(1)));
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
			return false;
		}
	}

	return true;
}

static bool TaxiRouteCenterlineCheck( const RunwayInfo& runway_info,
			 						  const TaxiRouteInfoVec_t& matching_taxiroutes,
			 						  string* msg,
			 						  const WED_Thing*& problem_thing)
{
	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = matching_taxiroutes.begin(); taxiroute_itr != matching_taxiroutes.end(); ++taxiroute_itr)
	{
		double METERS_TO_CENTER_THRESHOLD = 5.0;
		double p1_to_center_dist = sqrt(runway_info.runway_centerline_m.squared_distance_supporting_line(taxiroute_itr->taxiroute_segment_m.p1));
		double p2_to_center_dist = sqrt(runway_info.runway_centerline_m.squared_distance_supporting_line(taxiroute_itr->taxiroute_segment_m.p2));

		if( p1_to_center_dist > METERS_TO_CENTER_THRESHOLD ||
			p2_to_center_dist > METERS_TO_CENTER_THRESHOLD)
		{
			string taxiroute_name = ENUM_Desc((taxiroute_itr)->taxiroute_ptr->GetRunway());
			*msg = "Taxi route segement for runway " + taxiroute_name + " is not on the center line";
			problem_thing = taxiroute_itr->taxiroute_ptr;
			return false;
		}
	}

	return true;
}

static bool TaxiRouteRunwayTraversalCheck( const RunwayInfo& runway_info,
										   const TaxiRouteInfoVec_t& all_taxiroutes,
										   string* msg,
										   const WED_Thing*& problem_thing)
{
	for (TaxiRouteInfoVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin();
		taxiroute_itr != all_taxiroutes.end();
		++taxiroute_itr)
	{
#if DEV
		debug_mesh_segment(taxiroute_itr->taxiroute_segment_geo,0,1,0,0,1,0);
#endif
		if( runway_info.runway_corners_geo.inside(taxiroute_itr->taxiroute_segment_geo.p1) == true &&
			runway_info.runway_corners_geo.inside(taxiroute_itr->taxiroute_segment_geo.p2))
		{
			continue;
		}
		else
		{
			int intersected_sides = 0;

			Segment2 left_side = runway_info.runway_corners_geo.side(2);
			//debug_mesh_segment(left_side,0,0,1,0,0,1);

			Segment2 top_side = runway_info.runway_corners_geo.side(3);
			//debug_mesh_segment(top_side,0,0,1,0,0,1);

			Segment2 right_side = runway_info.runway_corners_geo.side(0);
			//debug_mesh_segment(right_side,0,0,1,0,0,1);

			Segment2 bottom_side = runway_info.runway_corners_geo.side(1);
			//debug_mesh_segment(bottom_side,0,0,1,0,0,1);

			Point2 p;

			intersected_sides = taxiroute_itr->taxiroute_segment_geo.intersect(left_side,   p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_itr->taxiroute_segment_geo.intersect(top_side,    p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_itr->taxiroute_segment_geo.intersect(right_side,  p) ? intersected_sides + 1 : intersected_sides;
			intersected_sides = taxiroute_itr->taxiroute_segment_geo.intersect(bottom_side, p) ? intersected_sides + 1 : intersected_sides;

			if(intersected_sides >= 2)
			{
				*msg = string("Taxi route segment " + taxiroute_itr->taxiroute_name) + " crosses runway " + runway_info.runway_name + " completely";
				problem_thing = static_cast<const WED_Thing*>(taxiroute_itr->taxiroute_ptr);
				return false;
			}
		}
	}

	return true;
}

static bool TaxiRouteSplitPathCheck( const RunwayInfo& runway_info,
									 const TaxiRouteInfoVec_t& all_taxiroutes,
									 string* msg,
									 const WED_Thing*& problem_thing)
{
	TaxiRouteNodeVec_t all_nodes;
	
	for (int i = 0; i < all_taxiroutes.size(); ++i)
	{
		all_nodes.push_back(all_taxiroutes[i].node_0);
		all_nodes.push_back(all_taxiroutes[i].node_1);
	}

	//Since we are storing pointers we can sort them numerically and see if any of them appear 3 or more times
	sort(all_nodes.begin(),all_nodes.end());

	DebugAssert(all_nodes.size() % 2 == 0 && all_nodes.size() > 0);

	for (int i = 0; i < all_nodes.size() - 3 - 1; i++)
	{
		const WED_TaxiRouteNode* node_1 = all_nodes[i + 0];
		const WED_TaxiRouteNode* node_2 = all_nodes[i + 1];
		const WED_TaxiRouteNode* node_3 = all_nodes[i + 2];

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
				*msg = "Taxi route node " + node_name + " is used three or more times in a runway's taxiroute";
				problem_thing = node_1;
				return false;
			}
		}
	}
	return true;
}

static bool TaxiRouteSquishedZCheck( const RunwayInfo& runway_info,
									 const TaxiRouteInfoVec_t& matching_routes,
									 string* msg,
									 const WED_Thing*& problem_thing)
{

	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = matching_routes.begin();
		taxiroute_itr != matching_routes.end();
		++taxiroute_itr)
	{
		Vector2 runway_centerline_vec = Vector2(runway_info.runway_centerline_m.p1,runway_info.runway_centerline_m.p2);
		Vector2 taxiroute_vec = Vector2(taxiroute_itr->taxiroute_segment_m.p1,taxiroute_itr->taxiroute_segment_m.p2);
#if DEV
		std::cout << "Runway Centerline (Lat/Lon): (" << runway_info.runway_centerline_geo.p1.x() << "," << runway_info.runway_centerline_geo.p1.y() << ")/(" <<
													runway_info.runway_centerline_geo.p2.x() << "," << runway_info.runway_centerline_geo.p2.y() << ")" << std::endl;
		std::cout << "Runway Centerline (Meters): (" <<  runway_info.runway_centerline_m.p1.x() <<   "," << runway_info.runway_centerline_m.p1.y() <<   ")/(" <<
													runway_info.runway_centerline_m.p2.x() <<   "," << runway_info.runway_centerline_m.p2.y() <<   ")" << std::endl;
		std::cout << "Runway Centerline (M->Vector): (" << runway_centerline_vec.x() << "," << runway_centerline_vec.y() << ")" << endl << endl;

		std::cout << "TaxiRoute Segment (Lat/Lon): (" << taxiroute_itr->taxiroute_segment_geo.p1.x() << "," << taxiroute_itr->taxiroute_segment_geo.p1.y() << ")/(" <<
													taxiroute_itr->taxiroute_segment_geo.p2.x() << "," << taxiroute_itr->taxiroute_segment_geo.p2.y() << ")" << std::endl;
		std::cout << "TaxiRoute Segment (Meters): (" <<  taxiroute_itr->taxiroute_segment_m.p1.x() <<   "," << taxiroute_itr->taxiroute_segment_m.p1.y() <<   ")/(" <<
													taxiroute_itr->taxiroute_segment_m.p2.x() <<   "," << taxiroute_itr->taxiroute_segment_m.p2.y() <<   ")" << std::endl;
		std::cout << "TaxiRoute Segment (M->Vector): (" << taxiroute_vec.x() << "," << taxiroute_vec.y() << ")" << std::endl << std::endl << std::endl;
#endif
		runway_centerline_vec /= runway_centerline_vec.normalize();
		taxiroute_vec /= taxiroute_vec.normalize();
#if DEV
		std::cout << "Runway Centerline Normalized (Lat/Lon): (" << runway_info.runway_centerline_geo.p1.x() << "," << runway_info.runway_centerline_geo.p1.y() << ")/(" <<
															   runway_info.runway_centerline_geo.p2.x() << "," << runway_info.runway_centerline_geo.p2.y() << ")" << std::endl;
		std::cout << "Runway Centerline Normalized (Meters): (" <<  runway_info.runway_centerline_m.p1.x() <<   "," << runway_info.runway_centerline_m.p1.y() <<   ")/(" <<
															   runway_info.runway_centerline_m.p2.x() <<   "," << runway_info.runway_centerline_m.p2.y() <<   ")" << std::endl << std::endl;

		std::cout << "TaxiRoute Segment Normalized (Lat/Lon): (" << taxiroute_itr->taxiroute_segment_geo.p1.x() << "," << taxiroute_itr->taxiroute_segment_geo.p1.y() << ")/(" <<
															   taxiroute_itr->taxiroute_segment_geo.p2.x() << "," << taxiroute_itr->taxiroute_segment_geo.p2.y() << ")" << std::endl;
		std::cout << "TaxiRoute Segment Normalized (Meters): (" <<  taxiroute_itr->taxiroute_segment_m.p1.x() <<   "," << taxiroute_itr->taxiroute_segment_m.p1.y() <<   ")/(" <<
															   taxiroute_itr->taxiroute_segment_m.p2.x() <<   "," << taxiroute_itr->taxiroute_segment_m.p2.y() <<   ")" << std::endl << std::endl;
#endif
		double dot_product = runway_centerline_vec.dot(taxiroute_vec);
#if DEV
		std::cout << "runway_center_vec.dot(taxiroute_vec) = " << dot_product << std::endl;
		std::cout << "-------------------------------------------------------" << std::endl;
#endif
		double ANGLE_THRESHOLD = 0.990;
		dot_product = 1.0 - dot_product;
		if((dot_product) < ANGLE_THRESHOLD || (dot_product) > 1.0)
		{
			*msg = "Taxi route segement " + taxiroute_itr->taxiroute_name + " is too sharply bent";
			problem_thing = static_cast<const WED_Thing*>(taxiroute_itr->taxiroute_ptr);
			return false;
		}
	}

	return true;
}

static bool RunwayHasAnyMatchingTaxiRoute( const RunwayInfo& runway_info,
										   const TaxiRouteInfoVec_t& all_taxiroutes,
										   string* msg,
										   const WED_Thing*& problem_thing)
{
}

static bool RunwayHasTotalCoverage( const RunwayInfo& runway_info,
									const TaxiRouteInfoVec_t& matching_taxiroutes,
									string* msg,
									const WED_Thing*& problem_thing)
{
	double total_length_m = 0.0;
	for(TaxiRouteInfoVec_t::const_iterator taxiroute_itr = matching_taxiroutes.begin(); taxiroute_itr != matching_taxiroutes.end(); ++taxiroute_itr)
	{
		//Add up all the lengths of the runway, see if it is within the threshold of how much coverage there must be
		total_length_m += sqrt(taxiroute_itr->taxiroute_segment_m.squared_length());
	}

	double diff = abs(runway_info.runway_ptr->GetLength() - total_length_m);
	double COVERAGE_THRESHOLD = 1;//You should be at most 1 meter less coverage
	if(diff > COVERAGE_THRESHOLD)
	{
		*msg = "Runway " + runway_info.runway_name + " is not sufficiently covered with taxi routes.";
		problem_thing = runway_info.runway_ptr;
		return false;
	}

	return true;
}
//-----------------------------------------------------------------------------

//--Hot zone checks------------------------------------------------------------

//Returns polygon in meters
static Polygon2 MakeHotZoneHitBox( const RunwayInfo& runway_info,
								   int runway_number,
								   bool is_arriving)
{
	const double HITZONE_WIDTH_THRESHOLD_M = 10.00;
	const double HITZONE_OVERFLY_THRESHOLD_M = 1500.00;

	Polygon2 runway_hit_box_m = runway_info.runway_corners_m;
	Point2& bottom_left  = runway_hit_box_m[0];
	Point2& top_left     = runway_hit_box_m[1];
	Point2& top_right    = runway_hit_box_m[2];
	Point2& bottom_right = runway_hit_box_m[3];

	/* 
	l_extension  1   r_extension
	<-------1_______2------->
		   |         |
		   |         |
		0  |         | 2
		   |         |
		   0---------3
		        3
	*/

	Segment2 left_side   = runway_info.runway_corners_m.side(0);
	Segment2 top_side    = runway_info.runway_corners_m.side(1);
	Segment2 right_side  = runway_info.runway_corners_m.side(2);
	Segment2 bottom_side = runway_info.runway_corners_m.side(3);

	Vector2 lb_extension_vec = Vector2(bottom_right, bottom_left);
	lb_extension_vec.normalize();
	lb_extension_vec *= HITZONE_WIDTH_THRESHOLD_M;
	left_side.move_by_vector(lb_extension_vec);
	top_left = left_side.p1;
	bottom_left = left_side.p2;

	Vector2 rb_extension_vec = lb_extension_vec * -1;
	rb_extension_vec.normalize();
	rb_extension_vec *= HITZONE_WIDTH_THRESHOLD_M;
	right_side.move_by_vector(rb_extension_vec);
	top_right = right_side.p2;
	bottom_right = right_side.p1;
	
	Segment2 arrival_side;
	Segment2 departure_side;
	
	if(runway_number < 18 && false)
	{
		if(is_arriving == true)
		{
			arrival_side = bottom_side;
			//We won't be using departure side
			//Extend the bottom side
			Vector2 bottom_vec = Vector2(bottom_side.p1, bottom_side.p2);
			
			Vector2 rotated_bottom_vec = bottom_vec.perpendicular_cw();
			rotated_bottom_vec.normalize();
			rotated_bottom_vec *= HITZONE_OVERFLY_THRESHOLD_M;
			
			runway_hit_box_m[3] = Segment2(runway_hit_box_m[3],bottom_vec).p1;
			runway_hit_box_m[0] = Segment2(runway_hit_box_m[3],bottom_vec).p2;			
		}
		else
		{
			departure_side = top_side;
			//Make the 
		}
	}
	else
	{
		if(is_arriving == true)
		{
			arrival_side = top_side;
			//We won't be using departure side
		}
		else
		{
			departure_side = bottom_side;
		}
	}

#if DEV
	Polygon2 runway_hit_box_geo = runway_hit_box_m;

	runway_hit_box_geo[0] = translator.Reverse(runway_hit_box_m[0]);
	runway_hit_box_geo[1] = translator.Reverse(runway_hit_box_m[1]);
	runway_hit_box_geo[2] = translator.Reverse(runway_hit_box_m[2]);
	runway_hit_box_geo[3] = translator.Reverse(runway_hit_box_m[3]);

	debug_mesh_segment(runway_hit_box_geo.side(0), 0, 0, 1,0,0,1);
	debug_mesh_segment(runway_hit_box_geo.side(1), 0, 0, 1,0,0,1);
	debug_mesh_segment(runway_hit_box_geo.side(2), 0, 0, 1,0,0,1);
	debug_mesh_segment(runway_hit_box_geo.side(3), 0, 0, 1,0,0,1);
#endif
	return runway_hit_box_m;
}

static void DoHotZoneChecks( const RunwayInfo& runway_info,
							 const TaxiRouteInfoVec_t& all_taxiroutes,
							 string* msg,
							 const WED_Thing*& problem_thing)
{
	//For the left and right side of the runway
	for (int i = 0; i < 2; i++)
	{
		AptRunway_t apt_runway;
		runway_info.runway_ptr->Export(apt_runway);

		//if id[0] < 18
			//test if on or before (since the landing is south)
		MakeHotZoneHitBox(runway_info,1,true);
	}
}
//-----------------------------------------------------------------------------
void DoATCRunwayChecks(const WED_Airport& apt, string* msg, const WED_Thing*& problem_thing)
{
	Bbox2 box;
	apt.GetBounds(gis_Geo, box);
	CreateTranslatorForBounds(box,translator);
	
	TaxiRouteVec_t all_taxiroutes_plain;
	CollectRecursive<back_insert_iterator<TaxiRouteVec_t>>(static_cast<const WED_Thing*>(&apt),back_inserter<TaxiRouteVec_t>(all_taxiroutes_plain));
	
	//
	TaxiRouteInfoVec_t all_taxiroutes;
	for (int i = 0; i < all_taxiroutes_plain.size(); i++)
	{
		all_taxiroutes.push_back(all_taxiroutes_plain[i]);
	}

	vector<const RunwayInfo> potentially_active_runways = CollectPotentiallyActiveRunways(apt,all_taxiroutes);

	//Pre-check
	//- Does this active runway even have any taxi routes associated with it?
	for(vector<const RunwayInfo>::const_iterator runway_info_itr = potentially_active_runways.begin();
		runway_info_itr != potentially_active_runways.end();
		++runway_info_itr)
	{
#if DEV
		debug_mesh_polygon((*runway_info_itr).runway_corners_geo,1,0,0);
		debug_mesh_segment((*runway_info_itr).runway_centerline_geo,1,0,0,1,0,0);
#endif
		if(TaxiRouteRunwayTraversalCheck( *runway_info_itr, all_taxiroutes, msg, problem_thing) == false)
		{
			break;
		}

		TaxiRouteInfoVec_t matching_taxiroutes = FilterMatchingRunways(*runway_info_itr, all_taxiroutes);
		bool passes_centerline_checks = false;
		if(TaxiRouteSplitPathCheck(*runway_info_itr, all_taxiroutes, msg, problem_thing))
		{
			if(TaxiRouteCenterlineCheck(*runway_info_itr, matching_taxiroutes, msg, problem_thing))
			{
				if(AllTaxiRouteNodesInRunway(*runway_info_itr, matching_taxiroutes, msg, problem_thing))
				{
					if(RunwayHasTotalCoverage(*runway_info_itr, matching_taxiroutes, msg, problem_thing))
					{
						if(TaxiRouteSquishedZCheck(*runway_info_itr, matching_taxiroutes, msg, problem_thing))
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

		bool passes_hotzone_checks = true;
		DoHotZoneChecks(*runway_info_itr,all_taxiroutes,msg,problem_thing);
		//For the left and right side
		
	}
	return;
}
