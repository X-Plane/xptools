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

static int kIsToolDirectional[] = { 0, 1, 1, 1, 1, 0, 0, 1 };
static int kIsAirport[]			= { 1, 1, 1, 1, 1, 1, 1, 0 };
static const char * kCreateCmds[] = {
	"Airport Beacon",
	"Taxiway Sign",
	"Helipad",
	"Light Fixture",
	"Ramp Start",
	"Tower Viewpoint",
	"Windsock",
	"Object"
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
		beacon_kind		(tool==create_Beacon		?this:NULL,"Kind",			"","",Airport_Beacon,beacon_Airport),
		sign_text		(tool==create_Sign			?this:NULL,"Text",			"","","{@L}A"),
		sign_style		(tool==create_Sign			?this:NULL,"Style",			"","",Sign_Style,style_Default),
		sign_height		(tool==create_Sign			?this:NULL,"Size",			"","",Sign_Size,size_MediumTaxi),
		heli_surface	(tool==create_Helipad		?this:NULL,"Surface",		"","",Surface_Type,surf_Concrete),
		heli_markings	(tool==create_Helipad		?this:NULL,"Markings",		"","",Helipad_Markings,heli_Mark_Default),
		heli_shoulder	(tool==create_Helipad		?this:NULL,"Shoulder",		"","",Shoulder_Type,shoulder_None),
		heli_roughness	(tool==create_Helipad		?this:NULL,"Roughness",		"","",0.25,4,2),
		heli_edgelights	(tool==create_Helipad		?this:NULL,"Edge Lights",	"","",Heli_Lights,heli_Yellow),
		light_kind		(tool==create_Lights		?this:NULL,"Fixture Type",	"","",Light_Fixt,light_VASI),
		light_angle		(tool==create_Lights		?this:NULL,"Approach Angle","","",3.0,4,2),
		tower_height	(tool==create_TowerViewpoint?this:NULL,"Tower Height",	"","",25.0,5,1),
		windsock_lit	(tool==create_Windsock		?this:NULL,"Lit",			"","",0),
		resource		(tool==create_Object		?this:NULL,"Object",		"","",""),
		sign_clipboard	(tool==create_Sign			?this:NULL,"Use Clipboard",	"","",0)
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
	WED_Thing * host = WED_GetCreateHost(GetResolver(), kIsAirport[mType], idx);

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
		if(sign_clipboard.value && GUI_GetTextFromClipboard(ct))
			sign->SetName(ct);
		else
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
			string n = resource.value;
			string::size_type p = n.find_last_of("/\\:");
			if(p != n.npos) n.erase(0,p+1);
			obj->SetName(n);
		}
		break;
	}

	DebugAssert((kIsToolDirectional[mType] && new_pt_h != NULL) || (!kIsToolDirectional[mType] && new_pt_h == NULL));

	new_pt_obj->SetLocation(gis_Geo,pts[0]);
	if (new_pt_h) new_pt_h->SetHeading(VectorDegs2NorthHeading(pts[0],pts[0],Vector2(pts[0],dirs_hi[0])));

	static int n = 0;
	static int h = 0;
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


