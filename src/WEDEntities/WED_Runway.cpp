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
#include "WED_Airport.h"
#include "WED_HierarchyUtils.h"
#include "WED_TaxiRoute.h"
#include "WED_AirportSign.h"
#include "WED_ATCFlow.h"
#include "WED_ATCRunwayUse.h"
#include "WED_ToolUtils.h"
#include "GISUtils.h"
#include "XESConstants.h"
#include "AptDefs.h"
#include "MathUtils.h"
#include "STLUtils.h"
#include "PlatformUtils.h"

#include <sstream>


DEFINE_PERSISTENT(WED_Runway)
TRIVIAL_COPY(WED_Runway, WED_GISLine_Width)

WED_Runway::WED_Runway(WED_Archive * a, int i) : WED_GISLine_Width(a,i),
	surface			(this,PROP_Name("Surface",				XML_Name("runway","surface")),		Surface_Type,	surf_Concrete),
	shoulder		(this,PROP_Name("Shoulder",				XML_Name("runway","shoulder")),		Shoulder_Type,	shoulder_None),
	roughness		(this,PROP_Name("Roughness",			XML_Name("runway","roughness")),	0.25,4,2),
	center_lites	(this,PROP_Name("Centerline Lights",	XML_Name("runway","center_lites")),	1),
	edge_lites		(this,PROP_Name("Edge Lights",			XML_Name("runway","edge_lites")),	Edge_Lights,	edge_MIRL),
	remaining_signs	(this,PROP_Name("Distance Signs",		XML_Name("runway","distance_signs")),1),

	disp1			(this,PROP_Name("Displaced Threshold 1",XML_Name("runway","displaced1")),	0,6,1),
	blas1			(this,PROP_Name("Blastpad 1",			XML_Name("runway","blastpad1")),	0,6,1),
	mark1			(this,PROP_Name("Markings 1",			XML_Name("runway","markings1")),	Runway_Markings,mark_NonPrecis),
	appl1			(this,PROP_Name("Approach Lights 1",	XML_Name("runway","app_lites1")),	Light_App,		app_MALSF),
	tdzl1			(this,PROP_Name("TDZ Lights 1",			XML_Name("runway","TDZL1")),		1),
	reil1			(this,PROP_Name("REIL strobes 1",		XML_Name("runway","REIL1")),		REIL_Lights,	reil_None),

	disp2			(this,PROP_Name("Displaced Threshold 2",XML_Name("runway","displaced2")),	0,6,1),
	blas2			(this,PROP_Name("Blastpad 2",			XML_Name("runway","blastpad2")),	0,6,1),
	mark2			(this,PROP_Name("Markings 2",			XML_Name("runway","markings2")),	Runway_Markings,mark_NonPrecis),
	appl2			(this,PROP_Name("Approach Lights 2",	XML_Name("runway","app_lites2")),	Light_App,		app_MALSF),
	tdzl2			(this,PROP_Name("TDZ Lights 2",			XML_Name("runway","TDZL2")),		1),
	reil2			(this,PROP_Name("REIL strobes 2",		XML_Name("runway","REIL2")),		REIL_Lights,	reil_None)
{
}

WED_Runway::~WED_Runway()
{
}

bool	WED_Runway::Cull(const Bbox2& b) const
{
	Bbox2	me;
	GetBounds(gis_Geo,me);
	Point2	c[8];
	int n;
	if(GetCornersBlas1(c))
	for(n=0;n<4;++n)
		me+=c[n];
	if(GetCornersBlas2(c))
	for(n=0;n<4;++n)
		me+=c[n];
	if(GetCornersShoulders(c))	
	for(n=0;n<8;++n)
		me+=c[n];
	return b.overlap(me);	
}

