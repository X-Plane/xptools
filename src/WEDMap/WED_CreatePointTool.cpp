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

#include "WED_CreatePointTool.h"
#include "WED_Archive.h"
#include "WED_AirportBeacon.h"
#include "WED_Airport.h"
#include "WED_AirportSign.h"
#include "WED_Helipad.h"
#include "WED_LightFixture.h"
#include "WED_EnumSystem.h"
#include "WED_RampPosition.h"
#include "WED_TowerViewpoint.h"
#include "WED_ToolUtils.h"
#include "WED_Windsock.h"
#include "WED_ObjPlacement.h"
#include "ISelection.h"
#include "GISUtils.h"
#include "WED_ToolUtils.h"
#include "IResolver.h"
#include "GUI_Clipboard.h"
#include "WED_TruckDestination.h"
#include "WED_TruckParkingLocation.h"

static int kIsToolDirectional[] = { 0, 1, 1, 1, 1, 0, 0, 1, 1, 1 };
static int kIsAirport[]			= { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 };
static const char * kCreateCmds[] = {
	"Airport Beacon",
	"Taxiway Sign",
	"Helipad",
	"Light Fixture",
	"Ramp Start",
	"Tower Viewpoint",
	"Windsock",
	"Object",
	"Service Truck",
	"Service Truck Destination"
};

WED_CreatePointTool::WED_CreatePointTool(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer,
									IResolver *			resolver,
									WED_Archive *		archive,
									CreatePoint_t		tool) :
	WED_CreateToolBase(tool_name,host, zoomer, resolver,archive,
	1,								// min pts
	1,								// max pts
	kIsToolDirectional[tool],		// curve allowed
	kIsToolDirectional[tool],		// curve required?
	0,								// close allowed
	0),								// close required?
	mType(tool),
		beacon_kind		(tool==create_Beacon		?this:NULL,"Kind",			XML_Name("",""),Airport_Beacon,beacon_Airport),
		sign_text		(tool==create_Sign			?this:NULL,"Text",			XML_Name("",""),"{@L}A"),
		sign_style		(tool==create_Sign			?this:NULL,"Style",			XML_Name("",""),Sign_Style,style_Default),
		sign_height		(tool==create_Sign			?this:NULL,"Size",			XML_Name("",""),Sign_Size,size_MediumTaxi),
		heli_surface	(tool==create_Helipad		?this:NULL,"Surface",		XML_Name("",""),Surface_Type,surf_Concrete),
		heli_markings	(tool==create_Helipad		?this:NULL,"Markings",		XML_Name("",""),Helipad_Markings,heli_Mark_Default),
		heli_shoulder	(tool==create_Helipad		?this:NULL,"Shoulder",		XML_Name("",""),Shoulder_Type,shoulder_None),
		heli_roughness	(tool==create_Helipad		?this:NULL,"Roughness",		XML_Name("",""),0.25,4,2),
		heli_edgelights	(tool==create_Helipad		?this:NULL,"Edge Lights",	XML_Name("",""),Heli_Lights,heli_Yellow),
		light_kind		(tool==create_Lights		?this:NULL,"Fixture Type",	XML_Name("",""),Light_Fixt,light_VASI),
		light_angle		(tool==create_Lights		?this:NULL,"Approach Angle",XML_Name("",""),3.0,4,2),
		tower_height	(tool==create_TowerViewpoint?this:NULL,"Tower Height",	XML_Name("",""),25.0,5,1),
		windsock_lit	(tool==create_Windsock		?this:NULL,"Lit",			XML_Name("",""),0),
		resource		(tool==create_Object		?this:NULL,"Object",		XML_Name("",""),""),
		show_level		(tool==create_Object		?this:NULL,"Show with",		XML_Name("",""),ShowLevel,show_Level1),
		ramp_type		(tool==create_RampStart		?this:NULL,"Ramp Start Type",XML_Name("",""   ), ATCRampType, atc_Ramp_Misc),
		equip_type		(tool==create_RampStart		?this:NULL,"Equipment Type",XML_Name("",""), ATCTrafficType, 0),
		width			(tool==create_RampStart		?this:NULL,"Size",	        XML_Name("",""), ATCIcaoWidth, width_E),
		ramp_op_type	(tool==create_RampStart		?this:NULL,"Ramp Operation Type",XML_Name("",""), RampOperationType, ramp_operation_None),
		airlines		(tool==create_RampStart		?this:NULL,"Airlines",      XML_Name("",""), ""),
		truck_type		(tool==create_TruckParking	?this:NULL,"Truck Type",	XML_Name("",""), ATCServiceTruckType, atc_ServiceTruck_FuelTruck_Prop),
		baggage_car_count(tool==create_TruckParking	?this:NULL,"Baggage Cars",	XML_Name("",""), 3, 1),
		truck_types		(tool==create_TruckDestination?this:NULL,"Truck Types", XML_Name("",""), ATCServiceTruckType, 0)
{
}

