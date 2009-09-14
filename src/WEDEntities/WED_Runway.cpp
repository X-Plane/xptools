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

#include "WED_Runway.h"
#include "WED_EnumSystem.h"
#include "GISUtils.h"
#include "XESConstants.h"
#include "AptDefs.h"

DEFINE_PERSISTENT(WED_Runway)
TRIVIAL_COPY(WED_Runway, WED_GISLine_Width)

WED_Runway::WED_Runway(WED_Archive * a, int i) : WED_GISLine_Width(a,i),
	surface			(this,"Surface",					"WED_runway",	"surface",				Surface_Type,	surf_Concrete),
	shoulder		(this,"Shoulder",					"WED_runway",	"shoulder",				Shoulder_Type,	shoulder_None),
	roughness		(this,"Roughness",					"WED_runway",	"roughness",			0.25,4,2),
	center_lites	(this,"Centerline Lights",			"WED_runway",	"center_lites",			1),
	edge_lites		(this,"Edge Lights",				"WED_runway",	"edge_lites",			Edge_Lights,	edge_MIRL),
	remaining_signs	(this,"Distance Signs",				"WED_runway",	"distance_signs",		1),

//	id1				(this,"Identifier 1",				"WED_runway",	"id1",					"04"),
	disp1			(this,"Displaced Threshhold 1",		"WED_runway",	"displaced1",			0,6,1),
	blas1			(this,"Blastpad 1",					"WED_runway",	"blastpad1",			0,6,1),
	mark1			(this,"Markings 1",					"WED_runway",	"markings1",			Runway_Markings,	mark_NonPrecis),
	appl1			(this,"Approach Lights 1",			"WED_runway",	"app_lites1",			Light_App,			app_MALSF),
	tdzl1			(this,"TDZ Lights 1",				"WED_runway",	"TDZL1",				1),
	reil1			(this,"REIL 1",						"WED_runway",	"REIL1",				REIL_Lights,		reil_None),

//	id2				(this,"Identifier 2",				"WED_runway",	"id2",					"22"),
	disp2			(this,"Displaced Threshhold 2",		"WED_runway",	"displaced2",			0,6,1),
	blas2			(this,"Blastpad 2",					"WED_runway",	"blastpad2",			0,6,1),
	mark2			(this,"Markings 2",					"WED_runway",	"markings2",			Runway_Markings,	mark_NonPrecis),
	appl2			(this,"Approach Lights 2",			"WED_runway",	"app_lites2",			Light_App,			app_MALSF),
	tdzl2			(this,"TDZ Lights 2",				"WED_runway",	"TDZL2",				1),
	reil2			(this,"REIL 2",						"WED_runway",	"REIL2",				REIL_Lights,		reil_None)
{
}

WED_Runway::~WED_Runway()
{
}


bool		WED_Runway::GetCornersBlas1(Point2 corners[4]) const
{
	GetCorners(gis_Geo,corners);
	corners[1] = corners[0];
	corners[2] = corners[3];
	if (blas1.value == 0.0) return false;
	Point2	bounds[4];
	GetCorners(gis_Geo,bounds);

	Point2	p1, p2;
	GetSource()->GetLocation(gis_Geo,p1);
	GetTarget()->GetLocation(gis_Geo,p2);

	double my_len = LonLatDistMeters(p1.x(),p1.y(),p2.x(),p2.y());
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
	GetCorners(gis_Geo,corners);
	corners[0] = corners[1];
	corners[3] = corners[2];
	if (blas2.value == 0.0) return false;
	Point2	bounds[4];
	GetCorners(gis_Geo,bounds);

	Point2	p1, p2;
	GetSource()->GetLocation(gis_Geo,p1);
	GetTarget()->GetLocation(gis_Geo,p2);

	double my_len = LonLatDistMeters(p1.x(),p1.y(),p2.x(),p2.y());
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
	GetCorners(gis_Geo,corners);
	corners[1] = corners[0];
	corners[2] = corners[3];
	if (disp1.value == 0.0) return false;
	Point2	bounds[4];
	GetCorners(gis_Geo,bounds);

	Point2	p1, p2;
	GetSource()->GetLocation(gis_Geo,p1);
	GetTarget()->GetLocation(gis_Geo,p2);

	double my_len = LonLatDistMeters(p1.x(),p1.y(),p2.x(),p2.y());
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
	GetCorners(gis_Geo,corners);
	corners[0] = corners[1];
	corners[3] = corners[2];
	if (disp2.value == 0.0) return false;
	Point2	bounds[4];
	GetCorners(gis_Geo,bounds);

	Point2	p1, p2;
	GetSource()->GetLocation(gis_Geo,p1);
	GetTarget()->GetLocation(gis_Geo,p2);

	double my_len = LonLatDistMeters(p1.x(),p1.y(),p2.x(),p2.y());
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
	GetCorners(gis_Geo,corners);
	GetCorners(gis_Geo,corners+4);
	if (shoulder.value == shoulder_None) return false;
	Point2	bounds[4];
	GetCorners(gis_Geo,bounds);

	Point2	p1, p2;
	GetSource()->GetLocation(gis_Geo,p1);
	GetTarget()->GetLocation(gis_Geo,p2);

	double my_len = LonLatDistMeters(p1.x(),p1.y(),p2.x(),p2.y());
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
	if (n < 0.0) n = 0.0;
	double m = GetLength() - GetDisp2();
	if (n > m) n = m;

	disp1 = n;
}

void		WED_Runway::SetDisp2(double n)
{
	if (n < 0.0) n = 0.0;
	double m = GetLength() - GetDisp1();
	if (n > m) n = m;

	disp2 = n;
}

void		WED_Runway::SetBlas1(double n)
{
	if (n < 0.0) n = 0.0;
	blas1 = n;
}

