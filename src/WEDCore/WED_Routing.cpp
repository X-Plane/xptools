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

#include "WED_Routing.h"
#include "AptRouting.h"
#include "MapDefs.h"
#include "WED_Globals.h"
#include "WED_ToolUtils.h"
#include "AptDefs.h"
#include "AptAlgs.h"
#include "WED_AptIE.h"

void WED_generate_routes(
					const vector<Polygon2>&		in_pavement,
					const vector<Point2>&		in_poi,	
					WED_route_t&				out_route)
{
	Pmwx		pavement;
	cgal_net_t	routing;

	if (!make_map_with_skeleton(in_pavement,in_poi,routing))
		printf("ERROR: we failed to make a layout.\n");

//	make_map_from_polygons(in_pavement, pavement, 0.0001, 0.0001);
	
	for(Pmwx::Edge_iterator e = pavement.edges_begin(); e != pavement.edges_end(); ++e)
	{
//		if(e->face()->contained() != e->twin()->face()->contained())
//			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),1,1,1, 1,1,1);
//		else
//			debug_mesh_line(cgal2ben(e->source()->point()),cgal2ben(e->target()->point()),0.5,0.5,0.5 , 0.5,0.5,0.5);
	}

//	make_cgal_net_for_map(pavement, routing, in_poi);

	printf("We have %d edges and %d nodes.\n", routing.edges.size(), routing.nodes.size());



//	find_all_routes(routing);	
//	routing.purge_unused_edges();
	routing.purge_unused_nodes();	
	
	for(vector<cgal_node_t *>::iterator n = routing.nodes.begin(); n != routing.nodes.end(); ++n)
	{
//		cout << " Node " << *n << " is at " << (*n)->loc << " and has " << (*n)->poi.size() << " pois.\n";
//		debug_mesh_point(cgal2ben((*n)->loc), 1,1,(*n)->poi.empty() ? 0 : 1);
	}
	for(vector<cgal_edge_t *>::iterator e = routing.edges.begin(); e != routing.edges.end(); ++e)
	{
//		debug_mesh_line(cgal2ben((*e)->src->loc),cgal2ben((*e)->dst->loc), 
//			0,0.3,0.1,
//			0,0.3,0.1);
//			1,1,(*e)->used ? 1 : 0,   
//			1,1,(*e)->used ? 1 : 0);
//		cout << " edge from " << (*e)->src << " to " << (*e)->dst << "\n";
	}
}

/*
static void collect_poi(IGISEntity * e, vector<Point2>& poi)
{
	IGISComposite * c;
	WED_Runway * rwy;
	WED_RampPosition * ramp;
	switch(e->GetGISClass()) {
	case gis_Composite:
		if((c = SAFE_CAST(IGISComposite, e)) != NULL)
		{
			int nn = c->GetNumEntities();
			for(int n = 0; n < nn; ++n)
				collect_poi(c->GetNthEntity(n), poi);
		}
		break;
	case gis_Line_Width:
		if((rwy = SAFE_CAST(WED_Runway, e)) != NULL)
		{
			Segment2	s;
			rwy->GetSource()->GetLocation(s.p1);
			rwy->GetTarget()->GetLocation(s.p2);
			
			Point2	e[2] = { s.p1, s.p2 };
			Point2 c;
			double h, l;
			
			Quad_2to1(e, c, h, l);
			
			poi.push_back(s.midpoint(1.0 / l));
			poi.push_back(s.midpoint(1.0 - 1.0 / l));
		}
		break;
	case gis_Point_Heading:
		if((ramp = SAFE_CAST(WED_RampPosition, e)) != NULL)
		{
			Point2 p;
			ramp->GetLocation(p);
			poi.push_back(p);
		}
		break;
	}
}
*/	

void WED_MakeRouting(IResolver * in_resolver)
{
	#if DEV
	gMeshLines.clear();
	gMeshPoints.clear();
	#endif

	WED_Thing	*	wrl = WED_GetWorld(in_resolver);

	AptVector apts;

	AptExportRecursive(wrl, apts);

	for(AptVector::iterator a = apts.begin(); a != apts.end(); ++a)
	{
		vector<Polygon2>	windings;
		GetAptPolygons(*a, 0.00001, windings);
/*
		for(vector<Polygon2>::iterator w = windings.begin(); w != windings.end(); ++w)
		{
			for(int n = 0; n < w->size(); ++n)
			{
				debug_mesh_point(w->at(n),1,1,0);
				Segment2 s(w->side(n));
				debug_mesh_line(s.p1,s.p2,1,0,0,   0, 1, 0);
			}
		}
*/		
		vector<Point2>	poi;
		
		GetAptPOI(&*a, poi);
		WED_route_t r;
		WED_generate_routes(windings, poi,r);
		
		
	}
}
