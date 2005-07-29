#include "Skeleton.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
#include "MapDefs.h"
#include "XESConstants.h"
#include "AssertUtils.h"

/*

	Insetting a polygon - THEORY OF OPERATION
	
	Insetting an arbitary nested set of concave polygons is very complex.  Huge thanks to Fernando Cacciola for basically
	teaching me via a series of emails how do to this right.
	
	This is an explanation based on first principles...it was NOT obvious how I came to this implementation - the
	code has been totally rewritten at least once.
	
	BASICS: TOPOLOGY AND DELETION
	
	The naive approach to insetting a polygon is: move each suportting line for each segment inward, and update the
	vertices based on the intersections of the new supporting lines.  This works well as long as the inset does
	not cross.  But we want a robust algorithm that can handle degenerate cases.
	
	A few observations provide the foundation:
	
	1. Every final segment in the inset polygon is a result of exactly one edge being inset exactly the "inset amount".
	
	2. Some edges have no contribution and must be deleted.  (But no edges are ever created.)
	
	3. The order of the edges may not be the order in the initial polygon.
	
	Therefore what we need to do is a combination of topological rearrangements of who is connected to whom, as well
	as a series of strategic deletes, and when we are done, we have a simpler polygon that, while probably not simple,
	will be simple when inset the candidate amount.
	
	3-D CONCEPTUALIZATION
	
	We will use the 3-d notion of "XYT" space where the polygon exists in the XY plane and the dimension T represents
	the passage of time as the sides move in.  If we were generating a roof from a housing side, T would really be 
	a spatial dimension - rather than saying 'how long before this edge contributes a zero length side to the inset'
	we would say 'how high up the roof does the face of the roof based on this edge go to'.
	
	It is important to understand that we can work in 3-d because we can use 3-d geometric primitives to "solve"
	important problems, rather than try to use 2-d primitives parametrically in time.
	
	EVENTS
	
	The key to correctly making these topological rearrangements (deleting an edge is a certain kind of topological
	event) is to understand the idea of events.
	
	An event is a location in space/time where the topology of the polygon changes.  For an inset, you can think of
	this as 'how long can we inset before something happens, and where does it happen'.  For a roof this would be
	'where are the corners'.
	
	If we imagine the edges of the sides moving through XYT space along a diagonal (sweeping out a roof piece)
	then all events occur at the intersection of 3 supporting planes for each side.  This is no surprise; 3
	points define a plane.
	
	One special case: if two sides are parallel then their planes form one plane and we don't have a clear 
	intersection point.  We can just take the normal to the edge and use that to pick one among the line.
	
	The set of possible events is the intersection of every supporting plane in the polygon with all others.
	It is important to realize that no event can possibly exist outside this set.  Furthermore since sides 
	are not created, this set cannot grow!!  (However this is an upper bound on events.)
	
	The set of actual events is the subset of possible events that make topological sense at the time we hit
	them.  If we take all events in time order, we can check to make sure that the event takes place within
	the finite swept region that the sides trace out.  If it does not, it is a possible but not actual event,
	which is a result of intersecting infinite planes.  (Or finite planes that are too big - remember topological
	events can cut off whole areas of the polygon ruling out events.)
	
	We can define 3 kinds of events:
	
	- Bisector events.  When 3 adjacent convex sides come together, it means the middle side's contribution
		has gone to zero.  We eliminate the middle side and connect the middle side's neighbors directly.
	- Reflex events.  When a reflex vertex (concave vertex) hits another side, it splits it.  We need to split
		the side in two.  (Note that this does not so much create a new side as clone the existing one!)
		We then need to rewire the four involved edges and possibly define sub polygons or merge polygons.
	- Vertex events.  When two or more reflex vertices hit the same point, we have a vertex event.  We 
		do not need to split, but do need to reorder the sides.
		
	STRATEGY
	
	We manage reflex/vertex and bisector events very differently.  For reflex events we generate the full
	set of events up front and then check each one as we encounter it to see if it is topologically actual
	or just possible.
	
	For bisector events, we generate only the actual bisector events up front (by running around the rings
	of the polygons).  Then as we update topology, we destroy and recreate bisector events on the fly.
	
	The goal here is to reduce up-front processing so that small insets won't require huge numbers of
	events.  We pre-build all reflex events because the set of 'revealed' vertex events can be very large
	for a topological change; it's faster to build them up front than to do the deltas.
	
	

*/

#if !DEV
nuke
#endif
#include "WED_Globals.h"

#if DEV
static	Bbox2	sVertexLimit;
#endif

#if DEV
#define DELETED_VERTEX  (SK_Vertex * ) 0xDEADBEEF
#define DELETED_EDGE    (SK_Edge *   ) 0xDEADBEEF
#define DELETED_POLYGON (SK_Polygon *) 0xDEADBEEF
#endif

struct	SK_Vertex;
struct	SK_Edge;
struct	SK_Polygon;
struct	SK_Event;
typedef multimap<double, SK_Event *>		EventMap;
static bool	SK_SafeIntersect(const Plane3& plane1, const Plane3& plane2, const Point3& common_corner, double t, Point3& cross);
static bool	SK_SafeIntersect2(const Plane3& plane1, const Plane3& plane2, const Plane3& plane3, const Point3& common_corner, Point3& cross);
static bool	SK_SafeIntersect3(const Plane3& plane1, const Plane3& plane2, const Plane3& plane3, const Point3& common_corner1, const Point3& common_corner2, Point3& cross);
static void	SK_DestroyEventsForEdge(SK_Edge * the_edge, EventMap& ioMap, bool inReflex);


/* Edge Structure
*
* inset_width is the original inset of the line, for convenience.
* The supporting plane is a 3-d plane in XYT space that contains 
* the plane swept by the line as it is inset.  The normal to this
* plane points "DOWN" (toward negative time) so that intersecting
* the plane with a constant time yields a line whose normal points
* in the same direction as this edge is moving (into the polyogn)/
*
*/
struct	SK_Edge {

	double			inset_width;		//	Original inset width from input app
	Plane3			supporting_plane;	//	This is the supporting plane of the edge in XYT space.
	Segment2		ends;				//	Original end-points - we need this to negate invalid edits.

	SK_Vertex * 	prev;
	SK_Vertex *		next;
	SK_Polygon * 	owner;
	
	set<SK_Event *>	events;
	
#if DEV
	~SK_Edge() { prev = DELETED_VERTEX; next = DELETED_VERTEX; owner = DELETED_POLYGON; }
#endif	
	
};

/* Vertex structure
 * 
 * Vertices may be created as we work, or they may be destroyed.  Vertices have a 
 * location, but it is only of interest when we create the initial polygon.
 *
 */
struct	SK_Vertex {

