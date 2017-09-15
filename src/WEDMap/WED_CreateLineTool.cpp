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

#include "WED_CreateLineTool.h"
#include "IResolver.h"
#include "WED_Airport.h"
#include "WED_ToolUtils.h"
#include "WED_Runway.h"
#include "ISelection.h"
#include "WED_EnumSystem.h"
#include "WED_RunwayNode.h"
#include "WED_Sealane.h"
#include "XESConstants.h"
#include "GISUtils.h"
static const char * kCreateCmds[] = {
	"Runway", "Sealane"
};

WED_CreateLineTool::WED_CreateLineTool(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer,
									IResolver *			resolver,
									WED_Archive *		archive,
									CreateLine_t		tool) :
	WED_CreateToolBase(tool_name,host, zoomer, resolver,archive,
	2,								// min pts
	2,								// max pts
	0,								// curve allowed
	0,								// curve required?
	0,								// close allowed
	0),								// close required?
	mType(tool),
		// Ben says: toolbar for runway create tool BARELY fits on 1024 x 768 res, so all tool names are intentionally super-short.
		rwy_surface			(tool==create_Runway	?this:NULL,"Surface",  XML_Name("",""),Surface_Type,	surf_Concrete),
		rwy_shoulder		(tool==create_Runway	?this:NULL,"Shoulder", XML_Name("",""),Shoulder_Type,shoulder_None),
		rwy_roughness		(tool==create_Runway	?this:NULL,"Rough",	   XML_Name("",""),0.25,4,2),
		rwy_center_lites	(tool==create_Runway	?this:NULL,"Center",   XML_Name("",""),1),
		rwy_edge_lights		(tool==create_Runway	?this:NULL,"Edge",	   XML_Name("",""),Edge_Lights,	edge_MIRL),
		rwy_remaining_signs	(tool==create_Runway	?this:NULL,"Dist",	   XML_Name("",""),1),
		rwy_markings		(tool==create_Runway	?this:NULL,"Marks",	   XML_Name("",""),Runway_Markings,	mark_NonPrecis),
		rwy_app_lights		(tool==create_Runway	?this:NULL,"Appch",	   XML_Name("",""),Light_App,		app_MALSF),
		rwy_tdzl			(tool==create_Runway	?this:NULL,"TDZL",	   XML_Name("",""),0),
		rwy_reil			(tool==create_Runway	?this:NULL,"REIL",	   XML_Name("",""),REIL_Lights,		reil_None),
		sea_buoys			(tool==create_Sealane	?this:NULL,"Buoys",	   XML_Name("",""),1)
{
}

WED_CreateLineTool::~WED_CreateLineTool()
{
}

void	WED_CreateLineTool::AcceptPath(
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
	WED_Thing * host = WED_GetCreateHost(GetResolver(), true, true, idx);

	WED_GISLine_Width * obj = NULL;

	WED_Runway * rwy;
	WED_Sealane * sea;

	switch(mType) {
	case create_Runway:
		obj = rwy = WED_Runway::CreateTyped(GetArchive());
		rwy->SetSurface(rwy_surface.value);
		rwy->SetShoulder(rwy_shoulder.value);
		rwy->SetRoughness(rwy_roughness.value);
		rwy->SetCenterLights(rwy_center_lites.value);
		rwy->SetEdgeLights(rwy_edge_lights.value);
		rwy->SetRemainingSigns(rwy_remaining_signs.value);
		rwy->SetMarkings1(rwy_markings.value);
		rwy->SetAppLights1(rwy_app_lights.value);
		rwy->SetTDZL1(rwy_tdzl.value);
		rwy->SetREIL1(rwy_reil.value);
		rwy->SetMarkings2(rwy_markings.value);
		rwy->SetAppLights2(rwy_app_lights.value);
		rwy->SetTDZL2(rwy_tdzl.value);
		rwy->SetREIL2(rwy_reil.value);
		break;
	case create_Sealane:
		obj = sea = WED_Sealane::CreateTyped(GetArchive());
		sea->SetBuoys(sea_buoys.value);
		break;
	}

	WED_RunwayNode * n1 = WED_RunwayNode::CreateTyped(GetArchive());
	WED_RunwayNode * n2 = WED_RunwayNode::CreateTyped(GetArchive());

	n1->SetParent(obj,0);
	n2->SetParent(obj,1);
	n1->SetName("Start");
	n2->SetName("End");

	obj->GetSource()->SetLocation(gis_Geo,pts[0]);
	obj->GetTarget()->SetLocation(gis_Geo,pts[1]);
	obj->SetWidth(50.0);
	static int n = 0;
	++n;
	obj->SetParent(host, idx);

	int h = obj->GetHeading();
	if (h < 0) h += 360;
	if (h > 180)
	{
		obj->GetSource()->SetLocation(gis_Geo,pts[1]);
		obj->GetTarget()->SetLocation(gis_Geo,pts[0]);
		h = obj->GetHeading();
		if (h < 0) h += 360;
	}
	h /= 10;
	if (h < 1) h = 1;

	sprintf(buf,"%02d/%02d",h,h+18);
	obj->SetName(buf);

	ISelection * sel = WED_GetSelect(GetResolver());
	sel->Clear();
	sel->Select(obj);

	GetArchive()->CommitCommand();

}


const char *	WED_CreateLineTool::GetStatusText(void)
{
	static char buf[256];
	if (WED_GetCurrentAirport(GetResolver()) == NULL)
	{
		sprintf(buf,"You must create an airport before you can add a %s.",kCreateCmds[mType]);
		return buf;
	}
	return NULL;
}

bool		WED_CreateLineTool::CanCreateNow(void)
{
	return WED_GetCurrentAirport(GetResolver()) != NULL;
}

void	WED_CreateLineTool::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	WED_CreateToolBase::GetNthPropertyDict(n, dict);
	if(n == PropertyItemNumber(&rwy_surface) && rwy_surface.value != surf_Water)
	{
		dict.erase(surf_Water);
	}
}