WED_CreatePointTool::~WED_CreatePointTool()
{
}

void	WED_CreatePointTool::AcceptPath(
							const vector<Point2>&	pts,
							const vector<Point2>&	dirs_lo,
							const vector<Point2>&	dirs_hi,
							const vector<int>		has_dirs,
							const vector<int>		has_split,
							int						closed)
{
		char buf[256];

	sprintf(buf, "Create %s",kCreateCmds[mType]);

	GetArchive()->StartCommand(buf);

	int idx;
	WED_Thing * host = WED_GetCreateHost(GetResolver(), kIsAirport[mType], true, idx);

	WED_GISPoint * new_pt_obj = NULL;
	WED_GISPoint_Heading * new_pt_h = NULL;

	WED_AirportBeacon * beacon;
	WED_AirportSign * sign;
	WED_Helipad * helipad;
	WED_LightFixture * lights;
	WED_RampPosition * ramp;
	WED_TowerViewpoint * tower;
	WED_Windsock * sock;
	WED_ObjPlacement * obj;
		string ct;

	switch(mType) {
	case create_Beacon:
		new_pt_obj = beacon = WED_AirportBeacon::CreateTyped(GetArchive());
		beacon->SetKind(beacon_kind.value);
		break;
	case create_Sign:
		new_pt_obj = new_pt_h = sign = WED_AirportSign::CreateTyped(GetArchive());
		sign->SetStyle(sign_style.value);
		sign->SetHeight(sign_height.value);
		sign->SetName(sign_text.value);
		break;
	case create_Helipad:
		new_pt_obj = new_pt_h = helipad = WED_Helipad::CreateTyped(GetArchive());
		helipad->SetWidth(50.0);
		helipad->SetLength(50.0);
		helipad->SetSurface(heli_surface.value);
		helipad->SetMarkings(heli_markings.value);
		helipad->SetShoulder(heli_shoulder.value);
		helipad->SetRoughness(heli_roughness.value);
		helipad->SetEdgeLights(heli_edgelights.value);
		break;
	case create_Lights:
		new_pt_obj = new_pt_h = lights = WED_LightFixture::CreateTyped(GetArchive());
		lights->SetLightType(light_kind.value);
		lights->SetAngle(light_angle.value);
		break;
	case create_RampStart:
		new_pt_obj = new_pt_h = ramp = WED_RampPosition::CreateTyped(GetArchive());
		ramp->SetType(ramp_type.value);
		ramp->SetEquipment(equip_type.value);
		ramp->SetWidth(width.value);
		ramp->SetRampOperationType(ramp_op_type.value);
		ramp->SetAirlines(airlines.value);
		break;
	case create_TowerViewpoint:
		new_pt_obj = tower = WED_TowerViewpoint::CreateTyped(GetArchive());
		tower->SetHeight(tower_height.value);
		break;
	case create_Windsock:
		new_pt_obj = sock = WED_Windsock::CreateTyped(GetArchive());
		sock->SetLit(windsock_lit.value);
		break;
	case create_Object:
		{
			new_pt_obj = new_pt_h = obj = WED_ObjPlacement::CreateTyped(GetArchive());
			obj->SetResource(resource.value);
			obj->SetShowLevel(ENUM_Export(show_level.value));
			string n = resource.value;
			string::size_type p = n.find_last_of("/\\:");
			if(p != n.npos) n.erase(0,p+1);
			obj->SetName(n);
		}
		break;
	case create_TruckParking:
		{
			WED_TruckParkingLocation * t;
			new_pt_obj = new_pt_h = t = WED_TruckParkingLocation::CreateTyped(GetArchive());
			t->SetTruckType(truck_type.value);
			t->SetNumberOfCars(baggage_car_count.value);
		}
		break;
	case create_TruckDestination:
		{
			WED_TruckDestination * t;
			new_pt_obj = new_pt_h = t = WED_TruckDestination::CreateTyped(GetArchive());
			t->SetTruckTypes(truck_types.value);			
		}
		break;
	}

	DebugAssert((kIsToolDirectional[mType] && new_pt_h != NULL) || (!kIsToolDirectional[mType] && new_pt_h == NULL));

	new_pt_obj->SetLocation(gis_Geo,pts[0]);
	if (new_pt_h) new_pt_h->SetHeading(VectorDegs2NorthHeading(pts[0],pts[0],Vector2(pts[0],dirs_hi[0])));

	static int n = 0;
	new_pt_obj->SetParent(host, idx);
	if (mType == create_Helipad)
		sprintf(buf,"H%d",++n);
	else
		sprintf(buf,"New %s %d",kCreateCmds[mType],++n);
	if (mType != create_Sign && mType != create_Object)
		new_pt_obj->SetName(buf);

	ISelection * sel = WED_GetSelect(GetResolver());
	sel->Clear();
	sel->Select(new_pt_obj);

	GetArchive()->CommitCommand();

}

