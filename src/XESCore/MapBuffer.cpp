/* 
 * Copyright (c) 2008, Laminar Research.
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

#include "MapBuffer.h"
#include <CGAL/Arr_batched_point_location.h>
#if DEV
	#include "GISTool_Globals.h"
#endif

// Set to 1 to show the raw inset ring per polygon.
#define	SHOW_RAW_RING	0

// Set to put colored points on vertices
#define SHOW_VERTEX_CHOICE 0

/***************************************************************************************************************************************
 * POLYGON TAGGING
 ***************************************************************************************************************************************/

void	TagPolygon(
				const Polygon_2&			in_polygon,
				TaggedPolygon_t&			out_polygon)
{
	out_polygon.clear();
	out_polygon.reserve(in_polygon.size());
	for(int n = 0; n < in_polygon.size(); ++n)
		out_polygon.push_back(Curve_2(in_polygon.edge(n),n));
}
				
void	UntagPolygon(
				const TaggedPolygon_t&		in_polygon,
				Polygon_2&					out_polygon)
{
	out_polygon.clear();
	for(int n = 0; n < in_polygon.size(); ++n)
	{
		#if DEV
			int p = n ? (n - 1) : (n + in_polygon.size() - 1);
			DebugAssert(in_polygon[p].target() == in_polygon[n].source());
		#endif
		out_polygon.push_back(in_polygon[n].source());
	}
}

/***************************************************************************************************************************************
 * NAIVE RING CONSTRUCTION
 ***************************************************************************************************************************************/
/*
	Basically we are going to construct a simple ring around the polygon.  Here's the trick: as the ring crosses itself, parts of the ring
	are "inside" and "outside" the ring - this determination is actually correct when the ring twists in on itself!  So we build the naive 
	ring and then fix it later.
*/


// Trivial construction to move a segment to the left by "dist".
// Probably a better way to do this...not sure what it is.
inline	void	MoveSegLeft_(const Segment_2& l1, double dist, Segment_2& l2, Vector_2& offset_vector)
{
	offset_vector = Vector_2(l1.source(), l1.target()).perpendicular(CGAL::COUNTERCLOCKWISE);
	offset_vector = normalize(offset_vector) * dist;
	l2 = Segment_2(l1.source() + offset_vector, l1.target() + offset_vector);
}


bool IsTriangleInverted(
					const Polygon_2& in_polygon,
					const Polygon_2& new_polygon,					
					const RingInset_t *		in_insets,
					double					in_inset)
{
	Line_2	l0(in_polygon.edge(0));
	Line_2	l1(in_polygon.edge(1));
	Line_2	l2(in_polygon.edge(2));
	
	if(!l2.has_on_positive_side(new_polygon.vertex(0))) return true;
	if(!l0.has_on_positive_side(new_polygon.vertex(1))) return true;
	if(!l1.has_on_positive_side(new_polygon.vertex(2))) return true;
	
	double	dist0 = in_insets ? in_inset * in_insets->at(0) : in_inset;
	double	dist1 = in_insets ? in_inset * in_insets->at(1) : in_inset;
	double	dist2 = in_insets ? in_inset * in_insets->at(2) : in_inset;
	
	dist0 *= dist0;
	dist1 *= dist1;
	dist2 *= dist2;
	
	double	d0= CGAL::to_double(squared_distance(l0,new_polygon.vertex(2)));
	double	d1= CGAL::to_double(squared_distance(l1,new_polygon.vertex(0)));
	double	d2= CGAL::to_double(squared_distance(l2,new_polygon.vertex(1)));
	
	if(d0 < dist0) return true;
	if(d1 < dist1) return true;
	if(d2 < dist2) return true;
	
	return false;
}



