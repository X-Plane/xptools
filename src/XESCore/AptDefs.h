/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef APTDEFS_H
#define APTDEFS_H

#include <vector>
#include "CompGeomDefs2.h"


enum {
	// File format record codes
	apt_airport 		= 1,
	apt_rwy_old 		= 10,			// Legacy runway/taxiway record from 810 and earlier.
	apt_tower_loc 		= 14,
	apt_startup_loc 	= 15,
	apt_seaport 		= 16,
	apt_heliport 		= 17,
	apt_beacon 			= 18,
	apt_windsock 		= 19,
	apt_freq_awos 		= 50,
	apt_freq_ctaf 		= 51,
	apt_freq_del 		= 52,
	apt_freq_gnd 		= 53,
	apt_freq_twr 		= 54,
	apt_freq_app 		= 55,
	apt_freq_dep 		= 56,
	apt_done 			= 99,
	// These records are new with X-Plane 850!
	apt_sign 			= 20,
	apt_papi 			= 21,

	apt_rwy_new 		= 100,			// These replace the old type 10 record.
	apt_sea_new 		= 101,
	apt_heli_new 		= 102,
	apt_taxi_new 		= 110,
	apt_free_chain		= 120,
	apt_boundary 		= 130,

	apt_lin_seg 		= 111,
	apt_lin_crv 		= 112,
	apt_rng_seg 		= 113,
	apt_rng_crv 		= 114,
	apt_end_seg		 	= 115,
	apt_end_crv 		= 116,

	apt_flow_def		= 1000,			// 1000 <traffic flow name, must be unique to the ICAO airport>
	apt_flow_wind		= 1001,			// 1001 <metar icao> <wind dir min> <wind dir max> <wind max speed>
	apt_flow_ceil		= 1002,			// 1002 <metar icao> <ceiling minimum>
	apt_flow_vis		= 1003,			// 1003 <metar icao> <vis minimum>
	apt_flow_time		= 1004,			// 1004 <zulu time start> <zulu time end>
	
	apt_flow_rwy_rule	= 1100,
	apt_flow_pattern	= 1101,
	
	apt_taxi_header		= 1200,			// 1200 <name>
	apt_taxi_node		= 1201,			// 1201 <lat> <lon> <type> <id, 0 based sequence, ascending> <name>
	apt_taxi_edge		= 1202,			// 1202 <src> <dst> <oneway flag> <runway flag/taxi width> <name>
	apt_taxi_shape		= 1203,			// 1203 <lat> <lon>
	apt_taxi_active		= 1204,			// 1204 type|flags runway,list
#if HAS_CURVED_ATC_ROUTE
	apt_taxi_control	= 1205,			// 1205 <lat> <lon
#endif
	apt_taxi_truck_edge = 1206,			// 1206 <src> <dst> <oneway flag> <name>

	apt_startup_loc_new	= 1300,			// 1300 lat lon heading misc|gate|tie_down|hangar traffic name
	apt_startup_loc_extended = 1301,	// 1301 size opertaions_type airline_list
	apt_meta_data = 1302,				// 1302 <key> <value>
	
	apt_truck_parking	= 1400,			// 1400 lat lon heading type cars name
	apt_truck_destination = 1401,		// 1401 lat lon heading type|type|type... name
	
	// Surface codes
	apt_surf_none		= 0,
	apt_surf_asphalt,
	apt_surf_concrete,
	apt_surf_grass,
	apt_surf_dirt,
	apt_surf_gravel,
	apt_surf_asphalt_heli,			// these are 810 only
	apt_surf_concrete_heli,
	apt_surf_grass_heli,
	apt_surf_dirt_heli,
	apt_surf_asphalt_line,
	apt_surf_concrete_line,
	apt_surf_dry_lake,				// all versions
	apt_surf_water,
	apt_surf_ice,					// 850 only
	apt_surf_transparent,

	// Light Fixture Codes (850)
	apt_gls_vasi			= 1,
	apt_gls_papi_left,
	apt_gls_papi_right,
	apt_gls_papi_20,
	apt_gls_vasi_tricolor,
	apt_gls_wigwag,
	// VASI codes (810)
	apt_gls_none_810 = 1,
	apt_gls_vasi_810,
	apt_gls_papi_810,
	apt_gls_papi20_810,

