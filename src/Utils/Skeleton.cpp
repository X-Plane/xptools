#include "Skeleton.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
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

#define GRAPHIC_LOGGING 0
#define LOG_SKELETONS 0
#define HEAVY_VALIDATION 0

#if GRAPHIC_LOGGING
#include "WED_Globals.h"
#endif

#if DEV
//static	Bbox2	sVertexLimit;
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
static bool	SK_SafeIntersect(SK_Edge * plane1, SK_Edge * plane2, const Point3& common_corner, double t, Point3& cross);
static bool	SK_SafeIntersect(SK_Edge * plane1, SK_Edge * plane2, const Point3& common_corner, Line3& cross);
static bool	SK_SafeIntersect2(SK_Edge * plane1, SK_Edge * plane2, SK_Edge * plane3, const Point3& common_corner, Point3& cross);
static bool	SK_SafeIntersect3(SK_Edge * plane1, SK_Edge * plane2, SK_Edge * plane3, const Point3& common_corner1, const Point3& common_corner2, Point3& cross);
static void	SK_DestroyEventsForEdge(SK_Edge * the_edge, EventMap& ioMap, bool inReflex);

static void AssertThrowQuiet(const char * msg, const char * file, int line) { throw msg; }



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
//	bool		is_reflex;		// True if this is a reflex vertex.
	double		time_base;		// Time of creation
	
	SK_Edge * 	prev;
	SK_Edge *	next;
	SK_Polygon * owner;
	
	void GetLocationAtTime(Point3& p, double t) const { 
		if (!SK_SafeIntersect(prev, next, Point3(location.x, location.y, 0), t, p))
			AssertPrintf("Failed intersection.");
	 }

	bool	IsReflex(void) const { 
		Vector2 prev_seg = Vector2(prev->supporting_plane.n.dx,prev->supporting_plane.n.dy).perpendicular_cw();
		Vector2 next_seg = Vector2(next->supporting_plane.n.dx,next->supporting_plane.n.dy).perpendicular_cw();
		
//		Vector2	prev_seg(prev->ends.p1, prev->ends.p2);
//		Vector2	next_seg(next->ends.p1, next->ends.p2);
		return prev_seg.right_turn(next_seg);
	}
	
#if DEV
	~SK_Vertex() { prev = DELETED_EDGE; next = DELETED_EDGE; owner = DELETED_POLYGON; }
#endif	
	
};

struct	SK_Polygon {
	SK_Polygon *		parent;
	set<SK_Polygon *>	children;
	SK_Edge *			ccb;
	