// BuildPointSequence
// This builds a naive buffer around a polygon ring.  We then later use winding depth to find the real buffered area.
// The requirements for the input sequence are the same as for the overall buffering API.
static void	BuildPointSequence(
					const Polygon_2&		input_seq,
					const RingInset_t *		in_insets,
					double					in_inset,
					Polygon_2&				out_curves)
{
	// Special case: if wer are buffering outward, simply reverse the CCW of the polygon and inset.  This is so that we don't 
	// have to handle the negative-inset antennas cases!
	if(in_inset < 0.0)
	{
		Polygon_2	rev(input_seq);
		rev.reverse_orientation();
		BuildPointSequence(rev,in_insets,-in_inset, out_curves);
		out_curves.reverse_orientation();
		return;
	}
	
	int n = 0;
	vector<Segment_2>	segments;
	vector<Vector_2>	offsets;
	DebugAssert(input_seq.size() >= 2);
	
	// First we calculate the inset edges of each side of the polygon in a vacuum.
	
	for (n = 0; n < input_seq.size(); ++n)
	{
		Segment_2	edge(input_seq.edge(n)), seg;
		Vector_2	off;
		DebugAssert(edge.target() != edge.source());		
		MoveSegLeft_(edge, (in_insets == NULL) ? in_inset : (in_insets->at(n) * in_inset), seg, off);
		segments.push_back(seg);
		offsets.push_back(off);
	}

	// Now we go through and find each vertex, the intersection of the supporting
	// lines of the edges.  For the very first and last point if not in a polygon,
	// we don't use the intersection, we just find where that segment ends for a nice
	// crips 90 degree angle.
	
	for (n = 0; n < input_seq.size(); ++n)
	{
		int 				prev = n-1;					// Index numbers of the 3 vertices.
		int					next = n+1;
		DebugAssert(prev >= -1);
		DebugAssert(next <= input_seq.size());
		if(prev ==-1)				 prev = input_seq.size()-1;
		if(next == input_seq.size()) next = 0;
		
		int					outgoing = prev;			// Index number of the outgoing and incoming segment for each vertex.
		int					incoming = n;
		
		double				end_cap_inset = in_insets ? in_insets->at(n) * in_inset : in_inset;
		
		// There are going to be a number of cases based on the angle of the previous and next segments.  Remember that we are insetting
		// and CCW, so a right turn is a "reflex" vertex.
		
		if(input_seq[prev] == input_seq[next])
		{
			// CASE 1: ANTENNA (perfect 180 degree turn)
			// The two sides go in exactly opposite directions.  Since the polygon MUST be perfectly noded, we simply detect this
			// because our previous and next vertiecs are exactly the same before offset.
			// In this case, we construct an extra new side at 90 degrees (an end cap) to cap off the antenna.
		
			#if SHOW_VERTEX_CHOICE
				debug_mesh_point(cgal2ben(input_seq[n]),0.2,0.2,1);
			#endif	

			Segment_2	new_side(segments[outgoing].target(), segments[incoming].source()), new_side2;
			Vector_2	off;
			MoveSegLeft_(new_side, end_cap_inset, new_side2, off);

			out_curves.push_back(new_side2.source());
			out_curves.push_back(new_side2.target());
		
		} 
		else if (CGAL::left_turn(input_seq[prev],input_seq[n], input_seq[next]))
		{
			// CASE 2: LEFT TURN
			#if SHOW_VERTEX_CHOICE
				debug_mesh_point(cgal2ben(input_seq[n]),0,1,0);
			#endif			
			// We form two rays, each pointing toward the vertex.  This way if the two segments can in intersect by extending toward 
			// the vertex, we find that intersection.  We do not intersect lines, because for very small line segment and very large insets,
			// the two lines will be on the wrong sides of each other, and an infinite line intersection will be "behind" the vertex (e.g. in the
			// open side).  To keep the topology correct, when the rays do not interset, we insert the segment end points and the vertex itself.  This
			// forms a clcokwise winding of "bogus" geometry - the topology check later makes the clockwise winding into negative space, which just 
			// removes area close to the vertex, which is fine.
			//
			// (This "reverse" ring has the property of being N meters from the edge, so IF it were to crash into
			// another part of the polygon, that part of the polygon would technically not be in the buffered area.)
			//
			// Note that in all likelinhood if the rays do not intersect, the angle _must_ be accute, but we don't need to utilize this fact.
			Ray_2	ray1(segments[outgoing].source(),segments[outgoing].target());
			Ray_2	ray2(segments[incoming].target(),segments[incoming].source());	// Turn around outgoing so it points toward the vertex.
			Point_2	p;
			CGAL::Object r = CGAL::intersection(ray1, ray2);
			if (CGAL::assign(p, r))
				out_curves.push_back(p);											// Found an intersection, use it
			else
			{
				out_curves.push_back(segments[outgoing].target());					// Build the CW winding
				out_curves.push_back(input_seq[n]);
				out_curves.push_back(segments[incoming].source());
			}
		}
		else if (CGAL::right_turn(input_seq[prev],input_seq[n], input_seq[next]))
		{
			// CASE 3: RIGHT TURN (REFLEX VERTEX)
			#if SHOW_VERTEX_CHOICE
				debug_mesh_point(cgal2ben(input_seq[n]),1,0,0);
			#endif					
			// Right turn is a reflex turn.  We are going to build out each edge by its offset, the intersect.  These offsets help make a nice angular 
			// connection - if they don't intersect, we add both, but see below.
			
			// S1 and S2 are like the outgoing and incoming offset segments, but extended on the vertex end a bit.
			Segment_2	s1(segments[outgoing].source(), segments[outgoing].target() + offsets[outgoing].perpendicular(CGAL::CLOCKWISE));
			Segment_2	s2(segments[incoming].source() + offsets[incoming].perpendicular(CGAL::COUNTERCLOCKWISE), segments[incoming].target());

			Point_2	p;
			CGAL::Object r = CGAL::intersection(s1, s2);
			if (CGAL::assign(p, r))
				out_curves.push_back(p);
			else
			{
				// If the angle is less than 90 degrees, the lack of intersection of the segments with "extra end cap" can mean that 
				// connecting them arbitrarily will cause a CCW ring, which is bad - it adds positive space to the boundary, causing
				// art artifacts.  So only join the "extended" segments if we have an acute angle.
				
				// Why does this work?  Well, the BAD case happens when we have introduced a LEFT turn in our ring, which creates the POTENTIAL for a CCW
				// winding (positive space).  This happens if another line in just the right direction cuts off and isolates our left turn.
				
				// But that left turn (from the end of the outgoing to the start of the incoming, to the next segment) can only happen if the reflex angle
				// is obtuse - in the obtuse case, the extension of the outgoing and incoming segments are going approximately in OPPOSITE directions, which
				// can cause this tight left turn.
				
				// Frankly in the case of an obtuse reflex angle where we do NOT have ends meeting, we probably should NOT be trying to add end caps...just
				// connecting the two is fine.
				if(CGAL::angle(input_seq[prev],input_seq[n], input_seq[next]) == CGAL::ACUTE)
				{
					out_curves.push_back(s1.target());
					out_curves.push_back(s2.source());
				}
				else
				{
					out_curves.push_back(segments[outgoing].target());
					out_curves.push_back(segments[incoming].source());
				}
			}
		}
		else
		{
			// CASE 4: PARALLEL LINES
			#if SHOW_VERTEX_CHOICE
				debug_mesh_point(cgal2ben(input_seq[n]),1,1,0);
			#endif							
			// Almost perfectly straight lines.  In this case, we add only one point, but if the offset distances differ, add two to keep the segments connected
			// without changing the segments themselves.
			if(in_insets && in_insets->at(outgoing) != in_insets->at(incoming))
			{
				out_curves.push_back(segments[outgoing].target());
				out_curves.push_back(segments[incoming].source());
			}
			else				
			{
				// Note: in this case technically we don't HAVE to add the vertex - but if we ever want to correlate the resulting sides to the source sides,
				// we would need this vertex to have two output segments for two input segments.
				out_curves.push_back(CGAL::midpoint(segments[incoming].source(), segments[outgoing].target()));
			}			
		}
	}	
}

