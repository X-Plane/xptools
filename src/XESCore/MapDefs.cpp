/* 
 * Copyright (c) 2004, Laminar Research.
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
 
 /*
 
 	NOTES ON MAP TOPOLOGICAL PERFORMANCE:
 	
 	Locate quality - 
 		Locate will always locate vertices exactly because it uses a horizontal walk-along-line technique.
 		Locate will only locate halfedges exactly if there exists a clean intersection between a horizontal
 			line and the halfedge...so halfedges with clean slope (e.g. 45 degrees) and low-precision coordinates
 			will work okay.
 		Locate with faces will always work.
 	Ray-Shoot Quality
 		Ray shoot to a vertex will always work.
* 		Ray shoot to a halfedge will work when there exists a clean intersection between the ray being shot and the
 			halfedge, which is to say, it's a real toss-up.
 		Ray shoot to a face will always work.
** 		Ray shoot through a vertex will only work when there is a clean intersection between a halfedge touching the ray
 			and the ray itself - in other words, almost never.
 		Ray shoot through a halfedge will always work.

	Edge-Insert Quality
		Edge insert will FAIL if you insert an edge, cross it with another edge, and then re-insert the same edge, unless the
			edge that is inserted twice is horizontal or vertical.  There is no workaround yet for this.

*	Wrokaround: split the half-edge in advance, which lets you name the split point.  Now you are ray-shooting to a
		vertex, which is guaranteed.
**	Workaround: ray shoot to the vertex you desire to hit, then from then proceed.

	PERFORMANCE:
		Halfedge Locate is about linear to the number of edges per face avg and number of faces - a bit slow but okay.
		Ray shoot time is linear to the number of edges per face avg.
		Edge insert with prelocation is linear to the number of real edges inserted times the avg number of edges per face.
		
		Halfedge remove time is constant.
 	
 
 */
 
#include "MapDefs.h"
#include "ParamDefs.h"
#include "AssertUtils.h"
#include "CompGeomUtils.h"
#include "XBuckets_inline.h"

// This causes validate to check for dupe vertices.  Makes validate real slow.
#define VALIDATE_CHECK_NO_DUPE_VERTICES 0
// This causes add-vertex to check for new vertices.  This makes adding vertices real slow.
#define CHECK_NEW_VERTICES_FOR_DUPES 1
// This causese deleting a halfedge to check that it is not used anywhere.  This makes deleting
// halfedges real slow.
#define CHECK_DELETE_USED_HALFEDGE 0


#define DEAD_VERTEX (GISVertex *) 0xDEADBEEF
#define DEAD_HALFEDGE (GISHalfedge *) 0xDEADBEEF
#define DEAD_FACE (GISFace *) 0xDEADBEEF

inline  bool parallel_check(const Segment2& trial, const Segment2& range, Point2& cross)
{
	Vector2	range_front(range.p1, range.p2);
	Vector2	range_back(range.p2, range.p1);
	
	if (range_front.dot(Vector2(range.p1, trial.p2)) > 0.0 &&
		range_back.dot(Vector2(range.p2, trial.p1)) > 0.0)
	{
		cross = range.p1;
		return true;
	}

	if (range_front.dot(Vector2(range.p1, trial.p1)) > 0.0 &&
		range_back.dot(Vector2(range.p2, trial.p2)) > 0.0)
	{
		cross = range.p2;
		return true;
	}

	return false;		
}

GISFace::GISFace() : 
	mLinkPrev(NULL),
	mLinkNext(NULL),
	mOuterCCB(NULL),
	mTerrainType(terrain_Natural)
{
	mAreaFeature.mFeatType = NO_VALUE;
}

GISFace::GISFace(const GISFace& x) : 
	mLinkPrev(NULL),
	mLinkNext(NULL),
	mOuterCCB(NULL),
	mTerrainType	(x.mTerrainType		),
	mParams			(x.mParams			),
	mPointFeatures	(x.mPointFeatures	),
	mPolygonFeatures(x.mPolygonFeatures	),
	mAreaFeature	(x.mAreaFeature		),
	mObjs			(x.mObjs			),
	mPolyObjs		(x.mPolyObjs		)
{
}

GISFace::~GISFace()
{
}

bool		GISFace::IsWater(void) const { return mTerrainType == terrain_Water || mOuterCCB == NULL; }
bool		GISFace::TerrainMatch(const GISFace& rhs) const { return mTerrainType == rhs.mTerrainType; }
bool		GISFace::AreaMatch(const GISFace& rhs) const { return (mTerrainType == rhs.mTerrainType && mAreaFeature.mFeatType == rhs.mAreaFeature.mFeatType); }

void	GISFace::set_outer_ccb(GISHalfedge * outer) 			
{
	if (outer)
		DebugAssert(mLinkPrev != NULL != NULL);
	else
		DebugAssert(mOuterCCB == NULL && mLinkPrev == NULL);
	mOuterCCB = outer; 
}
void	GISFace::add_hole(GISHalfedge * inner) 					
{ 
	DebugAssert(mHoles.count(inner) == 0);
	mHoles.insert(inner); 
}
void	GISFace::delete_hole(GISHalfedge * inner) 				
{ 
	DebugAssert(mHoles.count(inner) != 0);
	mHoles.erase(inner); 
}


#pragma mark -

GISHalfedge::GISHalfedge() : 
	mLinkPrev(NULL),
	mLinkNext(NULL),
	mNext(NULL),
	mTwin(NULL),
	mFace(NULL),
	mTarget(NULL),

	mDominant(false), 
	mTransition(0), 
	mInset(0.0)
{
}

GISHalfedge::GISHalfedge(const GISHalfedge& x) : 
	mLinkPrev(NULL),
	mLinkNext(NULL),
	mNext(NULL),
	mTwin(NULL),
	mFace(NULL),
	mTarget(NULL),
	
	mDominant	(x.mDominant	),
	mTransition	(x.mTransition	),
	mSegments	(x.mSegments	),
	mInset		(x.mInset		),
	mParams		(x.mParams		)
{
}

void	GISHalfedge::SwapDominance(void)
{
	GISHalfedge * other = mTwin;
	::swap(this->mDominant, other->mDominant);
	::swap(this->mInset, other->mInset);
	::swap(this->mSegments, other->mSegments);
	::swap(this->mTransition, other->mTransition);
	::swap(this->mParams, other->mParams);
}

GISHalfedge::~GISHalfedge()
{
}


bool				GISHalfedge::is_on_outer_ccb(void) const
{
	const GISHalfedge * it = this;
	do {
		if (mFace->outer_ccb() == it) return true;
		it = it->next();
	} while (it != this);
	return false;	
}

GISHalfedge *		GISHalfedge::get_hole_rep(void)
{
	// Hole rep - find the halfedge that is used to represent the hole ring we're on
	// if we are in a hole.  Return NULL if we're in a CCB.  We do this by traversing
	// the ring and seeing if we're the hole rep.  This is slow, so if we notice we're
	// the CCB, do a quick bail.  This will hopefully get us out fast at least some of
	// the time, especially since our maps have a lot more CCBs than holes.
	GISHalfedge * it = this;
	GISHalfedge * ret = NULL;
	do {
#if !DEV
		if (mFace->outer_ccb() == it) return NULL;
		if (mFace->is_hole_ccb(it)) return it;
#endif
#if DEV
		if (mFace->is_hole_ccb(it))
		{
			DebugAssert(ret == NULL);
			ret = it;
				
		}
#endif		
		it = it->next();
	} while (it != this);
	return ret;		
}

GISHalfedge *		GISHalfedge::get_leftmost(void)
{
	GISHalfedge * it, * best;
	best = it = this;
	Point2	pos = target()->point();
	do {
		if (it->target()->point().x < pos.x)
		{
			best = it;
			pos = it->target()->point();
		}

		if (it->target()->point().x == pos.x &&
			it->target()->point().y < pos.y)
		{
			best = it;
			pos = it->target()->point();
		}
		
		it = it->next();
	} while (it != this);
	return best;
}


GISHalfedge *		GISHalfedge::points_to_me(void)
{
	Halfedge_around_vertex_circulator 	circ(this->source()->incident_halfedges());
	Halfedge_around_vertex_circulator	stop(circ);
	do {
		if (circ->next() == this) return circ;
		++circ;
	} while (circ != stop);
	
	Assert(!"Error: twin's pre-edge not found around target vertex.\n");
	return NULL;
}


GISHalfedge *	GISHalfedge::get_pre_twin(void)
{
	return twin()->points_to_me();
}



#pragma mark -

GISVertex::GISVertex() : 
	mHalfedge(NULL),
	mTunnelPortal(false)
{
}

GISVertex::GISVertex(const GISVertex& x) : 
	mHalfedge(NULL),
	mTunnelPortal(x.mTunnelPortal)
{
}

GISVertex::~GISVertex()
{
}

unsigned int GISVertex::degree() const
{
	GISHalfedge * e = mHalfedge;;
	unsigned int n = 0;
 	if (e != NULL)
 	{
 		do {
 			++n;
 			e = e->next()->twin();
 		} while (e != mHalfedge);
 	}
	return n;
}

/*
 * rightmost_rising
 *
 * This attempts to return the rightmost rising egde from a vertex.  It returns:
 * - If there is only one halfedge incident to the vertex, that halfedge.
 * - If there are two or more, the first one CCW from a hypothetical vector 1, 0.
 * The rightmost_rising edge will only be the right horizontal if the horizontal is the only
 * halfedge incident to this face.  But it may actually go left or lower depending on the 
 * configuration.
 *
 * The returned halfedge will always (1) point to the vertex 'this' and (2) its face will
 * always own the space just to the right of the vertex.
 
 * (Note that even if it's around on the left side, the lack of interceding halfedges means
 * we have continuous 2-d space so we know who owns that point.  If the horizontal is found, 
 * its left edge edge owns the space if and only if there are no other halfedges that it
 * connects to - a sole horizontal antenna to the right.)
 *
 */
GISHalfedge *	GISVertex::rightmost_rising()
{
	DebugAssert(mHalfedge != NULL);
	GISHalfedge * right_most = NULL;
	Halfedge_around_vertex_circulator v_circ, v_stop, v_next;
	v_circ = v_stop = this->incident_halfedges();
	do {
		v_next = v_circ;
		++v_next;
		
		// Special case: if we have only one halfedge, well, it is the rightmost-rising.
		if (v_next == v_circ)	
			return v_next;
	
		// Special case horizontal lines to keep Is_CCW_Between from bitching.
		if (v_circ->source()->point().y == v_circ->target()->point().y &&
			v_circ->source()->point().x  > v_circ->target()->point().x)
		{
			++v_circ;
			continue;
		}
		if (v_next->source()->point().y == v_next->target()->point().y &&
			v_next->source()->point().x  > v_next->target()->point().x)
		{
			right_most = v_next;
			break;
		}
		
		if (Is_CCW_Between(
			Vector2(v_next->target()->point(), v_next->source()->point()),
			Vector2(1.0, 0.0),
			Vector2(v_circ->target()->point(), v_circ->source()->point())))
		{
			right_most = v_circ;
			break;
		}
		++v_circ;
	} while (v_circ != v_stop);
	DebugAssert(right_most != NULL);
	return right_most;
}



#pragma mark -

Pmwx::Pmwx()
{
	mVertices = mHalfedges = 0;
	mFaces = 1;
	mUnbounded = new GISFace;
	mFirstVertex = mLastVertex = NULL;
	mFirstHalfedge = mLastHalfedge= NULL;
	mFirstFace = mLastFace = mUnbounded;
	mUnbounded->mTerrainType = terrain_Water;	
}

Pmwx::Pmwx(const Pmwx& rhs)
{
	mVertexIndex.clear();
	mVertices = mHalfedges = 0;
	mFaces = 1;
	mUnbounded = new GISFace;
	mFirstVertex = mLastVertex = NULL;
	mFirstHalfedge = mLastHalfedge= NULL;
	mFirstFace = mLastFace = mUnbounded;
	mUnbounded->mTerrainType = terrain_Water;	
	*this = rhs;
}


Pmwx::Pmwx(const GISFace& rhs)
{
	mVertices = mHalfedges = 0;
	mFaces = 1;
	mUnbounded = new GISFace;
	mFirstVertex = mLastVertex = NULL;
	mFirstHalfedge = mLastHalfedge= NULL;
	mFirstFace = mLastFace = mUnbounded;
	mUnbounded->mTerrainType = terrain_Water;	
	mVertexIndex.clear();
	*this = rhs;
}


Pmwx::~Pmwx()
{	
	GISVertex * v, * killv;
	GISHalfedge * h, * killh;
	GISFace * f, * killf;
	for (v = mFirstVertex; v; )
	{
		killv = v;
		v = v->mLinkNext;
		delete killv;
	}
	for (h = mFirstHalfedge; h; )
	{
		killh = h;
		h = h->mLinkNext;
		delete killh;
	}
	for (f = mFirstFace; f; )
	{
		killf = f;
		f = f->mLinkNext;
		delete killf;
	}
}

struct  HashPtr {
	size_t operator()(const void * p) const { return (reinterpret_cast<size_t>(p)); }
};

