/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "WED_CreateEdgeTool.h"
#include "WED_ToolUtils.h"
#include "WED_AirportNode.h"
#include "WED_TaxiRoute.h"

static const char * kCreateCmds[] = { "Taxiway Route Line" };
static const int kIsAirport[] = { 1 };

WED_CreateEdgeTool::WED_CreateEdgeTool(
					const char *		tool_name,
					GUI_Pane *			host,
					WED_MapZoomerNew *	zoomer,
					IResolver *			resolver,
					WED_Archive *		archive,
					CreateEdge_t		tool) :
	WED_CreateToolBase(tool_name, host, zoomer, resolver, archive,
	2,						// min pts,
	99999999,				// max pts - yes, I am a hack.
	1,						// curve allowed?
	0,						// curve required?
	1,						// close allowed?
	0),						// close required
	mType(tool),
	mOneway(tool == create_TaxiRoute ? this : NULL, "Oneway", "", "", 1)
{
}

WED_CreateEdgeTool::~WED_CreateEdgeTool()
{
}

void		WED_CreateEdgeTool::AcceptPath(
			const vector<Point2>&	pts,
			const vector<Point2>&	dirs_lo,
			const vector<Point2>&	dirs_hi,
			const vector<int>		has_dirs,
			const vector<int>		has_split,
			int						closed)
{
		char buf[256];

	int idx;
	WED_Thing * host = GetHost(idx);
	if (host == NULL) return;

	string cname = string("Create ") + kCreateCmds[mType];

	GetArchive()->StartCommand(cname.c_str());

	ISelection *	sel = WED_GetSelect(GetResolver());
	sel->Clear();

	WED_GISEdge *	new_edge = NULL;
	WED_TaxiRoute *	tr = NULL;
	
	static int n = 0;
	++n;
	
	switch(mType) {
	case create_TaxiRoute:
		new_edge = tr = WED_TaxiRoute::CreateTyped(GetArchive());
		tr->SetOneway(mOneway.value);
		
		sprintf(buf,"New taxiway route %d",n);
		tr->SetName(buf);
		break;
	}
	
	WED_Thing * src = NULL, * dst = NULL;
	
	int last = pts.size()-1;
	int stop = pts.size()-1;
	if(closed)
	{
		last = 0;
		stop = pts.size();
	}
//						double	frame_dist = fabs(GetZoomer()->YPixelToLat(0)-GetZoomer()->YPixelToLat(3));
	double	dist=0.00001*0.00001;
	FindNear(host, NULL, pts[0],src,dist);
			dist=0.00001*0.00001;
	FindNear(host, NULL, pts[last],dst,dist);
	
	WED_AirportNode * c;
	
	if(src == NULL)
	{
		src = c = WED_AirportNode::CreateTyped(GetArchive());
		src->SetParent(host,idx);
		src->SetName(buf);
		c->SetLocation(gis_Geo,pts[0]);
	}
	if(dst == NULL)
	{
		dst = c = WED_AirportNode::CreateTyped(GetArchive());
		dst->SetParent(host,idx);
		dst->SetName(buf);
		c->SetLocation(gis_Geo,pts[last]);
	}
	new_edge->AddSource(src,0);
	new_edge->AddSource(dst,1);
	for(int n = 1; n < stop; ++n)
	{
		WED_AirportNode * i = WED_AirportNode::CreateTyped(GetArchive());
		i->SetParent(new_edge,n-1);
		i->SetLocation(gis_Geo,pts[n]);
		sprintf(buf,"Curve point %d",n);
		i->SetName(buf);
	}

	sel->Select(new_edge);
	
	new_edge->SetParent(host,idx);
	
	GetArchive()->CommitCommand();
}

bool		WED_CreateEdgeTool::CanCreateNow(void)
{
	int n;
	return GetHost(n) != NULL;
}

WED_Thing *	WED_CreateEdgeTool::GetHost(int& idx)
{
		return WED_GetCreateHost(GetResolver(), kIsAirport[mType], idx);
}

const char *		WED_CreateEdgeTool::GetStatusText(void)
{
	static char buf[256];
	int n;
	if (GetHost(n) == NULL)
	{
			sprintf(buf,"You must create an airport before you can add a %s.",kCreateCmds[mType]);
		return buf;
	}
	return NULL;
}

void WED_CreateEdgeTool::FindNear(WED_Thing * host, IGISEntity * ent, const Point2& loc, WED_Thing *& out_thing, double& out_dsq)
{
	IGISEntity * e = ent ? ent : dynamic_cast<IGISEntity*>(host);
	WED_Thing * t = host ? host : dynamic_cast<WED_Thing *>(ent);
	if(e && t)
	{
		Point2	l;
		IGISPoint * p;
		IGISPointSequence * ps;
		IGISComposite * c;
	
		switch(e->GetGISClass()) {
		case gis_Point:
		case gis_Point_Bezier:
		case gis_Point_Heading:
		case gis_Point_HeadingWidthLength:
			if((p = dynamic_cast<IGISPoint *>(e)) != NULL)
			{
				p->GetLocation(gis_Geo,l);
				double my_dist = Segment2(loc,l).squared_length();
				if(my_dist < out_dsq)
				{
					out_thing = t;
					out_dsq = my_dist;
				}
			}
			break;
		case gis_PointSequence:
		case gis_Line:
		case gis_Line_Width:
		case gis_Ring:
		case gis_Chain:
			if((ps = dynamic_cast<IGISPointSequence*>(e)) != NULL)
			{
				for(int n = 0; n < ps->GetNumPoints(); ++n)
					FindNear(NULL,ps->GetNthPoint(n), loc, out_thing, out_dsq);
			}
			break;
		case gis_Composite:
			if((c = dynamic_cast<IGISComposite *>(e)) != NULL)
			{
				for(int n = 0; n < c->GetNumEntities(); ++n)
					FindNear(NULL,c->GetNthEntity(n), loc, out_thing, out_dsq);
			}
		}
	}
	else
	{
		for(int n = 0; n < host->CountChildren(); ++n)
			FindNear(host->GetNthChild(n), NULL, loc, out_thing, out_dsq);
	}
}