const char *	WED_CreatePointTool::GetStatusText(void)
{
	static char buf[256];
	if (WED_GetCurrentAirport(GetResolver()) == NULL && kIsAirport[mType])
	{
		sprintf(buf,"You must create an airport before you can add a %s.",kCreateCmds[mType]);
		return buf;
	}
	return NULL;
}

bool		WED_CreatePointTool::CanCreateNow(void)
{
	return WED_GetCurrentAirport(GetResolver()) != NULL || !kIsAirport[mType];
}

void		WED_CreatePointTool::SetResource(const string& r)
{
	resource.value = r;
}

void		WED_CreatePointTool::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	WED_CreateToolBase::GetNthPropertyInfo(n, info);
	if(n == PropertyItemNumber(&sign_text))
	{
		DebugAssert(info.prop_kind == prop_String);
		info.prop_kind = prop_TaxiSign;
	}
	
	//Disable the baggage car count text field if the type is not Baggage_Train
	PropertyVal_t prop;
	truck_type.GetProperty(prop);
	if (prop.int_val != atc_ServiceTruck_Baggage_Train && n == PropertyItemNumber(&baggage_car_count))
	{
		info.prop_name = "."; //The special hardcoded "disable me" string, see IPropertyObject.h
		info.can_edit = false;
		info.can_delete = false;
	}
}

void		WED_CreatePointTool::GetNthProperty(int n, PropertyVal_t& val) const
{
	WED_CreateToolBase::GetNthProperty(n, val);
	if(n == PropertyItemNumber(&sign_text))
	{
		DebugAssert(val.prop_kind == prop_String);
		val.prop_kind = prop_TaxiSign;
	}
}

void		WED_CreatePointTool::SetNthProperty(int n, const PropertyVal_t& val)
{
	PropertyVal_t v(val);
	if(n == PropertyItemNumber(&sign_text))
	{
		DebugAssert(v.prop_kind == prop_TaxiSign);
		v.prop_kind = prop_String;
	}
	WED_CreateToolBase::SetNthProperty(n, v);
}