/***************************************************************************************************************************************
 * DEPTH FINDING
 ***************************************************************************************************************************************/
/*
	Given a non-simple polygon ring inserted into an arrangement, the depth of each face becomes +1 each time we cross a CCW winding and -1
	each time we cross a CW winding.  The unbounded face is 0.  Thus we can determien the "inside" and "outside" of a non-simple polygon...
	every face with positive depth is inside.
*/

static void visit_face(Face_handle f, set<Face_handle>& to_visit, int depth);

// Visit each hal-edge bounding the face "from".  If we haven't touched the adjacent face, propagate the depth.
static void visit_ccb(Face_handle from, set<Face_handle>& to_visit, Pmwx::Ccb_halfedge_circulator stop, int depth)
{
	Pmwx::Ccb_halfedge_circulator circ=stop;
	do {
		Face_handle f = circ->twin()->face();
		if(to_visit.count(f))
		{
			visit_face(f,to_visit,depth
					+ circ->twin()->data().mTransition
					- circ->		data().mTransition);
		}
		#if DEV
		else 
		{
			// Dev only case - confirm that the previously computed depth matches what we would get from this edge - this would
			// detect topology errors!
			DebugAssert(f->data().mTerrainType == depth
									+ circ->twin()->data().mTransition
									- circ->		data().mTransition);			
		}
		#endif
	}
	while (++circ != stop);
}