Pmwx& Pmwx::operator=(const Pmwx& rhs)
{
	clear();
		
	// OPTIMIZE - we should examine the quality of the hash table that's being bulit off memory pointers.
	hash_map<const GISHalfedge *, GISHalfedge *, HashPtr>	halfedges;
	hash_map<const GISVertex   *, GISVertex   *, HashPtr>	vertices;
	hash_map<const GISFace     *, GISFace     *, HashPtr>	faces;
	
		  GISHalfedge 	* en;
		  GISVertex		* vn;
		  GISFace		* fn;
	const GISHalfedge 	* ei;
	const GISVertex		* vi;
	const GISFace		* fi;
	
	for (ei = rhs.mFirstHalfedge; ei; ei = ei->mLinkNext)
	{
		en = new_halfedge(ei);
		halfedges[ei] = en;
	}
	for (fi = rhs.mFirstFace; fi; fi = fi->mLinkNext)
	{
		if (fi->is_unbounded())
			fn = unbounded_face();
		else
			fn = new_face(fi);
		faces[fi] = fn;
	}
	for (vi = rhs.mFirstVertex; vi; vi = vi->mLinkNext)
	{
		vn = new_vertex(vi->point());
		vertices[vi] = vn;
	}



	for (en = mFirstHalfedge, ei = rhs.mFirstHalfedge; en; en = en->mLinkNext, ei = ei->mLinkNext)
	{
		en->set_target(vertices	[ei->target()	]);
		en->set_next(halfedges	[ei->next()		]);
		en->set_twin(halfedges	[ei->twin()		]);
		en->set_face(faces		[ei->face()		]);
	}
	for (fn = mFirstFace, fi = rhs.mFirstFace; fn; fn = fn->mLinkNext, fi = fi->mLinkNext)
	{
		if (fi->is_unbounded())
			DebugAssert(fn->is_unbounded());
		else
			fn->set_outer_ccb(halfedges[fi->outer_ccb()]);
		for (Holes_iterator h = fi->holes_begin(); h != fi->holes_end(); ++h)
			fn->add_hole(halfedges[*h]);
	}
	for (vn = mFirstVertex, vi = rhs.mFirstVertex; vn; vn = vn->mLinkNext, vi = vi->mLinkNext)
	{
		vn->set_halfedge(halfedges[ vi->halfedge() ]);
	}

	
	return *this;
}

Pmwx& Pmwx::operator=(const GISFace& rhs)
{
	clear();
	const GISHalfedge * iter, * stop;
	map<Point2, GISVertex *, lesser_y_then_x> pt_index;
	set<const GISHalfedge *>						  added;
	map<Point2, GISVertex *, lesser_y_then_x>::iterator i1, i2;
	GISHalfedge * nh;
		
	
	if (!rhs.is_unbounded())
	{
		iter = stop = &*rhs.outer_ccb();
		do {
			
			if (added.count(iter) == 0)
			{			
				i1 = pt_index.find(iter->source()->point());
				i2 = pt_index.find(iter->target()->point());
				if (i1 != pt_index.end())
				{
					if (i2 != pt_index.end())
					{
						/* CASE 1 - Both points already in. */
						nh = nox_insert_edge_between_vertices(i1->second, i2->second);
					} 
					else
					{
						/* Case 2 - Point 1 in, point 2 new. */
						nh = nox_insert_edge_from_vertex(i1->second, iter->target()->point());
						pt_index[iter->target()->point()] = nh->target();
					}
				} 
				else
				{
					if (i2 != pt_index.end())
					{
						/* Case 3 - Point 1 new, point 2 in. */
						nh = nox_insert_edge_from_vertex(i2->second, iter->source()->point())->twin();
						pt_index[iter->source()->point()] = nh->source();
					} 
					else
					{
						/* Case 4 - both points new. */
						nh = nox_insert_edge_in_hole(iter->source()->point(), iter->target()->point());
						pt_index[iter->source()->point()] = nh->source();
						pt_index[iter->target()->point()] = nh->target();
					}
				}
				
				added.insert(iter);
				added.insert(iter->twin());
				if (!nh->mDominant) nh = nh->twin();
				nh->mSegments.insert(nh->mSegments.end(), iter->mSegments.begin(),iter->mSegments.end());		
				nh->mParams.insert(iter->mParams.begin(),iter->mParams.end());
			}			

			iter = iter->mNext;
		} while (iter != stop);
	}
	
	for (Holes_iterator h = rhs.holes_begin(); h != rhs.holes_end(); ++h)
	{
		iter = stop = *h;
		do {

			if (added.count(iter) == 0)
			{			
				i1 = pt_index.find(iter->source()->point());
				i2 = pt_index.find(iter->target()->point());
				if (i1 != pt_index.end())
				{
					if (i2 != pt_index.end())
					{
						/* CASE 1 - Both points already in. */
						nh = nox_insert_edge_between_vertices(i1->second, i2->second);
					} 
					else
					{
						/* Case 2 - Point 1 in, point 2 new. */
						nh = nox_insert_edge_from_vertex(i1->second, iter->target()->point());
						pt_index[iter->target()->point()] = nh->target();
					}
				} 
				else
				{
					if (i2 != pt_index.end())
					{
						/* Case 3 - Point 1 new, point 2 in. */
						nh = nox_insert_edge_from_vertex(i2->second, iter->source()->point())->twin();
						pt_index[iter->source()->point()] = nh->source();
					} 
					else
					{
						/* Case 4 - both points new. */
						nh = nox_insert_edge_in_hole(iter->source()->point(), iter->target()->point());
						pt_index[iter->source()->point()] = nh->source();
						pt_index[iter->target()->point()] = nh->target();
					}
				}
				
				added.insert(iter);
				added.insert(iter->twin());
				if (!nh->mDominant) nh = nh->twin();
				nh->mSegments.insert(nh->mSegments.end(), iter->mSegments.begin(),iter->mSegments.end());		
				nh->mParams.insert(iter->mParams.begin(),iter->mParams.end());
			}			

			iter = iter->mNext;
		} while (iter != stop);
	}

	DebugAssert(rhs.is_unbounded() || mUnbounded->holes_count() == 1);
	GISFace * copy = rhs.is_unbounded() ? mUnbounded : (*mUnbounded->holes_begin())->twin()->face();
	DebugAssert(copy->is_unbounded() == rhs.is_unbounded());
	DebugAssert(copy->holes_count() == rhs.holes_count());

	return *this;	
}

void Pmwx::swap(Pmwx& rhs)
{
	::swap(mFirstVertex, rhs.mFirstVertex);
	::swap(mLastVertex, rhs.mLastVertex);
	::swap(mFirstHalfedge, rhs.mFirstHalfedge);
	::swap(mLastHalfedge, rhs.mLastHalfedge);
	::swap(mFirstFace, rhs.mFirstFace);
	::swap(mLastFace, rhs.mLastFace);

	::swap(mVertices, rhs.mVertices);
	::swap(mHalfedges, rhs.mHalfedges);
	::swap(mFaces, rhs.mFaces);

	::swap(mUnbounded, rhs.mUnbounded);
	
	mVertexIndex.swap(rhs.mVertexIndex);
}

#pragma mark -

/*******************************************************************************************
 * LOW LEVEL ALLOCATION
 *******************************************************************************************/

GISVertex *		Pmwx::new_vertex(const Point2& inPt)
{	
	++mVertices;
	GISVertex * v = new GISVertex;
	v->mLinkNext = NULL;
	v->mLinkPrev = mLastVertex;
	if (mLastVertex != NULL)
		mLastVertex->mLinkNext = v;
	mLastVertex = v;
	if (mFirstVertex == NULL)
		mFirstVertex = v;

	v->mPoint = inPt;
#if CHECK_NEW_VERTICES_FOR_DUPES
	DebugAssert(mVertexIndex.count(inPt) == 0);
#endif	
	mVertexIndex[inPt] = v;
	return v;
}


GISHalfedge *	Pmwx::new_halfedge()
{
	++mHalfedges;
	GISHalfedge * v = new GISHalfedge;
	v->mLinkNext = NULL;
	v->mLinkPrev = mLastHalfedge;
	if (mLastHalfedge != NULL)
		mLastHalfedge->mLinkNext = v;
	mLastHalfedge = v;
	if (mFirstHalfedge == NULL)
		mFirstHalfedge = v;
	return v;
}

GISHalfedge *	Pmwx::new_halfedge(const GISHalfedge * rhs)
{
	++mHalfedges;
	GISHalfedge * v = new GISHalfedge(*rhs);
	v->mLinkNext = NULL;
	v->mLinkPrev = mLastHalfedge;
	if (mLastHalfedge != NULL)
		mLastHalfedge->mLinkNext = v;
	mLastHalfedge = v;
	if (mFirstHalfedge == NULL)
		mFirstHalfedge = v;
	return v;
}


GISHalfedge *	Pmwx::new_edge()
{
	GISHalfedge * h1 = new_halfedge();
	GISHalfedge * h2 = new_halfedge();
	h1->set_twin(h2);
	h2->set_twin(h1);
	h1->mDominant = true;
	h2->mDominant = false;
	return h1;
}

GISHalfedge *	Pmwx::new_edge(const GISHalfedge * rhs)
{
	GISHalfedge * h1;
	GISHalfedge * h2;

	// MAKE THESE in the right order to preserve std dominance or sh-t goes down.
	if (rhs->mDominant)
	{
		h1 = new_halfedge(rhs);
		h2 = new_halfedge(rhs->twin());
	} else {
		h2 = new_halfedge(rhs->twin());
		h1 = new_halfedge(rhs);
	}

	h1->set_twin(h2);
	h2->set_twin(h1);
	return h1;
}

GISFace *		Pmwx::new_face()
{
	++mFaces;
	GISFace * v = new GISFace;
	v->mLinkNext = NULL;
	v->mLinkPrev = mLastFace;
	if (mLastFace != NULL)
		mLastFace->mLinkNext = v;
	mLastFace = v;
	if (mFirstFace == NULL)
		mFirstFace = v;
	return v;
}


GISFace *		Pmwx::new_face(const GISFace * rhs)
{
	++mFaces;
	GISFace * v = new GISFace(*rhs);
	v->mLinkNext = NULL;
	v->mLinkPrev = mLastFace;
	if (mLastFace != NULL)
		mLastFace->mLinkNext = v;
	mLastFace = v;
	if (mFirstFace == NULL)
		mFirstFace = v;
	return v;
}

void	Pmwx::MoveFaceToMe(Pmwx * old, GISFace * inFace)
{
	DebugAssert(old != this);
	DebugAssert(old->mFaces > 0);
	DebugAssert(!inFace->is_unbounded());
	old->mFaces--;
	if (inFace->mLinkPrev)
		inFace->mLinkPrev->mLinkNext = inFace->mLinkNext;
	else
		old->mFirstFace = inFace->mLinkNext;
	
	if (inFace->mLinkNext)
		inFace->mLinkNext->mLinkPrev = inFace->mLinkPrev;
	else
		old->mLastFace = inFace->mLinkPrev;
	
	this->mFaces++;
	if (mLastFace == NULL)
	{
		mLastFace = inFace;
		mFirstFace = inFace;
		inFace->mLinkNext = NULL;
		inFace->mLinkPrev = NULL;		
	} else {
		mLastFace->mLinkNext = inFace;
		inFace->mLinkPrev = mLastFace;
		inFace->mLinkNext = NULL;
		mLastFace = inFace;
	}	
}

void	Pmwx::MoveVertexToMe(Pmwx * old, GISVertex * inVertex)
{
	DebugAssert(old->mVertices > 0);
	DebugAssert(old != this);
	
	old->mVertices--;
	if (inVertex->mLinkPrev)
		inVertex->mLinkPrev->mLinkNext = inVertex->mLinkNext;
	else
		old->mFirstVertex = inVertex->mLinkNext;
	
	if (inVertex->mLinkNext)
		inVertex->mLinkNext->mLinkPrev = inVertex->mLinkPrev;
	else
		old->mLastVertex = inVertex->mLinkPrev;
	
	this->mVertices++;
	if (mLastVertex == NULL)
	{
		mLastVertex = inVertex;
		mFirstVertex = inVertex;
		inVertex->mLinkNext = NULL;
		inVertex->mLinkPrev = NULL;		
	} else {
		mLastVertex->mLinkNext = inVertex;
		inVertex->mLinkPrev = mLastVertex;
		inVertex->mLinkNext = NULL;
		mLastVertex = inVertex;
	}	
}