	Point2		location;		// This is the original location of the vertex as its base time.
	bool		is_reflex;		// True if this is a reflex vertex.
	double		time_base;		// Time of creation
	
	SK_Edge * 	prev;
	SK_Edge *	next;
	SK_Polygon * owner;
	
	void GetLocationAtTime(Point3& p, double t) const { 
		if (!SK_SafeIntersect(prev->supporting_plane, next->supporting_plane, Point3(location.x, location.y, 0), t, p))
			AssertPrintf("Failed intersection.");
	 }
	
#if DEV
	~SK_Vertex() { prev = DELETED_EDGE; next = DELETED_EDGE; owner = DELETED_POLYGON; }
#endif	
	
};

struct	SK_Polygon {
	SK_Polygon *		parent;
	set<SK_Polygon *>	children;
	SK_Edge *			ccb;
	
#if DEV
	~SK_Polygon() { ccb = DELETED_EDGE; parent = DELETED_POLYGON; }
#endif
};


/* Event Struct
 * 
 * An event consists of three edges that come together at the same time in a single point during a sweep.
 * If the event is a reflex event, the flag is set.  It has a ref to itself in the prioriy Q.
 *
 */
struct	SK_Event {
	SK_Edge *			e1;
	SK_Edge *			e2;
	SK_Edge *			e3;		// If we are a reflex event this will be the edge.
	
	Point3				cross;
	bool				reflex_event;

	EventMap::iterator	self_ref;	
	
#if DEV
	~SK_Event() { e1 = DELETED_EDGE; e2 = DELETED_EDGE; e3 = DELETED_EDGE; }
#endif	
	
	
};

/***************************************************************************************************
 * POLYGON UTILITY ROUTINES
 ***************************************************************************************************/
#define COPLANAR_CHECK 0.999

// Save intersection:
// Normally we find a vertex's evolution by crossing the two planes that embed their sides with
// a horizontal plane T = k for time K.  But....if the two sides are parallel, we don't get an
// intersection.  This routine special-cases that particular case.
bool	SK_SafeIntersect(const Plane3& plane1, const Plane3& plane2, const Point3& common_corner, double t, Point3& cross)
{
	Plane3	time_plane(Point3(0,0,t), Vector3(0,0,1));		

	Vector2	p1_dir(plane1.n.dx, plane1.n.dy);
	Vector2	p2_dir(plane2.n.dx, plane2.n.dy);
	
	p1_dir.normalize();
	p2_dir.normalize();

	if (p1_dir.dot(p2_dir) > COPLANAR_CHECK)
	{
		// Special case - planes are coplanar.  We create a crossing plane and go from there.
		
		// This vector is roughly normal to both planes, along the ground.
		Vector2	av_dir(p1_dir.dx + p2_dir.dx, p1_dir.dy + p2_dir.dy);
		av_dir.normalize();
		
		// This is a plane that cuts the two edges in half...the point we are looking for is roughly along here.
		Plane3	cut_plane(common_corner, Vector3(av_dir.dy, -av_dir.dx, 0.0));
		
		// Now things get a bit NASTY: conceivably the two adjacent sides might have DIFFERing slope.  This is a discontinuity and
		// has no good solution.  For now: le hack!
		
		Segment3	s;
		
		if (time_plane.intersect(plane1, cut_plane, s.p1))
		if (time_plane.intersect(plane2, cut_plane, s.p2))
		{
			cross = s.midpoint();
			return true;
		}
		return false;		
	
	} else {
	
		// Default case - just do a 3-plane cross	
		return time_plane.intersect(plane1, plane2, cross);	
	}
}

bool	SK_SafeIntersect2(const Plane3& plane1, const Plane3& plane2, const Plane3& plane3, const Point3& common_corner, Point3& cross)
{
	Vector2	p1_dir(plane1.n.dx, plane1.n.dy);
	Vector2	p2_dir(plane2.n.dx, plane2.n.dy);
	
	p1_dir.normalize();
	p2_dir.normalize();

	if (p1_dir.dot(p2_dir) > COPLANAR_CHECK)
	{
		// Special case - planes are coplanar.  We create a crossing plane and go from there.
		
		// This vector is roughly normal to both planes, along the ground.
		Vector2	av_dir(p1_dir.dx + p2_dir.dx, p1_dir.dy + p2_dir.dy);
		av_dir.normalize();
		
		// This is a plane that cuts the two edges in half...the point we are looking for is roughly along here.
		Plane3	cut_plane(common_corner, Vector3(av_dir.dy, -av_dir.dx, 0.0));
		
		// Now things get a bit NASTY: conceivably the two adjacent sides might have DIFFERing slope.  This is a discontinuity and
		// has no good solution.  For now: le hack!
		
		Segment3	s;
		
		if (plane3.intersect(plane1, cut_plane, s.p1))
		if (plane3.intersect(plane2, cut_plane, s.p2))
		{
			cross = s.midpoint();
			return true;
		}
		return false;		
	
	} else {
	
		// Default case - just do a 3-plane cross	
		return plane3.intersect(plane1, plane2, cross);	
	}	
}

bool	SK_SafeIntersect3(const Plane3& plane1, const Plane3& plane2, const Plane3& plane3, const Point3& common_corner1, const Point3& common_corner2, Point3& cross)
{
	Vector2	p1_dir(plane1.n.dx, plane1.n.dy);
	Vector2	p2_dir(plane2.n.dx, plane2.n.dy);
	Vector2	p3_dir(plane3.n.dx, plane3.n.dy);
	
	p1_dir.normalize();
	p2_dir.normalize();
	p3_dir.normalize();

	bool	flat_left = p1_dir.dot(p2_dir) > COPLANAR_CHECK;
	bool	flat_right = p2_dir.dot(p3_dir) > COPLANAR_CHECK;

	if (flat_left && flat_right) return false;

	if (flat_left)
	{
		// Special case - planes are coplanar.  We create a crossing plane and go from there.
		
		// This vector is roughly normal to both planes, along the ground.
		Vector2	av_dir(p1_dir.dx + p2_dir.dx, p1_dir.dy + p2_dir.dy);
		av_dir.normalize();
		
		// This is a plane that cuts the two edges in half...the point we are looking for is roughly along here.
		Plane3	cut_plane(common_corner1, Vector3(av_dir.dy, -av_dir.dx, 0.0));
		
		// Now things get a bit NASTY: conceivably the two adjacent sides might have DIFFERing slope.  This is a discontinuity and
		// has no good solution.  For now: le hack!
		
		Segment3	s;
		
		if (plane3.intersect(plane1, cut_plane, s.p1))
		if (plane3.intersect(plane2, cut_plane, s.p2))
		{
			cross = s.midpoint();
			return true;
		}
		return false;		
	
	}
	else if (flat_right)
	{
		// Special case - planes are coplanar.  We create a crossing plane and go from there.
		
		// This vector is roughly normal to both planes, along the ground.
		Vector2	av_dir(p2_dir.dx + p3_dir.dx, p2_dir.dy + p3_dir.dy);
		av_dir.normalize();
		
		// This is a plane that cuts the two edges in half...the point we are looking for is roughly along here.
		Plane3	cut_plane(common_corner2, Vector3(av_dir.dy, -av_dir.dx, 0.0));
		
		// Now things get a bit NASTY: conceivably the two adjacent sides might have DIFFERing slope.  This is a discontinuity and
		// has no good solution.  For now: le hack!
		
		Segment3	s;
		
		if (plane1.intersect(plane2, cut_plane, s.p1))
		if (plane1.intersect(plane3, cut_plane, s.p2))
		{
			cross = s.midpoint();
			return true;
		}
		return false;		
	
	} else {
	
		// Default case - just do a 3-plane cross	
		return plane3.intersect(plane1, plane2, cross);	
	}
}


