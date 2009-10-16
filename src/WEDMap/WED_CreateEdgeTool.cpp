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
#include "WED_MapZoomerNew.h"
#include "WED_GISUtils.h"

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
	mOneway(tool == create_TaxiRoute ? this : NULL, "Oneway", "", "", 1),
	mName(tool == create_TaxiRoute ? this : NULL, "Name", "", "", "N")
{
}

WED_CreateEdgeTool::~WED_CreateEdgeTool()
{
}

struct sort_by_seg_rat {
	sort_by_seg_rat(const Point2& i) : a(i) { }
	Point2	a;
	bool operator()(const Point2& p1, const Point2& p2) const {
		return a.squared_distance(p1) < a.squared_distance(p2);
	}
};

static void SortSplits(const Segment2& s, vector<Point2>& splits)
{
	sort(splits.begin(), splits.end(), sort_by_seg_rat(s.p1));
}



void		WED_CreateEdgeTool::AcceptPath(
			const vector<Point2>&	in_pts,
			const vector<Point2>&	dirs_lo,
			const vector<Point2>&	dirs_hi,
			const vector<int>		has_dirs,
			const vector<int>		has_split,
			int						closed)
{
	vector<Point2>	pts(in_pts);
	int idx;
	WED_Thing * host = GetHost(idx);
	if (host == NULL) return;

	string cname = string("Create ") + kCreateCmds[mType];

	GetArchive()->StartCommand(cname.c_str());

	ISelection *	sel = WED_GetSelect(GetResolver());
	sel->Clear();

	for(int p = 1; p < pts.size(); ++p)
	{
		vector<Point2>	splits;
		int dummy;
		SplitByLine(GetHost(dummy), NULL, Segment2(pts[p-1],pts[p]), splits);
		SortSplits(Segment2(pts[p-1],pts[p]), splits);
		pts.insert(pts.begin() + p, splits.begin(),splits.end());
		p += splits.size();
	}

	WED_GISEdge *	new_edge = NULL;
	WED_TaxiRoute *	tr = NULL;
	
	static int n = 0;
	int stop = closed ? pts.size() : pts.size()-1;
	int start = 0;
	
	WED_Thing * last = NULL;
	while(start < stop)
	{
		++n;	
		switch(mType) {
		case create_TaxiRoute:
			new_edge = tr = WED_TaxiRoute::CreateTyped(GetArchive());
			tr->SetOneway(mOneway.value);
			
			tr->SetName(mName);
			break;
		}
	
		WED_AirportNode * c;
		WED_Thing * src = last, * dst = NULL;
		double frame_dist = fabs(GetZoomer()->YPixelToLat(5)-GetZoomer()->YPixelToLat(0));
		double	dist=frame_dist*frame_dist;
		if(src == NULL)	
		{
			FindNear(host, NULL, pts[start % pts.size()],src,dist);
			if(src != NULL) WED_SplitEdgeIfNeeded(src, mName.value);
		}
		if(src == NULL)
		{
			src = c = WED_AirportNode::CreateTyped(GetArchive());
			src->SetParent(host,idx);
			src->SetName(mName.value + "_start");
			c->SetLocation(gis_Geo,pts[0]);
		}
		new_edge->AddSource(src,0);

		int cidx = 0;		
		int p = start + 1;
		while(p <= stop)
		{
			dist=frame_dist*frame_dist;
			FindNear(host, NULL, pts[p % pts.size()],dst,dist);
			if(dst != NULL)
			{
				WED_SplitEdgeIfNeeded(dst, mName.value);
				new_edge->AddSource(dst,1);
				sel->Insert(new_edge);	
				new_edge->SetParent(host,idx);
				
				last = dst;
				start = p;
				break;
			}
			else if(p == stop)
			{
				dst = c = WED_AirportNode::CreateTyped(GetArchive());
				dst->SetParent(host,idx);
				dst->SetName(mName.value+"_stop");
				c->SetLocation(gis_Geo,pts[p]);
				new_edge->AddSource(dst,1);
				sel->Insert(new_edge);	
				new_edge->SetParent(host,idx);

				start = p;
				break;
			}
			else
			{
				src = c = WED_AirportNode::CreateTyped(GetArchive());
				src->SetParent(new_edge,cidx);
				src->SetName(mName.value+"_internal");
				c->SetLocation(gis_Geo,pts[p]);
				++p;
				++cidx;
			}
		}
	}	

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


void WED_CreateEdgeTool::SplitByLine(WED_Thing * host, IGISEntity * ent, const Segment2& splitter, vector<Point2>& out_splits)
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
		case gis_PointSequence:
		case gis_Line:
		case gis_Line_Width:
		case gis_Ring:
		case gis_Chain:			
			if((ps = dynamic_cast<IGISPointSequence*>(e)) != NULL)
			{
				int ss = ps->GetNumSides();
				for(int s = 0; s < ss; ++s)
				{
					Segment2 side;
					Bezier2 bez;
					if(!ps->GetSide(gis_Geo,s,side,bez))
					{
						Point2 x;
						if(splitter.intersect(side,x))
						{
							out_splits.push_back(x);
							ps->SplitSide(x, 0.001);
						}
					}
				}
			}
			break;
		case gis_Composite:
			if((c = dynamic_cast<IGISComposite *>(e)) != NULL)
			{
				for(int n = 0; n < c->GetNumEntities(); ++n)
					SplitByLine(NULL,c->GetNthEntity(n), splitter, out_splits);
			}
		}
	}
	else
	{
		for(int n = 0; n < host->CountChildren(); ++n)
			SplitByLine(host->GetNthChild(n), NULL, splitter, out_splits);
	}
}