void	Pmwx::MoveHalfedgeToMe(Pmwx * old, GISHalfedge * inHalfedge)
{
	DebugAssert(old->mHalfedges > 0);
	DebugAssert(old != this);
	old->mHalfedges--;
	if (inHalfedge->mLinkPrev)
		inHalfedge->mLinkPrev->mLinkNext = inHalfedge->mLinkNext;
	else
		old->mFirstHalfedge = inHalfedge->mLinkNext;
	
	if (inHalfedge->mLinkNext)
		inHalfedge->mLinkNext->mLinkPrev = inHalfedge->mLinkPrev;
	else
		old->mLastHalfedge = inHalfedge->mLinkPrev;
	
	this->mHalfedges++;
	
	DebugAssert(mLastHalfedge != NULL);
	// HALFEDGES ARE SPECIAL - we do NOT just add this guy to the end.
	// We MUST preserve the "dominant me, my twin" pattern that we have 
	// or else file I/O will bork.

	if (inHalfedge->mDominant)
	{
		// Insert before our twin.
		if (inHalfedge->mTwin == mFirstHalfedge)
		{
			mFirstHalfedge = inHalfedge;
			inHalfedge->mLinkNext = inHalfedge->mTwin;
			inHalfedge->mTwin->mLinkPrev = inHalfedge;
			inHalfedge->mLinkPrev = NULL;
		} else {
			inHalfedge->mTwin->mLinkPrev->mLinkNext = inHalfedge;
			inHalfedge->mLinkPrev = inHalfedge->mTwin->mLinkPrev;

			inHalfedge->mTwin->mLinkPrev = inHalfedge;
			inHalfedge->mLinkNext = inHalfedge->mTwin;
		}
	} else {
		// Insert after our twin
		if (inHalfedge->mTwin == mLastHalfedge)
		{
			inHalfedge->mTwin->mLinkNext = inHalfedge;
			inHalfedge->mLinkPrev = inHalfedge->mTwin;
			mLastHalfedge = inHalfedge;
			inHalfedge->mLinkNext = NULL;
		} else {
			inHalfedge->mTwin->mLinkNext->mLinkPrev = inHalfedge;
			inHalfedge->mLinkNext = inHalfedge->mTwin->mLinkNext;
			inHalfedge->mTwin->mLinkNext = inHalfedge;
			inHalfedge->mLinkPrev = inHalfedge->mTwin;
		}
	}
}

void	Pmwx::MoveEdgeToMe(Pmwx * old, GISHalfedge * inHalfedge)
{
	DebugAssert(old->mHalfedges > 0);
	DebugAssert(old != this);
	old->mHalfedges--;
	if (inHalfedge->mLinkPrev)
		inHalfedge->mLinkPrev->mLinkNext = inHalfedge->mLinkNext;
	else
		old->mFirstHalfedge = inHalfedge->mLinkNext;
	
	if (inHalfedge->mLinkNext)
		inHalfedge->mLinkNext->mLinkPrev = inHalfedge->mLinkPrev;
	else
		old->mLastHalfedge = inHalfedge->mLinkPrev;
	
	this->mHalfedges++;
	
	if (mLastHalfedge == NULL)
	{
		mLastHalfedge = mFirstHalfedge = inHalfedge;
		inHalfedge->mLinkPrev = inHalfedge->mLinkNext = NULL;
	} else {
		mLastHalfedge->mLinkNext = inHalfedge;
		inHalfedge->mLinkPrev = mLastHalfedge;
		inHalfedge->mLinkNext = NULL;
		mLastHalfedge = inHalfedge;
	}
	
	MoveHalfedgeToMe(old, inHalfedge->mTwin);
}

void	Pmwx::UnindexVertex(GISVertex * v)
{
#if CHECK_NEW_VERTICES_FOR_DUPES
	DebugAssert(mVertexIndex.count(v->mPoint) == 1);
	DebugAssert(mVertexIndex[v->mPoint] = v);
#endif
	mVertexIndex.erase(v->mPoint);
}

void	Pmwx::ReindexVertex(GISVertex * v)
{
#if CHECK_NEW_VERTICES_FOR_DUPES
	DebugAssert(mVertexIndex.count(v->mPoint) == 0);
#endif
	mVertexIndex[v->mPoint] = v;
}
	

/*******************************************************************************************
 * OVERALL QUERIES
 *******************************************************************************************/


void	Pmwx::clear()
{
	mVertexIndex.clear();

	GISVertex * v, * killv;
	GISHalfedge * h, * killh;
	GISFace * f, * killf;
	for (v = mFirstVertex; v; )
	{
		killv = v;
		v = v->mLinkNext;
		delete killv;
	}
	for (h = mFirstHalfedge; h; )
	{
		killh = h;
		h = h->mLinkNext;
		delete killh;
	}
	for (f = mFirstFace; f; )
	{
		killf = f;
		f = f->mLinkNext;
		if (killf != mUnbounded)
			delete killf;	
	}
	mVertices = mHalfedges = 0;
	mFaces = 1;
	mFirstVertex = mLastVertex = NULL;
	mFirstHalfedge = mLastHalfedge= NULL;
	mFirstFace = mLastFace = mUnbounded;
	mUnbounded->mLinkNext = NULL;
	mUnbounded->mTerrainType = terrain_Water;		
	mUnbounded->mHoles.clear();
}

#define	NOT_VALID(x)	{ reason = (x); file = __FILE__; line = __LINE__; goto not_valid; }

bool			Pmwx::is_valid() const
{
	char * file = NULL;
	int line = NULL;
	char * reason = NULL;
	if (mFaces < 1) 
		NOT_VALID("Zero faces")
	if (mVertices < 0) 
		NOT_VALID("Negative Vertex count")
	if (mHalfedges < 0) 
		NOT_VALID("Negative halfedge count")
	
	if (mVertices == 0 && mFirstVertex) 
		NOT_VALID("Zero vertices but have first vertex ptr")
	if (mVertices == 0 && mLastVertex) 
		NOT_VALID("Zero vertices but have last vertex")
	if (mHalfedges == 0 && mFirstHalfedge) 
		NOT_VALID("Zero halfedges but have frist halfedge")
	if (mHalfedges == 0 && mLastHalfedge) 
		NOT_VALID("zero halfedges but have last halfedge")
	
	int fc = 0, vc = 0, hc = 0;
	for (GISVertex * v = mFirstVertex; v; v = v->mLinkNext, ++vc)
	{
		if (v->mLinkNext == DEAD_VERTEX)
			NOT_VALID("We have a dead vertex in our main vertex list.")
	
#if VALIDATE_CHECK_NO_DUPE_VERTICES	
		if (mVertexIndex.count(v->mPoint) != 1)
			NOT_VALID("Non-indexed vertex.")
		if (mVertexIndex.find(v->mPoint)->second != v)
			NOT_VALID("Vertex index pts back to wrong vertex!.")
#endif		

		if (!v->mLinkNext && v != mLastVertex) 
			NOT_VALID("Vertex has no next but is not last vertex")
		if (v->mLinkNext && v->mLinkNext->mLinkPrev != v) 
			NOT_VALID("Vertex's next's prev is not us.")
		if (v->mLinkPrev && v->mLinkPrev->mLinkNext != v) 
			NOT_VALID("Vertex'sp rev's next is not us.")
		
		if (v->halfedge()->target() != v) 
			NOT_VALID("Verticex's halfedge does not point back to vertex.")
		
		GISHalfedge * ih = v->halfedge();
		do {
			if (ih->mLinkNext == DEAD_HALFEDGE)
				NOT_VALID("We have a dead halfedge in our vertex circualation.")
			if (ih->target() != v) 
				NOT_VALID("Halfedge in vertex circulation does not point back to vertex.")
			ih = ih->next()->twin();
		} while (ih != v->halfedge());
	}
	
	for (GISFace * f = mFirstFace; f; f = f->mLinkNext, ++fc)
	{
		if (f->mLinkNext == DEAD_FACE)
			NOT_VALID("We have a dead face in our main list.")
		if (!f->mLinkNext && f != mLastFace) 
			NOT_VALID("Face has no next but is not last face.")
		if (f->mLinkNext && f->mLinkNext->mLinkPrev != f) 
			NOT_VALID("Face's next's prev is not face.")
		if (f->mLinkPrev && f->mLinkPrev->mLinkNext != f) 
			NOT_VALID("FAce's prev's next is not face.")
		
		if (f->is_unbounded() && f != mUnbounded) 
			NOT_VALID("Face is unbounded but is not the unbounded face.")
		if (!f->is_unbounded() && f == mUnbounded) 
			NOT_VALID("Face is not unbounded but is the unbounded face.")
		
		if (!f->is_unbounded())
		{
			if (f->outer_ccb()->face() != f) 
				NOT_VALID("Face's CCB's face is not face.")
			GISHalfedge * hc = f->outer_ccb();
			if (hc->mLinkNext == DEAD_HALFEDGE)
				NOT_VALID("Our outer CCB rep is a dead halfedge.")
			do {
				if (hc->mLinkNext == DEAD_HALFEDGE)
					NOT_VALID("We have a dead halfedge in our face CCB.")
				if (hc->face() != f) 
					NOT_VALID("A halfedge on the CCB doesn't point back to us.")
				if (hc->face() == f && hc->twin()->face() == f && hc->next() == hc->twin() && hc->twin()->next() == hc)
					NOT_VALID("We have an island antenna on a CCB.")
				hc = hc->next();
			} while (hc != f->outer_ccb());			
			
		}
		
		for (Holes_iterator hi = f->holes_begin(); hi != f->holes_end(); ++hi)
		{
			if ((*hi)->face() != f) 
				NOT_VALID("Face's Hole's face is not face.")
			GISHalfedge * hc = *hi;
			if (hc->mLinkNext == DEAD_HALFEDGE)
				NOT_VALID("A hole rep is a dead halfedge.")
			do {
				if (hc != *hi && f->is_hole_ccb(hc))
					NOT_VALID("Multiple hole reps in a hole ring.")
				if (hc->mLinkNext == DEAD_HALFEDGE)
					NOT_VALID("We have a dead halfedge in a hole CCB.")
				if (hc->face() != f) 
					NOT_VALID("Halfedge in a hole circ doesn't have face.")
				hc = hc->next();
			} while (hc != *hi);
		}			
	}
	
	for (GISHalfedge * h = mFirstHalfedge; h; h = h->mLinkNext, ++hc)
	{
		if (h->mLinkNext == DEAD_HALFEDGE)
			NOT_VALID("We have a dead halfedge in our main halfedge list.")
		if (!h->mLinkNext && h != mLastHalfedge) 
			NOT_VALID("Halfedge has no next but is not last halfedge")
		if (h->mLinkNext && h->mLinkNext->mLinkPrev != h) 
			NOT_VALID("Halfedge's next's prev is not halfedge.")
		if (h->mLinkPrev && h->mLinkPrev->mLinkNext != h) 
			NOT_VALID("Halfedge's prev's next is not halfedge.")
		
		if (h->next()->face() != h->face()) 
			NOT_VALID("Halfedge's next does not have same face as halfedge")
		if (h->twin()->twin() != h) 
			NOT_VALID("Halfedge's twin does not point back to us.")
	}
	
	
	// validate all faces
	
	if (mFaces != fc) 
		NOT_VALID("FAce count is out of sync.")
	if (mVertices != vc) 
		NOT_VALID("Vertex count is out of sync.")
	if (mHalfedges != hc) 
		NOT_VALID("Halfedge count is out of sync.")

	return true;	
	
not_valid:
#if DEV
	printf("Validation check failed: %s (%s:%d.)\n", reason, file, line);
#endif
	return false;
}


/*******************************************************************************************
 * LOCATION
 *******************************************************************************************/


#pragma mark -

/*
 * Utility func: better_xcross
 *
 * The problem is simple: x intersections are not reliable when taking a sliver VERY close to a vertex - 
 * two segments with distinct angles may NOT produce distinct intercepts because the fraction down the segment
 * is virtually nothing.
 *
 * This routine hacks around it - if the segments have only one vertex in common and the intersection
 * point is NOT the common vertex, we do a slope compare to see which is reall rightward.  By multiplying and
 * not dividing, we avoid rounding errors a bit and can pull out the true answer.
 *
 */
inline bool better_xcross(const Segment2& best, double best_x, const Segment2& trial, double trial_x, double y_intercept)
{
	// First case out equal segments.
		 if (best.p1 == trial.p1 && best.p2 == trial.p2) 
		return false;
	else if (best.p2 == trial.p2 && best.p1 == trial.p2) 
		return false;
	
	// If we have a horizontal segment, just do intercept - compare - this is typically left
	// over from a previosu horizotnal seg we found otherwise.
	else if (best.p1.y == best.p2.y || trial.p1.y == trial.p2.y)
		return trial_x > best_x;
	
	// Typical special case of a common vertex to avoid slivering artifacts.
	else if (best.p1 == trial.p1)
	{
		// Skip out if we're hitting the shared vertex head on.
		if (y_intercept == best.p1.y) return false;
		// Build two vectors going out from the shared vertex
		Vector2	vbest(best.p1, best.p2);
		Vector2 vtrial(trial.p1, trial.p2);
		// Make sure they all go up so we don't get a sign inversion.  We know they both go up or down - otherwise they couldn't
		// both intercept other than at the shared point.
		if (vbest.dy < 0.0) vbest.dy = -vbest.dy;
		if (vtrial.dy < 0.0) vtrial.dy = -vtrial.dy;
		// Do a safe slope compare.  Oh yeah - we know they're not horizontal - for that case this isn't called.
		return (vtrial.dx * vbest.dy) > (vbest.dx * vtrial.dy);
		
	} 
	else if (best.p1 == trial.p2)
	{
		if (y_intercept == best.p1.y) return false;
		Vector2	vbest(best.p1, best.p2);
		Vector2 vtrial(trial.p2, trial.p1);
		if (vbest.dy < 0.0) vbest.dy = -vbest.dy;
		if (vtrial.dy < 0.0) vtrial.dy = -vtrial.dy;
		return (vtrial.dx * vbest.dy) > (vbest.dx * vtrial.dy);
	} 
	else if (best.p2 == trial.p1)
	{
		if (y_intercept == best.p2.y) return false;
		Vector2	vbest(best.p2, best.p1);
		Vector2 vtrial(trial.p1, trial.p2);
		if (vbest.dy < 0.0) vbest.dy = -vbest.dy;
		if (vtrial.dy < 0.0) vtrial.dy = -vtrial.dy;
		return (vtrial.dx * vbest.dy) > (vbest.dx * vtrial.dy);
	} 
	else if (best.p2 == trial.p2)
	{
		if (y_intercept == best.p2.y) return false;
		Vector2	vbest(best.p2, best.p1);
		Vector2 vtrial(trial.p2, trial.p1);
		if (vbest.dy < 0.0) vbest.dy = -vbest.dy;
		if (vtrial.dy < 0.0) vtrial.dy = -vtrial.dy;
		return (vtrial.dx * vbest.dy) > (vbest.dx * vtrial.dy);
	}
	// Last resort - just intercept compare.
	else 
		return trial_x > best_x;
}

