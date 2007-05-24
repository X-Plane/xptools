#include "WED_Runway.h"
#include "WED_EnumSystem.h"
#include "GISUtils.h"

DEFINE_PERSISTENT(WED_Runway)

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

bool			WED_Runway::PtWithin		(const Point2& p	 ) const
{
	if (WED_GISLine_Width::PtWithin(p)) return true;
	
	Point2	corners[8];
	
	if (GetCornersBlas1(corners) && inside_polygon_pt(corners,corners+4,p)) return true;
	if (GetCornersBlas2(corners) && inside_polygon_pt(corners,corners+4,p)) return true;
	if (GetCornersShoulders(corners) && 
		(inside_polygon_pt(corners,corners+4,p) || inside_polygon_pt(corners+4,corners+8,p))) return true;
		
	return false;
}

bool		WED_Runway::GetCornersBlas1(Point2 corners[4]) const
{
	GetCorners(corners);
	corners[1] = corners[0];
	corners[2] = corners[3];
	if (blas1.value == 0.0) return false;
	Point2	bounds[4];
	GetCorners(bounds);
	
	Point2	p1, p2;
	GetSource()->GetLocation(p1);
	GetTarget()->GetLocation(p2);

	double my_len = LonLatDistMeters(p1.x,p1.y,p2.x,p2.y);
	if (my_len == 0.0) return false;
	
	double frac = blas1.value / my_len;
	
	corners[0] = Segment2(bounds[0],bounds[1]).midpoint(-frac);
	corners[1] = bounds[0];
	corners[2] = bounds[3];
	corners[3] = Segment2(bounds[3],bounds[2]).midpoint(-frac);

	return true;
}

bool		WED_Runway::GetCornersBlas2(Point2 corners[4]) const
{
	GetCorners(corners);
	corners[0] = corners[1];
	corners[3] = corners[2];
	if (blas2.value == 0.0) return false;
	Point2	bounds[4];
	GetCorners(bounds);
	
	Point2	p1, p2;
	GetSource()->GetLocation(p1);
	GetTarget()->GetLocation(p2);

	double my_len = LonLatDistMeters(p1.x,p1.y,p2.x,p2.y);
	if (my_len == 0.0) return false;
	
	double frac = blas2.value / my_len;
	
	corners[0] = bounds[1];
	corners[1] = Segment2(bounds[0],bounds[1]).midpoint(1.0 + frac);
	corners[2] = Segment2(bounds[3],bounds[2]).midpoint(1.0 + frac);
	corners[3] = bounds[2];

	return true;
}

bool		WED_Runway::GetCornersDisp1(Point2 corners[4]) const
{
	GetCorners(corners);
	corners[1] = corners[0];
	corners[2] = corners[3];
	if (disp1.value == 0.0) return false;
	Point2	bounds[4];
	GetCorners(bounds);
	
	Point2	p1, p2;
	GetSource()->GetLocation(p1);
	GetTarget()->GetLocation(p2);

	double my_len = LonLatDistMeters(p1.x,p1.y,p2.x,p2.y);
	if (my_len == 0.0) return false;
	
	double frac = disp1.value / my_len;
	
	corners[0] = bounds[0];
	corners[1] = Segment2(bounds[0],bounds[1]).midpoint(frac);
	corners[2] = Segment2(bounds[3],bounds[2]).midpoint(frac);
	corners[3] = bounds[3];

	return true;
}

bool		WED_Runway::GetCornersDisp2(Point2 corners[4]) const
{
	GetCorners(corners);
	corners[0] = corners[1];
	corners[3] = corners[2];
	if (disp2.value == 0.0) return false;
	Point2	bounds[4];
	GetCorners(bounds);
	
	Point2	p1, p2;
	GetSource()->GetLocation(p1);
	GetTarget()->GetLocation(p2);

	double my_len = LonLatDistMeters(p1.x,p1.y,p2.x,p2.y);
	if (my_len == 0.0) return false;
	
	double frac = disp2.value / my_len;
	
	corners[0] = Segment2(bounds[0],bounds[1]).midpoint(1.0-frac);
	corners[1] = bounds[1];
	corners[2] = bounds[2];
	corners[3] = Segment2(bounds[3],bounds[2]).midpoint(1.0-frac);

	return true;
}

