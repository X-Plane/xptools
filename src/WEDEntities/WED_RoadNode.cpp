//
//  WED_RoadNode.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/15/15.
//
//

#include "GISUtils.h"
#include "WED_RoadEdge.h"
#include "WED_RoadNode.h"

#if ROAD_EDITING
DEFINE_PERSISTENT(WED_RoadNode)
TRIVIAL_COPY(WED_RoadNode,WED_GISPoint)

WED_RoadNode::WED_RoadNode(WED_Archive * a, int i) : WED_GISPoint(a,i)
{
}

WED_RoadNode::~WED_RoadNode()
{
}


void WED_RoadNode::Rotate(GISLayer_t l,const Point2& ctr, double a)
{
	if(a == 0.0) return;
	if(l == gis_Geo)
	{
		Bezier2 b;
		vector<pair<WED_GISEdge*, Vector2> > new_lo_s;
		vector<pair<WED_GISEdge*, Vector2> > new_hi_s;

		set<WED_Thing *> viewers;
		this->GetAllViewers(viewers);

		//check for bez in edges that are viewers ,
		for(auto viewer : viewers)
		{
			if(viewer->GetClass() == WED_RoadEdge::sClass)
			{
				WED_GISEdge * edge = static_cast<WED_GISEdge *>(viewer);
				IGISPoint * gp1 = edge->GetNthPoint(0);
			    IGISPoint * gp2 = edge->GetNthPoint(edge->GetNumPoints()-1);

				if(edge->GetSide(gis_Geo,-1,b))
				{
					if( this == gp1 && b.p1 != b.c1)
					{
						Vector2	v_old_lo = VectorLLToMeters(ctr, Vector2(ctr,b.c1));
						double old_len_lo = sqrt(v_old_lo.squared_length());
						double old_ang_lo = VectorMeters2NorthHeading(ctr,ctr,v_old_lo);
						Vector2	v_new_lo;
						NorthHeading2VectorMeters(ctr, ctr, old_ang_lo + a, v_new_lo);
						v_new_lo.normalize();
						v_new_lo *= old_len_lo;
						v_new_lo = VectorMetersToLL(ctr,v_new_lo);
						new_lo_s.push_back(make_pair(edge,v_new_lo));
					}
					if( this == gp2 && b.p2 != b.c2 )
					{
						Vector2	v_old_hi = VectorLLToMeters(ctr, Vector2(ctr,b.c2));
						double old_len_hi = sqrt(v_old_hi.squared_length());
						double old_ang_hi = VectorMeters2NorthHeading(ctr,ctr,v_old_hi);
						Vector2	v_new_hi;
						NorthHeading2VectorMeters(ctr, ctr, old_ang_hi + a, v_new_hi);
						v_new_hi.normalize();
						v_new_hi *= old_len_hi;
						v_new_hi = VectorMetersToLL(ctr,v_new_hi);
						new_hi_s.push_back(make_pair(edge,v_new_hi));
					}
				}
			}
		}

		// rotate me
		WED_GISPoint::Rotate(l,ctr,a);

		//rotate bez in edges
		for(int i = 0; i < new_lo_s.size(); ++i )
		{
			new_lo_s[i].first->GetSide(gis_Geo,-1,b);
			b.c1 = ctr + new_lo_s[i].second;
			new_lo_s[i].first->SetSideBezier(gis_Geo,b,-1);
		}
		for(int i = 0; i < new_hi_s.size(); ++i )
		{
			new_hi_s[i].first->GetSide(gis_Geo,-1,b);
			b.c2 = ctr + new_hi_s[i].second;
			new_hi_s[i].first->SetSideBezier(gis_Geo,b,-1);
		}
	}
}

#endif