GISVertex *		Pmwx::locate_vertex(const Point2& p)
{
	VertexMap::iterator i = mVertexIndex.find(p);
	return (i == mVertexIndex.end()) ? NULL : i->second;
}

GISHalfedge *	Pmwx::locate_point(const Point2& p, Locate_type& loc)
{
	GISVertex *	quick;
	if ((quick = locate_vertex(p)) != NULL)
	{
		loc = locate_Vertex;
		return quick->halfedge();
		
	}
	GISFace *		owner = mUnbounded;				// This is the face we are searching inside.
	GISFace *		next_found = NULL;				// This is the best find we have so far.
	Point2 			best = Point2(-9.9e9, p.y);
	GISHalfedge * 	it, * stop;
	double			x_cross;
	Holes_iterator	hole;
	Segment2		best_seg = Segment2(Point2(-9.9e9, p.y), Point2(-9.9e9, p.y));
	
	// We're going to keep looking in a face until we can't look any more.
	
	while (1)
	{
		next_found = NULL;
		
		// Strange eh?  Do each hole and then the outer CCB.  A hack to avoid
		// writing this code twice.
		for (hole = owner->holes_begin(); ; ++hole)
		{
			// Done with holes and no outer CCB - bail
			if (hole == owner->holes_end() && owner->is_unbounded())
				break;
			if (hole == owner->holes_end())
				it = stop = owner->outer_ccb();
			else
				it = stop = *hole;
				
			// Go around an outer CCB/hole, looking for better intersections.
			do
			{
				Segment2	seg(it->source()->point(), it->target()->point());
				
				// Non-horizontal intersection case - do we cross this line?
				if ((seg.p1.y < p.y && p.y <= seg.p2.y) ||
					(seg.p1.y > p.y && p.y >= seg.p2.y))
				{
					x_cross = seg.x_at_y(p.y);
					if (better_xcross(best_seg, best.x, seg, x_cross, p.y) && x_cross <= p.x)		// Ben sez: require x_cross to always rise - this is needed when we have a corrupt map
					{											// to fail with a bad pick.  Otherwise we can loop forever - a fate worse than death!!!
						best.x = x_cross;
						best_seg = seg;

						// If we hit the point directly, mark an edge/vertex hit and bail now.
						if (best == p)
						{
							DebugAssert(p != seg.p1);
							if (seg.p2 == p)
								loc = locate_Vertex;
							else
								loc = locate_Halfedge;
							return it;
						}
						
						// If we hit the end of the segment (a vertex directly) this case requires special handling...there might be multiple
						// adjacent faces to this vertex and us, not just one!  So we do a point circulation to sort this out.
						// The face from the line just above the ray we're shooting must be the face we want to continue with.
						
						if (seg.p2.y == p.y)
						{
							GISHalfedge * right_most = it->target()->rightmost_rising();
							DebugAssert(right_most != NULL);
							next_found = right_most->face();							
						}
						else 
						{
							// We decide whether we're on the left or right side of this segment based on its going up or down. (It can't
							// be horizontal.  If the next face is us, replace null.)
							if (seg.p1.y < seg.p2.y)
							{
								next_found = it->twin()->face();
							} else {
								next_found = it->face();
							}
						}		
					}
				}
				
				// Horizontal edge case
				if (p.y == seg.p1.y && p.y == seg.p2.y)
				{
					//  If we hit this edge, we hit a vertex or halfedge - bail now.
					if ((seg.p1.x <= p.x && p.x <= seg.p2.x) || (seg.p2.x <= p.x && p.x <= seg.p1.x))
					{
						if (seg.p1 == p || seg.p2 == p)
							loc = locate_Vertex;
						else
							loc = locate_Halfedge;
						if (seg.p1 == p) 
							return it->twin();
						else
							return it;
					}
					
					if (best.x < seg.p2.x && seg.p2.x <= p.x)
					{
						DebugAssert(seg.p2.x < p.x);
						best.x = seg.p2.x;
						best_seg = seg;
						GISHalfedge * right_most = it->target()->rightmost_rising();
						DebugAssert(right_most != NULL);
						next_found = right_most->face();							
					}
				}
				it = it->next();
			} while (it != stop);
			
			if (hole == owner->holes_end())
				break;
		}
		
		// If we didn't find a better face, the hole we found is best.
		if (next_found == NULL || next_found == owner)
		{
			loc = locate_Face;
			if (owner->is_unbounded())
			{
				if (owner->holes_begin() == owner->holes_end())
					return NULL;
				return *owner->holes_begin();
			} else {				
				return owner->outer_ccb();
			}
		}
		
		owner = next_found;
	}
}

GISHalfedge *	Pmwx::ray_shoot(
						const Point2&		start,
						Locate_type			start_type,
						GISHalfedge *		start_hint,
						const Point2&		dest,
						Point2&				crossing,
						Locate_type&		loc)
{
	if (empty())
	{
		crossing = dest;
		loc = locate_Face;
		return NULL;
	}

		GISVertex *			exclude_vertex = NULL;		// If we are coming from a certain entity, 
		GISHalfedge *		exclude_halfedge = NULL;	// Make sure we don't double-hit!
		Vector2				ray(start, dest);
		ray.normalize();
		GISFace *			search_face = NULL;
		Segment2			seg;
		double 				this_dist_sqr;
		Point2				this_cross;
	if (start_type == locate_Face)
	{
		if (start_hint) search_face = start_hint->face();
		else			search_face = mUnbounded;
		
		DebugAssert(search_face != NULL);
	}

	// Preflight - if our locate type is a vertex or point, it's possible that we don't have to cross a face to get to our destination.
	if (start_type == locate_Vertex)
	{
		exclude_vertex = start_hint->target();
		// Vertex special casing - go through all of our halfedges and see if our point is along a vector or something.
		GISHalfedge * circ = start_hint;
		
		// Special case: one antenna - any face will do if we are a face search.
		if (start_hint->next() == start_hint->twin())
			search_face = start_hint->face();
		
		do {			
			GISHalfedge * next = circ->next()->twin();
			// Perhaps we have a halfedge pointing right at our destination?
			if (circ->source()->point() == dest)
			{
				crossing = dest; 
				loc = locate_Vertex;
				return circ->twin();
			}
			if (circ->target()->point() == dest)
			{
				crossing = dest; 
				loc = locate_Vertex;
				return circ;
			}
			if (next->source()->point() == dest)
			{
				crossing = dest; 
				loc = locate_Vertex;
				return next->twin();
			}
			if (next->target()->point() == dest)
			{
				crossing = dest; 
				loc = locate_Vertex;
				return next;
			}

			// Maybe a halfedge overlaps our ray?
			Segment2	circ_seg(circ->target()->point(), circ->source()->point());
			Vector2		circ_vec(circ_seg.p1, circ_seg.p2);
			circ_vec.normalize();

			if (ray.dot(circ_vec)==1.0)
			{
				if (circ_seg.collinear_has_on(dest))
				{
					crossing = dest;
					loc = locate_Halfedge;
					return circ->twin();
				} else {
					crossing = circ->source()->point();
					loc = locate_Vertex;
					return circ->twin();
				}
			}
			Segment2	next_seg(next->target()->point(), next->source()->point());
			Vector2		next_vec(next_seg.p1, next_seg.p2);
			next_vec.normalize();
			if (ray.dot(next_vec)==1.0)
			{
				if (next_seg.collinear_has_on(dest))
				{
					crossing = dest;
					loc = locate_Halfedge;
					return next->twin();
				} else {
					crossing = next->source()->point();
					loc = locate_Vertex;
					return next->twin();
				}
			}
			
			// Okay no match.  Well, see if we're in between these two halfedges - that means that
			// we have found the face we will be in.
			if (circ != next)
			if (Is_CCW_Between(Vector2(circ->target()->point(), next->source()->point()),
							   ray,
							   Vector2(next->target()->point(), circ->source()->point())))
			{
				search_face = circ->face();
			}
			
			circ = circ->next()->twin();
		} while (circ != start_hint);
		
		DebugAssert(search_face != NULL);
	}
	
	// Preflight - if our locate type is a halfedge, perhaps we can just shoot down the halfedge?
	
	if (start_type == locate_Halfedge)
	{
		exclude_halfedge = start_hint;
		seg = Segment2(start_hint->source()->point(), start_hint->target()->point());
		double		dot = ray.dot(Vector2(seg.p1, seg.p2));
		// Okay we're parallel (same or opposite direction.  We either stop in the segment or after/at the end of it.
		if (dot == 1.0)
		{
			// Seg and dot go in same direction
			if (seg.collinear_has_on(dest))
			{
				crossing = dest;
				loc = locate_Halfedge;
				return start_hint;
			} else {
				crossing = start_hint->target()->point();
				loc = locate_Vertex;
				return start_hint;
			}
		} 
		else if (dot == -1.0) 
		{
			if (seg.collinear_has_on(dest))
			{
				crossing = dest;
				loc = locate_Halfedge;
				return start_hint->twin();
			} else {
				crossing = start_hint->source()->point();
				loc = locate_Vertex;
				return start_hint->twin();
			}
		}
		else
		{
			// Remainder - we're not on the segment.  Are we on it's left side?			
			// Rotate us 90 degrees left, use as a normal
			Vector2 face_halfplane_normal(Vector2(seg.p1, seg.p2).perpendicular_ccw());
			if (face_halfplane_normal.dot(ray) > 0)
				search_face = start_hint->face();
			else
				search_face = start_hint->twin()->face();
		}
		DebugAssert(search_face != NULL);
	}
	
	// Main search - given a search face, shoot the ray through every segment we can find, get the closest result!

	GISHalfedge *	best_he = NULL;		// Contains the best intersection we have
	double			best_dist_sqr;		// sqr dist to this guy
	Point2			best_pt;			// best pt so far
	Segment2		tryseg(start, dest);
	
	GISHalfedge * it = (search_face == NULL || search_face->is_unbounded()) ? NULL : search_face->outer_ccb();
	if (it)
	do {
		seg = Segment2(it->source()->point(), it->target()->point());
		if (seg.could_intersect(tryseg))
		if (seg.intersect(tryseg, this_cross) || (Line2(tryseg) == Line2(seg) && parallel_check(tryseg, seg, this_cross)))
		{
			this_dist_sqr = Segment2(start, this_cross).squared_length();
			if (best_he == NULL || (this_dist_sqr < best_dist_sqr))
			if (it != exclude_halfedge)
			if (this_cross != start)
			if (exclude_vertex == NULL || this_cross != exclude_vertex->point())
			{
				best_he = it;
				best_dist_sqr = this_dist_sqr;
				best_pt = this_cross;
			}
		}
		
		it = it->next();
	} while (it != search_face->outer_ccb());

	for (Holes_iterator hole = search_face->holes_begin(); hole != search_face->holes_end(); ++hole)
	{
		it = *hole;
		do {
			seg = Segment2(it->source()->point(), it->target()->point());
			if (seg.could_intersect(tryseg))
			if (seg.intersect(tryseg, this_cross)  || (Line2(tryseg) == Line2(seg) && parallel_check(tryseg, seg, this_cross)))
			{
				this_dist_sqr = Segment2(start, this_cross).squared_length();
				if (best_he == NULL || (this_dist_sqr < best_dist_sqr))
				if (it != exclude_halfedge)
				if (this_cross != start)
				if (exclude_vertex == NULL || this_cross != exclude_vertex->point())
				{
					best_he = it;
					best_dist_sqr = this_dist_sqr;
					best_pt = this_cross;
				}
			}
			
			it = it->next();
		} while (it != *hole);
	}

	if (best_he == NULL)
	{
		crossing = dest;
		loc = locate_Face;		
		if (search_face->is_unbounded()) 
			return *search_face->holes_begin();
		else 
			return search_face->outer_ccb();
	} else {
	
		if (best_pt == best_he->target()->point())
		{
			crossing = best_pt;
			loc = locate_Vertex;
			return best_he;
		} else if (best_pt == best_he->source()->point()) {
			crossing = best_pt;
			loc = locate_Vertex;
			return best_he->twin();
		} else {
			crossing = best_pt;
			loc = locate_Halfedge;
			return best_he;
		}
	}

}

/*******************************************************************************************
 * TOPOLOGY EDITING
 *******************************************************************************************/


#pragma mark -

