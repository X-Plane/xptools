/*
 * Copyright (c) 2016, Laminar Research.
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

#ifndef WED_VALIDATEATCRUNWAYCHECKS_H
#define WED_VALIDATEATCRUNWAYCHECKS_H

#include "CompGeomUtils.h"
#include "GISUtils.h"

class WED_Airport;
class WED_ResourceMgr;

#include "AptDefs.h"
#include "WED_Runway.h"
#include "WED_TaxiRoute.h"
#include "WED_GISPoint.h"

#include "WED_EnumSystem.h"

struct validation_error_t;

struct RunwayInfo
{
	RunwayInfo(WED_Runway * runway, CoordTranslator2 translator) : runway_ptr(runway)
	{
		runway->GetName(name);

		AptRunway_t apt_runway;
		runway_ptr->Export(apt_runway);

		runway_numbers[0] = runway_ptr->GetRunwayEnumsOneway().first;
		runway_numbers[1] = runway_ptr->GetRunwayEnumsOneway().second;

		runway_ops[0] = 0;
        runway_ops[1] = 0; // nneds to be set externally, e.g. by evaluation flows

		Point2 bounds[4];
		runway->GetCorners(gis_Geo,bounds);
		for (int i=0; i<4; ++i)
		{
			corners_geo.push_back(bounds[i]);
//			corners_m.push_back(translator.Forward(bounds[i]));
		}
		Point2 ends[2];
		runway->GetSource()->GetLocation(gis_Geo,ends[0]);
		runway->GetTarget()->GetLocation(gis_Geo,ends[1]);

		centerline_geo = Segment2(ends[0], ends[1]);
		centerline_m = Segment2(translator.Forward(ends[0]), translator.Forward(ends[1]));
		dir_1m = Vector2(centerline_m.p1, centerline_m.p2);
		dir_1m.normalize();
		
		dir_vec_1m   = Vector2(bounds[0], bounds[1]) / LonLatDistMeters(bounds[0],bounds[1]);
		width_vec_1m = Vector2(bounds[1], bounds[2]) / LonLatDistMeters(bounds[1],bounds[2]);
	}

	WED_Runway* runway_ptr;  // Pointer to the underlying runway class
	string name;      // Name of this runway

	// [0] is north end, [1] south end information
	int runway_numbers[2];          // enum from ATCRunwayOneway
	int runway_ops[2];              // operations type as per flow_info, 1 = arrival, 2 = departure, 3 = both 
	
	Polygon2 corners_geo;    // corners in lat/lon
//	Polygon2 corners_m;      // corners in meters
	Segment2 centerline_geo; // center line in lat/lon. p1 is source, p2 is target
	Segment2 centerline_m;   // center line in meters.  p1 is source, p2 is target
	
	Vector2 dir_1m;		 	 // vector of 1m length, in runway direction
	
	Vector2 dir_vec_1m;      // vector of 1m length, in runway length direction
	Vector2 width_vec_1m;    // vector of 1m length, in runway width direction

	bool IsHotForArrival(int runway_number) const
	{
		if(runway_number <= atc_18R)
			return runway_ops[0] & 0x1;
		else
			return runway_ops[1] & 0x1;
	}

	bool IsHotForDeparture(int runway_number) const
	{
		if(runway_number <= atc_18R)
			return runway_ops[0] & 0x2;
		else
			return runway_ops[1] & 0x2;
	}
};

struct TaxiRouteInfo
{
	TaxiRouteInfo(WED_TaxiRoute* taxiroute, CoordTranslator2 translator) : ptr(taxiroute)
	{
		AptRouteEdge_t apt_route;
		AptServiceRoadEdge_t dummy;
		taxiroute->Export(apt_route, dummy);
		is_aircraft_route = taxiroute->AllowAircraft();

		if (is_aircraft_route)
		{
			name = apt_route.name;
			hot_arrivals   = set<string>(apt_route.hot_arrive);
			hot_departures = set<string>(apt_route.hot_depart);
		}
		else
		{
			name = dummy.name;
		}
		for (int i = 0; i < taxiroute->CountSources(); i++)
		{
			WED_GISPoint* point = dynamic_cast<WED_GISPoint*>(taxiroute->GetNthSource(i));
			if (point != NULL)
				nodes.push_back(point);
		}
		DebugAssert(nodes.size() >= 2);
		Bezier2 b;
		taxiroute->GetSide(gis_Geo, 0, b);
		segment_geo=b.as_segment(); 
		segment_m = Segment2(translator.Forward(segment_geo.p1),translator.Forward(segment_geo.p2));
	}

	WED_TaxiRoute* ptr;    // Pointer to the original WED_TaxiRoute in WED's data model
	bool is_aircraft_route;
	string name;
	Segment2 segment_geo; 			 // location is lat/lon
	Segment2 segment_m;  			 // location is meters

	set<string> hot_arrivals;
	set<string> hot_departures;

	//Source nodes of the taxiroute. Usually TaxiRouteNodes but sometimes something else
	vector<WED_GISPoint*> nodes;
};

void WED_DoATCRunwayChecks(WED_Airport& apt, validation_error_vector& msgs, /*const TaxiRouteVec_t& all_taxiroutes_plain,*/
	const vector<WED_Runway *>& all_runways, const set<int>& legal_rwy_oneway, const set<int>& legal_rwy_twoway, WED_ResourceMgr * res_mgr);

#endif