void		WED_Runway::SetBlas2(double n)
{
	if (n < 0.0) n = 0.0;
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


void		WED_Runway::Import(const AptRunway_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	GetSource()->SetLocation(gis_Geo,x.ends.p1  );
	GetTarget()->SetLocation(gis_Geo,x.ends.p2  );
				 SetWidth	(x.width_mtr);

	surface			= ENUM_Import(Surface_Type,		x.surf_code				);
	shoulder		= ENUM_Import(Shoulder_Type,	x.shoulder_code			);
	roughness		=								x.roughness_ratio		 ;
	center_lites	=								x.has_centerline		 ;
	edge_lites		= ENUM_Import(Edge_Lights,		x.edge_light_code		);
	remaining_signs =								x.has_distance_remaining ;

	if (surface == -1)
	{
		print_func(ref,"Error importing runway: surface code %d is illegal (not a member of type %s).\n", x.surf_code, DOMAIN_Fetch(surface.domain));
		surface = surf_Concrete;
	}
	if (shoulder == -1)
	{
		print_func(ref,"Error importing runway: shoulder code %d is illegal (not a member of type %s).\n", x.shoulder_code, DOMAIN_Fetch(shoulder.domain));
		shoulder = shoulder_None;
	}
	if (edge_lites == -1)
	{
		print_func(ref,"Error importing runway: edge light code %d is illegal (not a member of type %s).\n", x.edge_light_code, DOMAIN_Fetch(edge_lites.domain));
		edge_lites = edge_MIRL;
	}

	string	full = x.id[0] + string("/") + x.id[1];
	SetName(full);

	disp1 =									x.disp_mtr		[0] ;
	blas1 =									x.blas_mtr		[0] ;
	mark1 = ENUM_Import(Runway_Markings,	x.marking_code	[0]);
	appl1 = ENUM_Import(Light_App,			x.app_light_code[0]);
	tdzl1 =									x.has_tdzl		[0] ;
	reil1 = ENUM_Import(REIL_Lights,		x.reil_code		[0]);
	if (mark1 == -1)
	{
		print_func(ref,"Error importing runway: low-end marking code %d is illegal (not a member of type %s).\n", x.marking_code[0], DOMAIN_Fetch(mark1.domain));
		mark1 = mark_NonPrecis;
	}
	if (appl1 == -1)
	{
		print_func(ref,"Error importing runway: low-end approach lights code %d is illegal (not a member of type %s).\n", x.app_light_code[0], DOMAIN_Fetch(appl1.domain));
		appl1 = app_None;
	}
	if (reil1 == -1)
	{
		print_func(ref,"Error importing runway: low-end reil code %d is illegal (not a member of type %s).\n", x.reil_code[0], DOMAIN_Fetch(reil1.domain));
		reil1 = reil_None;
	}

	disp2 =									x.disp_mtr		[1] ;
	blas2 =									x.blas_mtr		[1] ;
	mark2 = ENUM_Import(Runway_Markings,	x.marking_code	[1]);
	appl2 = ENUM_Import(Light_App,			x.app_light_code[1]);
	tdzl2 =									x.has_tdzl		[1] ;
	reil2 = ENUM_Import(REIL_Lights,		x.reil_code		[1]);

	if (mark2 == -1)
	{
		print_func(ref,"Error importing runway: high-end marking code %d is illegal (not a member of type %s).\n", x.marking_code[1], DOMAIN_Fetch(mark2.domain));
		mark2 = mark_NonPrecis;
	}
	if (appl2 == -1)
	{
		print_func(ref,"Error importing runway: high-end approach lights code %d is illegal (not a member of type %s).\n", x.app_light_code[1], DOMAIN_Fetch(appl2.domain));
		appl2 = app_None;
	}
	if (reil2 == -1)
	{
		print_func(ref,"Error importing runway: high-end reil code %d is illegal (not a member of type %s).\n", x.reil_code[1], DOMAIN_Fetch(reil2.domain));
		reil2 = reil_None;
	}



}

void		WED_Runway::Export(		 AptRunway_t& x) const
{
	GetSource()->GetLocation(gis_Geo,x.ends.p1  );
	GetTarget()->GetLocation(gis_Geo,x.ends.p2  );
							 x.width_mtr = GetWidth();

	x.surf_code				 = ENUM_Export(surface.value   );
	x.shoulder_code			 = ENUM_Export(shoulder.value  );
	x.roughness_ratio		 =			   roughness		;
	x.has_centerline		 =			   center_lites		;
	x.edge_light_code		 = ENUM_Export(edge_lites.value);
	x.has_distance_remaining =			   remaining_signs	;

	string	full;
	GetName(full);
	string::size_type p = full.find('/');
	if (p == full.npos)
	{
		x.id[0] = full;
		x.id[1] = "xxx";
	}
	else
	{
		x.id[0] = full.substr(0,p);
		x.id[1] = full.substr(p+1);
	}

	x.disp_mtr		[0] =			  disp1		  ;
	x.blas_mtr		[0] =			  blas1		  ;
	x.marking_code	[0] = ENUM_Export(mark1.value);
	x.app_light_code[0] = ENUM_Export(appl1.value);
	x.has_tdzl		[0] =			  tdzl1		  ;
	x.reil_code		[0] = ENUM_Export(reil1.value);

	x.disp_mtr		[1] =			  disp2		  ;
	x.blas_mtr		[1] =			  blas2		  ;
	x.marking_code	[1] = ENUM_Export(mark2.value);
	x.app_light_code[1] = ENUM_Export(appl2.value);
	x.has_tdzl		[1] =			  tdzl2		  ;
	x.reil_code		[1] = ENUM_Export(reil2.value);

}

