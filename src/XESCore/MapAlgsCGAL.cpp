/*
 *  MapAlgsCGAL.cpp
 *  SceneryTools
 *
 *  Created by Andrew McGregor on 17/04/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "MapAlgs.h"
#include "ParamDefs.h"
#include "GISUtils.h"
#include <iomanip>
#include "AssertUtils.h"
#include "CompGeomUtils.h"
#include "PolyRasterUtils.h"
#include "DEMDefs.h"
#include "MapAlgsCGAL.h"




#if 0
int SimplifyMap(const Pmwx& ioMap, bool inKillRivers, ProgressFunc func)
{
	vector<Pmwx::Halfedge_handle>	livelist;

	PROGRESS_START(func, 0, 2, "Finding half-edges to keep...")
	int ctr = 0;
	int tot = ioMap.number_of_halfedges();
	fprintf(stderr, "S");
	for (Pmwx::Halfedge_iterator itr = ioMap.halfedges_begin(); itr != ioMap.halfedges_end(); ++ctr)
    {
		PROGRESS_CHECK(func,0,2,"Finding half-edges to keep...",ctr,tot,1000);
		Halfedge_handle h = itr;

		bool	iWet = h->face()->data().IsWater();
		bool	oWet = h->twin()->face()->data().IsWater();
		bool	border = h->face()->is_unbounded() != h->twin()->face()->is_unbounded();
		bool	coastline = iWet != oWet;
		bool	lu_change = h->face()->data().mTerrainType != h->twin()->face()->data().mTerrainType;
		bool	road = !h->data().mSegments.empty();
		/*bool	stuff = h->face()->data().mAreaFeature[0].mFeatType != h->twin()->face()->data().mAreaFeature[0].mFeatType ||
		(h->face()->data().mAreaFeature[0].mFeatType != NO_VALUE &&
		 h->face()->data().mAreaFeature[0].mParams != h->twin()->face()->data().mAreaFeature[0].mParams);
		bool	river = h->data().mParams.find(he_IsRiver) != h->data().mParams.end();
		if (river && (iWet || oWet)) river = false;	// Wipe out rivers that are inside water bodies or coastlines too.
		if (inKillRivers) river = false;*/
		bool must_burn = h->data().mParams.count(he_MustBurn);

		if (!(/*!river && !stuff &&*/ !road && !coastline && !border && !lu_change && !must_burn))
			livelist.push_back(h);

		++itr;
    }
	PROGRESS_DONE(func, 0, 2, "Finding half-edges to keep...");

	tot = livelist.size();
	ctr = 0;

	PROGRESS_START(func, 1,2,"Copying halfedges...");
	for (vector<Pmwx::Halfedge_handle>::iterator iter = livelist.begin();
		 iter != livelist.end(); ++iter, ++ctr)
	{
		PROGRESS_CHECK(func,1,2,"Copying halfedges...",ctr,tot,1000);
		ioMap.remove_edge(*iter);
	}
	PROGRESS_DONE(func,1,2,"Copying halfedges...");
	return livelist.size();
}
#endif
/*
int SimplifyMap(Pmwx& ioMap, bool inKillRivers, ProgressFunc func)
{
	// OPTIMIZE: it would be possible to utilize the inherent 'creation order' in the Pmwx
	// to store the edges-to-die instead of a vector.  However this is probably NOT the major
	// bottleneck in this routine.

	// TODO: it would be nice to pass in a functor to evaluate edges.
	vector<Pmwx::Halfedge_handle>	deadList;

	PROGRESS_START(func, 0, 2, "Finding unneeded half-edges...")
	int ctr = 0;
	int tot = ioMap.number_of_halfedges();

	for (Pmwx::Halfedge_iterator he = ioMap.halfedges_begin();
		 he != ioMap.halfedges_end(); ++he, ++he, ++ctr, ++ctr)
	{
		PROGRESS_CHECK(func,0,2,"Finding unneeded half-edges...",ctr,tot,1000);
		Pmwx::Halfedge_handle h = he;
		if (!h->data().mDominant) h = h->twin();

		bool	iWet = h->face()->data().IsWater();
		bool	oWet = h->twin()->face()->data().IsWater();
		bool	border = h->face()->is_unbounded() != h->twin()->face()->is_unbounded();
		bool	coastline = iWet != oWet;
		bool	lu_change = h->face()->data().mTerrainType != h->twin()->face()->data().mTerrainType;
		bool	road = !h->data().mSegments.empty();
		bool	stuff = h->face()->data().mAreaFeature[0].mFeatType != h->twin()->face()->data().mAreaFeature[0].mFeatType ||
		(h->face()->data().mAreaFeature[0].mFeatType != NO_VALUE &&
		 h->face()->data().mAreaFeature[0].mParams != h->twin()->face()->data().mAreaFeature[0].mParams);
		bool	river = h->data().mParams.find(he_IsRiver) != h->data().mParams.end();
		if (river && (iWet || oWet)) river = false;	// Wipe out rivers that are inside water bodies or coastlines too.
		if (inKillRivers) river = false;
		bool must_burn = h->data().mParams.count(he_MustBurn);

		if (!river && !stuff && !road && !coastline && !border && !lu_change && !must_burn)
			deadList.push_back(he);
	}
	PROGRESS_DONE(func, 0, 2, "Finding unneeded half-edges...");

	tot = deadList.size();
	ctr = 0;

	PROGRESS_START(func, 1,2,"Deleting halfedges...");
	for (vector<Pmwx::Halfedge_handle>::iterator iter = deadList.begin();
		 iter != deadList.end(); ++iter, ++ctr)
	{
		PROGRESS_CHECK(func,1,2,"Deleting halfedges...",ctr,tot,1000);
		ioMap.remove_edge(*iter);
	}
	PROGRESS_DONE(func,1,2,"Deleting halfedges...");
	return deadList.size();
}
*/