	// Edge Light Codes (850)
	apt_edge_none = 0,
	apt_edge_LIRL,
	apt_edge_MIRL,
	apt_edge_HIRL,
	apt_heli_edge_none = 0,
	apt_heli_edge_yellow,
	// REIL Codes (850)
	apt_reil_none = 0,
	apt_reil_omni,
	apt_reil_uni,
	// Edge Light Codes (810)
	apt_edge_none_810 = 1,
	apt_edge_MIRL_810,
	apt_edge_REIL_810,
	apt_edge_CLL_810,
	apt_edge_TDZL_810,
	apt_edge_taxiway_810,

	// Approach Lights (850)
	apt_app_none = 0,
	apt_app_ALSFI,
	apt_app_ALSFII,
	apt_app_CALVERTI,
	apt_app_CALVERTII,
	apt_app_SSALR,
	apt_app_SSALF,
	apt_app_SALS,
	apt_app_MALSR,
	apt_app_MALSF,
	apt_app_MALS,
	apt_app_ODALS,
	apt_app_RAIL,
	// Approach lights (810)
	apt_app_none_810 = 1,
	apt_app_SSALS_810,
	apt_app_SALSF_810,
	apt_app_ALSFI_810,
	apt_app_ALSFII_810,
	apt_app_ODALS_810,
	apt_app_CALVERTI_810,
	apt_app_CALVERTII_810,

	// Shoulder codes
	apt_shoulder_none = 0,
	apt_shoulder_asphalt,
	apt_shoulder_concrete,

	// Runway markings
	apt_mark_none = 0,
	apt_mark_visual,
	apt_mark_non_precision,
	apt_mark_precision,
	apt_mark_non_precision_UK,	// 850 only
	apt_mark_precision_UK,
	// Helipad Markings
	apt_mark_heli_default = 0,	// 850 only

	// Airport beacons
	apt_beacon_none = 0,
	apt_beacon_airport,
	apt_beacon_seaport,
	apt_beacon_heliport,
	apt_beacon_military,

	// Sign codes
	apt_sign_small = 1,
	apt_sign_medium,
	apt_sign_large,
	apt_sign_large_distance,
	apt_sign_small_distance,

	// Sign Style
	apt_sign_style_default = 0,

	// Linear feature codes
	apt_line_none = 0,
	apt_line_solid_yellow,
	apt_line_broken_yellow,
	apt_line_double_solid_yellow,
	apt_line_runway_hold,
	apt_line_other_hold,
	apt_line_ils_hold,
	apt_line_ils_center,
	apt_line_wide_broken_yellow,
	apt_line_wide_double_broken_yellow,
	apt_line_solid_white = 20,
	apt_line_chequered_white,
	apt_line_broken_white,
	apt_line_Bsolid_yellow = 51,
	apt_line_Bbroken_yellow,
	apt_line_Bdouble_solid_yellow,
	apt_line_Brunway_hold,
	apt_line_Bother_hold,
	apt_line_Bils_hold,
	apt_line_Bils_center,
	apt_line_Bwide_broken_yellow,
	apt_line_Bwide_double_broken_yellow,
	apt_light_taxi_centerline = 101,
	apt_light_taxi_edge,
	apt_light_hold_short,
	apt_light_hold_short_flash,
	apt_light_hold_short_centerline,
	apt_light_bounary,
	
	// ATC Crap
	
	apt_pattern_left = 1,
	apt_pattern_right = 2,
	
	atc_traffic_heavies = 1,
	atc_traffic_jets = 2,
	atc_traffic_turbos = 4,
	atc_traffic_props = 8,
	atc_traffic_helis = 16,
	atc_traffic_fighters = 32,
	
	atc_traffic_all = (atc_traffic_heavies|atc_traffic_jets|atc_traffic_turbos|atc_traffic_props|atc_traffic_helis|atc_traffic_fighters),
	
	atc_op_arrivals = 1,
	atc_op_departures = 2,
	atc_op_all = (atc_op_arrivals | atc_op_departures),
	
	atc_ramp_misc = 0,
	atc_ramp_gate = 1,
	atc_ramp_tie_down = 2,
	atc_ramp_hangar = 3,
	
	atc_width_A = 0,
	atc_width_B = 1,
	atc_width_C = 2,
	atc_width_D = 3,
	atc_width_E = 4,
	atc_width_F = 5,

	ramp_operation_none = 0,
	ramp_operation_general_aviation = 1,
	ramp_operation_airline = 2,
	ramp_operation_cargo = 3,
	ramp_operation_military = 4,
	
