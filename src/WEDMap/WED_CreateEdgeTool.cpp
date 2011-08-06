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
#include "WED_RoadEdge.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_MapZoomerNew.h"
#include "WED_GISUtils.h"
#include "WED_EnumSystem.h"
#include "STLUtils.h"

#if AIRPORT_ROUTING

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
	mName(this, "Name", SQL_Name("",""),XML_Name("",""), "N"),
	mOneway(tool == create_TaxiRoute ? this : NULL, "Oneway", SQL_Name("",""),XML_Name("",""), 1),
	mRunway(tool == create_TaxiRoute ? this : NULL, "Runway", SQL_Name("",""),XML_Name("",""), ATCRunwayTwoway, atc_rwy_None),
	mHotDepart(tool == create_TaxiRoute ? this : NULL, "Departure", SQL_Name("",""),XML_Name("",""), ATCRunwayOneway,false),
	mHotArrive(tool == create_TaxiRoute ? this : NULL, "Arrival", SQL_Name("",""),XML_Name("",""), ATCRunwayOneway,false),
	mHotILS(tool == create_TaxiRoute ? this : NULL, "ILS", SQL_Name("",""),XML_Name("",""), ATCRunwayOneway,false),

	mLayer(tool == create_Road ? this : NULL, "Layer", SQL_Name("",""),XML_Name("",""), 0, 2),
	mSubtype(tool == create_Road ? this : NULL, "Type", SQL_Name("",""),XML_Name("",""), RoadSubType, road_Highway),
	
	mSlop(this, "Slop", SQL_Name("",""),XML_Name("",""), 10, 2)
{
}

WED_CreateEdgeTool::~WED_CreateEdgeTool()
{
}

struct sort_by_seg_rat {
	sort_by_seg_rat(const Point2& i) : a(i) { }
	Point2	a;
	bool operator()(const pair<IGISPointSequence *, Point2>& p1, const pair<IGISPointSequence *, Point2>& p2) const {
		return a.squared_distance(p1.second) < a.squared_distance(p2.second);
	}
	bool operator()(const Point2& p1, const Point2& p2) const {
		return a.squared_distance(p1) < a.squared_distance(p2);
	}
};

template <class A, class B>
struct compare_second {
	bool operator()(const pair<A,B>& lhs, const pair<A,B>& rhs) const {
		return lhs.second == rhs.second; }
};

static void SortSplits(const Segment2& s, vector<Point2>& splits)
{
	sort(splits.begin(), splits.end(), sort_by_seg_rat(s.p1));
	
	// Nuke dupe pts.  A hack?  NO!  Intentional.  When we GIS-iterate through our hierarchy
	// we pick up all our graph end pts many times - once as nodes, and once as the points making
	// up the pt sequences that is the edges.
	splits.erase(unique(splits.begin(),splits.end()),splits.end());
}

