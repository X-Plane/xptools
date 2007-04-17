#include "WED_Runway.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_Runway)
START_CASTING(WED_Runway)
INHERITS_FROM(WED_GISLine_Width)
END_CASTING

WED_Runway::WED_Runway(WED_Archive * a, int i) : WED_GISLine_Width(a,i),
	surface			(this,"Surface",					"WED_runway",	"surface",				Surface_Type,	surf_Concrete),
	shoulder		(this,"Shoulder",					"WED_runway",	"shoulder",				Shoulder_Type,	shoulder_None),
	roughness		(this,"Roughness",					"WED_runway",	"roughness",			0.25),
	center_lites	(this,"Centerline Lights",			"WED_runway",	"center_lites",			1),
	edge_lites		(this,"Edge Lights",				"WED_runway",	"edge_lites",			Edge_Lights,	edge_MIRL),
	remaining_signs	(this,"Distance Signs",				"WED_runway",	"distance_signs",		1),

	id1				(this,"Identifier 1",				"WED_runway",	"id1",					"04"),
	disp1			(this,"Displaced Threshhold 1",		"WED_runway",	"displaced1",			0),
	blas1			(this,"Blastpad 1",					"WED_runway",	"blastpad1",			0),
	mark1			(this,"Markings 1",					"WED_runway",	"markings1",			Runway_Markings,	mark_NonPrecis),
	appl1			(this,"Approach Lights 1",			"WED_runway",	"app_lites1",			Light_App,			app_MALSF),
	tdzl1			(this,"TDZ Lights 1",				"WED_runway",	"TDZL1",				1),
	reil1			(this,"REIL 1",						"WED_runway",	"REIL1",				REIL_Lights,		reil_None),

	id2				(this,"Identifier 2",				"WED_runway",	"id2",					"22"),
	disp2			(this,"Displaced Threshhold 2",		"WED_runway",	"displaced2",			0),
	blas2			(this,"Blastpad 2",					"WED_runway",	"blastpad2",			0),
	mark2			(this,"Markings 2",					"WED_runway",	"markings2",			Runway_Markings,	mark_NonPrecis),
	appl2			(this,"Approach Lights 2",			"WED_runway",	"app_lites2",			Light_App,			app_MALSF),
	tdzl2			(this,"TDZ Lights 2",				"WED_runway",	"TDZL2",				1),
	reil2			(this,"REIL 2",						"WED_runway",	"REIL2",				REIL_Lights,		reil_None)
{
}

WED_Runway::~WED_Runway()
{
}