static void visit_face(Face_handle f, set<Face_handle>& to_visit, int depth)
{
	DebugAssert(to_visit.count(f) == 1);
	to_visit.erase(f);															// Make sure we have not visited before.

	if(!f->is_unbounded())
		DebugAssert(f->holes_begin() == f->holes_end());
		
	f->data().mTerrainType = depth;
	
	if(!f->is_unbounded())
		visit_ccb(f,to_visit,f->outer_ccb(),depth);
	for(Pmwx::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
		visit_ccb(f,to_visit,*h,depth);
}

/***************************************************************************************************************************************
 * POLYGON BUFFERING
 ***************************************************************************************************************************************/

void	BufferPolygon(
				const Polygon_2&			in_polygon,
				const RingInset_t *			in_insets,
				double						in_inset,
				Polygon_set_2&				out_new_polygon)
{
	DebugAssert(in_polygon.size() >= 3 || in_inset < 0.0);

	// Step 1.
	// build the naive ring around the polygon.
	
	Polygon_2		inset_seq;
	TaggedPolygon_t	inset_crv;
	BuildPointSequence(in_polygon, in_insets, in_inset, inset_seq);
	if	(in_polygon.size() == 3 && 
		(in_polygon.is_counterclockwise_oriented() == (in_inset > 0.0)) &&
		IsTriangleInverted(in_polygon,inset_seq, in_insets,in_inset))
	{
		out_new_polygon.clear();
		return;
	}
	
	#if SHOW_RAW_RING
	for(int n = 0; n < inset_seq.size(); ++n)
		debug_mesh_line(
				cgal2ben(inset_seq.edge(n).source()),
				cgal2ben(inset_seq.edge(n).target()),
				1,0,0, 0,1,0);
	#endif
	TagPolygon(inset_seq,inset_crv);

	// Step 2. 
	// Insert the non-simple naive ring into an arrangement.
	Pmwx	arr;	
	CGAL::insert_curves(arr,inset_crv.begin(), inset_crv.end());

	#if DEV
	try {
	#endif

	// We are going to mark the "transition" field of each edge with 1 for each 
	// edge in the same direction of the original polygon that is on this edge.
	// This way we can count how many times we are crossing the boundary.  (In some
	// cases we will get many edges overlapping!)
	for(Pmwx::Edge_iterator he = arr.edges_begin(); he != arr.edges_end(); ++he)
	{
		he->data().mTransition=0;
		he->twin()->data().mTransition=0;
		for(EdgeKey_iterator k = he->curve().data().begin(); k != he->curve().data().end(); ++k)
		{
			Vector_2	curve_dir(inset_crv[*k].source(),inset_crv[*k].target());			
			Point_2		p(he->target()->point() + curve_dir);
			
			// If the original curve and half-edge go in the same direction, that half-edge gets the "count".
			// We cannot use the derived curve because each half-edge holds only one curve - instead we look up
			// our "key" in the source curve, because each half-edge can have many curves.
			// We COULD special-case the 1-key case, but Shark indicates that this isn't that expensive relative to
			// curve insertion!
			if(CGAL::angle(he->source()->point(),he->target()->point(),p) == CGAL::OBTUSE)		he->data().mTransition++;
			else																				he->twin()->data().mTransition++;
		}
	}

	// Step 3.
	// Visit all faces starting at unbounded and propagate depth.
	set<Pmwx::Face_handle>	all_faces;
	for(Pmwx::Face_iterator f = arr.faces_begin(); f != arr.faces_end(); ++f)
		all_faces.insert(f);

	visit_face(arr.unbounded_face(), all_faces, 0);
	DebugAssert(all_faces.empty());
	
	// Faces with depth > 0 are "inside"...this is the buffered area. 
	for(Pmwx::Face_iterator f = arr.faces_begin(); f != arr.faces_end(); ++f)
		f->set_contained(f->data().mTerrainType > 0);
	
	out_new_polygon = arr;	
	
	#if DEV
	} catch(...) {
		for(Pmwx::Edge_iterator e = arr.edges_begin(); e != arr.edges_end(); ++e)
		{
			if(e->data().mTransition > e->twin()->data().mTransition)
			{
			gMeshLines.push_back(pair<Point2,Point3>(cgal2ben(e->source()->point()),Point3(1,0,0)));
			gMeshLines.push_back(pair<Point2,Point3>(cgal2ben(e->target()->point()),Point3(0,1,0)));
			}
			else  if(e->data().mTransition < e->twin()->data().mTransition)
			{
			gMeshLines.push_back(pair<Point2,Point3>(cgal2ben(e->source()->point()),Point3(0,1,0)));
			gMeshLines.push_back(pair<Point2,Point3>(cgal2ben(e->target()->point()),Point3(1,0,0)));
			}
			else
			{
			gMeshLines.push_back(pair<Point2,Point3>(cgal2ben(e->source()->point()),Point3(1,1,0)));
			gMeshLines.push_back(pair<Point2,Point3>(cgal2ben(e->target()->point()),Point3(1,1,0)));
			}
		}
		
		throw;
	}
	#endif
}
				

// BufferPolygonWithHoles
// Same as above, but we are going to also buffer our holes using an opposite dir buffer.  We'll then
// composite together the two polygon sets.
void	BufferPolygonWithHoles(
				const Polygon_with_holes_2&	in_polygon,
				const PolyInset_t *			in_insets,
				double						in_inset,
				Polygon_set_2&				out_new_polygon)
{
	BufferPolygon(in_polygon.outer_boundary(), in_insets ? &in_insets->at(0) : NULL, in_inset, out_new_polygon);
	int n = 1;
	for(Polygon_with_holes_2::Hole_const_iterator h = in_polygon.holes_begin(); h != in_polygon.holes_end(); ++h, ++n)
	{
		Polygon_set_2	new_hole;
		Polygon_2		hh(*h);
		hh.reverse_orientation();
		BufferPolygon(hh, in_insets ? &in_insets->at(n) : NULL, -in_inset, new_hole);		
		out_new_polygon.difference(new_hole);
	}
}

void	ValidateBuffer(
				Pmwx&						arr,
				Face_handle					face,
				Locator&					l,
				Polygon_set_2&				ps)
{
/*
	typedef pair<Point_2, CGAL::Object>              Query_result;
	list<Point_2>       query_points;
	list<Query_result>  results;
	for(Pmwx::Vertex_const_iterator v = ps.arrangement().vertices_begin(); v != ps.arrangement().vertices_end(); ++v)
		query_points.push_back(v->point());

	CGAL::locate (arr, query_points.begin(), query_points.end(),
		  back_inserter (results));
	  
	for(list<Query_result>::iterator i = results.begin(); i != results.end(); ++i)
	{
		Face_const_handle f;
		if(!CGAL::assign(f,i->second))
			DebugAssert("Got a point that is not actually inside the face!");
		if(f != face)
			DebugAssert("Got wrong face.");
	}	
*/
	for(Pmwx::Vertex_const_iterator v = ps.arrangement().vertices_begin(); v != ps.arrangement().vertices_end(); ++v)
	{
		CGAL::Object o = l.locate(v->point());
		Face_const_handle f;
		if(!CGAL::assign(f,o))
		{
			DebugAssert(!"Got a point that is not actually inside the face!");
		}
		if(f != face)
		{
			DebugAssert(!"Got wrong face.");
		}
/*		printf("Pt was: %lf,%lf.\n",CGAL::to_double(v->point().x()),CGAL::to_double(v->point().y()));
		int ctr = 0;
		Pmwx::Ccb_halfedge_const_circulator c, s;
		c=s=f->outer_ccb();
		do {
			++ctr;
		} while(s != ++c);
		printf("Sides: %d\n", ctr);*/
		
		
	}
}