void	Pmwx::set_vertex_location(GISVertex * inVertex, const Point2& inPoint)
{
	VertexMap::iterator old = mVertexIndex.find(inVertex->mPoint);
#if DEV
	DebugAssert(old != mVertexIndex.end());
	DebugAssert(old->second == inVertex);
#endif
	if (old != mVertexIndex.end())
		mVertexIndex.erase(old);
	inVertex->mPoint = inPoint;
#if CHECK_NEW_VERTICES_FOR_DUPES
	DebugAssert(mVertexIndex.count(inPoint) == 0);
#endif
	mVertexIndex[inPoint] = inVertex;
}


GISHalfedge *			Pmwx::split_edge(GISHalfedge * inEdge, const Point2& split)
{
	DebugAssert(split != inEdge->source()->point());
	DebugAssert(split != inEdge->target()->point());
	DebugAssert(mVertexIndex.count(split) == 0);
	// Edge split.  The following items are affected:
	// There are 4 halfedges around this point.  Each of them may be affected.
	
	// The halfedge pointing to our twin now needs to point to the twin of the new edge, so
	// this is a change.
	
	// If our edge is the de facto halfedge of it's target, that's not true anymore.

	// We have to detect an edge case: if we are splitting an edge that is an antenna (e.g.. next = twin)
	// then the halfedge pointing to our twin (which must be adjusted for the split) is us.  But since we're
	// being split, the ptr is actually not who we thought by the time we get there.
	
	GISHalfedge *	points_to_our_twin = inEdge->get_pre_twin();
	bool			antenna = points_to_our_twin == inEdge;
	GISVertex *		our_old_vertex = inEdge->target();

	GISVertex *		new_ve = new_vertex(split);
	GISHalfedge *	new_he = new_edge(inEdge);
	
	// New vertex - easy.  It is pointed to by the old edge.
	new_ve->set_halfedge(inEdge);
	
	// If our old vertex was pointed to by us, that's changed.
	if (our_old_vertex->halfedge() == inEdge)
		our_old_vertex->set_halfedge(new_he);
		
	// Halfedges pointing to vertices: the new halfedge points to our old vertex.
	// Us and the new twin point to the vertex in question.  The old twin is unchanged.
	new_he->set_target(our_old_vertex);
	new_he->twin()->set_target(new_ve);
	inEdge->set_target(new_ve);

	// Faces: The new edge pair copies our faces.
	new_he->set_face(inEdge->face());
	new_he->twin()->set_face(inEdge->twin()->face());

	// Finally halfedge chaining: the new halfedge points to the next guy
	// on our CCB.  We point to our new halfedge.
	new_he->set_next(inEdge->next());
	inEdge->set_next(new_he);
	
	// And on the other side, some random halfedge that pointed to our
	// twin now points to the new twin, and of course the new twin points
	// at our twin..
	if (antenna)
		points_to_our_twin = new_he;
	points_to_our_twin->set_next(new_he->twin());
	new_he->twin()->set_next(inEdge->twin());

	return inEdge;	
}

GISHalfedge *			Pmwx::merge_edges(GISHalfedge * first, GISHalfedge * second)
{
#if DEV
	if (first->face() != second->face()) DebugAssert(!"Error - he's don't have same face.\n");
	if (first->twin()->face() != second->twin()->face()) DebugAssert(!"Error - he's twins don't have same face.\n");
	if (first->next() != second) DebugAssert(!"Error, first does not lead to second.\n");
	if (second->twin()->next() != first->twin()) DebugAssert("Error - second's twin does not lead to first's twin.\n");
#endif

	GISHalfedge * points_to_seconds_twin = second->get_pre_twin();
	
	// Vertices - make sure first's vertex gets advanced.
	// Also if second's target is using second as a ref, we need to fix that!
	first->set_target(second->target());
	if(second->target()->halfedge() == second)
		second->target()->set_halfedge(first);
		
	// Halfedges: slide past second.
	points_to_seconds_twin->set_next(first->twin());
	first->set_next(second->next());
	
	// Before we can get rid of second, fix faces.
	if (second->face()->outer_ccb() == second)
		second->face()->set_outer_ccb(first);
	if (second->twin()->face()->outer_ccb() == second->twin())
		second->twin()->face()->set_outer_ccb(first->twin());
		
	if (second->face()->is_hole_ccb(second)) { second->face()->delete_hole(second); second->face()->add_hole(first); }
	if (second->twin()->face()->is_hole_ccb(second->twin())) { second->twin()->face()->delete_hole(second->twin()); second->twin()->face()->add_hole(first->twin()); }
	
	delete_vertex(second->source());
	delete_edge(second);
	return first;	
}


/*
 * remove_edge
 *
 * When we remove an edge there are two general cases: if we are separating two faces, they by
 * definition merge.  If we have the same face on both sides, no face is destroyed because 
 * faces are onlly removed through merge.  We can compare our face with our twin's face to determine
 * the situation.
 *
 * Vertices: The target vertex of the edge or its twin will be removed if the edge's next is its twin.
 *
 * Faces: if the two faces do not match, we need to merge one.  Either one is fine.  If either halfedge
 * is not on the ccb of it's face, then it's a hole.  Wipe the hole out and keep the big cace, otherwise
 * nuke our twin's face.
 *
 * If the faces do match, then if the edges are both each other's twins, then we're an island.  Just
 * remove us from our face's hole list and we're done.
 *
 * If either of the edge's are each other's twins, we're just shortening a CCB - make sure we're not
 * our face's defining edges.
 *
 * If we have the same face but are not each other's twins, we will split into a hole.  See who has
 * the least X coordinate - the half from the edge to our twin or from our twin to us.  The one that's
 * least X is the face, the other becomes a new hole.
 *
 */

GISFace *			Pmwx::remove_edge(GISHalfedge * inEdge)
{
	// Check for twin-age.  Record either vertex as possibly being deleted.
	// If the vertex is not being deleted, make sure it doesn't have us as its twin.
	// Otherwise leave our vars 
	GISVertex *	target		= inEdge->target();
	GISVertex * twin_target	= inEdge->twin()->target();
	GISFace *	face		= inEdge->face();
	GISFace *	twin_face	= inEdge->twin()->face();
	GISHalfedge *	edge_prev = inEdge->points_to_me();
	GISHalfedge *	twin_prev = inEdge->twin()->points_to_me();
	
	GISHalfedge * old_hole;
	
	GISHalfedge * it, * stop;	
	
	GISFace * winner, * loser = NULL;		
	
	if (inEdge->next() != inEdge->twin())
	{
		if (target->halfedge() == inEdge)
			target->set_halfedge(inEdge->next()->twin());
		target = NULL;	// Preserve vertex
	}
	if (inEdge->twin()->next() != inEdge)
	{
		if (twin_target->halfedge() == inEdge->twin())
			twin_target->set_halfedge(inEdge->twin()->next()->twin());
		twin_target = NULL;	// Preserve vertex
	}
	
	// Figure out if this is the face elimination case or not.
	if (face == twin_face)
	{
		// Faces match.  Just wipe out the antenna.
		if (inEdge->next() == inEdge->twin() && inEdge->twin()->next() == inEdge)
		{
			// We are an island.  Just wipe us out as an island.
			vector<GISHalfedge *>::iterator i;
			
				 if (face->is_hole_ccb(inEdge))			face->delete_hole(inEdge);
			else if (face->is_hole_ccb(inEdge->twin()))	face->delete_hole(inEdge->twin());
			else DebugAssert(!"We have a single line floating that is not a hole.");

		} else if (inEdge->next() == inEdge->twin()) {
			// We are the first half of an antenna.  Simple.  Bypass around us
			// and make sure the face isn't using us.  Also a hole must not be
			// using us either.  (We could be an antenna sticking into the face
			// from a hole.)
			
			old_hole = inEdge->get_hole_rep();
			if (old_hole != NULL) 
			{
				face->delete_hole(old_hole);
				face->add_hole(inEdge->twin()->next());
			}

			if (face->outer_ccb() == inEdge || face->outer_ccb() == inEdge->twin())
				face->set_outer_ccb(inEdge->twin()->next());

			edge_prev->set_next(inEdge->twin()->next());

		} else if (inEdge->twin()->next() == inEdge) {
		
			// We are the second half of an antenna.  Simple.  Bypass around us
			// and make sure the face isn't using us.  Also a hole must not be
			// using us either.  (We could be an antenna sticking into the face
			// from a hole.)

			old_hole = inEdge->get_hole_rep();
			if (old_hole != NULL) 
			{
				face->delete_hole(old_hole);
				face->add_hole(inEdge->next());
			}

			if (face->outer_ccb() == inEdge || face->outer_ccb() == inEdge->twin())
				face->set_outer_ccb(inEdge->next());

			twin_prev->set_next(inEdge->next());

		} else {
		
			// We join two areas.  Those areas are either a CCB and an antenna that 
			// will become a hole, or two holes.  We'll handle the cases a bit
			// differently.
			
			old_hole = inEdge->get_hole_rep();
			if (old_hole != NULL)
			{
				// We are a hole that's being severed into two holes.  This is the
				// easy case.
				face->delete_hole(old_hole);
				DebugAssert(inEdge->get_hole_rep() == NULL);
				face->add_hole(inEdge->next());
				face->add_hole(inEdge->twin()->next());
			} else {
				// Hard case - we're an outer CCB - which side of the chain is becoming a hole inside the CCB?
				Point2	best1 = inEdge->target()->point();
				Point2	best2 = inEdge->twin()->target()->point();

				lesser_x_then_y	comp;

				for (it = inEdge->next(); it != inEdge->twin(); it = it->next())
				{
					if (comp(it->target()->point(), best1))
						best1 = it->target()->point();
				}
				for (it = inEdge->twin()->next(); it != inEdge; it = it->next())
				{
					if (comp(it->target()->point(), best2))
						best2 = it->target()->point();
				}
				
				DebugAssert(best1 != best2);
				if (comp(best1, best2))
				{
					face->set_outer_ccb(inEdge->next());
					face->add_hole(inEdge->twin()->next());
				} else {
					face->set_outer_ccb(inEdge->twin()->next());
					face->add_hole(inEdge->next());
				}
			}

			edge_prev->set_next(inEdge->twin()->next());
			twin_prev->set_next(inEdge->next());
		}
		
	} else {

		// Face Merge case.  First - we must figure out which face dies.  Either both faces are
		// neighbors, or one is in the other.  Here's how we find out...
		
		GISHalfedge *	edge_hole = inEdge->get_hole_rep();
		GISHalfedge *	twin_hole = inEdge->twin()->get_hole_rep();
		
		if (edge_hole == NULL && twin_hole == NULL)
		{
			// Neighbors case.  Delete twin's face.
			winner = inEdge->face();
			loser = inEdge->twin()->face();
			DebugAssert(winner != mUnbounded && loser != mUnbounded);
			if (winner->outer_ccb() == inEdge)
				winner->set_outer_ccb(inEdge->next());			
		} 
		else if (edge_hole == NULL) 
		{
			// Twin is the hole.
			winner = inEdge->twin()->face();
			loser = inEdge->face();
			DebugAssert(winner->is_hole_ccb(twin_hole));
			DebugAssert(loser != mUnbounded);
			if (twin_hole == inEdge->twin())
			{
				winner->delete_hole(inEdge->twin());
				winner->add_hole(inEdge->twin()->next());
			}
			if (winner->outer_ccb() == inEdge->twin())
				winner->set_outer_ccb(inEdge->twin()->next());		
		} 
		else if (twin_hole == NULL) 
		{
			// Our edge is the hole.
			winner = inEdge->face();
			loser = inEdge->twin()->face();
			DebugAssert(loser != mUnbounded);
			DebugAssert(winner->is_hole_ccb(edge_hole));
			if (edge_hole == inEdge)
			{
				winner->delete_hole(edge_hole);
				winner->add_hole(inEdge->next());
			}
			if (winner->outer_ccb() == inEdge)
				winner->set_outer_ccb(inEdge->next());
		}
		else {
			// This is a halfedge...both sides of it appear to be on holes in a bigger face.  But this
			// is impossible.  If both faces were holes then this edge would have to be part of both
			// of their CCBs.  If one is a hole the other must contain it.  
			
			// The one case where we get borked sometimes is due to a bug in the TIGER import code where
			// we eliminate a hole in a face's inside but not the outer face, because the outer face is
			// in the cull region and the inner is not.  The result is an edge whose one side is a hole
			// in the kept face and the other is part of the unbounded face, and therefore by definition
			// a hole too.
			
			// Before we assert that we are indeed totally f--ed, just double check that indeed the unbounded
			// face is one of the offenders, and this is indeed an orphaned hole so to speak.
//			DebugAssert(edge_hole->face() == mUnbounded || twin_hole->face() == mUnbounded);
			Assert(!"Found two holes that share an edge but have different faces.");
		}

		// Patch edges.
		edge_prev->set_next(inEdge->twin()->next());
		twin_prev->set_next(inEdge->next());
		
		// Now we have a face that will die.  We need to migrate its holes.
		for (Holes_iterator hole_iter = loser->holes_begin(); hole_iter != loser->holes_end(); ++hole_iter)
		{
			winner->add_hole(*hole_iter);
			stop = it = *hole_iter;
			do {
				it->set_face(winner);
				it = it->next();
			} while (it != stop);
		}
		
		// Also conform our CCB.  there is no need to do this for our twin...
		// the rings of the edge and the twin are now joined.
		
		it = stop = inEdge->next();
		do {
			it->set_face(winner);
			it = it->next();
		} while (it != stop);


		// Delete the loser face.
		loser->mOuterCCB = NULL; 
		delete_face(loser);
	}
		
	// Go back and nuke vertices and edge now that we're all done
	if (target)				delete_vertex(target);
	if (twin_target)		delete_vertex(twin_target);	
							delete_edge(inEdge);
							
	return loser;
}
		