/*
double	GetMapFaceAreaMeters(const Face_handle f)
{
	if (f->is_unbounded()) return -1.0;
	Polygon2	outer;
	Pmwx::Ccb_halfedge_circulator	circ = f->outer_ccb();
	Pmwx::Ccb_halfedge_circulator	start = circ;
	do {
		outer.push_back(Point2(CGAL::to_double(circ->source()->point().x()),CGAL::to_double(circ->source()->point().y())));
		++circ;
	} while (circ != start);

	CoordTranslator2 trans;

	CreateTranslatorForPolygon(outer, trans);

	for (int n = 0; n < outer.size(); ++n)
	{
		outer[n] = trans.Forward(outer[n]);
	}

	double me = outer.area();

	for (Pmwx::Hole_iterator h = f->holes_begin(); h != f->holes_end(); ++h)
	{
		Polygon2	ib;
		Pmwx::Ccb_halfedge_circulator	circ(*h);
		Pmwx::Ccb_halfedge_circulator	start = circ;
		do {
			ib.push_back(Point2(CGAL::to_double(circ->source()->point().x()),CGAL::to_double(circ->source()->point().y())));
			++circ;
		} while (circ != start);

		for (int n = 0; n < ib.size(); ++n)
			ib[n] = trans.Forward(ib[n]);

		me += ib.area();
	}
	return me;
}
*/
// Given a segment, move it to the left (based on its directionality) by a distance.
static	void	MoveSegLeft(const Segment2& l1, double dist, Segment2& l2)
{
	Vector2	v = Vector2(l1.p1, l1.p2).perpendicular_ccw();
	v.normalize();
	v *= dist;
	l2 = Segment2(l1.p1 + v, l1.p2 + v);
}