	int	num_sides(void) const { if (!ccb) return 0; SK_Edge * iter = ccb, * stop = ccb; int ctr = 0; do { ++ctr; iter=iter->next->next; } while (iter != stop); return ctr;}
	
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

void SK_NormalizeProjection(Plane3& p)
{
	Vector2	shadow(p.n.dx, p.n.dy);
	double len = sqrt(shadow.squared_length());
	if (len != 0.0)
	{
		len = 1.0 / len;
		shadow.normalize();	// utilize perfect correction to 1.0 for h/v vectors
		p.n.dx = shadow.dx;
		p.n.dy = shadow.dy;
		p.n.dz *= len;
		p.ndotp *= len;
	}
}

#define COPLANAR_CHECK 0.999


bool	SK_CloserToLine(const Line2& line1, const Line2& line2, const Point2& split, const Point2& p)
{
	double d1 = line1.distance_denormaled(p);
	double d2 = line2.distance_denormaled(p);
	
	if (line1.a * line2.a + line1.b * line2.b > COPLANAR_CHECK)
	{
		Vector2	to_pt(split, p);
		Vector2	nrmls(line1.a + line2.a, line1.b + line2.b);
		if (nrmls.right_turn(to_pt))
			return false;
		else
			return true;
	
	} else {
	
		if ((line1.a * line2.b - line1.b * line2.a) < 0.0)
			return d2 <= d1;
		else
			return d1 <= d2;	
			
	}
}


// First of all, ORDERED comparison.
// It turns out that the intersection of 2 or 3 planes depends oon the order of args to the math
// routine!! This is because the result is a sum of stuff and the summation causes rounding errors,
// whose magnitude depends on who gets rounded first.
//
// We need to make sure that given certain inputs we ALWAYS return the same result.  To solve this, we
// order the input planes by their byte-wise partial-weak-ordering, so we always compare the same way
// no matter what the input args are.  (The input order tends to depend on the current skeleton topology
// which changes over time!)  We use mem compare because address compares fail due to cloned edges, who
// have the same plane but new vertices.
inline bool	SK_FixedMemIsLess(unsigned const char * c1, unsigned const char * c2, unsigned int len)
{
	while (len)
	{
		if (*c1 < *c2)	return true;
		if (*c1 > *c2) 	return false;
		++c1, ++c2, --len;
	}
	return false;
}

inline bool SK_PlaneIsLess(const Plane3& p1, const Plane3& p2)
{
	return SK_FixedMemIsLess((unsigned char *) &p1, (unsigned char *) &p2, sizeof(Plane3));
}

bool	SK_OrderedIntersect(const Plane3& p1, const Plane3& p2, const Plane3& p3, Point3& cross)
{
	if (SK_PlaneIsLess(p1, p2) && SK_PlaneIsLess(p1, p3))
	{
		if (SK_PlaneIsLess(p2, p3))	return p1.intersect(p2, p3, cross);
		else						return p1.intersect(p3, p2, cross);
	}
	else if (SK_PlaneIsLess(p2, p3))
	{
		if (SK_PlaneIsLess(p1, p3))	return p2.intersect(p1, p3, cross);
									return p2.intersect(p3, p1, cross);
	}
	else
	{
		if (SK_PlaneIsLess(p1, p2)) 	return p3.intersect(p1, p2, cross);
		else							return p3.intersect(p2, p1, cross);
	}
}

bool	SK_OrderedIntersect(const Plane3& p1, const Plane3& p2, Line3& cross)
{
	if (SK_PlaneIsLess(p1, p2))	return p1.intersect(p2, cross);
	else						return p2.intersect(p1, cross);
}

 
// Second we have safe comparisons.  Besides being ordered, these detect the case where the intersected planes
// are coplanar facing the same way, and generate a simple bisector plane to solve the problem.  In other words,
// this allows us to know that the bisector of two colinear line segments is the perpedicular. 
 
bool	SK_SafeIntersect(SK_Edge * plane1, SK_Edge * plane2, const Point3& common_corner, double t, Point3& cross)
{
	Plane3	time_plane(Point3(0,0,t), Vector3(0,0,1));		

	Vector2	p1_dir(plane1->supporting_plane.n.dx, plane1->supporting_plane.n.dy);
	Vector2	p2_dir(plane2->supporting_plane.n.dx, plane2->supporting_plane.n.dy);
	
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
		
		if (SK_OrderedIntersect(plane1->supporting_plane, time_plane, cut_plane, s.p1))
		if (SK_OrderedIntersect(plane2->supporting_plane, time_plane, cut_plane, s.p2))
		{
			cross = s.midpoint();
			return true;
		}
		return false;		
	
	} else {
	
		// Default case - just do a 3-plane cross	
		return SK_OrderedIntersect(plane1->supporting_plane, plane2->supporting_plane, time_plane, cross);	
	}
}

bool	SK_SafeIntersect(SK_Edge * plane1, SK_Edge * plane2, const Point3& common_corner, Line3& cross)
{
	Vector2	p1_dir(plane1->supporting_plane.n.dx, plane1->supporting_plane.n.dy);
	Vector2	p2_dir(plane2->supporting_plane.n.dx, plane2->supporting_plane.n.dy);
	
	if (p1_dir.dot(p2_dir) > COPLANAR_CHECK)
	{
		// Special case - planes are coplanar.  We create a crossing plane and go from there.
		
		// This vector is roughly normal to both planes, along the ground.
		Vector2	av_dir(p1_dir.dx + p2_dir.dx, p1_dir.dy + p2_dir.dy);
		av_dir.normalize();
		
		// This is a plane that cuts the two edges in half...the point we are looking for is roughly along here.
		cross = Line3(common_corner, Vector3(av_dir.dx, av_dir.dy, 0));
		return true;
	
	} else {
	
		// Default case - just do a 3-plane cross	
		return SK_OrderedIntersect(plane1->supporting_plane, plane2->supporting_plane, cross);	
	}
}