	//First entry of the service truck types
	apt_truck_baggage_loader = 0,
	apt_truck_baggage_train,
	apt_truck_crew_car,
	apt_truck_crew_ferrari,
	apt_truck_crew_limo,
	apt_truck_fuel_jet,
	apt_truck_fuel_liner,
	apt_truck_fuel_prop,
	apt_truck_food,
	apt_truck_gpu,
	apt_truck_pushback,

	apt_truck_destination_fuel_farm = 0,
	apt_truck_destination_baggage_hall
	
};

inline bool apt_code_is_curve(int code) { return code == apt_lin_crv || code == apt_rng_crv || code == apt_end_crv; }
inline bool apt_code_is_end(int code) { return code == apt_end_seg || code == apt_end_crv; }
inline bool apt_code_is_ring(int code) { return code == apt_rng_seg || code == apt_rng_crv; }
inline bool apt_code_is_term(int code) { return apt_code_is_end(code) || apt_code_is_ring(code); }


struct	AptRunway_t {
	Segment2	ends;
	float		width_mtr;
	int			surf_code;
	int			shoulder_code;
	float		roughness_ratio;

	int			has_centerline;
	int			edge_light_code;
	int			has_distance_remaining;

	string		id[2];
	float		disp_mtr[2];
	float		blas_mtr[2];
	int			marking_code[2];
	int			app_light_code[2];
	int			has_tdzl[2];
	int			reil_code[2];
};
typedef vector<AptRunway_t>		AptRunwayVector;

struct	AptSealane_t {
	Segment2	ends;
	float		width_mtr;
	int			has_buoys;
	string		id[2];
};
typedef vector<AptSealane_t>	AptSealaneVector;

struct	AptHelipad_t {
	string		id;
	Point2		location;
	float		heading;
	float		length_mtr;
	float		width_mtr;
	int			surface_code;
	int			marking_code;
	int			shoulder_code;
	float		roughness_ratio;
	int			edge_light_code;
};
typedef	vector<AptHelipad_t>	AptHelipadVector;

struct	AptLinearSegment_t {
	int			code;
	Point2		pt;
	Point2		ctrl;
	set<int>	attributes;
};
typedef	vector<AptLinearSegment_t>		AptPolygon_t;

struct	AptTaxiway_t {
	int						surface_code;
	float					roughness_ratio;
	float					heading;
	AptPolygon_t			area;
	string					name;
};
typedef vector<AptTaxiway_t>	AptTaxiwayVector;

struct	AptBoundary_t {
	AptPolygon_t			area;
	string					name;
};
typedef vector<AptBoundary_t>	AptBoundaryVector;

struct AptMarking_t {
	AptPolygon_t			area;
	string					name;
};
typedef vector<AptMarking_t>	AptMarkingVector;

struct	AptPavement_t {
	Segment2	ends;	// Endpoint locations
	float		width_ft;	// Width in feet
	string		name;	// lo or taxiway letter or blank

	int			surf_code;
	int			shoulder_code;
	int			marking_code;
	float		roughness_ratio;
	int			distance_markings;

	int			blast1_ft;	// Length of blast-pads in feet.
	int			disp1_ft;
	int			vap_lites_code1;
	int			edge_lites_code1;
	int			app_lites_code1;
	int			vasi_angle1;	//  x100

	int			blast2_ft;
	int			disp2_ft;
	int			vap_lites_code2;
	int			edge_lites_code2;
	int			app_lites_code2;
	int			vasi_angle2;	// x100
};
typedef vector<AptPavement_t>	AptPavementVector;

struct	AptGate_t {
	Point2		location;
	float		heading;
	int			type;
	int			equipment;
	int			width;			// icao width code
	string		name;
	int			ramp_op_type;     // ramp operations type
	string		airlines;
};
typedef vector<AptGate_t>		AptGateVector;

struct	AptTowerPt_t {
	Point2		location;
	float		height_ft;
	int			draw_obj;		// not used in 850
	string		name;
};

struct	AptBeacon_t {
	Point2		location;
	int			color_code;
	string		name;
};

struct AptWindsock_t {
	Point2		location;
	int			lit;
	string		name;
};
typedef vector<AptWindsock_t>	AptWindsockVector;

struct	AptLight_t {
	Point2		location;
	int			light_code;
	float		heading;
	float		angle;
	string		name;
};
typedef vector<AptLight_t>		AptLightVector;