static void SortSplits(const Segment2& s, vector<pair<IGISPointSequence *, Point2 > >& splits)
{
	sort(splits.begin(), splits.end(), sort_by_seg_rat(s.p1));
	splits.erase(unique(splits.begin(),splits.end(), compare_second<IGISPointSequence*,Point2>()),splits.end());
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
	double frame_dist = fabs(GetZoomer()->YPixelToLat(mSlop.value)-GetZoomer()->YPixelToLat(0));

	/************************************************************************************************
	 * FIRST SNAPPING PASS - NODE TO NODE
	 ************************************************************************************************/
	
	// For each node we want to add, we are going to find a nearby existing node - and if we find one, we
	// lock our location to theirs.  This "direct hit" will get consoldiated during create.  (By moving our
	// path first, we don't get false intersections when the user meant to hit end to end.)
	for(int p = 0; p < pts.size(); ++p)
	{
		double	dist=frame_dist*frame_dist;
		WED_Thing * who = NULL;
		FindNear(host, NULL, WED_TaxiRoute::sClass, pts[p],who,dist);
		if (who != NULL)
		{
			IGISPoint * pp = dynamic_cast<IGISPoint *>(who);
			if(pp)
				pp->GetLocation(gis_Geo,pts[p]);
		}	
	}
	
	/************************************************************************************************
	 * SECOND SNAPPING PASS - LOCK NEW PTS TO EXISTING EDGES
	 ************************************************************************************************/
	// Next: we need to see if our ndoes go near existing by existing edges...in that case,
	// split the edges and snap us over.
	for (int p = 0; p < pts.size(); ++p)
	{
		double dist=frame_dist*frame_dist;
		IGISPointSequence * seq = NULL;
		FindNearP2S(host, NULL, WED_TaxiRoute::sClass,pts[p], seq, dist);
		if(seq)
			seq->SplitSide(pts[p], 0.001);		
	}
	
	/************************************************************************************************
	 * THIRD SNAPPING PASS - SPLIT NEW EDGES NEAR TO EXISTING PTS
	 ************************************************************************************************/
	for(int p = 1; p < pts.size(); ++p)
	{
		vector<Point2>	splits;
		SplitByPts(host, NULL, WED_TaxiRoute::sClass, Segment2(pts[p-1],pts[p]), splits,frame_dist*frame_dist);
//		printf("At index %d, got %d splits from pts.\n", p, splits.size());
		SortSplits(Segment2(pts[p-1],pts[p]), splits);

		pts.insert(pts.begin()+p,splits.begin(), splits.end());
		p += splits.size();
		
//		printf("p = %d\n", p);
//		for(int n = 0; n < pts.size(); ++n)
//			printf("    %d = %lf,%lf\n", n,pts[n].x(),pts[n].y());		
	}

	/************************************************************************************************
	 * FOURTH SNAPPING PASS - PRE-INTERSECT LINES WE WILL GO THROUGH
	 ************************************************************************************************/
	// Now that we've snapped all we can, look for real non-end point segment intersections.  Cut the
	// existing segment using "split" and save the exact point.  This way we will have exact hits on
	// nodes later and consolidate.

	for(int p = 1; p < pts.size(); ++p)
	{
		vector<pair<IGISPointSequence *, Point2> >	splits;
		SplitByLine(host, NULL, WED_TaxiRoute::sClass, Segment2(pts[p-1],pts[p]), splits);
		for(vector<pair<IGISPointSequence *, Point2> >::iterator s = splits.begin(); s != splits.end(); ++s)
			s->first->SplitSide(s->second,0.001);
//		printf("At index %d, got %d splits.\n", p, splits.size());
		SortSplits(Segment2(pts[p-1],pts[p]), splits);
		for(vector<pair<IGISPointSequence *, Point2> >::iterator s = splits.begin(); s != splits.end(); ++s)
		{			
			pts.insert(pts.begin()+p,s->second);
			++p;
		}	
		
//		printf("p = %d\n", p);
//		for(int n = 0; n < pts.size(); ++n)
//			printf("    %d = %lf,%lf\n", n,pts[n].x(),pts[n].y());
	}

	/************************************************************************************************
	 *
	 ************************************************************************************************/

	WED_GISEdge *	new_edge = NULL;
	WED_TaxiRoute *	tr = NULL;
	WED_RoadEdge * er = NULL;
	static int n = 0;
	int stop = closed ? pts.size() : pts.size()-1;
	int start = 0;

	WED_GISPoint * c;
	WED_Thing * src = NULL, * dst = NULL;
	double	dist=frame_dist*frame_dist;
	if(src == NULL)	
		FindNear(host, NULL, WED_TaxiRoute::sClass,pts[start % pts.size()],src,dist);
	if(src == NULL)
	{
		src = c = WED_AirportNode::CreateTyped(GetArchive());
		src->SetParent(host,idx);
		src->SetName(mName.value + "_start");
		c->SetLocation(gis_Geo,pts[0]);
	}

	int p = start + 1;
	while(p <= stop)
	{
		switch(mType) {
		case create_TaxiRoute:
			new_edge = tr = WED_TaxiRoute::CreateTyped(GetArchive());
			tr->SetOneway(mOneway.value);			
			tr->SetRunway(mRunway.value);
			tr->SetHotDepart(mHotDepart.value);
			tr->SetHotArrive(mHotArrive.value);
			tr->SetHotILS(mHotILS.value);
			tr->SetName(mName);
			break;
		case create_Road:
			new_edge = er = WED_RoadEdge::CreateTyped(GetArchive());
			er->SetSubtype(mSubtype.value);
			er->SetLayer(mLayer.value);
			er->SetName(mName);
			break;
		}
	
		new_edge->AddSource(src,0);
		dst = NULL;
		
		dist=frame_dist*frame_dist;
		FindNear(host, NULL, WED_TaxiRoute::sClass,pts[p % pts.size()],dst,dist);
		if(dst == NULL)
		{
			switch(mType) {
			case create_TaxiRoute:
				dst = c = WED_AirportNode::CreateTyped(GetArchive());
				break;
			case create_Road:
				dst = c = WED_SimpleBoundaryNode::CreateTyped(GetArchive());
				break;
			}
			dst->SetParent(host,idx);
			dst->SetName(mName.value+"_stop");
			c->SetLocation(gis_Geo,pts[p % pts.size()]);
		}		
		new_edge->AddSource(dst,1);

		// Do this last - half-built edge inserted the world destabilizes accessors.
		new_edge->SetParent(host,idx);
		sel->Insert(new_edge);	
	
//		printf("Added edge %d  from 0x%08x to 0x%08x\n", p, src, dst);
		src = dst;
		++p;
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

void WED_CreateEdgeTool::FindNear(WED_Thing * host, IGISEntity * ent, const char * filter, const Point2& loc, WED_Thing *& out_thing, double& out_dsq)
{
	IGISEntity * e = ent ? ent : dynamic_cast<IGISEntity*>(host);
	WED_Thing * t = host ? host : dynamic_cast<WED_Thing *>(ent);
	WED_Entity * et = t ? dynamic_cast<WED_Entity *>(t) : NULL;
	if(et && et->GetHidden()) return;
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
		case gis_Edge:
		case gis_Chain:
			if(filter == NULL || filter == t->GetClass())
			if((ps = dynamic_cast<IGISPointSequence*>(e)) != NULL)
			{
				for(int n = 0; n < ps->GetNumPoints(); ++n)
					FindNear(NULL,ps->GetNthPoint(n), filter, loc, out_thing, out_dsq);
			}
			break;
		case gis_Composite:
			if((c = dynamic_cast<IGISComposite *>(e)) != NULL)
			{
				for(int n = 0; n < c->GetNumEntities(); ++n)
					FindNear(NULL,c->GetNthEntity(n), filter, loc, out_thing, out_dsq);
			}
		}
	}
	else
	{
		for(int n = 0; n < host->CountChildren(); ++n)
			FindNear(host->GetNthChild(n), NULL, filter, loc, out_thing, out_dsq);
	}
}