bool	SK_SafeIntersect2(SK_Edge * plane1, SK_Edge * plane2, SK_Edge * plane3, const Point3& common_corner, Point3& cross)
{
	Vector2	p1_dir(plane1->supporting_plane.n.dx, plane1->supporting_plane.n.dy);
	Vector2	p2_dir(plane2->supporting_plane.n.dx, plane2->supporting_plane.n.dy);
	Vector2	p3_dir(plane3->supporting_plane.n.dx, plane3->supporting_plane.n.dy);

	if (p1_dir.dot(p3_dir) > COPLANAR_CHECK)	return false;
	if (p2_dir.dot(p3_dir) > COPLANAR_CHECK)	return false;
	
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
		
		if (SK_OrderedIntersect(plane1->supporting_plane, plane3->supporting_plane, cut_plane, s.p1))
		if (SK_OrderedIntersect(plane2->supporting_plane, plane3->supporting_plane, cut_plane, s.p2))
		{
			cross = s.midpoint();
			return true;
		}
		return false;		
	
	} else {
	
		// Default case - just do a 3-plane cross	
		return SK_OrderedIntersect(plane1->supporting_plane, plane2->supporting_plane, plane3->supporting_plane, cross);	
	}	
}

bool	SK_SafeIntersect3(SK_Edge * plane1, SK_Edge * plane2, SK_Edge * plane3, const Point3& common_corner1, const Point3& common_corner2, Point3& cross)
{
	Vector2	p1_dir(plane1->supporting_plane.n.dx, plane1->supporting_plane.n.dy);
	Vector2	p2_dir(plane2->supporting_plane.n.dx, plane2->supporting_plane.n.dy);
	Vector2	p3_dir(plane3->supporting_plane.n.dx, plane3->supporting_plane.n.dy);
	
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
		
		if (SK_OrderedIntersect(plane1->supporting_plane, plane3->supporting_plane, cut_plane, s.p1))
		if (SK_OrderedIntersect(plane2->supporting_plane, plane3->supporting_plane, cut_plane, s.p2))
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
		
		if (SK_OrderedIntersect(plane1->supporting_plane, plane2->supporting_plane, cut_plane, s.p1))
		if (SK_OrderedIntersect(plane1->supporting_plane, plane3->supporting_plane, cut_plane, s.p2))
		{
			cross = s.midpoint();
			return true;
		}
		return false;		
	
	} else {
	
		// Default case - just do a 3-plane cross	
		return SK_OrderedIntersect(plane1->supporting_plane, plane2->supporting_plane, plane3->supporting_plane, cross);	
	}
}

#pragma mark -




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
//	nv->is_reflex = false;

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

#pragma mark -

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
		SK_NormalizeProjection(ne->supporting_plane);
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

static SK_Polygon * SK_PolygonCreateComplex(SK_Polygon * parent, const ComplexPolygon2& inPoly, const ComplexPolygonWeight& weights)
{
	SK_Polygon * ret = NULL;
	for (int n = 0; n < inPoly.size(); ++n)
	{
		SK_Polygon * npoly = SK_PolygonCreate(n == 0 ? parent : ret, inPoly[n], &*weights[n].begin());
		if (n == 0) ret = npoly;		
	}
	
	return ret;
}

static void SK_InsetPolyIntoComplexPolygonList(SK_Polygon * world, ComplexPolygonVector& outPolys)
{
	outPolys.clear();
	
	outPolys.reserve(world->children.size());
	
	for (set<SK_Polygon *>::iterator outers = world->children.begin(); outers != world->children.end(); ++outers)
	{
		Assert((*outers)->ccb != NULL);
		outPolys.push_back(ComplexPolygon2());

		ComplexPolygon2& outerResult(outPolys.back());
		
		outerResult.reserve((*outers)->children.size() + 1);

		outerResult.push_back(Polygon2());
		
		Polygon2& ccb(outerResult.back());

		SK_Vertex * iter, * stop;
		iter = stop = (*outers)->ccb->next;
		do {
			ccb.push_back(iter->location);
			iter = iter->next->next;
		} while (iter != stop);

		for (set<SK_Polygon *>::iterator holes = (*outers)->children.begin(); holes != (*outers)->children.end(); ++holes)
		{
			Assert((*holes)->children.empty());
			Assert((*holes)->ccb != NULL);
			outerResult.push_back(Polygon2());
			Polygon2& hole(outerResult.back());
			iter = stop = (*holes)->ccb->next;
			do {
				hole.push_back(iter->location);
				iter = iter->next->next;
			} while (iter != stop);
			
		}
		
	}
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
		
//		if (!sVertexLimit.empty())
//			DebugAssert(sVertexLimit.contains(v->location));
		
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
//				iter->is_reflex = true;
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
				SK_NormalizeProjection(ne->supporting_plane);
				ne->ends.p1 = ne->ends.p2 = iter->location;
					
				vc->next = iter->next;
				vc->prev = ne;
				ne->next = vc;
				ne->prev = iter;
				iter->next->prev = vc;
				iter->next = ne;
				iter = vc;
//				vc->is_reflex = true;			
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
					if (v1.dot(v2) < -0.5)
					{
						DebugAssert(iter->IsReflex());
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
						SK_NormalizeProjection(ne->supporting_plane);
	
						ne->ends.p1 = ne->ends.p2 = iter->location;

						vc->next = iter->next;
						vc->prev = ne;
						ne->next = vc;
						ne->prev = iter;
						iter->next->prev = vc;
						iter->next = ne;
						iter = vc;
//						vc->is_reflex = true;								
					}
				}
			}
			
			iter = iter->next->next;
		} while (iter != stop);		
	}
}