/*
 * Edge Insertion
 * 
 * To insert an edge, we really insert a series of edges from the start to
 * end point, with a vertex inserted in every place where we have a point.
 * This will be up to two insertions from a vertex, zero or more insertions
 * between vertices, maybe only an insert in hole, and zero or more splits.
 
 *
 */
GISHalfedge *		Pmwx::insert_edge(const Point2& p1, const Point2& p2,
						void(* notifier)(GISHalfedge *, GISHalfedge *, void *), void * ref)
{
	GISHalfedge *	new_he;

	// Special case for empty map:
	if (empty())
	{
		new_he = insert_edge_in_hole(unbounded_face(), p1, p2);
		if (notifier) 
			notifier(NULL, new_he, ref);
		return new_he;
	}
	
	Point2			cur, 		found;
	Locate_type		cur_loc;
	GISHalfedge *	cur_he;
	
	// FIrst initialize - find our start point cold.  If it's on a halfedge
	// introduce a split.

	cur = p1;
	cur_he = locate_point(cur, cur_loc);
	
	return insert_edge(p1, p2, cur_he, cur_loc, notifier, ref);
}	
	
GISHalfedge * Pmwx::insert_edge(const Point2& p1, const Point2& p2, GISHalfedge * hint, Locate_type location,
						void(* notifier)(GISHalfedge *, GISHalfedge *, void *), void * ref)
{
	DebugAssert(p1 != p2);
	
	GISHalfedge *	new_he;

	Point2			cur, 		found;
	Locate_type		cur_loc, 	found_loc;
	GISHalfedge *	cur_he,		* found_he;
	GISHalfedge * 	last = NULL;
	cur = p1;
	cur_loc = location;
	cur_he = hint;

	if (cur_loc == locate_Halfedge)
	{
		cur_he = split_edge(cur_he, cur);
		if (notifier)
			notifier(cur_he, cur_he->next(), ref);
		cur_loc = locate_Vertex;
	}
		
	while (cur != p2)
	{
		// Shoot a ray toward p2 and see what we hit!
 		found_he = ray_shoot(cur, cur_loc, cur_he, 
							p2, 
							found, found_loc);
		
		// Of course if we hit an edge, split and go on
		if (found_loc == locate_Halfedge)
		{
			found_he = split_edge(found_he, found);
			if (notifier)
				notifier(found_he, found_he->next(), ref);
			
			found_loc = locate_Vertex;
		}
		
		// Four cases:
		if (cur_loc == locate_Face && found_loc == locate_Face)
		{
			// Two faces - make sure that this is the simple in-face case!
			// In fact, we can quit early here!
#if DEV
			DebugAssert(cur_he->face() == found_he->face());			
			DebugAssert(found == p2);
#endif			
			new_he = insert_edge_in_hole(cur_he->face(), cur, found);
			if (notifier) notifier(NULL, new_he, ref);			
			return new_he;			
		} 
		else if (cur_loc == locate_Face) 
		{
			// We're starting in a face, but ended up at a vertex.
			// Build from found_he to cur.
			new_he = insert_edge_from_vertex(get_preceding(found_he, cur), cur);
			if (notifier) notifier(NULL, new_he->twin(), ref);	
			last = new_he->twin();		
		} 
		else if (found_loc == locate_Face) 
		{
			// We're starting in vertex and ending in a face.
			// Build from cur_he to found, and we're done.
#if DEV
			DebugAssert(found == p2);
#endif						
			new_he = insert_edge_from_vertex(get_preceding(cur_he, found), found);
			if (notifier) notifier(NULL, new_he, ref);
			return new_he;
		}
		else 
		{
			// If our points are not connected, connect them.
			new_he = vertices_connected(cur_he->target(), found_he->target());
			if (new_he == NULL)	// Not connected - need to insert
			{
				new_he = insert_edge_between_vertices(
					get_preceding(cur_he, found),
					get_preceding(found_he, cur));
				if (notifier) notifier(NULL, new_he, ref);
			} else 
				if (notifier) notifier(new_he, NULL, ref);
			last = new_he;
		}
		
		cur = found;
		cur_he = found_he;
		cur_loc = found_loc;
	}
	DebugAssert(last != NULL);
	return last;
}

/*****************************************************************************
* SPECIALIZED TOPOLOGICAL EDITING - USEFUL WHEN YOU KNOW THINGS ABOUT THE MAP
*****************************************************************************/


GISFace * Pmwx::insert_ring(GISFace * parent, const vector<Point2>& inPoints)
{
	vector<GISVertex *>		vertices(inPoints.size());
	vector<GISHalfedge *>	edges(inPoints.size());
	GISFace *				nface = new_face();
	
	int		n, c = inPoints.size();
	
	for (n = 0; n < c; ++n)
	{
		edges[n] = new_edge();
		vertices[n] = new_vertex(inPoints[n]);
	}
	
	for (n = 0; n < c; ++n)
	{
		vertices[n]->set_halfedge(edges[n]);
		edges[n]->set_target(vertices[n]);
		edges[n]->twin()->set_target(vertices[(n+c-1)%c]);
		edges[n]->set_face(nface);
		edges[n]->twin()->set_face(parent);		
		edges[n]->set_next(edges[(n+1)%c]);
		edges[n]->twin()->set_next(edges[(n+c-1)%c]->twin());
	}
	parent->add_hole(edges[0]->twin());
	nface->set_outer_ccb(edges[0]);	
	return nface;
}

GISHalfedge *	Pmwx::nox_insert_edge_in_hole(const Point2& p1, const Point2& p2)
{
	if (empty())
		return insert_edge_in_hole(unbounded_face(), p1, p2);
		
	Locate_type le;
	
	GISHalfedge * hint = locate_point(p1, le);
	DebugAssert(le == locate_Face);
	return insert_edge_in_hole(hint ? hint->face() : unbounded_face(), p1, p2);
}

GISHalfedge *	Pmwx::nox_insert_edge_from_vertex(GISVertex * p1, const Point2& p2)
{
	return insert_edge_from_vertex(get_preceding(p1->halfedge(), p2), p2);
}

GISHalfedge *	Pmwx::nox_insert_edge_between_vertices(GISVertex * p1, GISVertex * p2)
{
	GISHalfedge * check = vertices_connected(p1, p2);
	if (check) return check;

	return insert_edge_between_vertices(
		get_preceding(p1->halfedge(), p2->point()),
		get_preceding(p2->halfedge(), p1->point()));
}

/*******************************************************************************************
 * MISC STUFF
 *******************************************************************************************/

double Pmwx::smallest_dist(Point2& p1, Point2& p2)
{
	if (mVertices < 2) return 0.0;
	double	small_sq = 9.9e9;
	
	for (GISVertex * i = mFirstVertex; i != NULL; i = i->mLinkNext)
	for (GISVertex * j = i->mLinkNext; j != NULL; j = j->mLinkNext)
	{
		DebugAssert(i != j);
		DebugAssert(i->mPoint != j->mPoint);
		
		double dx = i->mPoint.x - j->mPoint.x;
		double dy = i->mPoint.y - j->mPoint.y;
		double	local_sq = dx * dx + dy * dy;
		if (local_sq < small_sq)
		{
			p1 = i->mPoint;
			p2 = j->mPoint;
			small_sq = local_sq;
		}
	}
	
	return sqrt(small_sq);
}

/*******************************************************************************************
 * TOPOLOGY SUBROUTINE HELPERS!
 *******************************************************************************************/


#pragma mark -

// Given two vertices, returns a halfedge from v1 to v2 if it exists, or NULL
// if they are not directly connected.
GISHalfedge *		Pmwx::vertices_connected(GISVertex * v1, GISVertex * v2)
{
	GISHalfedge * stop = v1->halfedge();
	GISHalfedge * it = stop;
	// Go through all edges pointing at v1.
	do {
#if DEV
		DebugAssert(it->target() == v1);
#endif	
		// If our twin points at v2, we're connceted.
		if (it->twin()->target() == v2) return it->twin();
		// Go to next guy pointing at v1.
		it = it->next()->twin();
	} while (stop != it);
	return NULL;
}


// Given a halfedge pointing to a vertex, find the halfedge also pointing to the vertex
// that is just before the segment from that vertex to p.
GISHalfedge *	Pmwx::get_preceding(GISHalfedge * points_to_vertex, const Point2& p)
{
	// Special case - if there is only one halfedge pointing to the vertex, the halfedge
	// is by definition the one we want.
	if (points_to_vertex->next()->twin() == points_to_vertex) return points_to_vertex;
	
	GISHalfedge * it = points_to_vertex;
	do {
		GISHalfedge * next = it->next()->twin();
		// Make sure we only use one origin.
		DebugAssert(it->target() == next->target());
		DebugAssert(it->target() == points_to_vertex->target());
		// Make sure we have three distinct vectors!
		DebugAssert(it->source()->point() != next->source()->point());
		DebugAssert(it->source()->point() != p);
		if (Is_CCW_Between(
				Vector2(it->target()->point(), next->source()->point()),
				Vector2(it->target()->point(), p),
				Vector2(it->target()->point(), it->source()->point())))
		{
			return it;
		}

		it = next;		
	} while (it != points_to_vertex);
	Assert(!"Never found the insert CCB vector.");
	return points_to_vertex;
}

// Insert an edge entirely in a face.  Pass in the face, edge from p1 to p2 is returned.
GISHalfedge *	Pmwx::insert_edge_in_hole(GISFace * face, const Point2& p1, const Point2& p2)
{
	DebugAssert(p1 != p2);
	// Easy - we just make a self contained island.
	GISVertex * v1 = new_vertex(p2);
	GISVertex * v2 = new_vertex(p1);
	GISHalfedge * e = new_edge();
	e->set_target(v1);
	e->twin()->set_target(v2);
	v1->set_halfedge(e);
	v2->set_halfedge(e->twin());
	e->set_next(e->twin());
	e->twin()->set_next(e);
	e->set_face(face);
	e->twin()->set_face(face);
	face->add_hole(e);
	return e;
}

// Insert an edge into a face that touches one vertex.  Pass in a half-edge whose target is
// the insert point and whose face is the face to insert in.  Get back the edge that point to p.
// REQUIREMENT: adj be the halfedge preceeding the he we will insert!
GISHalfedge *	Pmwx::insert_edge_from_vertex(GISHalfedge * adj, const Point2& p)
{
	DebugAssert(p != adj->target()->point());
	
	// Also easy - we are making an antenna.
	GISVertex * v = new_vertex(p);
	GISHalfedge * e = new_edge();
	v->set_halfedge(e);
	e->set_target(v);
	e->twin()->set_target(adj->target());
	e->set_face(adj->face());
	e->twin()->set_face(adj->face());
	e->set_next(e->twin());
	e->twin()->set_next(adj->next());
	adj->set_next(e);
	return e;
}

// Insert an edge splitting a face.  The halfedge runs from the target of e1 to the target of
// e2.  The face that e1 and e2 have in common is the face that is split.  The halfedge
// from e1's target to e2's target is returned.  Note that if e1 and e2 do not have the same
// face then the edge insert is intersecting.
// REQUIREMENT: e1 and e2 must be the preceding edges to our new one.

/* TOPOLOGY 
There are a few cases:
 1a. e1 and e2 are both holes, but different holes.
 
 	These two holes are being joined. 	
 	One of the two hole needs to be deleted from the face.
 	A new face is not introduced.
 	No holes need to be moved to the new face.

 1b. e1 and e2 are both part of the same hole

	We are closing off this hole to become its own face.
 	The hole is not deleted.
 	A new face is introduced.
 	Some holes may need to be moved to the new face.
	
 2. e1 is a hole and e2 is not (or vice versa):
 
 	A hole is being connected to the CCB forming a new CCB.
 	The hole is deleted.
 	A new face is not introduced.
 	No holes need to be moved.
 
 3. e1 and e2. are both outer ccb
 
 	This halfedge splits the face in half.
 	No hole are deleted.
 	A new face is introduced.
 	Holes may need to be moved to the new face.
*/