#if 0
void	InsetPolygon2(
					  const Polygon2&				inChain,
					  const double *				inRatios,
					  double						inInset,
					  bool						inIsRing,
					  Polygon2&					outChain,
					  void						(* antennaFunc)(int n, void * ref),
					  void *						ref)
{
	if (!outChain.empty())
		outChain.clear();

	int n = 0;
	vector<Segment2>	segments, orig_segments;

	// First we calculate the inset edges of each side of the polygon.

	for (int n = 0, m = 1; n < inChain.size(); ++n, ++m)
	{
		Segment2	edge(inChain[n], inChain[m % inChain.size()]);
		orig_segments.push_back(edge);
		Segment2	seg;
		MoveSegLeft(edge, (inRatios == NULL) ? inInset : (inRatios[n] * inInset), seg);
		segments.push_back(seg);
	}

	// Now we go through and find each vertex, the intersection of the supporting
	// lines of the edges.  For the very first and last point if not in a polygon,
	// we don't use the intersection, we just find where that segment ends for a nice
	// crips 90 degree angle.

	int num_inserted = 0;
	int last_vertex = segments.size() - 1;

	for (int outgoing_n = 0; outgoing_n < segments.size(); ++outgoing_n)
	{
		// the Nth segment goes from the Nth vertex to the Nth + 1 vertex.
		// Therefore it is the "outgoing" segment.
		int 				incoming_n = outgoing_n - 1;
		if (incoming_n < 0)	incoming_n = last_vertex;

		/* We are going through vertex by vertex and determining the point(s) added
		 * by each pair of sides.  incoming is the first side and outgoing is the second
		 * in a CCW rotation.  There are 5 special cases:
		 *
		 * (1) The first point in a non-ring is determined only by the second side.
		 * (2) the last point in a non-ring is determined only by the first side.
		 * (3) If we have a side that overlaps exactly backward onto itself, we generate two
		 *     points to make a nice square corner around this 'antenna'.  Please note the
		 *     requirement that both sides be the same length!!
		 * (4) If two sides are almost colinear (or are colinear) then the intersection we would
		 *     normally use to find the intersect point will have huge precision problems.  In
		 *     this case we take an approximate point by just treating it as straight and splitting
		 *     the difference.  The inset will be a bit too thin, but only by a fractional amount that
		 *     is close to our precision limits anyway.
		 * (5) If two sides are an outward bend over sixty degrees, the bend would produce a huge jagged
		 *     sharp end.  We "mitre" this end by adding two points to prevent absurdity.
		 *
		 * GENERAL CASE: when all else fails, we inset both sides, and intersect - that's where the inset
		 * polygon turns a corner.
		 *
		 *****/

		if (outgoing_n == 0 && !inIsRing)
		{
			/* CASE 1 */
			// We're the first in a chain.  Outgoing vertex is always right.
			outChain.insert(outChain.end(), segments[outgoing_n].p1);
		}
		else if (outgoing_n == last_vertex && !inIsRing)
		{
			/* CASE 2 */
			// We're the last in a chain.  Incoming vertex is always right
			outChain.insert(outChain.end(), segments[incoming_n].p2);
		}
		else if (orig_segments[incoming_n].p1 == orig_segments[outgoing_n].p2)
		{
			/* CASE 3 */
			// Are the two sides in exactly opposite directions?  Special case...we have to add a vertex.
			// (This is almost always an "antenna" in the data, that's why we have to add the new side, the point of the antenna
			// becomes thick.  Since antennas have equal coordinates, an exact opposite test works.)
			Segment2	new_side(segments[incoming_n].p2, segments[outgoing_n].p1), new_side2;
			MoveSegLeft(new_side, (inRatios != NULL) ? (inRatios[outgoing_n] * inInset) : inInset, new_side2);
			//			new_side2 = new_side;
			outChain.insert(outChain.end(), new_side2.p1);
			outChain.insert(outChain.end(), new_side2.p2);
			if (antennaFunc) antennaFunc(outgoing_n + (num_inserted++), ref);
		} else {

			// These are the intersecting cases - we need a dot product to determine what to do.
			Vector2 v1(segments[incoming_n].p1,segments[incoming_n].p2);
			Vector2 v2(segments[outgoing_n].p1,segments[outgoing_n].p2);
			v1.normalize();
			v2.normalize();
			double dot = v1.dot(v2);

			if (dot > 0.999961923064)
			{
				/* CASE 4 */
				// Our sides are nearly colinear - don't trust intersect!
				outChain.insert(outChain.end(), Segment2(segments[incoming_n].p2, segments[outgoing_n].p1).midpoint());
			}
			else if (dot < -0.5 && !v1.left_turn(v2))
			{
				/* CASE 5 */
				// A sharp outward turn of more than 60 degrees - at this point the intersect point will be over
				// twice the road thickness from the intersect point.  Not good!
				Point2	p1(segments[incoming_n].p2);
				Point2	p2(segments[outgoing_n].p1);
				p1 += (v1 * ((inRatios == NULL) ? 1.0 : inRatios[outgoing_n]) *  inInset);
				p2 += (v2 * ((inRatios == NULL) ? 1.0 : inRatios[outgoing_n]) * -inInset);
				outChain.insert(outChain.end(), p1);
				outChain.insert(outChain.end(), p2);
				if (antennaFunc) antennaFunc(outgoing_n + (num_inserted++), ref);
			}
			else
			{
				/* GENERAL CASE */
				// intersect the supporting line of two segments.
				Line2	line1(segments[incoming_n]);
				Line2	line2(segments[outgoing_n]);
				Point2	p;
				if (line1.intersect(line2, p))
					outChain.insert(outChain.end(), p);
				else
					outChain.insert(outChain.end(), Segment2(segments[incoming_n].p2, segments[outgoing_n].p1).midpoint());
			}
		}
	}
}
#endif

void	CCBToPolygon(Halfedge_handle ccb, Polygon2& outPolygon, vector<double> * road_types, double (* weight_func)(const Halfedge_handle edge), Bbox_2 * outBounds)
{
	if (road_types != NULL) DebugAssert(weight_func != NULL);
	if (road_types == NULL) DebugAssert(weight_func == NULL);

	outPolygon.clear();
	if (road_types) road_types->clear();

	Halfedge_handle iter = ccb, stop = ccb;

	if (outBounds)	(*outBounds) = iter->source()->point().bbox();

	do {
		outPolygon.push_back(Point2(CGAL::to_double(iter->source()->point().x()), CGAL::to_double(iter->source()->point().y())));
		if (outBounds) (*outBounds) = (*outBounds) + iter->source()->point().bbox();
		if (road_types)
			road_types->push_back(weight_func(iter));
		iter = iter->next();
	} while (iter != stop);
}

void	FaceToComplexPolygon(const Face_handle face, vector<Polygon2>& outPolygon, vector<vector<double> > * road_types, double (* weight_func)(const Halfedge_handle edge), Bbox_2 * outBounds)
{
	outPolygon.clear();
	if (road_types)	road_types->clear();
	printf("_");
	if (!face->is_unbounded())
	{
		outPolygon.push_back(Polygon2());
		if (road_types) road_types->push_back(vector<double>());
		CCBToPolygon(face->outer_ccb(), outPolygon.back(), road_types ? &road_types->back() : NULL, weight_func, outBounds);
	}
	printf("_%d ", outPolygon.size());

	for (Pmwx::Hole_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
	{
		outPolygon.push_back(Polygon2());
		if (road_types) road_types->push_back(vector<double>());
		CCBToPolygon(*hole, outPolygon.back(), road_types ? &road_types->back() : NULL, weight_func, NULL);
	}
}