void WED_CreateEdgeTool::FindNearP2S(WED_Thing * host, IGISEntity * ent, const char * filter, const Point2& loc, IGISPointSequence *& out_thing, double& out_dsq)
{
	IGISEntity * e = ent ? ent : dynamic_cast<IGISEntity*>(host);
	WED_Thing * t = host ? host : dynamic_cast<WED_Thing *>(ent);
	WED_Entity * et = t ? dynamic_cast<WED_Entity *>(t) : NULL;
	if(et && et->GetHidden()) return;
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
		case gis_Edge:
		case gis_Chain:
			if(filter == NULL || t->GetClass() == filter)
			if((ps = dynamic_cast<IGISPointSequence*>(e)) != NULL)
			{
				int ns = ps->GetNumSides();
				for(int n = 0; n < ns; ++n)
				{
					Bezier2 b;
					Segment2 s;
					if(ps->GetSide(gis_Geo,n,s,b))
					{						
					}
					else					
					{
						if(loc != s.p1 && loc != s.p2)
						{
							double d = s.squared_distance(loc);
							if(d < out_dsq)
							{
								out_dsq = d;
								out_thing = ps;
							}
						}
					}
				}
			}
			break;
		case gis_Composite:
			if((c = dynamic_cast<IGISComposite *>(e)) != NULL)
			{
				for(int n = 0; n < c->GetNumEntities(); ++n)
					FindNearP2S(NULL,c->GetNthEntity(n), filter, loc, out_thing, out_dsq);
			}
		}
	}
	else
	{
		for(int n = 0; n < host->CountChildren(); ++n)
			FindNearP2S(host->GetNthChild(n), NULL, filter, loc, out_thing, out_dsq);
	}
}