bool	SK_ReflexEventPossible(SK_Event * inEvent)
{
	// The idea here is to evaluate this event to see if it is actually possible.
	// WE may have reflex interference between spatially far-apart but colinear parts of the
	// triangle.  So we need to evaluate our current topology.
	
	SK_Edge * e_this = inEvent->e3;	
	SK_Vertex * v_prev = e_this->prev;
	SK_Vertex * v_next = e_this->next;
	SK_Edge * e_prev = e_this->prev->prev;
	SK_Edge * e_next = e_this->next->next;
	
	// If topologically we're no longer reflex, bail.
	if (e_prev == inEvent->e1 || e_next == inEvent->e1 || e_prev == inEvent->e2 || e_next == inEvent->e2) return false;

	// HACK CITY: our geometry is NOT precise enough to detect reflex events that happen near T = 0 where a vertex interferes
	// with itself.  But this is NEVER a reflex event, so special-case this out.  I'm so evil.

	SK_Vertex * v_reflex = inEvent->e1->next;
	DebugAssert(v_reflex->is_reflex);
	if (v_reflex->location == v_prev->location || v_reflex->location == v_next->location)
		return false;
	
	Point3	p_one_left, p_zero_left(e_this->ends.p1.x, e_this->ends.p1.y, 0);
	Point3	p_one_right, p_zero_right(e_this->ends.p2.x, e_this->ends.p2.y, 0);

	// Speculation: if we can't find out when our prev or next vertex exists in time, we must have a side that is 
	// 180 dgrees opposite us, e.g. a pinch.  In this case NO event should hit us because we collapse!	
	if (!SK_SafeIntersect(e_prev->supporting_plane, e_this->supporting_plane, p_zero_left, inEvent->cross.z, p_one_left))
		return false;
//		AssertPrintf("Safe interect failed.\n");
	if (!SK_SafeIntersect(e_this->supporting_plane, e_next->supporting_plane, p_zero_right, inEvent->cross.z, p_one_right))
		return false;
//		AssertPrintf("Safe interect failed.\n");
	
	Vector3	move_left(p_zero_left, p_one_left);
	Vector3	move_right(p_zero_right, p_one_right);
	Vector3	move_left_flat(move_left);
	Vector3 move_right_flat(move_right);
	move_left_flat.dz = 0.0;
	move_right_flat.dz = 0.0;
	
	Vector3	prev_normal = move_left_flat.cross(move_left);
	Vector3	next_normal = move_right.cross(move_right_flat);
	
	Plane3	prev_plane(p_zero_left, prev_normal);
	Plane3	next_plane(p_zero_right, next_normal);
	
	return prev_plane.on_normal_side(inEvent->cross) &&
		   next_plane.on_normal_side(inEvent->cross);
}

bool	SK_BisectorEventPossible(SK_Event * inEvent)
{
	// The idea here is to evaluate this event to see if it is actually possible.
	// WE may have reflex interference between spatially far-apart but colinear parts of the
	// triangle.  So we need to evaluate our current topology.
	
	SK_Edge * e_this = inEvent->e2;	
	SK_Vertex * v_prev = e_this->prev->prev->prev;
	SK_Vertex * v_next = e_this->next->next->next;
	SK_Edge * e_prev = e_this->prev->prev;
	SK_Edge * e_next = e_this->next->next;
	SK_Edge * e_prevprev = e_this->prev->prev->prev->prev;
	SK_Edge * e_nextnext = e_this->next->next->next->next;

	if (e_next == e_prevprev) return true;
	if (e_prev == e_nextnext) return true;

	DebugAssert(v_prev != v_next);
	if (e_next == e_prev) return false;
	
	Point3	p_one_left, p_zero_left(e_prev->ends.p1.x, e_prev->ends.p1.y, 0);
	Point3	p_one_right, p_zero_right(e_next->ends.p2.x, e_next->ends.p2.y, 0);

	// Speculation: if we can't find out when our prev or next vertex exists in time, we must have a side that is 
	// 180 dgrees opposite us, e.g. a pinch.  In this case NO event should hit us because we collapse!	
	if (!SK_SafeIntersect(e_prevprev->supporting_plane, e_prev->supporting_plane, p_zero_left, inEvent->cross.z, p_one_left))
		return false;
	if (!SK_SafeIntersect(e_next->supporting_plane, e_nextnext->supporting_plane, p_zero_right, inEvent->cross.z, p_one_right))
		return false;
	
	Vector3	move_left(p_zero_left, p_one_left);
	Vector3	move_right(p_zero_right, p_one_right);
	Vector3	move_left_flat(move_left);
	Vector3 move_right_flat(move_right);
	move_left_flat.dz = 0.0;
	move_right_flat.dz = 0.0;
	
	Vector3	prev_normal = move_left_flat.cross(move_left);
	Vector3	next_normal = move_right.cross(move_right_flat);
	
	Plane3	prev_plane(p_zero_left, prev_normal);
	Plane3	next_plane(p_zero_right, next_normal);
	
	return prev_plane.on_normal_side(inEvent->cross) &&
		   next_plane.on_normal_side(inEvent->cross);
}


bool	SK_PointInRing(SK_Edge * ring, SK_Vertex * inVert, double time)
{
	// TODO - is this a problem?  we calculate the ring's shadow at time slice T.
	SK_Edge * stop = ring;
	int cross_counter = 0;
	Point3 vert;
	inVert->GetLocationAtTime(vert, time);
	Point2 inPoint(vert.x, vert.y);
	
	do {
	
		Point3	prev, next;
		
		ring->prev->GetLocationAtTime(prev, time);
		ring->next->GetLocationAtTime(next, time);
	
		Segment2	s(Point2(prev.x, prev.y), Point2(next.x, next.y));

			
		if ((s.p1.x < inPoint.x && inPoint.x < s.p2.x) ||
			(s.p2.x < inPoint.x && inPoint.x < s.p1.x) ||
			(s.p1.x == inPoint.x && s.p1.x < inPoint.x) ||
			(s.p2.x == inPoint.x && s.p2.x < inPoint.x))
		if (inPoint.y > s.y_at_x(inPoint.x))
			++cross_counter;
		ring = ring->next->next;
	} while (ring != stop);
	
	return (cross_counter % 2) == 1;
	
}

