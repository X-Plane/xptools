#ifndef APTDEFS_H
#define APTDEFS_H

#include <vector>
#include <hash_map>
#include "CompGeomDefs2.h"

struct	AptPavement_t {
	Segment2	ends;	// Endpoint locations

	int			blast1_ft;	// Length of blast-pads in feet.
	int			blast2_ft;
	int			disp1_ft;
	int			disp2_ft;
	
	double		width_ft;	// Width in feet
	string		name;	// lo or taxiway letter or blank
	
	int			vap_lites_code1;
	int			edge_lites_code1;
	int			app_lites_code1;
	int			vap_lites_code2;
	int			edge_lites_code2;
	int			app_lites_code2;
	
	int			surf_code;
	int			shoulder_code;
	int			marking_code;
	float		roughness_ratio;
	int			distance_markings;

#if OPENGL_MAP	
	vector<float>	quad_coords;
	vector<float>	quad_colors;
#endif	
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
	int			draw_obj;		// -1 if no tower specified, or bool
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
	int					default_buildings;


	AptPavementVector	pavements;
	AptGateVector		gates;
	AptTowerPt_t		tower;
	AptBeacon_t			beacon;
	AptWindsockVector	windsocks;
	AptATCFreqVector	atc;
	
	Bbox2				bounds;
};

typedef vector<AptInfo_t>	AptVector;

typedef hash_multimap<int,int>	AptIndex;

#endif