GISHalfedge *	Pmwx::insert_edge_between_vertices(GISHalfedge * e1, GISHalfedge * e2)
{
	DebugAssert(e1 != e2);
	DebugAssert(e1->target() != e2->target());
	DebugAssert(e1->target()->point() != e2->target()->point());
	DebugAssert(e1->face() == e2->face());

	GISHalfedge * e1_hole = e1->get_hole_rep();
	GISHalfedge * e2_hole = e2->get_hole_rep();

	GISFace *	  old_f = e1->face();
	GISFace *	  new_f = NULL;


	GISHalfedge * it, * stop;
	GISHalfedge * e = new_edge();
	
	// This stuff is invariant - set E's place in the map based on e1 and e2.
	e->set_target(e2->target());
	e->twin()->set_target(e1->target());	
	e->set_next(e2->next());
	e->twin()->set_next(e1->next());
	e1->set_next(e);
	e2->set_next(e->twin());
	e->set_face(old_f);			// For now use old face - code that makes
	e->twin()->set_face(old_f);	// new faces can fix this.

	
	/* Case 1a - e1 and e2 were on different holes. */
	if (e1_hole != e2_hole && e1_hole != NULL && e2_hole != NULL)
	{
 		// These two holes are being joined. 	
 		// One of the two hole needs to be deleted from the face.
		DebugAssert(old_f->is_hole_ccb(e2_hole));
 		old_f->delete_hole(e2_hole); 		
	}
	/* Case 1b - e1 and e2 are the same hole. */
	else if (e1_hole == e2_hole && e1_hole != NULL && e2_hole != NULL)
	{
		// We are closing off this hole to become its own face.
		// This is tricky: what's the outside and what's the inside
		// Well, the ring with e is either an outer ccb going CCW or
		// a hole going CW.  If e's ring is clockwise, that means 
		// e is actually the hole.
		
		// Minor detail - the hole's rep halfedge may be on the inside of
		// the new face.  Not good.  Remove it now, add a different hole
		// that's a sure bet.
		DebugAssert(old_f->is_hole_ccb(e1_hole));
		old_f->delete_hole(e1_hole);
		
		GISHalfedge * on_new_face = NULL;
		
		// Figuring out if our edge is in the new face or not is a real
		// pain in the ass.  There are basically two cases: one is where the
		// outer edge of the hole has antennas all over the place.  In this
		// case the left most on the outer ring may be more left than the left
		// most on the inner ring.
		// 
		// If the 
		
		GISHalfedge * our_ring_left = e->get_leftmost();
		GISHalfedge * twin_ring_left = e->twin()->get_leftmost();
		
		if (our_ring_left->target() == twin_ring_left->target())
		{
			// In this case our ring is truly just a ring - no antennas hanging off to give away who the outside is.
			// So...we do a signed area of the edge "E".  If it comes out positive, we know that the boundary from
			// E is counter clockwise and must be the CCB of the new face.
			double s1 = 0.0;
			it = stop = e;
			do {
				if (it != stop && it->next() != stop)
					s1 += Vector2(stop->source()->point(), it->source()->point()).signed_area(Vector2(it->source()->point(), it->target()->point()));
				it = it->next();
			} while (it != stop);
			
			DebugAssert(s1 != 0.0);			// Area should not be zero - we don't make faces without area!
			bool	e_is_ccw = (s1 > 0.0);

			if (e_is_ccw)
				on_new_face = e;
			else
				on_new_face = e->twin();
		} else {
		
			// Other case...one ring is more to the left than the other - leftmost antennas must be on the outside
			// so that can be used to determine our hole.
			
			// TODO: can we just do the above case?  That'd safe us a trip around a CCB.		
			if (our_ring_left->target()->point().x < twin_ring_left->target()->point().x)
				on_new_face = e->twin();
			else if (our_ring_left->target()->point().x == twin_ring_left->target()->point().x &&
					 our_ring_left->target()->point().y < twin_ring_left->target()->point().y)
				on_new_face = e->twin();
			else
				on_new_face = e;
		}
		// Now go through and set up the new face.
		new_f = new_face(old_f);
		new_f->set_outer_ccb(on_new_face);
		
		it = stop = on_new_face;
		do {
			it->set_face(new_f);
			it = it->next();
		} while (it != stop);

		old_f->add_hole(on_new_face->twin());

		// We need to remember the hole that is 'us' so we don't try to move ourselves later.
		// but since we're reassigned the hole that contains this new face, make sure we update
		// e1_hole for later.
		e1_hole = on_new_face->twin();
	}
	/* Case 2 - one of e1 or e2 is a hole, the other isn't. */
	else if (e1_hole != NULL || e2_hole != NULL)
	{		
		// A hole is being connected to the CCB forming a new CCB.
		if (e1_hole) old_f->delete_hole(e1_hole);
		if (e2_hole) old_f->delete_hole(e2_hole);	
	}
	/* Case 3 - e1 and e2 are both outer CCB. */
	else
	{
		// This halfedge splits the face in half.  The left side of our new edge
		// will be part of the new face.  For simplicity, set the old face's half
		// edge to be our twin, so we don't have to figure out if we stole its
		// representative halfedge.  Go through the new edge and make sure they all
		// point to our new face.		

		new_f = new_face(old_f);
		old_f->set_outer_ccb(e->twin());
		new_f->set_outer_ccb(e);
		
		it = stop = e;
		do {
			it->set_face(new_f);
			it = it->next();
		} while (it != stop);
	}
	
	// If we created a new face, some of the holes from old_face may actually be in the new face.
	// Why don't we use the even-odd crossing system?  Well...the problem is that we have to special
	// case the "V" shape, e.g. a perfect ray shoot over the end of a V should be ignored.  But
	// a horizontal segment inserted in the middle makes this difficult to determine.

	// So instead we use sort of a "rightmost" type algorithm where we simply find the side that's
	// closest to us and take advantage of its up/down orientation to determine whether we're inside
	// or outside.
	if (new_f)
	{
		set<GISHalfedge *>	moving;
		GISHalfedge * 		right_most;
		for (Holes_iterator hi = old_f->holes_begin(); hi != old_f->holes_end(); ++hi)
		{
			if (*hi == e1_hole) continue;
			it = stop = new_f->outer_ccb();
			Point2 p = (*hi)->target()->point();
			Point2 best = Point2(-9.9e9, p.y);
			Segment2 best_seg(best, best);
			double guess;
			bool inside = false;
			do {
				DebugAssert(it != *hi);
				Point2 p1 = it->source()->point();
				Point2 p2 = it->target()->point();
				Segment2 trial(p1, p2);
				
				// Non-horizontal line case: figure out if our intercept is better but not past the check point.
				// In that case the halfedge defines which face we are in.
				if ((p.y > p1.y && p.y <= p2.y) ||
					(p.y < p1.y && p.y >= p2.y))
				{
					guess = trial.x_at_y(p.y);
					DebugAssert(guess != p.x);
//					if (best.x <= guess && guess < p.x)
					if (better_xcross(best_seg, best.x, trial, guess, p.y) && guess < p.x)
					{
						if (p1.y < p2.y)							// If the side is rising, then its face is on the
							inside = (it->twin()->face() == new_f);	// left.  We're only in us if our twin's face is 
						else										// us.  If the side is falling, then the side's
							inside = (it->face() == new_f);			// face is us if we're inside.
					
						// Special case: if we intersect a vertex, this halfedge may not be the best one.  Find the
						// "rightmost_rising" to solve it.
						if (p2.y == p.y)
						{
							right_most = it->target()->rightmost_rising();
							DebugAssert(right_most != NULL);
							inside = (right_most->face() == new_f);
						}
						best.x = guess;
						best_seg = trial;
					}
				}
				
				// Horizontal case based on P2 (hence only using P2 in the X-test)
				// Again the vertex may not be the one that best defines the location.
				if (p1.y == p.y && p.y == p2.y &&  best.x < p2.x && p2.x < p.x)
				{
					right_most = it->target()->rightmost_rising();
					DebugAssert(right_most != NULL);
					inside = (right_most->face() == new_f);
					best.x = p2.x;
					best_seg = trial;
				}
				
				it = it->next();
			} while (it != stop);
			
			if (inside)
			{
				moving.insert(*hi);
			}		
		}
		
		for(set<GISHalfedge *>::iterator moved = moving.begin(); moved != moving.end(); ++moved)
		{
			DebugAssert(old_f->is_hole_ccb(*moved));
			old_f->delete_hole(*moved);
			new_f->add_hole(*moved);
			
			it = *moved;
			do {
				it->set_face(new_f);
				it = it->next();
			} while (it != *moved);
		}
	}
	
	return e;	
}

/*******************************************************************************************
 * DELETION
 *******************************************************************************************/

#pragma mark -

void			Pmwx::delete_vertex(GISVertex * ve)
{
#if DEV
	DebugAssert(mVertexIndex.count(ve->mPoint) == 1);
	DebugAssert(mVertexIndex[ve->mPoint] == ve);
#endif
	mVertexIndex.erase(ve->mPoint);

	// First patch us out of the inline
	if (ve->mLinkPrev)	ve->mLinkPrev->mLinkNext = ve->mLinkNext;
	if (ve->mLinkNext)	ve->mLinkNext->mLinkPrev = ve->mLinkPrev;
	
	// Also if we're first or last update.
	
	if (ve == mFirstVertex)		mFirstVertex = ve->mLinkNext;
	if (ve == mLastVertex)		mLastVertex = ve->mLinkPrev;
	
	--mVertices;
	
	ve->mLinkNext = DEAD_VERTEX;
	ve->mLinkPrev = DEAD_VERTEX;
	delete ve;
}

void			Pmwx::delete_halfedge(GISHalfedge * he)
{
#if CHECK_DELETE_USED_HALFEDGE
	for (GISFace * check_face = mFirstFace; check_face != NULL; check_face = check_face->mLinkNext)
	{
		DebugAssert(check_face->outer_ccb() != he);
		DebugAssert(!check_face->is_hole_ccb(he));
	}
#endif
	// First patch us out of the inline
	if (he->mLinkPrev)	he->mLinkPrev->mLinkNext = he->mLinkNext;
	if (he->mLinkNext)	he->mLinkNext->mLinkPrev = he->mLinkPrev;
	
	DebugAssert(he->face()->outer_ccb() != he);
	
	// Also if we're first or last update.
	
	if (he == mFirstHalfedge)		mFirstHalfedge = he->mLinkNext;
	if (he == mLastHalfedge)		mLastHalfedge = he->mLinkPrev;
	
	--mHalfedges;
	he->mLinkNext = DEAD_HALFEDGE;
	he->mLinkPrev = DEAD_HALFEDGE;	
	delete he;
}

void			Pmwx::delete_edge(GISHalfedge * he)
{
	GISHalfedge * twin = he->twin();
	delete_halfedge(he);
	delete_halfedge(twin);
}

void			Pmwx::delete_face(GISFace * fe)
{
	// First patch us out of tfe inline
	if (fe->mLinkPrev)	fe->mLinkPrev->mLinkNext = fe->mLinkNext;
	if (fe->mLinkNext)	fe->mLinkNext->mLinkPrev = fe->mLinkPrev;
	
	// Also if we're first or last update.
	
	if (fe == mFirstFace)		mFirstFace = fe->mLinkNext;
	if (fe == mLastFace)		mLastFace = fe->mLinkPrev;

	fe->mLinkNext = DEAD_FACE;
	fe->mLinkPrev = DEAD_FACE;	
	
	--mFaces;
	delete fe;
}

/*****************************************************************************
 * SPATIAL INDEXING
 *****************************************************************************/
#pragma mark -

void		Pmwx::FindFaceTouchesPt(const Point2& p1, vector<GISFace *>& ids)
{
	mFaceBuckets.FindTouchesPt(p1, ids);
}

void		Pmwx::FindFaceTouchesRect(const Point2& p1, const Point2& p2, vector<GISFace *>& ids)
{
	mFaceBuckets.FindTouchesRect(p1, p2, ids);
}

void		Pmwx::FindFaceFullyInRect(const Point2& p1, const Point2& p2, vector<GISFace *>& ids)
{
	mFaceBuckets.FindFullyInRect(p1, p2, ids);
}


void		Pmwx::FindHalfedgeTouchesPt(const Point2& p1, vector<GISHalfedge *>& ids)
{
	mHalfedgeBuckets.FindTouchesPt(p1, ids);
}
void		Pmwx::FindHalfedgeTouchesRect(const Point2& p1, const Point2& p2, vector<GISHalfedge *>& ids)
{
	mHalfedgeBuckets.FindTouchesRect(p1, p2, ids);
}
void		Pmwx::FindHalfedgeFullyInRect(const Point2& p1, const Point2& p2, vector<GISHalfedge *>& ids)
{
	mHalfedgeBuckets.FindFullyInRect(p1, p2, ids);
}

void		Pmwx::FindVerticesTouchesPt(const Point2& p1, vector<GISVertex *>& ids)
{
	mVertexBuckets.FindTouchesPt(p1, ids);
}

void		Pmwx::FindVerticesTouchesRect(const Point2& p1, const Point2& p2, vector<GISVertex *>& ids)
{
	mVertexBuckets.FindTouchesRect(p1, p2, ids);
}

void		Pmwx::FindVerticesFullyInRect(const Point2& p1, const Point2& p2, vector<GISVertex *>& ids)
{
	mVertexBuckets.FindFullyInRect(p1, p2, ids);
}