static SK_Vertex * SK_SplitEdge(SK_Edge * e0, const Point2& loc, double now)
{
	SK_Vertex * v0 = e0->prev;
	SK_Vertex * v1 = e0->next;
	SK_Vertex * nv = new SK_Vertex;
	SK_Edge * e1 = new SK_Edge;
	
	v0->next = e0;
	e0->prev = v0;
	
	e0->next = nv;
	nv->prev = e0;
	
	nv->next = e1;
	e1->prev = nv;
	
	e1->next = v1;
	v1->prev = e1;

	nv->location = loc;
	nv->time_base = now;
	nv->is_reflex = false;

	e1->inset_width = e0->inset_width;
	e1->supporting_plane = e0->supporting_plane;
	e1->ends = e0->ends;
	nv->owner = e0->owner;
	e1->owner = e0->owner;

	return nv;
}

static void SK_CombinePolys(SK_Polygon * live, SK_Polygon * die)
{
	Assert(live->parent == die->parent || live == die->parent);	
	live->children.insert(die->children.begin(), die->children.end());
	die->parent->children.erase(die);
	SK_Edge * iter, * stop;
	if (die->ccb)
	{
		iter = stop = die->ccb;
		do {
			iter->owner = live;
			iter->next->owner = live;
			iter = iter->next->next;
		} while (iter != stop);
	}
	delete die;
}

static SK_Polygon *	SK_PolygonCreate(
							SK_Polygon *			parent,
							const Polygon2& 		inPolygon, 
							const double * 			inInsets)
{
	SK_Polygon * child = new SK_Polygon;
	if (parent)
		parent->children.insert(child);

	child->parent = parent;	
	SK_Edge * last = NULL;
	SK_Vertex * first = NULL;
	for (int i = 0; i < inPolygon.size(); ++i)
	{
		SK_Vertex * nv = new SK_Vertex;
		nv->time_base = 0.0;
		nv->location = inPolygon[i];
		if (last) {
			nv->prev = last;
			last->next = nv;
		} else 
			first = nv;
		
		SK_Edge * ne = new SK_Edge;
		ne->prev = nv;
		nv->next = ne;
		ne->next = NULL;
		
		ne->inset_width = inInsets[i];

		// Supporting plane calculation - we need to form a plane through this line up a diagonal.  To do this we'll
		// cross a line along the edge of the plane flat along the original poly with a line up the side (from the base to
		// its destination).	
		ne->ends = inPolygon.side(i);
		Segment2	seg(inPolygon.side(i));	
		// This is a point along the base of the polygon.
		Vector3	along_poly(Point3(seg.p1.x, seg.p1.y, 0.0), Point3(seg.p2.x, seg.p2.y, 0.0));		
		// This is the normal in flat 2-d space to the inset line.
		// We will rescale it to be the length of the inset.
		Vector2	inset_flat(Vector2(seg.p1, seg.p2).perpendicular_ccw());
		inset_flat.normalize();
		inset_flat *= inInsets[i];
		// Build a 3-d version that goes up 1 unit in time - this is our final position.  This essentially defines the slope,  or
		// how fast we get there.
		Vector3	inset_amount(inset_flat.dx, inset_flat.dy, 1.0);
		// Now cross them to get our normal.
		Vector3	plane_normal(inset_amount.cross(along_poly));		
		ne->supporting_plane = Plane3(Point3(seg.p1.x, seg.p1.y, 0.0), plane_normal);
		last = ne;
		
		nv->owner = child;
		ne->owner = child;
	}
	
	if (last && first)
	{
		last->next = first;
		first->prev = last;
	}
		
	child->ccb = last;
	
	return child;
}