bool		WED_Runway::GetCornersShoulders(Point2 corners[8]) const
{
	GetCorners(corners);
	GetCorners(corners+4);
	if (shoulder.value == shoulder_None) return false;
	Point2	bounds[4];
	GetCorners(bounds);
	
	Point2	p1, p2;
	GetSource()->GetLocation(p1);
	GetTarget()->GetLocation(p2);

	double my_len = LonLatDistMeters(p1.x,p1.y,p2.x,p2.y);
	if (my_len == 0.0) return false;

	if (blas1.value != 0.0)
	{
		double frac = blas1.value / my_len;	
		corners[0] = Segment2(bounds[0],bounds[1]).midpoint(-frac);
		corners[3] = Segment2(bounds[3],bounds[2]).midpoint(-frac);
	}
	if (blas2.value != 0.0)
	{
		double frac = blas2.value / my_len;	
		corners[1] = Segment2(bounds[0],bounds[1]).midpoint(1.0 + frac);
		corners[2] = Segment2(bounds[3],bounds[2]).midpoint(1.0 + frac);
	}
	
	bounds[0] = corners[0];
	bounds[1] = corners[1];
	bounds[2] = corners[2];
	bounds[3] = corners[3];
	
	corners[0] = Segment2(bounds[0],bounds[3]).midpoint(-0.25);
	corners[1] = Segment2(bounds[1],bounds[2]).midpoint(-0.25);
	corners[2] = bounds[1];
	corners[3] = bounds[0];
	corners[4] = bounds[3];
	corners[5] = bounds[2];
	corners[6] = Segment2(bounds[1],bounds[2]).midpoint( 1.25);
	corners[7] = Segment2(bounds[0],bounds[3]).midpoint( 1.25);
	return true;
}

int			WED_Runway::GetSurface(void) const
{
	return surface.value;
}

int			WED_Runway::GetShoulder(void) const
{
	return shoulder.value;
}

void		WED_Runway::SetDisp1(double n)
{
		disp1 = n;
}

void		WED_Runway::SetDisp2(double n)
{
		disp2 = n;
}

void		WED_Runway::SetBlas1(double n)
{
		blas1 = n;
}

void		WED_Runway::SetBlas2(double n)
{
		blas2 = n;
}

double		WED_Runway::GetDisp1(void) const { return disp1.value; }
double		WED_Runway::GetDisp2(void) const { return disp2.value; }
double		WED_Runway::GetBlas1(void) const { return blas1.value; }
double		WED_Runway::GetBlas2(void) const { return blas2.value; }


	void		WED_Runway::SetSurface(int x) { surface = x; }
	void		WED_Runway::SetShoulder(int x) { shoulder = x; }
	void		WED_Runway::SetRoughness(double x) { roughness = x; }
	void		WED_Runway::SetCenterLights(int x) { center_lites = x; }
	void		WED_Runway::SetEdgeLights(int x) { edge_lites = x; }
	void		WED_Runway::SetRemainingSigns(int x) { remaining_signs = x; }
	void		WED_Runway::SetMarkings1(int x) { mark1 = x; }
	void		WED_Runway::SetAppLights1(int x) { appl1 = x; }
	void		WED_Runway::SetTDZL1(int x) { tdzl1 = x; }
	void		WED_Runway::SetREIL1(int x) { reil1 = x; }
	void		WED_Runway::SetMarkings2(int x) { mark2 = x; }
	void		WED_Runway::SetAppLights2(int x) { appl2 = x; }
	void		WED_Runway::SetTDZL2(int x) { tdzl2 = x; }
	void		WED_Runway::SetREIL2(int x) { reil2 = x; }
