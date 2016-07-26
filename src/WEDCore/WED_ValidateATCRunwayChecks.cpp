#include "WED_ValidateATCRunwayChecks.h"

#include <iterator>

#include "WED_EnumSystem.h"
#include "WED_Validate.h"

#include "WED_Airport.h"
#include "WED_ATCFlow.h"
#include "WED_ATCRunwayUse.h"
#include "WED_Runway.h"
#include "WED_TaxiRoute.h"

#include "AptDefs.h"

#include "CompGeomDefs2.h"
#include "CompGeomUtils.h"
#include "GISUtils.h"

typedef vector<const WED_ATCRunwayUse*> ATCRunwayUseVec_t;
typedef vector<const WED_ATCFlow*>   FlowVec_t;
typedef vector<const WED_Runway*>    RunwayVec_t;
typedef vector<const WED_TaxiRoute*> TaxiRouteVec_t;

static CoordTranslator2 translator;

RunwayVec_t CollectPotentiallyActiveRunways(WED_Thing* who)
{
	//Find all potentially active runways:
	//0 flows means treat all runways as potentially active
	//>1 means find all runways mentioned, ignoring duplicates

	RunwayVec_t potentially_active_runways;
	WED_Airport* apt = dynamic_cast<WED_Airport*>(who);
	FlowVec_t flows;
	
	CollectRecursive<back_insert_iterator<FlowVec_t>>(apt,back_inserter<FlowVec_t>(flows));

	RunwayVec_t all_runways;

	CollectRecursive<back_insert_iterator<RunwayVec_t>>(apt,back_inserter<RunwayVec_t>(all_runways));

	if(flows.size() == 0)
	{
		potentially_active_runways = all_runways;
	}
	else
	{
		ATCRunwayUseVec_t use_rules;

		CollectRecursive<back_insert_iterator<ATCRunwayUseVec_t>>(apt,back_inserter<ATCRunwayUseVec_t>(use_rules));

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

	return potentially_active_runways;
}

static WED_Thing* DoRunwayHasMatchingTaxiRoute( WED_Thing* who,
												string& msg,
												const RunwayVec_t& potentially_active_runways,
												const TaxiRouteVec_t& all_taxiroutes)
{
	for(RunwayVec_t::const_iterator runway_itr = potentially_active_runways.begin();
		runway_itr != potentially_active_runways.end();
		++runway_itr)
	{
		Point2 bounds[4];
		(*runway_itr)->GetCorners(gis_Geo,bounds);
		
		Polygon2 runway_bounds;
		runway_bounds.push_back(bounds[0]);
		runway_bounds.push_back(bounds[1]);
		runway_bounds.push_back(bounds[2]);
		runway_bounds.push_back(bounds[3]);
#if DEV
		debug_mesh_polygon(runway_bounds,1,0,0);
#endif

		Point2 ends[2];
		(*runway_itr)->GetSource()->GetLocation(gis_Geo,ends[0]);
		(*runway_itr)->GetTarget()->GetLocation(gis_Geo,ends[1]);

		Segment2 runway_centerline(ends[0], ends[1]);
#if DEV
		debug_mesh_segment(runway_centerline,1,0,0,1,0,0);
#endif

		for(TaxiRouteVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin();
			taxiroute_itr != all_taxiroutes.end();
			++taxiroute_itr)
		{
			Segment2 taxiroute_segment;
			Bezier2 bez;
			(*taxiroute_itr)->GetSide(gis_Geo, 0, taxiroute_segment, bez);
#if DEV
			debug_mesh_segment(taxiroute_segment,0,1,0,0,1,0);
#endif
			if( runway_bounds.inside(taxiroute_segment.p1) == true &&
				runway_bounds.inside(taxiroute_segment.p2))
			{
				continue;
			}
			else
			{
				int intersected_sides = 0;

				Segment2 left_side	 = Segment2(runway_bounds[2],runway_bounds[3]);
				//debug_mesh_segment(left_side,0,0,1,0,0,1);
				
				Segment2 top_side = Segment2(runway_bounds[3],runway_bounds[0]);
				//debug_mesh_segment(top_side,0,0,1,0,0,1);

				Segment2 right_side   = Segment2(runway_bounds[0],runway_bounds[1]);
				//debug_mesh_segment(right_side,0,0,1,0,0,1);

				Segment2 bottom_side    = Segment2(runway_bounds[1],runway_bounds[2]);
				//debug_mesh_segment(bottom_side,0,0,1,0,0,1);
				
				Point2 p;

				intersected_sides = taxiroute_segment.intersect(left_side,   p) ? intersected_sides + 1 : intersected_sides;
				intersected_sides = taxiroute_segment.intersect(top_side,    p) ? intersected_sides + 1 : intersected_sides;
				intersected_sides = taxiroute_segment.intersect(right_side,  p) ? intersected_sides + 1 : intersected_sides;
				intersected_sides = taxiroute_segment.intersect(bottom_side, p) ? intersected_sides + 1 : intersected_sides;

				if(intersected_sides >= 2)
				{
					string runway_name;
					(*runway_itr)->GetName(runway_name);

					string taxiroute_name;
					(*taxiroute_itr)->GetName(taxiroute_name);

					msg = string("Error: Taxiroute segment " + taxiroute_name) + " crosses runway " + runway_name + " completely";
					
					//Make it selectable. Remember, the data this pointer is refering too was not const to begin with
					//who =  static_cast<WED_Thing*>(const_cast<WED_TaxiRoute*>(*taxiroute_itr));
				}
			}
		}
	}

	return NULL;
}

static WED_Thing* DoTaxiRouteRunwayTraversalCheck( WED_Thing* who,
												   string& msg,
												   const RunwayVec_t& potentially_active_runways,
												   const TaxiRouteVec_t& all_taxiroutes)
{
	for(RunwayVec_t::const_iterator runway_itr = potentially_active_runways.begin();
		runway_itr != potentially_active_runways.end();
		++runway_itr)
	{
		Point2 bounds[4];
		(*runway_itr)->GetCorners(gis_Geo,bounds);
		
		Polygon2 runway_bounds;
		runway_bounds.push_back(bounds[0]);
		runway_bounds.push_back(bounds[1]);
		runway_bounds.push_back(bounds[2]);
		runway_bounds.push_back(bounds[3]);
#if DEV
		debug_mesh_polygon(runway_bounds,1,0,0);
#endif

		Point2 ends[2];
		(*runway_itr)->GetSource()->GetLocation(gis_Geo,ends[0]);
		(*runway_itr)->GetTarget()->GetLocation(gis_Geo,ends[1]);

		Segment2 runway_centerline(ends[0], ends[1]);
#if DEV
		debug_mesh_segment(runway_centerline,1,0,0,1,0,0);
#endif

		for(TaxiRouteVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin();
			taxiroute_itr != all_taxiroutes.end();
			++taxiroute_itr)
		{
			Segment2 taxiroute_segment;
			Bezier2 bez;
			(*taxiroute_itr)->GetSide(gis_Geo, 0, taxiroute_segment, bez);
#if DEV
			debug_mesh_segment(taxiroute_segment,0,1,0,0,1,0);
#endif
			if( runway_bounds.inside(taxiroute_segment.p1) == true &&
				runway_bounds.inside(taxiroute_segment.p2))
			{
				continue;
			}
			else
			{
				int intersected_sides = 0;

				Segment2 left_side	 = Segment2(runway_bounds[2],runway_bounds[3]);
				//debug_mesh_segment(left_side,0,0,1,0,0,1);
				
				Segment2 top_side = Segment2(runway_bounds[3],runway_bounds[0]);
				//debug_mesh_segment(top_side,0,0,1,0,0,1);

				Segment2 right_side   = Segment2(runway_bounds[0],runway_bounds[1]);
				//debug_mesh_segment(right_side,0,0,1,0,0,1);

				Segment2 bottom_side    = Segment2(runway_bounds[1],runway_bounds[2]);
				//debug_mesh_segment(bottom_side,0,0,1,0,0,1);
				
				Point2 p;

				intersected_sides = taxiroute_segment.intersect(left_side,   p) ? intersected_sides + 1 : intersected_sides;
				intersected_sides = taxiroute_segment.intersect(top_side,    p) ? intersected_sides + 1 : intersected_sides;
				intersected_sides = taxiroute_segment.intersect(right_side,  p) ? intersected_sides + 1 : intersected_sides;
				intersected_sides = taxiroute_segment.intersect(bottom_side, p) ? intersected_sides + 1 : intersected_sides;

				if(intersected_sides >= 2)
				{
					string runway_name;
					(*runway_itr)->GetName(runway_name);

					string taxiroute_name;
					(*taxiroute_itr)->GetName(taxiroute_name);

					msg = string("Error: Taxiroute segment " + taxiroute_name) + " crosses runway " + runway_name + " completely";
					
					//Make it selectable. Remember, the data this pointer is refering too was not const to begin with
					//who =  static_cast<WED_Thing*>(const_cast<WED_TaxiRoute*>(*taxiroute_itr));
				}
			}
		}
	}

	return NULL;
}

static WED_Thing* DoTaxiRouteCenterlineCheck( WED_Thing* who,
											  string& msg,
											  const RunwayVec_t& potentially_active_runways,
											  const TaxiRouteVec_t& all_taxiroutes)
{
	for(RunwayVec_t::const_iterator runway_itr = potentially_active_runways.begin();
		runway_itr != potentially_active_runways.end();
		++runway_itr)
	{
		const WED_Runway& runway = **runway_itr;
		string runway_name;
		runway.GetName(runway_name);
		
		TaxiRouteVec_t matching_taxiroutes;
		string taxiroute_name = "";
		for(TaxiRouteVec_t::const_iterator taxiroute_itr = all_taxiroutes.begin(); taxiroute_itr != all_taxiroutes.end(); ++taxiroute_itr)
		{
			taxiroute_name = ENUM_Desc((*taxiroute_itr)->GetRunway());
			if(runway_name == taxiroute_name)
			{
				matching_taxiroutes.push_back(*taxiroute_itr);
			}
		}

		for(TaxiRouteVec_t::const_iterator taxiroute_itr = matching_taxiroutes.begin(); taxiroute_itr != matching_taxiroutes.end(); ++taxiroute_itr)
		{
			//m for meters
			//Point2 runway_bounds_m;
			Segment2 runway_centerline_m;
			Segment2 taxiroute_segment_m;

			//Enter new scope purely for organizational purposes
			{
				Point2 runway_corners_geo[4];
				(*runway_itr)->GetCorners(gis_Geo,runway_corners_geo);
		
				Point2 runway_ends_geo[2];
				runway.GetSource()->GetLocation(gis_Geo,runway_ends_geo[0]);
				runway.GetTarget()->GetLocation(gis_Geo,runway_ends_geo[1]);
				runway_centerline_m = Segment2(translator.Forward(runway_ends_geo[0]), translator.Forward(runway_ends_geo[1]));

				Segment2 taxiroute_segment_geo;
				Bezier2 bez;
				(*taxiroute_itr)->GetSide(gis_Geo, 0, taxiroute_segment_geo, bez);
				taxiroute_segment_m = Segment2(translator.Forward(taxiroute_segment_geo.p1),translator.Forward(taxiroute_segment_geo.p2));
			}

			double METERS_TO_CENTER_THRESHOLD = 2.5;
			double p1_to_center_dist = sqrt(runway_centerline_m.squared_distance_supporting_line(taxiroute_segment_m.p1));
			double p2_to_center_dist = sqrt(runway_centerline_m.squared_distance_supporting_line(taxiroute_segment_m.p2));

			if( p1_to_center_dist > METERS_TO_CENTER_THRESHOLD ||
				p2_to_center_dist > METERS_TO_CENTER_THRESHOLD)
			{
				msg = "Taxi route " + taxiroute_name + " is not on the center line of its runway";
			}
		}
	}

	return NULL;
}

void DoATCRunwayChecks(WED_Thing* who, string& msg)
{
	Bbox2 box;
	dynamic_cast<WED_Airport*>(who)->GetBounds(gis_Geo, box);
	CreateTranslatorForBounds(box,translator);

	RunwayVec_t potentially_active_runways = CollectPotentiallyActiveRunways(who);
	TaxiRouteVec_t all_taxiroutes;
	CollectRecursive<back_insert_iterator<TaxiRouteVec_t>>(who,back_inserter<TaxiRouteVec_t>(all_taxiroutes));
	
	//Pre-check
	//- Does this active runway even have any taxi routes associated with it?
	
	DoRunwayHasMatchingTaxiRoute (   who,msg,potentially_active_runways,all_taxiroutes);
	DoTaxiRouteRunwayTraversalCheck( who,msg,potentially_active_runways,all_taxiroutes);
	DoTaxiRouteCenterlineCheck(      who,msg,potentially_active_runways,all_taxiroutes);
	return;
}