static SK_Polygon * SK_PolygonCreateFromFace(SK_Polygon * parent, GISFace * face)
{
	DebugAssert(!face->is_unbounded());
	DebugAssert(parent != NULL);
	Polygon2 poly;
	vector<double>	insets;
	
	Pmwx::Ccb_halfedge_circulator circ, stop;
	stop = circ = face->outer_ccb();
	do {
		poly.push_back(circ->source()->point());
		insets.push_back((double) circ->mTransition / (DEG_TO_NM_LAT * NM_TO_MTR));
		++circ;
	} while (stop != circ);
	
	SK_Polygon * ret = SK_PolygonCreate(parent, poly, &*insets.begin());
	
	for (Pmwx::Holes_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
	{
		poly.clear();
		insets.clear();
		stop = circ = *hole;
		do {
			poly.push_back(circ->source()->point());
			insets.push_back((double) circ->mTransition / (DEG_TO_NM_LAT * NM_TO_MTR));
			++circ;
		} while (stop != circ);
		
		SK_PolygonCreate(ret, poly, &*insets.begin());		
	}
	
	return ret;	
}

static void SK_InsetPolyIntoPmwx(SK_Polygon * poly, GISFace * parent, Pmwx& theMap)
{
	if (!poly->children.empty())
		Assert(poly->ccb != NULL);
	
	if (poly->ccb)
	{
		SK_Vertex * iter, * stop;
		iter = stop = poly->ccb->next;
		Polygon2 ring;
		do {
			ring.push_back(iter->location);
			iter = iter->next->next;
		} while (iter != stop);
		
		GISFace * me = theMap.insert_ring(parent, ring);
		
		for (set<SK_Polygon *>::iterator c = poly->children.begin(); c != poly->children.end(); ++c)
			SK_InsetPolyIntoPmwx(*c, me, theMap);
	}
}

static void SK_PolygonToPmwx(SK_Polygon * world, Pmwx& outMap)
{
	DebugAssert(world->ccb == NULL);
	outMap.clear();
	for (set<SK_Polygon *>::iterator c = world->children.begin(); c != world->children.end(); ++c)
		SK_InsetPolyIntoPmwx(*c, outMap.unbounded_face(), outMap);
}

static void SK_PolygonDestroy(SK_Polygon * ioPolygon)
{
	for (set<SK_Polygon *>::iterator c = ioPolygon->children.begin(); c != ioPolygon->children.end(); ++c)
		SK_PolygonDestroy(*c);
	SK_Edge * circ, * stop, * nuke;
	circ = stop = ioPolygon->ccb;
	if (circ)
	{
		do {
			nuke = circ;
			circ = circ->next->next;
			delete nuke->next;
			delete nuke;
		} while (circ != stop);
	}
	delete ioPolygon;
}

#pragma mark -

#if DEV
static void DebugValidatePoly(SK_Polygon * p)
{
	for (set<SK_Polygon*>::iterator c = p->children.begin(); c != p->children.end(); ++c)
	{
		DebugAssert((*c)->parent == p);
		DebugValidatePoly(*c);
	}
	if (p->ccb == NULL)
		return;
	SK_Edge * iter, * stop;
	iter = stop = p->ccb;
	do {
		SK_Vertex * v = iter->next;
		DebugAssert(iter->next->prev == iter);
		DebugAssert(iter->prev->next == iter);
		DebugAssert(v->next->prev == v);
		DebugAssert(v->prev->next == v);
		DebugAssert(v->owner == p);
		DebugAssert(iter->owner == p);
		
		if (!sVertexLimit.empty())
			DebugAssert(sVertexLimit.contains(v->location));
		
		iter = iter->next->next;
		
		SK_Edge * e = v->next;
		for (set<SK_Event*>::iterator evt = e->events.begin(); evt != e->events.end(); ++evt)
		{
			DebugAssert((*evt)->e1 == e || (*evt)->e2 == e || (*evt)->e3 == e);
		}
		
	} while (iter != stop);
}

static void DebugValidateEventMap(const EventMap& eventMap)
{
	for (EventMap::const_iterator evt = eventMap.begin(); evt != eventMap.end(); ++evt)
	{
		DebugAssert(evt->second->e1 != DELETED_EDGE);
		DebugAssert(evt->second->e2 != DELETED_EDGE);
		DebugAssert(evt->second->e3 != DELETED_EDGE);
		DebugAssert(evt->second->e1->next != DELETED_VERTEX);
		DebugAssert(evt->second->e2->next != DELETED_VERTEX);
		DebugAssert(evt->second->e3->next != DELETED_VERTEX);
		
		DebugAssert(evt->second->e1->events.count(evt->second) == 1);
		DebugAssert(evt->second->e2->events.count(evt->second) == 1);
		DebugAssert(evt->second->e3->events.count(evt->second) == 1);
	}
}

#endif

/***************************************************************************************************
 * ADVANCED MANIPULATORS
 ***************************************************************************************************/
#pragma mark -

/* Find all antennas in the polygon and create a zero length side forming a square edge so that 
 * we can extrude. */
static void SK_PolygonSplitAntennas(SK_Polygon * ioPolygon)
{
	for (set<SK_Polygon *>::iterator c = ioPolygon->children.begin(); c != ioPolygon->children.end(); ++c)
		SK_PolygonSplitAntennas(*c);
	
	SK_Vertex * iter, * stop;
	if (ioPolygon->ccb)
	{
		iter = stop = ioPolygon->ccb->next;
		do {
			if (iter->prev->prev->location == iter->next->next->location)
			{
				iter->is_reflex = true;
				double	d_prev = iter->prev->inset_width;
				double	d_next = iter->next->inset_width;
				
				Vector2	dir(iter->prev->prev->location, iter->location);
				dir.normalize();

				SK_Edge * ne = new SK_Edge;
				SK_Vertex * vc = new SK_Vertex;
				vc->time_base = iter->time_base;
				ne->owner = ioPolygon;
				vc->owner = ioPolygon;
				vc->location = iter->location;

				ne->inset_width = (0.5 * (d_prev + d_next));
				
				Vector2	inset_flat(iter->prev->prev->location, iter->location);				
				Vector3	along_poly(inset_flat.dy, -inset_flat.dx, 0.0);
				inset_flat.normalize();
				inset_flat *= ne->inset_width;
				Vector3	inset_amount(inset_flat.dx, inset_flat.dy, 1.0);
				Vector3	plane_normal(inset_amount.cross(along_poly));		
				ne->supporting_plane = Plane3(Point3(iter->location.x, iter->location.y, 0.0), plane_normal);
				ne->ends.p1 = ne->ends.p2 = iter->location;
					
				vc->next = iter->next;
				vc->prev = ne;
				ne->next = vc;
				ne->prev = iter;
				iter->next->prev = vc;
				iter->next = ne;
				iter = vc;
				vc->is_reflex = true;			
			}
		
			iter = iter->next->next;
		} while (iter != stop);
	}
}

/* Find all very sharp reflex vertices and mitre with a side similar to an antenna, to prevent insanely sharp vertices. */
static void SK_PolygonMitreReflexVertices(SK_Polygon * poly)
{
	for (set<SK_Polygon *>::iterator c = poly->children.begin(); c != poly->children.end(); ++c)
		SK_PolygonMitreReflexVertices(*c);
	
	if (poly->ccb)
	{
		SK_Vertex * iter, * stop;
		iter = stop = poly->ccb->next;
		do {
			Vector2	v1(iter->prev->supporting_plane.n.dy, -iter->prev->supporting_plane.n.dx);
			Vector2	v2(iter->next->supporting_plane.n.dy, -iter->next->supporting_plane.n.dx);
			if ((v1.dx != 0.0 || v1.dy != 0.0) && (v2.dx != 0.0 || v2.dy != 0.0))
			{
				if (v1.right_turn(v2))
				{
					v1.normalize();
					v2.normalize();
					
					if (v1.dot(v2) < -0.5)
					{
						DebugAssert(iter->is_reflex);
						double	d_prev = iter->prev->inset_width;
						double	d_next = iter->next->inset_width;
						
						Vector2	dir(v1.dx - v2.dx, v1.dy - v2.dy);
						dir.normalize();
						
						SK_Edge * ne = new SK_Edge;
						SK_Vertex * vc = new SK_Vertex;
						vc->time_base = iter->time_base;
						ne->owner = poly;
						vc->owner = poly;
						
						ne->inset_width = 0.5 * (d_prev + d_next);
						vc->location = iter->location;
						
						Vector2	inset_flat(dir);
						Vector3	along_poly(inset_flat.dy, -inset_flat.dx, 0.0);
						inset_flat.normalize();
						inset_flat *= ne->inset_width;
						Vector3	inset_amount(inset_flat.dx, inset_flat.dy, 1.0);
						Vector3	plane_normal(inset_amount.cross(along_poly));		
						ne->supporting_plane = Plane3(Point3(iter->location.x, iter->location.y, 0.0), plane_normal);

						ne->ends.p1 = ne->ends.p2 = iter->location;

						vc->next = iter->next;
						vc->prev = ne;
						ne->next = vc;
						ne->prev = iter;
						iter->next->prev = vc;
						iter->next = ne;
						iter = vc;
						vc->is_reflex = true;								
					}
				}
			}
			
			iter = iter->next->next;
		} while (iter != stop);		
	}
}

/* Reset all vertices in the polygon to a given time. */
static void SK_AdvanceVertices(SK_Polygon * poly, double advance_time)
{
	for (set<SK_Polygon *>::iterator c = poly->children.begin(); c != poly->children.end(); ++c)
		SK_AdvanceVertices(*c, advance_time);

	SK_Vertex * iter, * stop;
	if (poly->ccb)
	{
		iter = stop = poly->ccb->next;
		do {
			Point3		i;
			if (!SK_SafeIntersect(iter->prev->supporting_plane,iter->next->supporting_plane, Point3(iter->location.x, iter->location.y, 0), advance_time, i))
				AssertPrintf("Time intercept failed.\n");
			else
				iter->location = Point2(i.x, i.y);
			iter->time_base = advance_time;
			iter = iter->next->next;
		} while (iter != stop);
	}
}

/* Delete any empty polygons. */
static void SK_RemoveEmptyPolygons(SK_Polygon * who, EventMap& ioMap)
{
	for (set<SK_Polygon *>::iterator c = who->children.begin(); c != who->children.end(); )
	{
		set<SK_Polygon *>::iterator i(c);
		++c;
		SK_RemoveEmptyPolygons(*i, ioMap);
	}
	
	if (who->parent && who->ccb)
	{
		if (who->ccb->next->next == who->ccb->prev->prev)
		{
			SK_DestroyEventsForEdge(who->ccb, ioMap, true);
			SK_DestroyEventsForEdge(who->ccb, ioMap, false);
			SK_DestroyEventsForEdge(who->ccb->next->next, ioMap, true);
			SK_DestroyEventsForEdge(who->ccb->next->next, ioMap, false);
			DebugAssert(who->children.empty());
			who->parent->children.erase(who);
			SK_PolygonDestroy(who);
		}
	} else if (who->parent)
	{
		if (who->children.empty())
		{
			who->parent->children.erase(who);
			SK_PolygonDestroy(who);
		}
	}
}

/* Evaluate all vertices and mark reflex vertices. */
static void SK_PolygonMarkReflexVertices(SK_Polygon * poly)
{
	for (set<SK_Polygon *>::iterator c = poly->children.begin(); c != poly->children.end(); ++c)
		SK_PolygonMarkReflexVertices(*c);
	
	if (poly->ccb)
	{
		SK_Vertex * iter, * stop;
		iter = stop = poly->ccb->next;
		do {
			iter->is_reflex = false;
			
			Vector2	v1(iter->prev->supporting_plane.n.dy, -iter->prev->supporting_plane.n.dx);
			Vector2	v2(iter->next->supporting_plane.n.dy, -iter->next->supporting_plane.n.dx);
			if ((v1.dx != 0.0 || v1.dy != 0.0) && (v2.dx != 0.0 || v2.dy != 0.0))
			{
				if (v1.right_turn(v2))
				{
					iter->is_reflex = true;
				}
			}
			
			iter = iter->next->next;
		} while (iter != stop);		
	}
}


/***************************************************************************************************
 * INTERSECTION CALCULATIONS
 ***************************************************************************************************/
#pragma mark -

/* This routine checks for an event at or after the time and creates it, queueing it as needed. */
static SK_Event * SK_CheckCreateEvent(SK_Edge * a, SK_Edge * b, SK_Edge * c, double time_min, bool is_reflex, EventMap& ioMap)
{
#if DEV
	DebugAssert(a->next->next == b);
	DebugAssert(b->prev->prev == a);	
	DebugAssert((b->next->next == c) != is_reflex);
	DebugAssert((c->prev->prev == b) != is_reflex);
#endif
	
	Point3	cross, loc_ab(b->prev->location.x, b->prev->location.y, 0), loc_bc(b->next->location.x, b->next->location.y, 0);
	
	if (( is_reflex && SK_SafeIntersect2(a->supporting_plane, b->supporting_plane, c->supporting_plane, loc_ab, cross)) ||
		(!is_reflex && SK_SafeIntersect3(a->supporting_plane, b->supporting_plane, c->supporting_plane, loc_ab, loc_bc, cross)))
	{
		if (cross.z > time_min)
		if (cross.z <= 1.0)
		// Reflex filter: for non-reflex events, at least one event must not be reflex.  This is because we know that two reflexes will be "diverging".
		if (is_reflex || !b->prev->is_reflex || !b->next->is_reflex)
		{
			SK_Event * e = new SK_Event;
			
			e->e1 = a;
			e->e2 = b;
			e->e3 = c;
			e->cross = cross;
			e->reflex_event = is_reflex;
			
			a->events.insert(e);
			b->events.insert(e);
			c->events.insert(e);
			
			e->self_ref = ioMap.insert(EventMap::value_type(cross.z, e));
#if !DEV
nuke
#endif			
			gMeshPoints.push_back(Point2(cross.x, cross.y));
			if (is_reflex)
			{
				gMeshLines.push_back(Point2(c->prev->location.x, c->prev->location.y));
				gMeshLines.push_back(Point2(c->next->location.x, c->next->location.y));
			}
			return e;
		}
	}
	return NULL;
}

/* This routine tears down the doubly linked structure between the event Q,
 * the event, and the edges, removing one event and destroying it. */
static void	SK_DestroyEvent(SK_Event * the_event, EventMap& ioMap)
{
	DebugAssert(the_event->e1->events.count(the_event) == 1);
	DebugAssert(the_event->e2->events.count(the_event) == 1);
	DebugAssert(the_event->e3->events.count(the_event) == 1);
	ioMap.erase(the_event->self_ref);
	the_event->e1->events.erase(the_event);
	the_event->e2->events.erase(the_event);
	the_event->e3->events.erase(the_event);
	delete the_event;
}

/* This routine tears down all events that relate to a single edge.  Use this
 * before nuking an edge to make sure that it is no longer part of the queue. */
static void	SK_DestroyEventsForEdge(SK_Edge * the_edge, EventMap& ioMap, bool inReflex)
{
	set<SK_Event *> reflex_events;
	set<SK_Event *>::iterator eventIter;
	for (eventIter = the_edge->events.begin(); eventIter != the_edge->events.end(); ++eventIter)
		if ((*eventIter)->reflex_event == inReflex)
			reflex_events.insert(*eventIter);

	for (eventIter = reflex_events.begin(); eventIter != reflex_events.end(); ++eventIter)
	{
		SK_DestroyEvent(*eventIter,  ioMap);
	}
}

/* This creates all bisector events for a polygon world set. */
static void SK_CreateVertexEventsForPolygon(SK_Polygon * poly, double min_time, EventMap& ioMap)
{
	for (set<SK_Polygon *>::iterator c = poly->children.begin(); c != poly->children.end(); ++c)
		SK_CreateVertexEventsForPolygon(*c, min_time, ioMap);

	SK_Edge * iter, * stop;
	if (poly->ccb)
	{
		iter = stop = poly->ccb;
		do {
			SK_CheckCreateEvent(iter->prev->prev, iter, iter->next->next, min_time, false, ioMap);
			iter = iter->next->next;
		} while (iter != stop);
	}
}

/* This checks a reflex vertex against a polygon. */
static void SK_CreateReflexEventsForVertexAndPolygon(SK_Polygon * poly, SK_Vertex * vert, double min_time, EventMap& ioMap)
{
	for (set<SK_Polygon *>::iterator c = poly->children.begin(); c != poly->children.end(); ++c)
		SK_CreateReflexEventsForVertexAndPolygon(*c, vert, min_time, ioMap);

	SK_Edge * iter, * stop;
	if (poly->ccb)
	{
		iter = stop = poly->ccb;
		do {
			if (iter != vert->prev &&
				iter != vert->next &&
				iter != vert->prev->prev->prev &&
				iter != vert->next->next->next)
			{		
				SK_CheckCreateEvent(vert->prev, vert->next, iter, min_time, true, ioMap);
			}
			iter = iter->next->next;
		} while (iter != stop);
	}
}

/* This creates all reflex events for a polygon. */
static void SK_CreateReflexEventsForPolygon(SK_Polygon * poly, SK_Polygon * world, double min_time, EventMap& ioMap)
{
	for (set<SK_Polygon *>::iterator c = poly->children.begin(); c != poly->children.end(); ++c)
		SK_CreateReflexEventsForPolygon(*c, world, min_time, ioMap);

	SK_Vertex * iter, * stop;
	if (poly->ccb)
	{
		iter = stop = poly->ccb->next;
		do {
			if (iter->is_reflex)
				SK_CreateReflexEventsForVertexAndPolygon(world, iter, min_time, ioMap);
			iter = iter->next->next;
		} while (iter != stop);
	}
}


/***************************************************************************************************
 * MAIN ALGORITHM
 ***************************************************************************************************/
#pragma mark -




bool	SK_InsetPolygon(
					GISFace *				inPolygon,
					Pmwx&					outMap,
					int						inTerrainIn,
					int						inTerrainOut,
					int						steps)
{
#if !DEV
delete
#endif
	gMeshPoints.clear();
	gMeshLines.clear();

#if DEV
	{
		sVertexLimit = inPolygon->outer_ccb()->target()->point();
		Pmwx::Ccb_halfedge_circulator ec, es ;
		es = ec = inPolygon->outer_ccb();
		do {
			sVertexLimit += ec->target()->point();
			++ec;
		} while (ec != es);
	}
#endif

	SK_Polygon *	world = SK_PolygonCreate(NULL, Polygon2(), NULL);
	SK_Polygon *	poly = SK_PolygonCreateFromFace(world, inPolygon);

	// Do one-time prep: split antennas, mitre sharp corners and then find any
	// reflex vertices.
	SK_PolygonSplitAntennas(world);
//	SK_PolygonMitreReflexVertices(world);
#if !DEV
fix
#endif
	SK_PolygonMarkReflexVertices(world);

#if DEV	
		DebugValidatePoly(world);
#endif		

		EventMap	events;
	
	// Build up our original events
	
	SK_CreateVertexEventsForPolygon(world, 0.0, events);
	SK_CreateReflexEventsForPolygon(world, world, 0.0, events);

	double base_time = 0.0;
	while (!events.empty() && (base_time = events.begin()->first) <= 1.0 && --steps)
	{
#if DEV
		DebugValidateEventMap(events);
#endif	
		SK_Event * evt = events.begin()->second;
		bool made_change = false;
#if DEV
				
		gMeshLines.clear();
		gMeshLines.push_back(evt->e1->prev->location);
		gMeshLines.push_back(evt->e1->next->location);
		gMeshLines.push_back(evt->e2->prev->location);
		gMeshLines.push_back(evt->e2->next->location);
		gMeshLines.push_back(evt->e3->prev->location);
		gMeshLines.push_back(evt->e3->next->location);
		

#endif
		if (evt->reflex_event)
		{
			if (!SK_ReflexEventPossible(evt))
			{
				SK_DestroyEvent(evt, events);
				
			} else {

				////////////////////////////////////////////

				Point3 our_cross = evt->cross;		
				EventMap::iterator evtIter = events.begin();
				++evtIter;
				while (evtIter != events.end() && evtIter->first == base_time)
				{
					if (our_cross == evtIter->second->cross)
					if (evtIter->second->reflex_event && SK_ReflexEventPossible(evtIter->second))
						AssertPrintf("We do not yet handle multiple simultaneous vertex events!");
					if (our_cross == evtIter->second->cross)
					if (!evtIter->second->reflex_event && SK_BisectorEventPossible(evtIter->second))
						AssertPrintf("Unexpected bisector+reflex collision!");
					++evtIter;
				}

				////////////////////////////////////////////
				
		
				// The third edge is split by the reflex vertex of the first two.

				SK_Vertex * nv = SK_SplitEdge(evt->e3, Point2(evt->cross.x, evt->cross.y), evt->cross.z);
				SK_Vertex * ov = evt->e1->next;
				
				SK_Edge * el = nv->prev;
				SK_Edge * er = nv->next;
				
				SK_Edge * vl = evt->e1;
				SK_Edge * vr = evt->e2;
				
				SK_Polygon * eo = el->owner;
				SK_Polygon * vo = vl->owner;
				
				DebugAssert(el->next->next == er);
				DebugAssert(er->prev->prev == el);
				DebugAssert(vl->next->next == vr);
				DebugAssert(vr->prev->prev == vl);
				DebugAssert(ov->prev == vl);
				DebugAssert(ov->next == vr);
				
				// TOPOLOGICAL UPDATE
				
				if (eo == vo)
				{
					// TOPOLOGICAL SPLIT CASE - Since we have ony polygon, the reflex vertex servers it into two.
					// In this case we repatch the vertices first.
					
					swap(ov->next->prev, nv->next->prev);
					swap(ov->next, nv->next);

					SK_Polygon * op = eo;
					SK_Polygon * np = new SK_Polygon;
					op->ccb = vl;
					np->ccb = vr;
					np->parent = op->parent;
					np->parent->children.insert(np);
					
					SK_Edge * iter, * stop;
					iter = stop = np->ccb;
					do {
						iter->owner = np;
						iter->next->owner = np;
						iter = iter->next->next;
					} while (iter != stop);
					
					// MIGRATE CHILDREN
					
					set<SK_Polygon *>	moveTo;
					for (set<SK_Polygon *>::iterator c = op->children.begin(); c != op->children.end(); ++c)
					{
						if (SK_PointInRing(np->ccb, (*c)->ccb->next, base_time))
							moveTo.insert(*c);
					}
					for (set<SK_Polygon *>::iterator m = moveTo.begin(); m != moveTo.end(); ++m)
					{
						DebugAssert((*m)->parent == op);
						np->children.insert(*m);
						(*m)->parent = np;
						op->children.erase(*m);
					}
						
				} else {
				
					// TOPOLOGICAL MERGE CASE - Since we have two different polygons, some kind of merge is happening.
					
					// CASE 1 - edge is part of a child of reflex vertice's polygon
					if (eo->parent == vo)
					{
						SK_CombinePolys(vo, eo);
					} 
					// CASE 2 - reflex vertex is a child of edge's polygon
					else if (vo->parent == eo)
					{
						SK_CombinePolys(eo, vo);
					}
					// CASE 3 - both polygons are sibblings.
					else if (vo->parent == eo->parent)
					{
						SK_CombinePolys(eo, vo);
					} 
					else 			
					{
						AssertPrintf("We have a topolgoical merge event but the topology is not recognized.");
					}
					
					// Only now do we update the structure - preserving the rings was useful before.
					swap(ov->next->prev, nv->next->prev);
					swap(ov->next, nv->next);
					
				}
							
				// EVENT UPDATING
				// We need to go through and clone reflex events that reference EL to also reference ER.
				// Later on SK_ReflexEventPossible will filter one out.  This makes sense; when we have two
				// parallel edges they both make reflex events, one of which is bogus.
				
				// Another migration is needed:
				// For any events that have EL as the "second" event in the reflex vertex,
				// we need to migrate to ER.
				

				set<SK_Event *>	clones;
				EventMap::iterator nextEvt = events.begin();
				++nextEvt;
				for (; nextEvt != events.end(); ++nextEvt)
				{
					if (nextEvt->second->reflex_event && nextEvt->second->e3 == el)
						clones.insert(nextEvt->second);
					
					if (nextEvt->second->reflex_event && nextEvt->second->e1 == el)
					{
						el->events.erase(nextEvt->second);
						nextEvt->second->e1 = er;
						er->events.insert(nextEvt->second);
					}
				}
				
				for (set<SK_Event *>::iterator citer = clones.begin(); citer != clones.end(); ++citer)
				{
					SK_Event * clone = new SK_Event(**citer);
					clone->e3 = er;
					clone->e1->events.insert(clone);
					clone->e2->events.insert(clone);
					clone->e3->events.insert(clone);
					clone->self_ref = events.insert(EventMap::value_type(clone->cross.z, clone));
				}
				
#if DEV
				DebugValidateEventMap(events);
#endif				
				
				// We also need to dump vertex events that reference EL and rebuild then for EL and ER.
				// Same goes for all sides actually!
				
				SK_DestroyEventsForEdge(el, events, false);
				DebugValidateEventMap(events);
				SK_DestroyEventsForEdge(er, events, false);
				DebugValidateEventMap(events);
				SK_DestroyEventsForEdge(vl, events, false);
				DebugValidateEventMap(events);
				SK_DestroyEventsForEdge(vr, events, false);
				DebugValidateEventMap(events);
				
				SK_CheckCreateEvent(el->prev->prev->prev->prev, el->prev->prev, el, base_time, false, events);
				DebugValidateEventMap(events);
				SK_CheckCreateEvent(el->prev->prev, el, vr, base_time, false, events);
				DebugValidateEventMap(events);
				SK_CheckCreateEvent(el, vr, vr->next->next, base_time, false, events);
				DebugValidateEventMap(events);
				SK_CheckCreateEvent(vr, vr->next->next, vr->next->next->next->next, base_time, false, events);
				DebugValidateEventMap(events);

				SK_CheckCreateEvent(vl->prev->prev->prev->prev, vl->prev->prev, vl, base_time, false, events);
				DebugValidateEventMap(events);
				SK_CheckCreateEvent(vl->prev->prev, vl, er, base_time, false, events);
				DebugValidateEventMap(events);
				SK_CheckCreateEvent(vl, er, er->next->next, base_time, false, events);
				DebugValidateEventMap(events);
				SK_CheckCreateEvent(er, er->next->next, er->next->next->next->next, base_time, false, events);
				DebugValidateEventMap(events);

				
				SK_DestroyEvent(evt, events);
				DebugValidateEventMap(events);
				
				made_change = true;
			}

		} else {
		
			if (!SK_BisectorEventPossible(evt))
			{
				SK_DestroyEvent(evt, events);
			} else {
				
				// Middle edge is being deleted.
				
				SK_Edge * dead = evt->e2;
				SK_Edge * prev = evt->e1;
				SK_Edge * next = evt->e3;
				
				DebugAssert(dead->prev->prev == prev);
				DebugAssert(dead->next->next == next);
				
				// Wipe out ALL events for this edge, reflex or otherwise.
				SK_DestroyEventsForEdge(dead, events, true);
				SK_DestroyEventsForEdge(dead, events, false);
				
				prev->next = dead->next;
				dead->next->prev = prev;
				
				if (dead->owner->ccb == dead)
					dead->owner->ccb = next;
				
				delete dead->prev;
				delete dead;
				
				SK_CheckCreateEvent(prev->prev->prev, prev, next, base_time, false, events);
				SK_CheckCreateEvent(prev, next, next->next->next, base_time, false, events);
				
				made_change = true;
			}
		}
			

#if DEV	
		DebugValidatePoly(world);
		DebugValidateEventMap(events);
#endif		

		if (made_change)
		{
			SK_RemoveEmptyPolygons(world, events);		
#if DEV	
			DebugValidatePoly(world);
			DebugValidateEventMap(events);
#endif		
		} else {
#if DEV		
			gMeshLines.clear();
#endif
		}
		
#if DEV
		gMeshPoints.clear();
		for (EventMap::iterator eiter = events.begin(); eiter != events.end(); ++eiter)
		{
			gMeshPoints.push_back(Point2(eiter->second->cross.x,eiter->second->cross.y));
		}
#endif			

	}		

	if (events.empty())
		SK_AdvanceVertices(world, 1.0);
	else
		SK_AdvanceVertices(world, base_time);

	
	SK_PolygonToPmwx(world, outMap);
	for (Pmwx::Face_iterator face = outMap.faces_begin(); face != outMap.faces_end(); ++face)
	{
		if (face->is_unbounded())
			face->mTerrainType = inTerrainOut;
		else
			face->mTerrainType = (face->outer_ccb()->twin()->face()->is_unbounded()) ? inTerrainIn : inTerrainOut;
	}


#if DEV	
		DebugValidatePoly(world);
#endif		

	SK_PolygonDestroy(world);
	
	return steps != -1;
}