pair<int,int>	WED_Runway::GetRunwayEnumsOneway() const
{
	string name;
	GetName(name);
	
	vector<string> parts;
	tokenize_string(name.begin(),name.end(),back_inserter(parts), '/');
	
	if(parts.size() != 1 && parts.size() != 2)
		return pair<int,int>(atc_Runway_None,atc_Runway_None);
	
	int e1 = ENUM_LookupDesc(ATCRunwayOneway,parts[0].c_str());
	if(e1 == -1)
	{
		parts[0].insert(0,"0");
		e1 = ENUM_LookupDesc(ATCRunwayOneway,parts[0].c_str());
		if(e1 == -1)
			return pair<int,int>(atc_Runway_None,atc_Runway_None);
	}
	
	int e2 = atc_Runway_None;
	if(parts.size() == 2)
	{
		e2 = ENUM_LookupDesc(ATCRunwayOneway,parts[1].c_str());
		if(e2 == -1)
		{
			parts[1].insert(0,"0");
			e2 = ENUM_LookupDesc(ATCRunwayOneway,parts[1].c_str());
			if(e2 == -1)
				e2 = atc_Runway_None;
		}
	}
	
	return make_pair(e1, e2);
}
	
int				WED_Runway::GetRunwayEnumsTwoway() const
{
	string name;
	GetName(name);
	int e1 = ENUM_LookupDesc(ATCRunwayTwoway,name.c_str());
	if(e1 != -1)
		return e1;

	string namez(name);
	namez.insert(0,"0");
	e1 = ENUM_LookupDesc(ATCRunwayTwoway,namez.c_str());
	if(e1 != -1)
		return e1;
	
	name += "/XXX";
	namez += "/XXX";
	
	e1 = ENUM_LookupDesc(ATCRunwayTwoway,name.c_str());
	if(e1 != -1)
		return e1;

	e1 = ENUM_LookupDesc(ATCRunwayTwoway,namez.c_str());
	if(e1 != -1)
		return e1;
	return atc_rwy_None;
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

	double my_len = LonLatDistMeters(p1,p2);
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

	double my_len = LonLatDistMeters(p1,p2);
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

	double my_len = LonLatDistMeters(p1,p2);
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

	double my_len = LonLatDistMeters(p1,p2);
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

	double my_len = LonLatDistMeters(p1,p2);
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
double		WED_Runway::GetRoughness(void) const { return roughness.value; }


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
	roughness		= fltlim(						x.roughness_ratio,0.0f,1.0f);
	center_lites	=								x.has_centerline		 ;
	edge_lites		= ENUM_Import(Edge_Lights,		x.edge_light_code		);
	remaining_signs =								x.has_distance_remaining ;

	if (surface == -1)
	{
		print_func(ref,"Error importing runway: surface code %d is illegal (not a member of type %s).\n", x.surf_code, DOMAIN_Desc(surface.domain));
		surface = surf_Concrete;
	}
	if (shoulder == -1)
	{
		print_func(ref,"Error importing runway: shoulder code %d is illegal (not a member of type %s).\n", x.shoulder_code, DOMAIN_Desc(shoulder.domain));
		shoulder = shoulder_None;
	}
	if (edge_lites == -1)
	{
		print_func(ref,"Error importing runway: edge light code %d is illegal (not a member of type %s).\n", x.edge_light_code, DOMAIN_Desc(edge_lites.domain));
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
		print_func(ref,"Error importing runway: low-end marking code %d is illegal (not a member of type %s).\n", x.marking_code[0], DOMAIN_Desc(mark1.domain));
		mark1 = mark_NonPrecis;
	}
	if (appl1 == -1)
	{
		print_func(ref,"Error importing runway: low-end approach lights code %d is illegal (not a member of type %s).\n", x.app_light_code[0], DOMAIN_Desc(appl1.domain));
		appl1 = app_None;
	}
	if (reil1 == -1)
	{
		print_func(ref,"Error importing runway: low-end reil code %d is illegal (not a member of type %s).\n", x.reil_code[0], DOMAIN_Desc(reil1.domain));
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
		print_func(ref,"Error importing runway: high-end marking code %d is illegal (not a member of type %s).\n", x.marking_code[1], DOMAIN_Desc(mark2.domain));
		mark2 = mark_NonPrecis;
	}
	if (appl2 == -1)
	{
		print_func(ref,"Error importing runway: high-end approach lights code %d is illegal (not a member of type %s).\n", x.app_light_code[1], DOMAIN_Desc(appl2.domain));
		appl2 = app_None;
	}
	if (reil2 == -1)
	{
		print_func(ref,"Error importing runway: high-end reil code %d is illegal (not a member of type %s).\n", x.reil_code[1], DOMAIN_Desc(reil2.domain));
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
	x.roughness_ratio		 = fltlim     (roughness,0.0f,1.0f);
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


void	WED_Runway::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	WED_GISLine_Width::GetNthPropertyDict(n, dict);
	if(n == PropertyItemNumber(&surface) && surface.value != surf_Water)
	{
		dict.erase(surf_Water);
	}
}


void  WED_Runway::PropEditCallback(int before)
{
	static int    old_enum;            // we want to catch changes of the name property, only
	static pair<int,int> old_enum_1wy;
	static set<int> old_all_rwys;
	static WED_Airport * apt ;
	
	if (before)
	{
		StateChanged(wed_Change_Properties);
		old_enum = GetRunwayEnumsTwoway();
		old_enum_1wy = GetRunwayEnumsOneway();
		apt = WED_GetParentAirport(this);
		if (apt) WED_GetAllRunwaysTwoway(apt, old_all_rwys);
	}
	else
	{
		int new_enum = GetRunwayEnumsTwoway();
		
		if (old_enum == atc_rwy_None) return;
		
		if (new_enum == atc_rwy_None)
		{
			int	res = ConfirmMessage("New Runway Name is illegal, Smart Runway Rename can not be applied.", 
						"Proceed with new name", "Keep old name");
			if(res == 0)
				SetName(string(ENUM_Desc(old_enum)));
			return;
		}
		else if(new_enum != old_enum)
		{
			int renamed_taxi=0;         // keep statistics of modified items
			int renamed_flows=0;
			int renamed_signs=0;
			
			vector<WED_TaxiRoute *>	taxi_routes;
			if(apt) CollectRecursive(apt,back_inserter(taxi_routes), WED_TaxiRoute::sClass);
			vector<WED_ATCFlow *> flows;
			if(apt) CollectRecursive(apt,back_inserter(flows), IgnoreVisiblity, TakeAlways, WED_ATCFlow::sClass);
			vector<WED_ATCRunwayUse *> uses;
			if(apt) CollectRecursive(apt, back_inserter(uses), IgnoreVisiblity, TakeAlways, WED_ATCRunwayUse::sClass);
			vector<WED_AirportSign *> signs;
			if(apt) CollectRecursive(apt, back_inserter(signs), WED_AirportSign::sClass);
			
			if (old_all_rwys.find(new_enum) != old_all_rwys.end())
			{
				DoUserAlert("Another runway of same name already exists, Smart Runway Rename will not be applied.");
				return;
			}
	
			if(apt) 
			{
				apt->CommitCommand();
				apt->StartCommand("Smart Runway Rename");
			}
			
			pair<int,int> new_enum_1wy = GetRunwayEnumsOneway();
			
			for(vector<WED_TaxiRoute *>::iterator t = taxi_routes.begin(); t != taxi_routes.end(); ++t)
			{
				// move all hotzone tags
				bool renamed = false;
				set<int> hotZ;
				hotZ = (*t)->GetHotArrive();
				if (hotZ.erase(old_enum_1wy.first))
				{
					renamed=true;
					hotZ.insert(new_enum_1wy.first);
				}
				if( hotZ.erase(old_enum_1wy.second))
				{
					renamed=true;
					hotZ.insert(new_enum_1wy.second);
				}
				(*t)->SetHotArrive(hotZ);
				
				hotZ = (*t)->GetHotDepart();
				if (hotZ.erase(old_enum_1wy.first))
				{
					renamed=true;
					hotZ.insert(new_enum_1wy.first);
				}
				if( hotZ.erase(old_enum_1wy.second))
				{
					renamed=true;
					hotZ.insert(new_enum_1wy.second);
				}
				(*t)->SetHotDepart(hotZ);
				
				hotZ = (*t)->GetHotILS();
				if (hotZ.erase(old_enum_1wy.first))
				{
					renamed=true;
					hotZ.insert(new_enum_1wy.first);
				}
				if( hotZ.erase(old_enum_1wy.second))
				{
					renamed=true;
					hotZ.insert(new_enum_1wy.second);
				}
				(*t)->SetHotILS(hotZ);
			
				if ((*t)->GetRunway() == old_enum)
				{
					renamed=true;
					(*t)->SetRunway(new_enum);
				}
				if (renamed) renamed_taxi++;
			}
			for(vector<WED_ATCFlow *>::iterator f = flows.begin(); f != flows.end(); ++f)
			{
				bool renamed = false;
				int r = (*f)->GetPatternRunway();
				if(r = old_enum_1wy.first)
				{
					renamed=true;
					(*f)->SetPatternRunway(new_enum_1wy.first);
				}
				else if(r = old_enum_1wy.second)
				{
					renamed=true;
					(*f)->SetPatternRunway(new_enum_1wy.second);
				}
				if (renamed) renamed_flows++;
			}
			for(vector<WED_ATCRunwayUse *>::iterator u = uses.begin(); u != uses.end(); ++u)
			{
				int r = (*u)->GetRunway();
				if(r = old_enum_1wy.first)
					(*u)->SetRunway(new_enum_1wy.first);
				else if(r = old_enum_1wy.second)
					(*u)->SetRunway(new_enum_1wy.second);
			}
			// create list of strings to replace
			vector<string> old_rwys, new_rwys;
			old_rwys.push_back(ENUM_Desc(old_enum_1wy.second));
			new_rwys.push_back(ENUM_Desc(new_enum_1wy.second));
			old_rwys.push_back(ENUM_Desc(old_enum_1wy.first));
			new_rwys.push_back(ENUM_Desc(new_enum_1wy.first));
			
			if (old_rwys.back()[0] == '0')  // also search & replace a sign that has no leading zero in rwy number
			{
				old_rwys.push_back(old_rwys.back().substr(1));
				new_rwys.push_back(new_rwys.back().substr(1));
			}
			for(vector<WED_AirportSign *>::iterator s = signs.begin(); s != signs.end(); ++s)
			{
				bool renamed = false;
				string label; (*s)->GetName(label);
				for (int i = 0; i < old_rwys.size(); ++i)
				{
					string old_rwy = old_rwys[i];
					string new_rwy = new_rwys[i];
					size_t pos;
					if ((pos = label.find(old_rwy)) != string::npos)
					{
						size_t len = label.length();
						size_t next_pos = pos + old_rwy.length();
						
						char prec_char = len > 0 ? label[pos-1] : '!';
						char next_char = next_pos < len ? label[next_pos] : '!';
						
						// gotta be a bit more conservative than just looking for a substring,
						// don't change a sign "GATE 12" when renaming Rwy 12.
						if (!isalnum(prec_char) && !isalnum(next_char))
						{
							renamed=true;
							label = label.replace(pos, old_rwy.length(), new_rwy);
							(*s)->SetName(label);
						}
					}
				}
				if (renamed) renamed_signs++;
			}

			if (renamed_flows || renamed_signs || renamed_taxi)
			{
				stringstream ss;
				ss << "Smart Runway Rename completed:\n" << renamed_taxi << " taxi route edges\n";
				ss << renamed_flows << " ATC flows\n" << renamed_signs << " taxi signs\nrefering to this runway were updated." ;
				DoUserAlert(ss.str().c_str());
			}
		}
	}
}

