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
	apt_light_bounary

};



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
	string		name;
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


struct AptInfo_t {
	int					kind_code;				// Enum
	string				icao;
	string				name;
	int					elevation_ft;
	int					has_atc_twr;
	int					default_buildings;		// not used in 850

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
	AptTowerPt_t		tower;
	AptBeacon_t			beacon;
	AptWindsockVector	windsocks;
	AptATCFreqVector	atc;

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