/* Reset all vertices in the polygon to a given time. */
static bool SK_AdvanceVertices(SK_Polygon * poly, double advance_time)
{
	for (set<SK_Polygon *>::iterator c = poly->children.begin(); c != poly->children.end(); ++c)
		if (!SK_AdvanceVertices(*c, advance_time))
			return false;

	SK_Vertex * iter, * stop;
	if (poly->ccb)
	{
		iter = stop = poly->ccb->next;
		do {
			Point3		i;
			if (!SK_SafeIntersect(iter->prev,iter->next, Point3(iter->location.x, iter->location.y, 0), advance_time, i))
#if DEV
				i = Point3(iter->location.x, iter->location.y, advance_time);
#else	
				return false;		
//				AssertPrintf("Time intercept failed.\n");
#endif				
			else
				iter->location = Point2(i.x, i.y);
			iter->time_base = advance_time;
			iter = iter->next->next;
		} while (iter != stop);
	}
	return true;
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
			Assert(who->children.empty());
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


/***************************************************************************************************
 * INTERSECTION CALCULATIONS
 ***************************************************************************************************/
#pragma mark -

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
	SK_Vertex * v_reflex = inEvent->e1->next;

	// TOPOLOGICAL TEST - we disqualify reflex events where the edge is adjacent to one of the segments of the reflex
	// vertex.  Why?  Well, this is a reflex and a bisector event!  We let the bisector code handle it.  We want less
	// reflex events - they're more expensive to handle and risk multiple simultaneous reflex events if we leave 'em 
	// around.

//	why do we need this in?  imagine a bisector event where: the sides are actually CLOSING in (e.g. the u-turn is more than 180
//	degrees.  Make one side an antenna...poof!  this isn't' a bisector event because both are reflexes and the intersection point will be in space.
//	Solution - this config defines that there be a reflex event corresponding to it (from the antenna) that cleans it up.

	// If topologically we're really a bisector event, bail.
//	if (e_prev == inEvent->e1 || e_next == inEvent->e1 || e_prev == inEvent->e2 || e_next == inEvent->e2) return false;

	// DISASSEMBLY TEST - if the edges making up the reflex vector no longer connect, well, something happened that makes us
	// no longer a reflex event!  Bal.
	if (inEvent->e1->next != inEvent->e2->prev)	return false;

	// NOT-SO-REFLEX TEST.  If we're actually not pointing like a reflex vector, perhaps due to some strange reassembly,
	// well that's damn surprising but it's clear that we don't want to run the event.
// HACK: shouldn't we just know this!?!  If our input sides made us reflex when we started, how can it change if we haven't 
// been disassembled?
//	DebugAssert(v_reflex->IsReflex());
	if (!v_reflex->IsReflex())	return false;

	// LOVE-THY-NEIGHBOR TEST: If we share a vertex with the edge we're hitting, that's a very bad sign; generally it means that 
	// our reflex vertex  has a neighbor that it thinks it hits at a time really close to 0.  Do not allow this!
	if (inEvent->e1->ends.p2 == e_this->ends.p1 ||
		inEvent->e1->ends.p2 == e_this->ends.p2 ||
		inEvent->e2->ends.p1 == e_this->ends.p1 ||
		inEvent->e2->ends.p1 == e_this->ends.p2)			return false;


	// SPATIAL FILTER: Now comes the real work...find lines that are the projection of the bisectors of the vertices adjacent to the
	// edge we split; this defines the area where the edge is legitimate.  If we're not in this area bail.
	// How we do this: well the bisector is the line of equidistance between the two base supporting lines that we are bisecting.
	// So: take the signed euclidian distance from each line.  If we are closer to the split edge than either
	// side edges, we must be inside the bisectors.
	
	// More details: we normalize the supporting lines that are built by taking the supporting plane's Ax+By+Cz+D=0
	// and substituting for Z=0.  By normalizing it, we can calculate the signed distance without a square route.
	// (Okay, the normal op has a square route, so this isn't perfect.)
	

	Line2	base_line_this(e_this->supporting_plane.n.dx,e_this->supporting_plane.n.dy, -e_this->supporting_plane.ndotp);
	Line2	base_line_prev(e_prev->supporting_plane.n.dx,e_prev->supporting_plane.n.dy, -e_prev->supporting_plane.ndotp);
	Line2	base_line_next(e_next->supporting_plane.n.dx,e_next->supporting_plane.n.dy, -e_next->supporting_plane.ndotp);
	
	Point2 cross_flat(inEvent->cross.x, inEvent->cross.y);

	bool	closer_prev = !SK_CloserToLine(base_line_prev, base_line_this, e_this->ends.p1, cross_flat);
	bool	closer_next = SK_CloserToLine(base_line_this, base_line_next, e_this->ends.p2, cross_flat);

	return closer_prev && closer_next;
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

	// TRIANGLE TEST - all bisector events from within a triangle are true, always!  I mean, dude, 
	// how can they not be?

	if (e_next == e_prevprev) return true;
	if (e_prev == e_nextnext) return true;

	// LINE TEST - we don't ever expect to have a single line (2-edge) loop...just be sure lest chaos
	// break out.
	DebugAssert(v_prev != v_next);
	if (e_next == e_prev) return false;
	
//	return true;
/*	
	Fernando says: no spatial cutoff filter is needed for bisector events - if the adjacent bisector cut us
	off, there would be another bisector event with an earlier time!
	
	// SPATIAL FILTER - see above for notes
	
	Point3	p_zero_left(e_prev->ends.p1.x, e_prev->ends.p1.y, 0);
	Point3	p_zero_right(e_next->ends.p2.x, e_next->ends.p2.y, 0);

	Line3	left_travel, right_travel;

	if (!SK_SafeIntersect(e_prev->prev->prev,e_prev,p_zero_left,left_travel))
		AssertPrintf("Safe Intersect Failed.");
	if (!SK_SafeIntersect(e_next,e_next->next->next,p_zero_right,right_travel))
		AssertPrintf("Safe Intersect Failed.");

	if (left_travel.v.dz < 0.0)		left_travel.v = -left_travel.v;
	if (right_travel.v.dz < 0.0)	right_travel.v = -right_travel.v;

	Line2	left_travel_flat(e_prev->prev->location,Vector2(left_travel.v.dx,left_travel.v.dy));
	Line2	right_travel_flat(e_next->next->location,Vector2(right_travel.v.dx,right_travel.v.dy));
	Point2	cross_flat(inEvent->cross.x, inEvent->cross.y);
		
	bool	on_prev = left_travel_flat.on_right_side(cross_flat);
	bool	on_next = !right_travel_flat.on_right_side(cross_flat);
	
	return on_prev  && on_next;
*/	

	// BEn says: try anyway for now
//	TODO Revisit this - we need to get all of our on-edge stuff right!

	Line2	base_line_prev(e_prev->supporting_plane.n.dx,e_prev->supporting_plane.n.dy, -e_prev->supporting_plane.ndotp);
	Line2	base_line_next(e_next->supporting_plane.n.dx,e_next->supporting_plane.n.dy, -e_next->supporting_plane.ndotp);
	Line2	base_line_prevprev(e_prev->prev->prev->supporting_plane.n.dx,e_prev->prev->prev->supporting_plane.n.dy, -e_prev->prev->prev->supporting_plane.ndotp);
	Line2	base_line_nextnext(e_next->next->next->supporting_plane.n.dx,e_next->next->next->supporting_plane.n.dy, -e_next->next->next->supporting_plane.ndotp);
	
	Point2 cross_flat(inEvent->cross.x, inEvent->cross.y);

	bool	closer_prev = !SK_CloserToLine(base_line_prevprev, base_line_prev, e_prev->ends.p1, cross_flat);
	bool	closer_next = SK_CloserToLine(base_line_next, base_line_nextnext, e_next->ends.p2, cross_flat);

	return closer_prev && closer_next;
}


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
	
	if (( is_reflex && SK_SafeIntersect2(a, b, c, loc_ab, cross)) ||
		(!is_reflex && SK_SafeIntersect3(a, b, c, loc_ab, loc_bc, cross)))
	{
		if (cross.z > 0.0)
		if (cross.z >= time_min || !is_reflex)
		if (cross.z <= 1.0)
		// Reflex filter: for non-reflex events, at least one event must not be reflex.  This is because we know that two reflexes will be "diverging".
		if (is_reflex || !b->prev->IsReflex() || !b->next->IsReflex())
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
			return e;
		}