void WED_CreateEdgeTool::SplitByLine(WED_Thing * host, IGISEntity * ent, const char* filter, const Segment2& splitter, vector<pair<IGISPointSequence *, Point2> >& out_splits)
{
	IGISEntity * e = ent ? ent : dynamic_cast<IGISEntity*>(host);
	WED_Thing * t = host ? host : dynamic_cast<WED_Thing *>(ent);
	WED_Entity * et = t ? dynamic_cast<WED_Entity *>(t) : NULL;
	if(et && et->GetHidden()) return;
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
		case gis_Edge:
		case gis_Chain:			
			if(filter == NULL || t->GetClass() == filter)
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
						if(splitter.p1 != side.p1 &&
						   splitter.p1 != side.p2 &&
						   splitter.p2 != side.p1 &&
						   splitter.p2 != side.p2)						
						if(splitter.intersect(side,x))
						{
							out_splits.push_back(pair<IGISPointSequence *, Point2>(ps, x));
//							ps->SplitSide(x, 0.001);
						}
					}
				}
			}
			break;
		case gis_Composite:
			if((c = dynamic_cast<IGISComposite *>(e)) != NULL)
			{
				for(int n = 0; n < c->GetNumEntities(); ++n)
					SplitByLine(NULL,c->GetNthEntity(n), filter, splitter, out_splits);
			}
		}
	}
	else
	{
		for(int n = 0; n < host->CountChildren(); ++n)
			SplitByLine(host->GetNthChild(n), NULL, filter, splitter, out_splits);
	}
}


void WED_CreateEdgeTool::SplitByPts(WED_Thing * host, IGISEntity * ent, const char * filter, const Segment2& splitter, vector<Point2>& out_splits, double dsq)
{
	IGISEntity * e = ent ? ent : dynamic_cast<IGISEntity*>(host);
	WED_Thing * t = host ? host : dynamic_cast<WED_Thing *>(ent);
	WED_Entity * et = t ? dynamic_cast<WED_Entity *>(t) : NULL;
	if(et && et->GetHidden()) return;
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
				double my_dist = splitter.squared_distance(l);
				if(my_dist < dsq && splitter.p1 != l && splitter.p2 != l)
				{
					out_splits.push_back(l);
				}
			}
			break;
		case gis_PointSequence:
		case gis_Line:
		case gis_Line_Width:
		case gis_Ring:
		case gis_Edge:
		case gis_Chain:			
			if(filter == NULL || filter == t->GetClass())
			if((ps = dynamic_cast<IGISPointSequence*>(e)) != NULL)
			{
				for(int n = 0; n < ps->GetNumPoints(); ++n)
					SplitByPts(NULL,ps->GetNthPoint(n), filter, splitter, out_splits, dsq);
			}
			break;

			break;
		case gis_Composite:
			if((c = dynamic_cast<IGISComposite *>(e)) != NULL)
			{
				for(int n = 0; n < c->GetNumEntities(); ++n)
					SplitByPts(NULL,c->GetNthEntity(n), filter, splitter, out_splits, dsq);
			}
		}
	}
	else
	{
		for(int n = 0; n < host->CountChildren(); ++n)
			SplitByPts(host->GetNthChild(n), NULL, filter, splitter, out_splits,dsq);
	}
}

void	WED_CreateEdgeTool::GetNthPropertyDict(int n, PropertyDict_t& dict)
{
	dict.clear();
	if(n == PropertyItemNumber(&mRunway))
	{
		WED_Airport * airport = WED_GetCurrentAirport(GetResolver());
		if(airport)
		{
			PropertyDict_t full;
			WED_CreateToolBase::GetNthPropertyDict(n,full);			
			set<int> legal;
			WED_GetAllRunwaysTwoway(airport, legal);
			legal.insert(mRunway.value);
			legal.insert(atc_rwy_None);
			dict.clear();
			for(PropertyDict_t::iterator f = full.begin(); f != full.end(); ++f)
			if(legal.count(f->first))
				dict.insert(PropertyDict_t::value_type(f->first,f->second));
		}
	}
	else if (n == PropertyItemNumber(&mHotDepart) ||
			 n == PropertyItemNumber(&mHotArrive) ||
			 n == PropertyItemNumber(&mHotILS))
	{
		WED_Airport * airport = WED_GetCurrentAirport(GetResolver());
		if(airport)
		{
			PropertyDict_t full;
			WED_CreateToolBase::GetNthPropertyDict(n,full);			
			set<int> legal;
			WED_GetAllRunwaysOneway(airport, legal);
			PropertyVal_t val;
			this->GetNthProperty(n,val);
			DebugAssert(val.prop_kind == prop_EnumSet);
			copy(val.set_val.begin(),val.set_val.end(),set_inserter(legal));
			dict.clear();
			for(PropertyDict_t::iterator f = full.begin(); f != full.end(); ++f)
			if(legal.count(f->first))
				dict.insert(PropertyDict_t::value_type(f->first,f->second));
		}
	}
	else
		WED_CreateToolBase::GetNthPropertyDict(n,dict);			
}


#endif