struct	AptSign_t {
	Point2		location;
	float		heading;
	int			style_code;
	int			size_code;
	string		text;
};
typedef vector<AptSign_t>		AptSignVector;

struct	AptATCFreq_t {
	int			freq;
	int			atc_type;
	string		name;
};
typedef vector<AptATCFreq_t>	AptATCFreqVector;

/************************************************************************************************************************
 * ATC INFO
 ************************************************************************************************************************/
struct AptRunwayRule_t {
	string			name;
	string			runway;
	int				operations;
	int				equipment;
	int				dep_freq;
	int				dep_heading_lo;		// lo == hi if "any" is okay.
	int				dep_heading_hi;		// This filters the use of the runway by where we are going, to keep traffic from crossing in-air.
	int				ini_heading_lo;		// This is the range of initial headings the tower can issue.
	int				ini_heading_hi;		
};	
typedef vector<AptRunwayRule_t>	AptRunwayRuleVector;

struct AptWindRule_t {
	string			icao;
	int				dir_lo_degs_mag;
	int				dir_hi_degs_mag;
	int				max_speed_knots;
};
typedef vector<AptWindRule_t>	AptWindRuleVector;

struct AptTimeRule_t {
	int				start_zulu;
	int				end_zulu;
};
typedef vector<AptTimeRule_t>	AptTimeRuleVector;

struct AptFlow_t {
	string						name;

	string						icao;
	int							ceiling_ft;
	float						visibility_sm;
	AptTimeRuleVector			time_rules;
	AptWindRuleVector			wind_rules;	
	int							pattern_side;
	string						pattern_runway;
	AptRunwayRuleVector			runway_rules;
};
typedef vector<AptFlow_t>		AptFlowVector;

struct AptRouteNode_t {
	string						name;
	int							id;
	Point2						location;
};

struct AptEdgeBase_t {
	int							src;
	int							dst;
	int							oneway;
	vector<pair<Point2, bool> >	shape;			// This is pairs of shape points and curved flags - true means curve control point
												// The end points are NOT included in shape.  It is a requirement that no more
												// than 2 adjacent curve control points exist without a regular point.  There is no min/max size requirement for shape.
};

struct AptRouteEdge_t : AptEdgeBase_t {
	string						name;
	int							runway;
	int							width;	// icao width code
	set<string>					hot_depart;
	set<string>					hot_arrive;
	set<string>					hot_ils;
	
};

struct AptServiceRoadEdge_t : AptEdgeBase_t {
	string						name;
};

struct AptNetwork_t {
	string						name;
	vector<AptRouteNode_t>		nodes;
	vector<AptRouteEdge_t>		edges;
	vector<AptServiceRoadEdge_t>service_roads;
};

struct AptTruckParking_t {
	string						name;
	Point2						location;
	float						heading;
	int							parking_type;
	int							train_car_count;
};
typedef vector<AptTruckParking_t> AptTruckParkingVector;

struct AptTruckDestination_t {
	string						name;
	Point2						location;
	float						heading;
	set<int>					truck_types;
};
typedef vector<AptTruckDestination_t> AptTruckDestinationVector;

struct AptInfo_t {
	int					kind_code;				// Enum
	string				icao;
	string				name;
	int					elevation_ft;
	int					has_atc_twr;
	int					default_buildings;		// not used in 850
	std::vector<std::pair<string,string> > meta_data; //Contains meta data for real and synthetic properties

	AptRunwayVector		runways;				// 850 structures
	AptSealaneVector	sealanes;
	AptHelipadVector	helipads;
	AptTaxiwayVector	taxiways;
	AptBoundaryVector	boundaries;
	AptMarkingVector	lines;
	AptLightVector		lights;
	AptSignVector		signs;

	AptPavementVector	pavements;				// 810 structures
	AptGateVector		gates;					// shared structures
	
	AptTruckParkingVector		truck_parking;
	AptTruckDestinationVector	truck_destinations;
	
	AptTowerPt_t		tower;
	AptBeacon_t			beacon;
	AptWindsockVector	windsocks;
	AptATCFreqVector	atc;

	AptFlowVector		flows;
	AptNetwork_t		taxi_route;

	Bbox2				bounds;

#if OPENGL_MAP
	struct AptLineLoop_t {
		float			rgb[3];
		Polygon2		pts;
	};
	vector<AptLineLoop_t>	ogl;
#endif


};

typedef vector<AptInfo_t>	AptVector;

typedef hash_multimap<int,int>	AptIndex;

#endif