#if LOG_SKELETONS
		if (cross.z > 0.0 && cross.z < time_min && !is_reflex) printf("MISSED ADDING %s - Time = %lf, bisector cross = %lf\n", is_reflex ? "REFLEX" : "BISECTOR", time_min, cross.z);
#endif		
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
			if (iter->IsReflex())
				SK_CreateReflexEventsForVertexAndPolygon(world, iter, min_time, ioMap);
			iter = iter->next->next;
		} while (iter != stop);
	}
}


/***************************************************************************************************
 * MAIN ALGORITHM
 ***************************************************************************************************/
#pragma mark -




int	SK_InsetPolygon(
					const ComplexPolygon2&		inPolygon,
					const ComplexPolygonWeight&	inWeight,
					ComplexPolygonVector&		outHoles,
					int							steps)	// -1 or step limit!
{
	if (steps == -1) steps = -2;	// we need this to NOT be -1 or when we fall trhough with 0 events we think we timed out.  Gross!  This could really use some cleanup

	AssertHandler_f dbg = InstallDebugAssertHandler(AssertThrowQuiet);
	AssertHandler_f rel = InstallAssertHandler(AssertThrowQuiet);

	try {	

#if GRAPHIC_LOGGING
	gMeshPoints.clear();
	gMeshLines.clear();
#endif

//	Bbox2	vertex_limit;
//	vertex_limit = inPolygon[0][0];
//	for (int n = 0; n < inPolygon.size(); ++n)
//	for (int m = 0; m < inPolygon[n].size(); ++m)
//		vertex_limit += inPolygon[n][m];


	SK_Polygon *	world = SK_PolygonCreate(NULL, Polygon2(), NULL);
	SK_Polygon * 	poly = SK_PolygonCreateComplex(world, inPolygon, inWeight);

	// Do one-time prep: split antennas, mitre sharp corners and then find any
	// reflex vertices.
	SK_PolygonSplitAntennas(world);
	SK_PolygonMitreReflexVertices(world);

#if DEV	
		DebugValidatePoly(world);
#endif		

		EventMap	events;
	
	// Build up our original events
	
	SK_CreateVertexEventsForPolygon(world, 0.0, events);
	SK_CreateReflexEventsForPolygon(world, world, 0.0, events);

#if LOG_SKELETONS
	printf("Starting events...%d events total\n", events.size());
#endif	
	double base_time = 0.0;
	while (!events.empty() && (base_time = events.begin()->first) <= 1.0 && --steps)
	{
#if GRAPHIC_LOGGING
		gMeshPoints.clear();
#endif	
		pair<EventMap::iterator, EventMap::iterator> possible_events = events.equal_range(base_time);

		SK_Event * evt = NULL;
		bool made_change = false;
		
		for (EventMap::iterator eventIter = possible_events.first; eventIter != possible_events.second; ++eventIter)
		{
			if (eventIter->second->reflex_event)
			if (SK_ReflexEventPossible(eventIter->second))
			{
				evt = eventIter->second;
				break;
			}
			if (!eventIter->second->reflex_event)
			if (SK_BisectorEventPossible(eventIter->second))
			{
				evt = eventIter->second;
				break;
			}
		}

#if GRAPHIC_LOGGING
		if (evt)
		{
			gMeshLines.clear();
			gMeshLines.push_back(pair<Point2,Point3>(evt->e1->ends.p1,Point3(0.7,0.3,0.2)));
			gMeshLines.push_back(pair<Point2,Point3>(evt->e1->ends.p2,Point3(0.7,0.3,0.2)));
			gMeshLines.push_back(pair<Point2,Point3>(evt->e2->ends.p1,Point3(0.7,0.3,0.2)));
			gMeshLines.push_back(pair<Point2,Point3>(evt->e2->ends.p2,Point3(0.7,0.3,0.2)));
			gMeshLines.push_back(pair<Point2,Point3>(evt->e3->ends.p1,Point3(0.3,0.3,0.7)));
			gMeshLines.push_back(pair<Point2,Point3>(evt->e3->ends.p2,Point3(0.3,0.3,0.7)));

//			gMeshLines.push_back(evt->e1->prev->location);
//			gMeshLines.push_back(evt->e1->next->location);
//			gMeshLines.push_back(evt->e2->prev->location);
//			gMeshLines.push_back(evt->e2->next->location);
//			gMeshLines.push_back(evt->e3->prev->location);
//			gMeshLines.push_back(evt->e3->next->location);
		}		

#endif

		
		if (evt == NULL)
		{
#if GRAPHIC_LOGGING
			gMeshLines.clear();
#endif			
			while(!events.empty() && events.begin()->first == base_time)
			{
				evt = events.begin()->second;
#if GRAPHIC_LOGGING
				gMeshLines.push_back(pair<Point2,Point3>(evt->e1->ends.p1,Point3(0.7,0.3,0.2)));
				gMeshLines.push_back(pair<Point2,Point3>(evt->e1->ends.p2,Point3(0.7,0.3,0.2)));
				gMeshLines.push_back(pair<Point2,Point3>(evt->e2->ends.p1,Point3(0.7,0.3,0.2)));
				gMeshLines.push_back(pair<Point2,Point3>(evt->e2->ends.p2,Point3(0.7,0.3,0.2)));
				gMeshLines.push_back(pair<Point2,Point3>(evt->e3->ends.p1,Point3(0.3,0.3,0.7)));
				gMeshLines.push_back(pair<Point2,Point3>(evt->e3->ends.p2,Point3(0.3,0.3,0.7)));
				gMeshPoints.push_back(pair<Point2,Point3>(Point2(evt->cross.x,evt->cross.y), Point3(1.0, 0.0, 0.0)));
#endif				
#if LOG_SKELETONS
				printf("Trashing %s vertex at time %lf - none in this time are possible, xon = %lf,%lf.\n", evt->reflex_event ? "reflex" : "bisector", base_time, evt->cross.x, evt->cross.y);
#endif
				SK_DestroyEvent(evt, events);
			}
		}
		
		else if (evt->reflex_event)
		{
#if LOG_SKELETONS
			printf("Executed reflect event, passed eval.t = %lf.\n", base_time);
#endif
#if GRAPHIC_LOGGING
			gMeshPoints.push_back(pair<Point2,Point3>(Point2(evt->cross.x,evt->cross.y), Point3(1.0, 1.0, 1.0)));
#endif
			////////////////////////////////////////////

			Point3 our_cross = evt->cross;		
			EventMap::iterator evtIter = events.begin();
			++evtIter;
			while (evtIter != events.end() && evtIter->first == base_time)
			{
//				if (our_cross == evtIter->second->cross)
//				if (evtIter->second->reflex_event && SK_ReflexEventPossible(evtIter->second))
//					AssertPrintf("We do not yet handle multiple simultaneous vertex events!");

// I think this case works...
//				if (our_cross == evtIter->second->cross)
//				if (!evtIter->second->reflex_event && SK_BisectorEventPossible(evtIter->second))
//					AssertPrintf("Unexpected bisector+reflex collision!");
				++evtIter;
			}

			////////////////////////////////////////////
			
	
			// The third edge is split by the reflex vertex of the first two.

			SK_Vertex * nv = SK_SplitEdge(evt->e3, Point2(evt->cross.x, evt->cross.y), evt->cross.z);
			SK_Vertex * ov = evt->e1->next;
			
			// Update old vertex pos - this is used for clamping and must be maintained properly.
			ov->location = Point2(evt->cross.x, evt->cross.y);
			
			
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
//				DebugValidateEventMap(events);
			SK_DestroyEventsForEdge(er, events, false);
//				DebugValidateEventMap(events);
			SK_DestroyEventsForEdge(vl, events, false);
//				DebugValidateEventMap(events);
			SK_DestroyEventsForEdge(vr, events, false);
//				DebugValidateEventMap(events);
			
			SK_CheckCreateEvent(el->prev->prev->prev->prev, el->prev->prev, el, base_time, false, events);
//				DebugValidateEventMap(events);
			SK_CheckCreateEvent(el->prev->prev, el, el->next->next, base_time, false, events);
//				DebugValidateEventMap(events);
			SK_CheckCreateEvent(vr->prev->prev, vr, vr->next->next, base_time, false, events);
//				DebugValidateEventMap(events);
			SK_CheckCreateEvent(vr, vr->next->next, vr->next->next->next->next, base_time, false, events);
//				DebugValidateEventMap(events);

			SK_CheckCreateEvent(vl->prev->prev->prev->prev, vl->prev->prev, vl, base_time, false, events);
//				DebugValidateEventMap(events);
			SK_CheckCreateEvent(vl->prev->prev, vl, vl->next->next, base_time, false, events);
//				DebugValidateEventMap(events);
			SK_CheckCreateEvent(er->prev->prev, er, er->next->next, base_time, false, events);
//				DebugValidateEventMap(events);
			SK_CheckCreateEvent(er, er->next->next, er->next->next->next->next, base_time, false, events);
//				DebugValidateEventMap(events);

			
			SK_DestroyEvent(evt, events);
//				DebugValidateEventMap(events);
			
			made_change = true;

		} else {

#if LOG_SKELETONS
			printf("Executed bisector event, t = %lf.\n", base_time);			
#endif	
#if GRAPHIC_LOGGING
			gMeshPoints.push_back(pair<Point2,Point3>(Point2(evt->cross.x,evt->cross.y), Point3(1.0, 1.0, 1.0)));
#endif			
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
			

#if HEAVY_VALIDATION
		DebugValidatePoly(world);
		DebugValidateEventMap(events);
#endif		

		if (made_change)
		{
			SK_RemoveEmptyPolygons(world, events);		
#if HEAVY_VALIDATION
			DebugValidatePoly(world);
			DebugValidateEventMap(events);
#endif
		}
		
	}		

#if GRAPHIC_LOGGING
	for (EventMap::iterator eiter = events.begin(); eiter != events.end(); ++eiter)
	{
		if (eiter->second->reflex_event)
			gMeshPoints.push_back(pair<Point2,Point3>(Point2(eiter->second->cross.x,eiter->second->cross.y), Point3(0.2, 0.2, 0.8)));
		else
			gMeshPoints.push_back(pair<Point2,Point3>(Point2(eiter->second->cross.x,eiter->second->cross.y), Point3(0.2, 0.8, 0.2)));
	}
#endif			

	bool valid = true;

	if (events.empty())
		valid = SK_AdvanceVertices(world, 1.0);
	else
 		valid = SK_AdvanceVertices(world, base_time);

#if LOG_SKELETONS 		
 	for (set<SK_Polygon *>::iterator i = world->children.begin(); i != world->children.end(); ++i)
 	{
 		printf("Poly has %d sides.\n", (*i)->num_sides());
 		for (set<SK_Polygon *>::iterator j = (*i)->children.begin(); j != (*i)->children.end(); ++j)
	 		printf("   Hole has %d sides.\n", (*j)->num_sides());
 	}
#endif 	
		
	if (valid)
		SK_InsetPolyIntoComplexPolygonList(world, outHoles);

	if (steps != -1 && valid)
	{
		for (ComplexPolygonVector::iterator poly = outHoles.begin(); poly != outHoles.end(); ++poly)
		for (ComplexPolygon2::iterator part = poly->begin(); part != poly->end(); ++part)
		for (Polygon2::iterator pt = part->begin(); pt != part->end(); ++pt)
		{
			if (!inPolygon.front().inside(*pt))
			{
				valid = false;
				goto bail;
			}
		}
	}
bail:	


#if DEV	
	if (valid)
		DebugValidatePoly(world);
#endif		

	SK_PolygonDestroy(world);

		InstallDebugAssertHandler(dbg);
		InstallAssertHandler(rel);
	
	return steps != -1 ? (valid ? skeleton_OK : skeleton_InvalidResult) : skeleton_OutOfSteps;
	} catch (...) {

	InstallDebugAssertHandler(dbg);
	InstallAssertHandler(rel);

		return skeleton_Exception;
	}
}


