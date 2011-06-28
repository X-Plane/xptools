/* 
 * Copyright (c) 2011, Laminar Research.
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

#include "MeshSimplify.h"
#include "MeshAlgs.h"		// for burn predicate
#include "MapHelpers.h"

//#include "GISTool_Globals.h"

inline CDT::Vertex_handle CDT_Recover_Handle(CDT::Vertex *the_vert)
{
	CDT::Face_handle f = the_vert->face();
	for(int n = 0; n < 3; ++n)
	{
		CDT::Vertex_handle v = f->vertex(n);
		if(&*v == the_vert)
			return v;
	}
	DebugAssert(!"Should not be here.");
}

MeshSimplify::MeshSimplify(CDT& in_mesh, mesh_error_f in_err) : mesh(in_mesh), err_f(in_err)
{
}

void MeshSimplify::simplify(double in_max_err)
{
	max_err = in_max_err;
	queue.clear();
	init_q();
//	printf("Q: %d vertices.\n", queue.size());
	
	while(!queue.empty())
	{
		CDT::Vertex_handle v = CDT_Recover_Handle((CDT::Vertex *) queue.begin()->second);
		queue.erase(queue.begin());
		v->info().self = queue.end();
		
		run_vertex(v);		
	}
}

void MeshSimplify::init_q(void)
{
	for(CDT::Finite_vertices_iterator q = mesh.finite_vertices_begin(); q != mesh.finite_vertices_end(); ++q)
	{
		q->info().self = queue.end();
		CDT::Vertex_handle p, r;
		double err;
		if(can_remove_locked(q,p,r))
		if((err = calc_remove_error(p,q,r)) < max_err)
		if(can_remove_topo(p,q,r))
		{
			q->info().self = queue.insert(VertexQueue::value_type(err, &*q));
		}
	}
}

void MeshSimplify::run_vertex(CDT::Vertex_handle q)
{
	// First, collect our neighbors.  
	
	// The platelet theorem: if a squatter x is blocking an ear pqr from being 
	// simplified, if x is the _only_ squatter, q and x _must_ be connected.
	// 
	// Therefore if the removed point q _is_ a squatter, any q in a pqr ear
	// that needs re-evaluating will be attached.  Thus circulating q
	// will find any points that need a re-evaluation.
	
	// Collect the neighbors first; once we remove q, the connectivity to
	// our neighbors will be gone!
	
	list<CDT::Vertex_handle>	neighbors;
	CDT::Vertex_circulator circ, stop;
	circ = stop = q->incident_vertices();
	do {
		neighbors.push_back(circ);
	} while(++circ != stop);
	
//	printf("   vertex has %d neighbors.\n", neighbors.size());
	
	CDT::Vertex_handle p,r;
	
	if(can_remove_locked(q,p,r))
	{
		//debug_mesh_point(cgal2ben(q->point()),1,1,0);
		CDT::Edge pq,qr;
		if(!mesh.is_edge(p,q,pq.first,pq.second))
		{
			Assert(!"Where is pq?");
		}
		if(!mesh.is_edge(q,r,qr.first,qr.second))
		{
			Assert(!"Where is qr?");
		}
		DebugAssert(mesh.is_constrained(pq));
		DebugAssert(mesh.is_constrained(qr));
		mesh.remove_constrained_edge(pq.first,pq.second);
		mesh.remove_constrained_edge(qr.first,qr.second);
		DebugAssert(!mesh.are_there_incident_constraints(q));
		mesh.remove(q);
		
		// DO NOT do this until q is gone!  PQR could be colinear...
		mesh.insert_constraint(p,r);
	}
	else
	{
		DebugAssert("!Why were we in the Q??");
	}
	
	for(list<CDT::Vertex_handle>::iterator n = neighbors.begin(); n != neighbors.end(); ++n)
	{
		update_q(*n);
	}
}

void		MeshSimplify::update_q(CDT::Vertex_handle q)
{
	bool	want_q = false;
	double	err = max_err;
	double	is_queued = q->info().self != queue.end();
	double	old_err = is_queued ? q->info().self->first : max_err;
	CDT::Vertex_handle p, r;
	
	if(can_remove_locked(q,p,r))
	if((err = calc_remove_error(p,q,r)) < max_err)
	if(can_remove_topo(p,q,r))
	{
		want_q = true;
	}
	
	if(want_q != is_queued)
	{
		if(want_q)
		{
			q->info().self = queue.insert(VertexQueue::value_type(err, &*q));
		}
		else
		{
			queue.erase(q->info().self);
			q->info().self = queue.end();
		}
	}
	else if(want_q && old_err != err)
	{
		// Only do the remove and insert thing if the error changed?
		queue.erase(q->info().self);
		q->info().self = queue.insert(VertexQueue::value_type(err, &*q));
	}
}

// This tells us if q can be removed from between p/r without causing a self-intersection.
// Requirements: pq and qr are the only constrained edges into q. 
// No vertices in mesh other than constraint ends.
bool	 MeshSimplify::can_remove_topo(CDT::Vertex_handle p, CDT::Vertex_handle q, CDT::Vertex_handle r)
{
	// The platelet idea: if there are any 'squatters' inside the angle pqr, at least one of them will
	// be DIRECTLY connected to q.  So if we circulate from r to p aronud q and find _no_ points inside
	// triangle pqr (or inside pr) then the triangle pqr is empty and we can simpify pqr to pr.
	
	// We need "QR"'s edge to circulate.

	CDT::Edge	qr;
	if(!mesh.is_edge(r,q,qr.first,qr.second))		// is_edge returns right hand tri, so go backward.
	{
		DebugAssert(!"BAD INPUT.  qr not edge.");
		return false;
	}
	
	// Ben's observation: while the original paper tested BOTH sides of the PQR angle, we really only need to
	// examine the space on the INNER side.  So measure the orientation.  We want to get PQR is a left turn (e.g.
	// counter clockwise from R to P is the space we care about.
	//
	// The original reference algorithm tested the far side of PQR, and the results are wrong - the far platelet
	// perimeter CAN legally cross PR, because PR should be a segment, not a line.  CGAL fixes this with the more
	// expensive "is_simple".
	CGAL::Orientation o = CGAL::orientation(p->point(),q->point(),r->point());
	
	if (o == CGAL::COLLINEAR) return true;						// colinear ? Can always simplify!  done!
	if (o == CGAL::RIGHT_TURN) return can_remove_topo(r,q,p);	// Are we backward?  just run in reverse order.
	
	// okay, so we have this:
	//
	//	P...x
	//	|  . .
	//	| .   .
	//	|.  f  .
	//	Q-------R
	//
	// PQR is a left-hand turn.  F is the face to the LEFT of QR.  "x" is the first vertext o examine that MIGHT be
	// inside the triangle PQR.  (If x == P, there are NO triangles and PQR is a triangle and we are done.
	//
	// Note that determining that PQR is a single face requires circulating Q, which is WORSE time than just starting
	// our scan and early-exiting.
	
	CDT::Face_circulator f = q->incident_faces(qr.first);
	
	while(1)
	{
		CDT::Vertex_handle v = f->vertex(CDT::cw(f->index(q)));
		if(v == p)	
			break;

		if(!CGAL::right_turn(r->point(),p->point(),v->point()))
			return false;
		++f;
	}
	return true;
	

#if 0
	
	// The original paper suggested that we iterate around the platelet of 'q' and make sure that the entire set of sub-contours
	// between p and r are both (1) fully on one side of the PR supporting line and (2) not on the SAME side of the PR supporting line.
	//
	// THIS IS NOT GOOD ENOUGH!!  OBSERVE:
	//
	//	2--1 7 6-5
	//  |  |   | |
	//  |  P   R |
	//  |   \ /  |
	//  |    Q   |
	//  3--------4
	//
	// In this case, one sub-contour is 1-6, the other is point 7.  Sequence 1-6 clearly straddles the line PR, yet simplifying PQR to PR
	// is FINE!  
	//
	// CGAL's code shows a better way: as long as P123456R and R7P are both (1) oriented the same way and (2) simple, we're good.  The non
	// simple test catches REAL intersections between the SEGMENT PR and the rest.  (Intersections of the supporting line outside of 
	// segment PR are of course fine.   If 7 was inside the trianle PQR then the orientation of R7P would be opposite of the other sequence,
	// catching the case that doens't intersect but is "fully inside".
	//
	//

	list<Point_2>	pr, rp;

	// Start with face to left of qr.
	CDT::Face_circulator f = q->incident_faces(qr.first);
	
	rp.push_back(r->point());
	while(1)
	{
		CDT::Vertex_handle v = f->vertex(CDT::cw(f->index(q)));
		if(v == p)	
			break;
		rp.push_back(v->point());
		++f;
	}
	rp.push_back(p->point());

	++f;
	
	pr.push_back(p->point());
	while(1)
	{
		CDT::Vertex_handle v = f->vertex(CDT::cw(f->index(q)));
		if(v == r)	
			break;
		pr.push_back(v->point());			
		++f;
	}
	pr.push_back(r->point());

	CGAL::Orientation o_pr = CGAL::COLLINEAR, o_rp = CGAL::COLLINEAR;
	// One of PR or RP could be zero-length if PQR is a triangle (with PR not constrained of course).
	// But both sequences can't be triangles!
	DebugAssert(pr.size() > 2 || rp.size() > 2);
	if(pr.size() == 2)
	{
		if (CGAL::is_simple_2(rp.begin(),rp.end()))
			return true;
		else
			return false;
	}
	else if(rp.size() == 2)
	{
		if (CGAL::is_simple_2(pr.begin(),pr.end()))
			return true;
		else
			return false;
	}
	else
	{
		if(CGAL::is_simple_2(pr.begin(),pr.end()))
			o_pr = CGAL::orientation_2(pr.begin(),pr.end());
		if(CGAL::is_simple_2(rp.begin(),rp.end()))
			o_rp = CGAL::orientation_2(rp.begin(),rp.end());
		
		return o_pr != CGAL::COLLINEAR && o_rp != CGAL::COLLINEAR && o_pr == o_rp;
	}
#endif	
}

// This tells us if q can be removed from a network topology standpoint - we can't do this
// if q's degree isn't 2.  If we CAN remove, p and r are set to the neighboring vertices.
bool MeshSimplify::can_remove_locked(CDT::Vertex_handle q, CDT::Vertex_handle& p, CDT::Vertex_handle& r)
{
	if(IsEdgeVertex(mesh, q)) return false;
	
	DebugAssert(q->info().orig_vertex != Vertex_handle());
	// First we circulate to find our two neighboring edges.  If we don't have degree two, these don't
	// exist and we can't be removed because we are a "node", not a vertex.
	int count = 0;
	CDT::Edge_circulator circ,stop;
	circ = stop = q->incident_edges();
	do
	{
		if(mesh.is_constrained(*circ))
		{
			CDT::Vertex_handle v = CDT_he_source(*circ);
			if(v == q)		   v = CDT_he_target(*circ);
			if(count == 0)
				p = v;
			else if(count == 1)
				r = v;
			else
				return false;	// More than 2 edges going into a node.
			++count;				
		}
	}
	while(++circ != stop);
	
//	if(count == 0)
//		debug_mesh_point(cgal2ben(q->point()),1,0,0);
	
	DebugAssert(count > 0);		// can't be no-constraint points!!
	if(count < 2) 
		return false;	// we're an antenna.  bail.

	// Confirm that these are REAL nodes, not just subdivisions.  We have to simplify first, then subdivide.
	// If we went in the other orderf, the simplify would undo the subdivide.

	DebugAssert(p->info().orig_vertex != Vertex_handle());
	DebugAssert(r->info().orig_vertex != Vertex_handle());
	
	// There are TWO cases where we cannot remove q EVEN if it seems like a good idea:
	// 1. PQR is a 3-sided loop...it will degenerate if we remove q.  Don't do that!
	
	CDT::Edge pr;
	if(mesh.is_edge(p,r,pr.first,pr.second))
	if(mesh.is_constrained(pr))
		return false;
		
	// 2. P & R are both nodes, and PR is an ambiguous path.  We have to keep q as an 'identifying' vertex!
	Vertex_handle v1 = p->info().orig_vertex;
	Vertex_handle v2 = r->info().orig_vertex;

	// If the degree of the original nodes are 2, then at least one end of the poly-line is not a node.
	// That means our poly-line has at least four verts (including nodes) - we can nuke one and still be
	// unambiguous.
	// And if the degree of the original nodes is 1, that means we're a dead-end on one end, and that 
	// node ONLY can ID us.  (can't have multiple paths to an antenna, duh.)
	// so...only if BOTH of our immediate neighbors are degree 3 or more do we have to cope with ambiguity.
	
	if(degree_with_predicate<Pmwx,must_burn_he>(v1) > 2)
	if(degree_with_predicate<Pmwx,must_burn_he>(v2) > 2)
	if(count_paths_to<Pmwx,must_burn_he>(v1,v2) > 1)							// dual paths and we are a 3-node poly-line, better not simplify more!
		return false;
	
	return true;
}


// Given constrained pairs pq and qr, can we remove q?  This test simply looks at how
// much error we would pick up.
double MeshSimplify::calc_remove_error(CDT::Vertex_handle p, CDT::Vertex_handle q, CDT::Vertex_handle r)
{
	Vertex_handle p_orig = p->info().orig_vertex;
	Vertex_handle q_orig = q->info().orig_vertex;
	Vertex_handle r_orig = r->info().orig_vertex;
	DebugAssert(p_orig != Vertex_handle());
	DebugAssert(q_orig != Vertex_handle());
	DebugAssert(r_orig != Vertex_handle());
	DebugAssert((degree_with_predicate<Pmwx,must_burn_he>(q_orig) == 2));
	
	// First: can I pull Q AT ALL?
	double err = err_f(p_orig->point(),q_orig->point(),r_orig->point());
	if(err >= max_err)
		return err;
	
	// Now walk along all spokes (there are only two) until we hit p or r.  Since we are walking on the original
	// we are re-testing previously nuked pts to see if they are okay with PR as their supporting line.  (Previously
	// they were on one of PQ or QR, which might have less error.)
	Pmwx::Halfedge_around_vertex_circulator circ, stop;
	circ = stop = q_orig->incident_halfedges();
	do {
		Halfedge_handle h = circ->twin();		// Direction AWAY from Q - walk along this until we hit p or r?
		if(must_burn_he(h))
		{
			while(h->target() != p_orig && h->target() != r_orig)
			{
				// If PR as a supporting line fails this point, we're done.
				err = max(err,err_f(p_orig->point(), h->target()->point(), r_orig->point()));
				if(err >= max_err)
					return err;
					
				// Go to the next.  If we didn't run into q or r, we better not hit a y-split
				// because that means we're topologically FUBAR.
				Halfedge_handle o = h;
				h = next_he_pred<Pmwx,must_burn_he>(h);
//				if(h == Halfedge_handle())
//				{
//					debug_mesh_line(cgal2ben(o->source()->point()),
//									cgal2ben(o->target()->point()),0,1,1,1,0,1);
//					debug_mesh_point(cgal2ben(p->point()),1,0,0);
//					debug_mesh_point(cgal2ben(q->point()),0,1,0);
//					debug_mesh_point(cgal2ben(r->point()),0,0,1);
//				}
				DebugAssert(h != Halfedge_handle());
			}
		}
	} while(++circ != stop);
	return err;
}

