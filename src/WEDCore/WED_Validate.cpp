/* 
 * Copyright (c) 2013, Laminar Research.
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

#include "WED_Validate.h"

#include "WED_Globals.h"
#include "WED_Sign_Parser.h"
#include "WED_Runway.h"
#include "WED_Sealane.h"
#include "WED_Helipad.h"
#include "WED_Airport.h"
#include "WED_AirportSign.h"
#include "WED_ToolUtils.h"
#include "WED_FacadePlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_ObjPlacement.h"
#include "WED_StringPlacement.h"
#include "WED_LinePlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_FacadeNode.h"
#include "WED_RampPosition.h"
#include "WED_TaxiRoute.h"
#include "WED_ATCFlow.h"
#include "WED_LibraryMgr.h"
#include "WED_AirportBoundary.h"
#include "WED_GISUtils.h"
#include "WED_Group.h"
#include "WED_ATCRunwayUse.h"
#include "WED_ATCWindRule.h"
#include "WED_EnumSystem.h"
#include "WED_Taxiway.h"


#include "AptDefs.h"
#include "IResolver.h"

#include "PlatformUtils.h"

#define MAX_LON_SPAN_GATEWAY 0.2
#define MAX_LAT_SPAN_GATEWAY 0.2


static set<string>	s_used_rwy;
static set<string>	s_used_hel;
static set<string>	s_icao;
static set<string>	s_flow_names;
static set<int>		s_legal_rwy_oneway;
static set<int>		s_legal_rwy_twoway;

static bool GetThingResouce(WED_Thing * who, string& r)
{
	WED_ObjPlacement * obj;
	WED_FacadePlacement * fac;
	WED_ForestPlacement * fst;
	WED_LinePlacement * lin;
	WED_StringPlacement * str;
	WED_PolygonPlacement * pol;
	
	#define CAST_WITH_CHECK(CLASS,VAR) \
	if(who->GetClass() == CLASS::sClass && (VAR = dynamic_cast<CLASS *>(who)) != NULL) { \
		VAR->GetResource(r); \
		return true; } 
	
	CAST_WITH_CHECK(WED_ObjPlacement,obj)
	CAST_WITH_CHECK(WED_FacadePlacement,fac)
	CAST_WITH_CHECK(WED_ForestPlacement,fst)
	CAST_WITH_CHECK(WED_LinePlacement,lin)
	CAST_WITH_CHECK(WED_StringPlacement,str)
	CAST_WITH_CHECK(WED_PolygonPlacement,pol)
	
	return false;
	
	
}

static WED_Thing * ValidateRecursive(WED_Thing * who, WED_LibraryMgr * lib_mgr)
{
	string name, n1, n2;
	string::size_type p;
	int i;
	who->GetName(name);
	string msg;
	
	// Don't validate hidden stuff - we won't export it!
	WED_Entity * ee = dynamic_cast<WED_Entity *>(who);
	if(ee && ee->GetHidden())
		return NULL;

	//--Taxi Sign Validation-----------------------------------
	if(who->GetClass() == WED_AirportSign::sClass)
	{
		
		WED_AirportSign * airSign = dynamic_cast<WED_AirportSign*>(who);
		string signName;
		airSign->GetName(signName);
		
		//Create the necisary parts for a parsing operation
		parser_in_info in(signName);
		parser_out_info out;
		
		ParserTaxiSign(in,out);
		int MAX_ERRORS = 12;//TODO - Is this good?
		for (int i = 0; i < MAX_ERRORS && i < out.errors.size(); i++)
		{
			msg += out.errors[i].msg;
			msg += '\n';
		}
	}
	//---------------------------------------------------------

	//------------------------------------------------------------------------------------
	// CHECKS FOR GENERAL APT.DAT BOGUSNESS
	//------------------------------------------------------------------------------------			
	
	if(who->GetClass() == WED_Taxiway::sClass)
	{
		WED_Taxiway * twy = dynamic_cast<WED_Taxiway*>(who);
		IGISPointSequence * ps;
		ps = twy->GetOuterRing();
		if(!ps->IsClosed() || ps->GetNumSides() < 3)
			msg = "Outer boundary of taxiway is not at least 3 sided.";
		else
		for(int h = 0; h < twy->GetNumHoles(); ++h)
		{
			ps = twy->GetNthHole(h);
			if(!ps->IsClosed() || ps->GetNumSides() < 3)
			{
				// Ben says: two-point holes are INSANELY hard to find.  So we do the rare thing and intentionally
				// hilite the hole so that the user can nuke it.				
				msg = "Hole of taxiway is not at least 3 sided.";
				WED_Thing * h = dynamic_cast<WED_Thing *>(ps);
				if(h) 
				{
					DoUserAlert(msg.c_str());
					return h;
				}
			}
		}
	}
	
	if (who->GetClass() == WED_Runway::sClass || who->GetClass() == WED_Sealane::sClass)
	{
		if (s_used_rwy.count(name))	msg = "The runway/sealane name '" + name + "' has already been used.";
		s_used_rwy.insert(name);
		p = name.find("/");
		if (p != name.npos)
		{
			n1 = name.substr(0,p);
			n2 = name.substr(p+1);
		} else
			n1 = name;

		int suf1 = 0, suf2 = 0;
		int	num1 = -1, num2 = -1;

		if (n1.empty())	msg = "The runway/sealane '" + name + "' has an empty low-end name.";
		else {
			int suffix = n1[n1.length()-1];
			if (suffix < '0' || suffix > '9')
			{
				if (suffix == 'L' || suffix == 'R' || suffix == 'C') suf1 = suffix;
				else msg = "The runway/sealane '" + name + "' has an illegal suffix for the low-end runway.";
				n1.erase(n1.length()-1);
			}

			for (i = 0; i < n1.length(); ++i)
			if (n1[i] < '0' || n1[i] > '9')
			{
				msg = "The runway/sealane '" + name + "' has an illegal characters in its low-end name.";
				break;
			}
			if (i == n1.length())
			{
				num1 = atoi(n1.c_str());
			}
			if (num1 < 1 || num1 > 36)
			{
				msg = "The runway/sealane '" + name + "' has an illegal low-end number, which must be between 1 and 36.";
				num1 = -1;
			}
		}

		if (p != name.npos)
		{
			if (n2.empty())	msg = "The runway/sealane '" + name + "' has an empty high-end name.";
			else {
				int suffix = n2[n2.length()-1];
				if (suffix < '0' || suffix > '9')
				{
					if (suffix == 'L' || suffix == 'R' || suffix == 'C') suf2 = suffix;
					else msg = "The runway/sealane '" + name + "' has an illegal suffix for the high-end runway.";
					n2.erase(n2.length()-1);
				}

				for (i = 0; i < n2.length(); ++i)
				if (n2[i] < '0' || n2[i] > '9')
				{
					msg = "The runway/sealane '" + name + "' has an illegal characters in its high-end name.";
					break;
				}
				if (i == n2.length())
				{
					num2 = atoi(n2.c_str());
				}
				if (num2 < 19 || num2 > 36)
				{
					msg = "The runway/sealane '" + name + "' has an illegal high-end number, which must be between 19 and 36.";
					num2 = -1;
				}
			}
		}

		if (suf1 != 0 && suf2 != 0)
		{
			if ((suf1 == 'L' && suf2 != 'R') ||
				(suf1 == 'R' && suf2 != 'L') ||
				(suf1 == 'C' && suf2 != 'C'))
					msg = "The runway/sealane '" + name + "' has mismatched suffixes - check L vs R, etc.";
		}
		if (num1 != -1 && num2 != -1)
		{
			if (num2 < num1)
				msg = "The runway/sealane '" + name + "' has mismatched runway numbers - the low number must be first.'";
			else if (num2 != num1 + 18)
				msg = "The runway/sealane '" + name + "' has mismatched runway numbers - high end is not the reciprocal of the low-end.";
		}

		if (msg.empty())
		{
			WED_GISLine_Width * lw = dynamic_cast<WED_GISLine_Width *>(who);
			if (lw->GetWidth() < 1.0) msg = "The runway/sealane '" + name + "' must be at least one meter wide.";

			WED_Runway * rwy = dynamic_cast<WED_Runway *>(who);
			if (rwy)
			{
				if (rwy->GetDisp1() + rwy->GetDisp2() > rwy->GetLength()) msg = "The runway/sealane '" + name + "' has overlapping displaced threshholds.";
			}
		}
	}
	if (who->GetClass() == WED_Helipad::sClass)
	{
		if (s_used_hel.count(name))	msg = "The helipad name '" + name + "' has already been used.";
		s_used_hel.insert(name);

		n1 = name;
		if (n1.empty())	msg = "The selected helipad has no name.";
		else {
			if (n1[0] != 'H')	msg = "The helipad '" + name + "' does not start with the letter H.";
			else {
				n1.erase(0,1);
				for (int i = 0; i < n1.length(); ++i)
				{
					if (n1[i] < '0' || n1[i] > '9')
					{
						msg = "The helipad '" + name + "' conntains illegal characters in its name.  It must be in the form H<number>.";
						break;
					}
				}
			}
		}
		if (msg.empty())
		{
			WED_Helipad * heli = dynamic_cast<WED_Helipad *>(who);
			if (heli->GetWidth() < 1.0) msg = "The helipad '" + name + "' is less than one meter wide.";
			if (heli->GetLength() < 1.0) msg = "The helipad '" + name + "' is less than one meter long.";
		}
	}
	if(who->GetClass() == WED_Airport::sClass)
	{
		s_used_hel.clear();
		s_used_rwy.clear();
		s_flow_names.clear();
		s_legal_rwy_oneway.clear();
		s_legal_rwy_twoway.clear();
		
		WED_Airport * apt = dynamic_cast<WED_Airport *>(who);
		if(who)
		{
			WED_GetAllRunwaysOneway(apt,s_legal_rwy_oneway);
			WED_GetAllRunwaysTwoway(apt,s_legal_rwy_twoway);
		
			string icao;
			apt->GetICAO(icao);
			if(icao.empty())
			{
				msg = "The airport '" + name + "' has an empty ICAO code.";
			}
			else
			{
				if(s_icao.count(icao))
				{
					msg = "The airport ICAO code '" + icao + "' is used twice in your WED project file.";
				}
				else
					s_icao.insert(icao);
			}
		}
	}
	
	//------------------------------------------------------------------------------------
	// CHECKS FOR V10 DSF OVERLAY EXTENSIONS
	//------------------------------------------------------------------------------------		
	
	if(who->GetClass() == WED_FacadePlacement::sClass)
	{
		WED_FacadePlacement * fac = dynamic_cast<WED_FacadePlacement*>(who);
		DebugAssert(who);
		if(gExportTarget == wet_xplane_900 && fac->HasCustomWalls())
		{
			msg = "Custom facade wall choices are only supported in X-Plane 10 and newer.";
		}
		
		if(fac->GetNumHoles() > 0)
		{
			msg = "Facades may not have holes in them.";
		}
		
		if(gExportTarget == wet_xplane_900 && WED_HasBezierPol(fac))
			msg = "Curved facades are only supported in X-Plane 10 and newer.";

	}
	
	if(who->GetClass() == WED_ForestPlacement::sClass)
	{
		WED_ForestPlacement * fst = dynamic_cast<WED_ForestPlacement *>(who);
		DebugAssert(fst);
		if(gExportTarget == wet_xplane_900 && fst->GetFillMode() != dsf_fill_area)
			msg = "Line and point forests are only supported in X-Plane 10 and newer.";
	}
	
	//------------------------------------------------------------------------------------
	// CHECKS FOR ATC FIELD BUGS
	//------------------------------------------------------------------------------------	
	
	if(gExportTarget != wet_xplane_900)	// not even legal for v9
	{
		WED_ATCFlow * flow;
		WED_ATCWindRule * wind;
		WED_TaxiRoute * taxi;
		
		if(who->GetClass() == WED_ATCFlow::sClass && ((flow = dynamic_cast<WED_ATCFlow *>(who)) != NULL))
		{
			AptFlow_t exp;
			flow->Export(exp);
			if(exp.icao.empty())
				msg = "ATC Flow '" + name + "' has a blank ICAO code for its METAR source.";
				
			if(name.empty())
				msg = "An ATC flow has a blank name.  You must name every flow.";
				
			if(s_flow_names.count(name) > 0)
			{
				msg = "You have two airport flows named '" + name + "'.  Every ATC flow name must be unique.";				
			}
			else 
				s_flow_names.insert(name);
				
			if(s_legal_rwy_oneway.count(flow->GetPatternRunway()) == 0)
				msg = "The pattern runway " + string(ENUM_Desc(flow->GetPatternRunway())) + " is illegal for the ATC flow '" + name + "' because it is not a runway at this airport.";
		}

		if(who->GetClass() == WED_ATCWindRule::sClass && ((wind = dynamic_cast<WED_ATCWindRule *>(who)) != NULL))
		{
			AptWindRule_t exp;
			wind->Export(exp);
			if(exp.icao.empty())
				msg = "ATC wind rule '" + name + "' has a blank ICAO code for its METAR source.";
		}
		
		if(who->GetClass() == WED_TaxiRoute::sClass && ((taxi = dynamic_cast<WED_TaxiRoute *>(who)) != NULL))
		{
			// See bug http://dev.x-plane.com/bugbase/view.php?id=602 - blank names are okay!
//			if (name.empty() && !taxi->IsRunway())
//			{
//				msg = "This taxi route has no name.  All taxi routes must have a name so that ATC can give taxi instructions.";
//			}
			
			if(taxi->HasInvalidHotZones(s_legal_rwy_oneway))
			{
				msg = "The taxi route '" + name + "' has hot zones for runways not present at its airport.";
			}
			
			if(taxi->IsRunway())
			if(s_legal_rwy_twoway.count(taxi->GetRunway()) == 0)
			{
				msg = "The taxi route '" + name + "' is set to a ruwnay not present at the airport.";
			}
		}		
	}
	
	//------------------------------------------------------------------------------------
	// CHECKS FOR V10 APT.DAT FEATURES
	//------------------------------------------------------------------------------------	
	
	if(who->GetClass() == WED_RampPosition::sClass)
	{
		AptGate_t	g;
		WED_RampPosition * ramp = dynamic_cast<WED_RampPosition*>(who);
		DebugAssert(ramp);
		ramp->Export(g);
		
		if(gExportTarget == wet_xplane_900)
		if(g.equipment != 0)
		if(g.type != atc_ramp_misc || g.equipment != atc_traffic_all)
			msg = "Gates with specific traffic and types are only suported in X-Plane 10 and newer.";
			
		if(g.equipment == 0)
			msg = "Gates must have at least one valid type of equipment selected.";
	}

	if(gExportTarget == wet_xplane_900)
	{
		if(who->GetClass() == WED_ATCFlow::sClass)
		{
			msg = "ATC flow information is only supported in x-Plane 10 and newer.";
		}
		if(who->GetClass() == WED_TaxiRoute::sClass)
		{
			msg = "ATC taxi routes are only supported in x-Plane 10 and newer.";
		}
	}
	
	if(who->GetClass() == WED_ATCRunwayUse::sClass)
	{
		WED_ATCRunwayUse * use = dynamic_cast<WED_ATCRunwayUse *>(who);
		AptRunwayRule_t urule;
		use->Export(urule);
		if(urule.operations == 0)
			msg = "ATC runway use must support at least one operation type.";
		else if(urule.equipment == 0)
			msg = "ATC runway use must support at least one equipment type.";
	}
	
	//------------------------------------------------------------------------------------
	// CHECKS FOR SUBMISSION TO GATEWAY
	//------------------------------------------------------------------------------------

	if(gExportTarget == wet_gateway)
	{
		if(who->GetClass() != WED_Group::sClass)
		if(WED_GetParentAirport(who) == NULL)
			msg = "You cannot export airport overlays to the X-Plane Airport Gateway if overlay elements are outside airports in the hierarchy.";

		if(who->GetClass() == WED_Airport::sClass)
		{
			WED_Airport * apt = dynamic_cast<WED_Airport *>(who);
			Bbox2 bounds;
			apt->GetBounds(gis_Geo, bounds);
			if(bounds.xspan() > MAX_LON_SPAN_GATEWAY ||
			   bounds.yspan() > MAX_LAT_SPAN_GATEWAY)
			{
				msg = "This airport is too big.  Perhaps a random part of the airport has been dragged to another part of the world?";
			}
		
		}

		#if !GATEWAY_IMPORT_FEATURES
		if(who->GetClass() == WED_AirportBoundary::sClass)
		{
			if(WED_HasBezierPol(dynamic_cast<WED_AirportBoundary*>(who)))
				msg = "Do not use bezier curves in airport boundaries.";
		}
		#endif

		if(who->GetClass() == WED_DrapedOrthophoto::sClass)
			msg = "Draped orthophotos are not allowed in the global airport database.";

		string res;
		if(GetThingResouce(who,res))
		{
			if(!lib_mgr->IsResourceDefault(res))
				msg = "The library path '" + res + "' is not part of X-Plane's default installation and cannot be submitted to the global airport database.";
		}
	}

	//------------------------------------------------------------------------------------

	if (!msg.empty())
	{
		DoUserAlert(msg.c_str());
		return who;
	}

	int nn = who->CountChildren();
	for (int n = 0; n < nn; ++n)
	{
		WED_Thing * fail = ValidateRecursive(who->GetNthChild(n), lib_mgr);
		if (fail) return fail;
	}
	
	if(who->GetClass() == WED_Airport::sClass)
	{
		if(s_used_hel.empty() && s_used_rwy.empty())
		{
			msg = "The airport '" + name + "' contains no runways, sea lanes, or helipads.";
			DoUserAlert(msg.c_str());
			return who;
		}
	}
	
	return NULL;
}

bool	WED_ValidateApt(IResolver * resolver, WED_Thing * wrl)
{
	s_used_hel.clear();
	s_used_rwy.clear();
	s_flow_names.clear();
	s_legal_rwy_oneway.clear();
	s_icao.clear();
	if(wrl == NULL) wrl = WED_GetWorld(resolver);
	ISelection * sel = WED_GetSelect(resolver);
	
	WED_LibraryMgr * lib_mgr = 	WED_GetLibraryMgr(resolver);
	
	WED_Thing * bad_guy = ValidateRecursive(wrl, lib_mgr);
	if (bad_guy)
	{
		wrl->StartOperation("Select Invalid");
		sel->Select(bad_guy);
		wrl->CommitOperation();
		return false;
	}
	return true;
}