void		Pmwx::Index(void)
{
	mFaceBuckets.RemoveAllAndDestroy();
	mHalfedgeBuckets.RemoveAllAndDestroy();
	mVertexBuckets.RemoveAllAndDestroy();

	Point2	minp(9999, 9999), maxp(-9999, -9999);
	for (Pmwx::Vertex_iterator i = vertices_begin(); i != vertices_end(); ++i)
	{
		minp.x = min(minp.x, i->point().x);
		minp.y = min(minp.y, i->point().y);
		maxp.x = max(maxp.x, i->point().x);
		maxp.y = max(maxp.y, i->point().y);		
	}
	double xd = maxp.x - minp.x;	double yd = maxp.y - minp.y;
	xd *= 0.001;					yd *= 0.001;
	minp.x -= xd;					minp.y -= yd;
	maxp.x += xd;					maxp.y += yd;
	mFaceBuckets.Reset(8, minp, maxp, bucket_Organize_Test);
	mHalfedgeBuckets.Reset(8, minp, maxp, bucket_Organize_Test);
	mVertexBuckets.Reset(8, minp, maxp, bucket_Organize_Test);

	for (Pmwx::Face_iterator i = faces_begin(); i != faces_end(); ++i)
	{
		if (!i->is_unbounded())
			mFaceBuckets.Insert(i);
	}

	for (Pmwx::Halfedge_iterator i = halfedges_begin(); i != halfedges_end(); ++i)
	{
		if (i->mDominant)
			mHalfedgeBuckets.Insert(i);
	}

	for (Pmwx::Vertex_iterator i = vertices_begin(); i != vertices_end(); ++i)
	{
		mVertexBuckets.Insert(i);
	}
}


void	MapFaceBucketTraits::GetObjectBounds(Object o, Point2& p1, Point2& p2)
{
	Polygon2	poly;
	
	if (o->is_unbounded())
	{
		p1 = Point2(-9999.0, -9999.0);
		p2 = Point2(-9998.0, -9998.0);
		return;
	}

	Pmwx::Ccb_halfedge_circulator	circ = o->outer_ccb();
	Pmwx::Ccb_halfedge_circulator	start = circ;
	o->mBoundsCache = Bbox2(circ->source()->point());
	do {
		o->mBoundsCache += circ->source()->point();
		++circ;
	} while (circ != start);

	p1 = Point2(o->mBoundsCache.xmin(), o->mBoundsCache.ymin());
	p2 = Point2(o->mBoundsCache.xmax(), o->mBoundsCache.ymax());		
}

bool	MapFaceBucketTraits::ObjectTouchesPoint(Object o, const Point2& p)
{
	if (o->is_unbounded()) return false;
	
	if (p.x < o->mBoundsCache.xmin() ||
		p.x > o->mBoundsCache.xmax() ||
		p.y < o->mBoundsCache.ymin() ||
		p.y > o->mBoundsCache.ymax())	return false;

	Polygon2	poly;
	Pmwx::Ccb_halfedge_circulator	circ = o->outer_ccb();
	Pmwx::Ccb_halfedge_circulator	start = circ;
	do {
		poly.push_back(circ->source()->point());
		
		++circ;
	} while (circ != start);

	if (poly.inside(p))
	{
		for (Pmwx::Holes_iterator h = o->holes_begin(); h != o->holes_end(); ++h)
		{
			Polygon2	poly2;
			circ = *h;
			start = circ;
			do {
				poly2.push_back(circ->source()->point());
				
				++circ;
			} while (circ != start);
			if (poly2.inside(p))
				return false;
		}
		return true;
	}
	return false;
}

bool	MapFaceBucketTraits::ObjectTouchesRect(Object o, const Point2& p1, const Point2& p2)
{
	if (o->is_unbounded()) return false;

	Bbox2			selection(
						p1.x,
						p1.y,
						p2.x,
						p2.y);

	return o->mBoundsCache.overlap(selection);	
}

bool	MapFaceBucketTraits::ObjectFullyInRect(Object o, const Point2& p1, const Point2& p2)
{
	if (o->is_unbounded()) return false;

	Bbox2	selection(
					p1.x,
					p1.y,
					p2.x,
					p2.y);

	return (
			o->mBoundsCache.xmin() >= selection.xmin() &&
			o->mBoundsCache.ymin() >= selection.ymin() &&
			o->mBoundsCache.xmax() <= selection.xmax() &&
			o->mBoundsCache.ymax() <= selection.ymax());
}

void	MapFaceBucketTraits::DestroyObject(Object o)
{
}


#pragma mark -

void	MapHalfedgeBucketTraits::GetObjectBounds(Object o, Point2& p1, Point2& p2)
{
	Bbox2	box(o->source()->point());
	box += o->target()->point();
	
	p1 = Point2(box.xmin(), box.ymin());
	p2 = Point2(box.xmax(), box.ymax());		
}

bool	MapHalfedgeBucketTraits::ObjectTouchesPoint(Object o, const Point2& p)
{
	return false;

	Segment2	seg(o->source()->point(), o->target()->point());
	
	Point2	proj = seg.projection(p);
	if (!seg.collinear_has_on(proj)) return false;
	
	return true;
}

bool	MapHalfedgeBucketTraits::ObjectTouchesRect(Object o, const Point2& p1, const Point2& p2)
{
	Bbox2	selection(
						p1.x,
						p1.y,
						p2.x,
						p2.y);
	Bbox2	box(o->source()->point());
	 box += o->target()->point();

	// Warning: this isn't quite right...it's overzealous.
	return box.overlap(selection);	
}

bool	MapHalfedgeBucketTraits::ObjectFullyInRect(Object o, const Point2& p1, const Point2& p2)
{
	Bbox2	selection(
						p1.x,
						p1.y,
						p2.x,
						p2.y);
	Bbox2	box(o->source()->point());
	box += o->target()->point();

	return (
			box.xmin() >= selection.xmin() &&
			box.ymin() >= selection.ymin() &&
			box.xmax() <= selection.xmax() &&
			box.ymax() <= selection.ymax());
}

void	MapHalfedgeBucketTraits::DestroyObject(Object o)
{
}


#pragma mark -

void	MapVertexBucketTraits::GetObjectBounds(Object o, Point2& p1, Point2& p2)
{
	p1 = o->point();
	p2 = o->point();
}

bool	MapVertexBucketTraits::ObjectTouchesPoint(Object o, const Point2& p)
{
	return o->point() == p;
}

bool	MapVertexBucketTraits::ObjectTouchesRect(Object o, const Point2& p1, const Point2& p2)
{
	Bbox2	selection(
						p1.x,
						p1.y,
						p2.x,
						p2.y);
	Bbox2	box(o->point());

	// Warning: need to check edge cases here!
	return box.overlap(selection);
}

bool	MapVertexBucketTraits::ObjectFullyInRect(Object o, const Point2& p1, const Point2& p2)
{
	Bbox2	selection(
						p1.x,
						p1.y,
						p2.x,
						p2.y);
	Bbox2	box(o->point());

	return (
			box.xmin() >= selection.xmin() &&
			box.ymin() >= selection.ymin() &&
			box.xmax() <= selection.xmax() &&
			box.ymax() <= selection.ymax());
}

void	MapVertexBucketTraits::DestroyObject(Object o)
{
}


#pragma mark -

template class XBuckets<Pmwx::Face_handle, MapFaceBucketTraits>;
template class XBuckets<Pmwx::Halfedge_handle, MapHalfedgeBucketTraits>;
template class XBuckets<Pmwx::Vertex_handle, MapVertexBucketTraits>;


#if 0
	// Special case empty map.
	if (empty())
	{
		loc = locate_Face;
		return NULL;
	}
	
	Point2			best_guess;			// This is the best point we've got so far.
	GISHalfedge*	guess_owner = NULL;	// This is who owns it.
	bool			inited = false;		// This is whether we've got any...
	Segment2		seg;
	bool			inside = false;
	
	GISHalfedge * circ, * stop;
	// Go through every hole in the unbounded face.  Try to find the intersection with the horizontal
	// line through P that is closest but less than P.
	for (Holes_iterator hole = mUnbounded->holes_begin(); hole != mUnbounded->holes_end(); ++hole)
	{
		circ = stop = *hole;
		do {
			// See if this segment intercepts the horizontal line.
			seg = Segment2(circ->source()->point(), circ->target()->point());
			if (seg.p1.y < p.y && p.y <= seg.p2.y ||
	  		    seg.p1.y > p.y && p.y >= seg.p2.y)
			{
				// If so, calc the x-crossing.  Is it the first one we have
				// or better than last time?  And is it to the left still?
				double	x_cross = seg.x_at_y(p.y);
				if ((!inited || x_cross >= best_guess.x) && x_cross <= p.x)
				{
					best_guess.x = x_cross; best_guess.y = p.y;
					guess_owner = circ;
					inited = true;
					inside = seg.p1.y < seg.p2.y;					
				}
			}
			if (seg.p1.y == p.y && seg.p2.y == p.y)
			{
				if ((seg.p1.x <= p.x && seg.p2.x >= p.x) ||
					(seg.p1.x >= p.x && seg.p2.x <= p.x))
				{
					loc = (seg.p1 == p || seg.p2 == p) ? locate_Vertex : locate_Halfedge;
					if (seg.p1 == p)
						return circ->twin();
					else
						return circ;
				}
			}
			circ = circ->next();
		} while (circ != stop);
	}
	
	// Special case - if we didn't find anything fair game, the point is either above or below or to
	// the left of the map completely.  That's ok, return unbounded face.
	if (!inited)
	{
		loc = locate_Face;
		return (*(mUnbounded->mHoles.begin()));
	}

	if (!inside)
	{
		if (!guess_owner) {
			loc = locate_Face;
			return (*(mUnbounded->mHoles.begin()));
		}
		
		if (p == guess_owner->target()->point())
			loc = locate_Vertex; 
		else if (p == guess_owner->source()->point()) {
			guess_owner = guess_owner->twin();
			loc = locate_Vertex;
		} else if (p == best_guess) 
			loc = locate_Halfedge;
		else 
			loc = locate_Face;
		return guess_owner;
	}

	// hrm...the way this is written if we don't find a better seg,
	// we don't re-pick ourselves because our last guess is equala to
	// our new guess.  Flip our edge now because if we fall out of the
	// loop it would be nice to fall out on the right side.
	guess_owner = guess_owner->twin();
	
	Point2			last_guess = best_guess;
	GISHalfedge *	last_owner = guess_owner;
	bool			found = true;
					inside = false;

	while (last_guess != p)
	{
		found = false;

		// First try the CCB that our last guess was on.
		circ = last_owner;
		do {		
			seg = Segment2(circ->source()->point(), circ->target()->point());
			if ((seg.p1.y < p.y && p.y <= seg.p2.y) ||
				(seg.p1.y > p.y && p.y >= seg.p2.y))
			{
				// If so, calc the x-crossing.  Is it the first one we have
				// or better than last time?  And is it to the left still?
				double	x_cross = seg.x_at_y(p.y);
				if (x_cross > best_guess.x && x_cross <= p.x)
				{
					best_guess.x = x_cross; best_guess.y = p.y;
					guess_owner = circ;
					found = true;
					inside = seg.p1.y < seg.p2.y;
				}
			}	
			if (seg.p1.y == p.y && seg.p2.y == p.y)
			{
				if ((seg.p1.x <= p.x && seg.p2.x >= p.x) ||
					(seg.p1.x >= p.x && seg.p2.x <= p.x))
				{
					loc = (seg.p1 == p || seg.p2 == p) ? locate_Vertex : locate_Halfedge;
					if (seg.p1 == p)
						return circ->twin();
					else
						return circ;					
				}
			}
			circ = circ->next();
		} while (circ != last_owner);
		
		if (last_owner->is_on_outer_ccb())
		{
			for (Holes_iterator hole = last_owner->face()->holes_begin(); hole != last_owner->face()->holes_end(); ++hole)
			{
				circ = stop = *hole;
				do {
					// See if this segment intercepts the horizontal line.
					seg = Segment2(circ->source()->point(), circ->target()->point());
					if ((seg.p1.y < p.y && p.y <= seg.p2.y) ||
						(seg.p1.y > p.y && p.y >= seg.p2.y))
					{
						// If so, calc the x-crossing.  Is it the first one we have
						// or better than last time?  And is it to the left still?
						double	x_cross = seg.x_at_y(p.y);
						if ((!inited || x_cross >= best_guess.x) && x_cross <= p.x)
						{
							best_guess.x = x_cross; best_guess.y = p.y;
							guess_owner = circ;
							found = true;
							inside = seg.p1.y < seg.p2.y;
						}
					}	
					if (seg.p1.y == p.y && seg.p2.y == p.y)
					{
						if ((seg.p1.x < p.x && seg.p2.x >= p.x) ||
							(seg.p1.x > p.x && seg.p2.x <= p.x))
						{
							loc = (seg.p1 == p || seg.p2 == p) ? locate_Vertex : locate_Halfedge;
							if (seg.p1 == p)
								return circ->twin();
							else
								return circ;					
						}
					}
					circ = circ->next();
				} while (circ != stop);				
			}
		}
		
		if (!found)	break;
		if (!inside) break;
		last_guess = best_guess;
		last_owner = guess_owner->twin();		
	}

	// Okay we stopped.  If found is set the last iteration did find a better guess and stopped because the gues was
	// on the money.  If found is false, we stopped because this seems to be the best guess.
	if (!found)
		guess_owner = last_owner;

	if (p == guess_owner->target()->point())
		loc = locate_Vertex; 
	else if (p == guess_owner->source()->point()) {
		guess_owner = guess_owner->twin();
		loc = locate_Vertex;
	} else if (p == best_guess) 
		loc = locate_Halfedge;
	else 
		loc = locate_Face;
	return guess_owner;
#endif