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
#ifndef MAPDEFS_H
#define MAPDEFS_H

#include <set>
#include <vector>
#include <map>
#include <list>
using std::list;

#include "CompGeomDefs2.h"

/************************************************************************
 * BASIC STRUCTURE TYPES
 ************************************************************************/

/* GISParamMap
 *
 * A param map is a mapping of enumerated properties to their values.
 * Params are applied to all areas and are used to control land class
 * instantiation. */
typedef	map<int, double>	GISParamMap;

/* GISPointFeature_t 
 *
 * A point feature is a 0-dimensional thing in the GIS data.
 * A point feature contains a seriers of parameters indicating
 * vaguely what it is.  They are stored within the GT-polygon
 * that contains them.
 *
 */
struct	GISPointFeature_t {
	int				mFeatType;
	GISParamMap		mParams;
	Point2			mLocation;	
	bool			mInstantiated;
};
typedef vector<GISPointFeature_t>	GISPointFeatureVector;

/* GISPolygonFeature_t
 *
 * An area feature is a polygon that is fully contained within
 * a GT-polygon.  We deviate from more typical GIS standards
 * in representing area features non-topologically because 
 * we aren't terribly interested in doing topological operations.
 * By comparison we do want to know the containing GT land block
 * and not have it be split by a feature that's right on the street.
 *
 */
struct GISPolygonFeature_t {
	int				mFeatType;
	GISParamMap		mParams;
	Polygon2		mShape;
	bool			mInstantiated;
};
typedef vector<GISPolygonFeature_t>	GISPolygonFeatureVector;

/* GISAreaFeature_t 
 *
 * An area feature describes the entire GT-polygon, typically 
 * overriding the treatment and population of that GT-polygon.
 *
 */
struct GISAreaFeature_t {
	int				mFeatType;
	GISParamMap		mParams;
};
//typedef vector<GISAreaFeature_t> GISAreaFeatureVector;

/* GISObjPlacement_t
 *
 * A single object placed somewhere in or on a GIS entity.
 * A lcoation is provided even for points for simplicity.
 * Derived tells whether this object was added by automatic
 * generation or by the user. */
struct	GISObjPlacement_t {
	int				mRepType;
	Point2			mLocation;
	double			mHeading;
	bool			mDerived;
};
typedef vector<GISObjPlacement_t>	GISObjPlacementVector;

/* GISPolyObjPlacement_t
 * 
 * A single placement of a prototype by its polygon and height.
 * Derived info is saved just like a normal object. */
struct	GISPolyObjPlacement_t {
	int					mRepType;
	vector<Polygon2>	mShape;
	Point2				mLocation;	// Nominal center - used primarily for debugging!
	double				mHeight;
	bool				mDerived;
};
typedef vector<GISPolyObjPlacement_t>	GISPolyObjPlacementVector;	

/* GISNetworkSegment_t
 * 
 * A single road or other item along a network.  Each end has a height
 * stored in terms of levels from the network definition. */
struct	GISNetworkSegment_t {
	int				mFeatType;
	int				mRepType;
	double			mSourceHeight;
	double			mTargetHeight;
};
typedef vector<GISNetworkSegment_t>		GISNetworkSegmentVector;

/************************************************************************
 * MAP TYPES
 ************************************************************************/

class	GISFace;
class	GISVertex;
class	GISHalfedge;
class	Pmwx;



/************************************************************************
 * HALFEDGE
 ************************************************************************/

	
class	GISHalfedge {

	class	Halfedge_iterator : public iterator<bidirectional_iterator_tag, GISHalfedge> { 
		GISHalfedge *	ptr;
	public:
		Halfedge_iterator() : ptr(NULL) { }
		explicit Halfedge_iterator(GISHalfedge * he) : ptr(he) { }
		Halfedge_iterator(const Halfedge_iterator& rhs) : ptr(rhs.ptr) { }
		Halfedge_iterator& operator=(GISHalfedge * rhs) { ptr = rhs; return *this; }
		Halfedge_iterator& operator=(const Halfedge_iterator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Halfedge_iterator& rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const Halfedge_iterator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Halfedge_iterator& rhs) const { return ptr < rhs.ptr; }
		GISHalfedge& operator*() const { return *ptr; }
		GISHalfedge * operator->() const { return ptr; }
		Halfedge_iterator& operator++() { ptr = ptr->mLinkNext; return *this; }
		Halfedge_iterator operator++(int) { GISHalfedge * old = ptr; ptr = ptr->mLinkNext; return Halfedge_iterator(old); }
		Halfedge_iterator& operator--() { ptr = ptr->mLinkPrev; return *this; }
		Halfedge_iterator operator--(int) { GISHalfedge * old = ptr; ptr = ptr->mLinkPrev; return Halfedge_iterator(old); }
		operator GISHalfedge * () const { return ptr; }
	};

	class	Halfedge_const_iterator  : public iterator<bidirectional_iterator_tag, const GISHalfedge> {
		const GISHalfedge *	ptr;
	public:
		Halfedge_const_iterator() : ptr(NULL) { }
		explicit Halfedge_const_iterator(const GISHalfedge * he) : ptr(he) { }
		Halfedge_const_iterator(const Halfedge_const_iterator& rhs) : ptr(rhs.ptr) { }
		Halfedge_const_iterator& operator=(const GISHalfedge * he) { ptr = he; return *this; }
		Halfedge_const_iterator& operator=(const Halfedge_const_iterator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Halfedge_const_iterator& rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const Halfedge_const_iterator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Halfedge_const_iterator& rhs) const { return ptr < rhs.ptr; }
		const GISHalfedge& operator*() const { return *ptr; }
		const GISHalfedge * operator->() const { return ptr; }
		Halfedge_const_iterator& operator++() { ptr = ptr->mLinkNext; return *this; }
		Halfedge_const_iterator operator++(int) { const GISHalfedge * old = ptr; ptr = ptr->mLinkNext; return Halfedge_const_iterator(old); }
		Halfedge_const_iterator& operator--() { ptr = ptr->mLinkPrev; return *this; }
		Halfedge_const_iterator operator--(int) { const GISHalfedge * old = ptr; ptr = ptr->mLinkPrev; return Halfedge_const_iterator(old); }
		operator const GISHalfedge * () const { return ptr; }
	};

	class Ccb_halfedge_circulator : public iterator<bidirectional_iterator_tag, GISHalfedge> {
		GISHalfedge *	ptr;
	public:
		Ccb_halfedge_circulator() : ptr(NULL) { }
		explicit Ccb_halfedge_circulator(GISHalfedge * he) : ptr(he) { }
		Ccb_halfedge_circulator(const Ccb_halfedge_circulator& rhs) : ptr(rhs.ptr) { }
		Ccb_halfedge_circulator& operator=(GISHalfedge * he) { ptr = he; return *this; }
		Ccb_halfedge_circulator& operator=(const Ccb_halfedge_circulator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Ccb_halfedge_circulator& rhs) const { return ptr == rhs.ptr; }
		bool operator==(	  GISHalfedge * rhs) const { return ptr == rhs; }					// This is needed because GCC is frickin' stupid
		bool operator==(const GISHalfedge * rhs) const { return ptr == rhs; }
		bool operator!=(const Ccb_halfedge_circulator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Ccb_halfedge_circulator& rhs) const { return ptr < rhs.ptr; }
		GISHalfedge& operator*() const { return *ptr; }
		GISHalfedge * operator->() const { return ptr; }
		Ccb_halfedge_circulator& operator++() { ptr = ptr->mNext; return *this; }
		Ccb_halfedge_circulator operator++(int) { GISHalfedge * old = ptr; ptr = ptr->mNext; return Ccb_halfedge_circulator(old); }
		operator GISHalfedge * () const { return ptr; }
	};

	class Ccb_halfedge_const_circulator {
		const GISHalfedge *	ptr;
	public:
		typedef bidirectional_iterator_tag iterator_category;
		Ccb_halfedge_const_circulator() : ptr(NULL) { }
		explicit Ccb_halfedge_const_circulator(const GISHalfedge * he) : ptr(he) { }
		Ccb_halfedge_const_circulator(const Ccb_halfedge_const_circulator& rhs) : ptr(rhs.ptr) { }
		Ccb_halfedge_const_circulator& operator=(const GISHalfedge* rhs) { ptr = rhs; return *this; }
		Ccb_halfedge_const_circulator& operator=(const Ccb_halfedge_const_circulator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Ccb_halfedge_const_circulator& rhs) const { return ptr == rhs.ptr; }
		bool operator==(	  GISHalfedge * rhs) const { return ptr == rhs; }
		bool operator==(const GISHalfedge * rhs) const { return ptr == rhs; }
		bool operator!=(const Ccb_halfedge_const_circulator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Ccb_halfedge_const_circulator& rhs) const { return ptr < rhs.ptr; }
		const GISHalfedge& operator*() const { return *ptr; }
		const GISHalfedge * operator->() const { return ptr; }
		Ccb_halfedge_const_circulator& operator++() { ptr = ptr->mNext; return *this; }
		Ccb_halfedge_const_circulator operator++(int) { const GISHalfedge * old = ptr; ptr = ptr->mNext; return Ccb_halfedge_const_circulator(old); }
		operator const GISHalfedge * () const { return ptr; }
	};
	
	
	class Halfedge_around_vertex_circulator : public iterator<bidirectional_iterator_tag, GISHalfedge> {
		GISHalfedge *	ptr;
	public:
		Halfedge_around_vertex_circulator(GISHalfedge * he = NULL) : ptr(he) { }
		Halfedge_around_vertex_circulator(const Halfedge_around_vertex_circulator& rhs) : ptr(rhs.ptr) { }
		Halfedge_around_vertex_circulator& operator=(const Halfedge_around_vertex_circulator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Halfedge_around_vertex_circulator& rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const Halfedge_around_vertex_circulator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Halfedge_around_vertex_circulator& rhs) const { return ptr < rhs.ptr; }
		GISHalfedge& operator*() const { return *ptr; }
		GISHalfedge * operator->() const { return ptr; }
		Halfedge_around_vertex_circulator& operator++() { ptr = ptr->mNext->mTwin; return *this; }
		Halfedge_around_vertex_circulator operator++(int) { GISHalfedge * old = ptr; ptr = ptr->mNext->mTwin; return Halfedge_around_vertex_circulator(old); }
		operator GISHalfedge * () const { return ptr; }
	};

	class Halfedge_around_vertex_const_circulator {
		const GISHalfedge *	ptr;
	public:
		typedef bidirectional_iterator_tag iterator_category;
		Halfedge_around_vertex_const_circulator(const GISHalfedge * he = NULL) : ptr(he) { }
		Halfedge_around_vertex_const_circulator(const Halfedge_around_vertex_const_circulator& rhs) : ptr(rhs.ptr) { }
		Halfedge_around_vertex_const_circulator& operator=(const Halfedge_around_vertex_const_circulator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Halfedge_around_vertex_const_circulator& rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const Halfedge_around_vertex_const_circulator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Halfedge_around_vertex_const_circulator& rhs) const { return ptr < rhs.ptr; }
		const GISHalfedge& operator*() const { return *ptr; }
		const GISHalfedge * operator->() const { return ptr; }
		Halfedge_around_vertex_const_circulator& operator++() { ptr = ptr->mNext->mTwin; return *this; }
		Halfedge_around_vertex_const_circulator operator++(int) { const GISHalfedge * old = ptr; ptr = ptr->mNext->mTwin; return Halfedge_around_vertex_const_circulator(old); }
		operator const GISHalfedge * () const { return ptr; }
	};	
	
		
	GISHalfedge *	mLinkPrev;
	GISHalfedge *	mLinkNext;

	GISHalfedge *	mNext;
	GISHalfedge *	mTwin;
	GISFace *		mFace;
	GISVertex *		mTarget;

	GISHalfedge *	mNextIndex;
	friend class	MapHalfedgeBucketTraits;

public:

	GISHalfedge *		next_index()	{ return mNextIndex; }

	GISHalfedge *		twin() 			{ return mTwin; }
	const GISHalfedge *	twin() const	{ return mTwin; }
	GISFace *			face() 			{ return mFace; }
	const GISFace *		face() const	{ return mFace; }
	GISVertex *			target()	 	{ return mTarget; }
	const GISVertex *	target() const	{ return mTarget; }
	GISVertex *			source() 		{ return mTwin->mTarget; }
	const GISVertex *	source() const	{ return mTwin->mTarget; }
	GISHalfedge *		next() 			{ return mNext; }
	const GISHalfedge *	next() const	{ return mNext; }
	
	void set_next(GISHalfedge * next) 	{ mNext = next; }
	void set_face(GISFace * face) 		{ mFace = face; }
	void set_twin(GISHalfedge * twin) 	{ mTwin = twin; }
	void set_target(GISVertex * target) { mTarget = target; }

	GISHalfedge *		points_to_me(void);				// Returns the halfedge that has this as its next
	bool				is_on_outer_ccb(void) const;	// Returns true if this edge is part on an outer CCB
	GISHalfedge *		get_pre_twin(void);				// Returns the halfedge that has our twin as its next
	GISHalfedge *		get_hole_rep(void);				// If we are a hole, returns the halfedge that "represents" the hole, or NULL if we're part of a CCB.
	GISHalfedge *		get_leftmost(void);				// Return the leftmost in my ring.  If there is a vertical segment, this may return that seg, or a seg pointing to it.

					GISHalfedge();
					GISHalfedge(const GISHalfedge&);
	virtual			~GISHalfedge();

	void			SwapDominance(void);			// USE WITH CAUTION!!!!

	bool						mDominant;			// Is non-sided info stored on this
													// Halfedge or my twin?

	GISHalfedge *		dominant() 			{ return mDominant ? this : mTwin; }
	const GISHalfedge *	dominant() const	{ return mDominant ? this : mTwin; }


	int							mTransition;		// Transition type ID
	GISNetworkSegmentVector		mSegments;			// Network segments along us
	GISParamMap					mParams;

	double						mInset;				// Largest unusable inset for this side
	
	bool						mMark;				// Temporary, for algorithms

#if OPENGL_MAP
	float						mGL[4];				// Pre-expanded line!
	float						mGLColor[3];	
#endif
	
	
private:

	GISHalfedge&	operator=(const GISHalfedge&);

	friend			class	Pmwx;
	friend			class	GISFace;
	friend			class	GISVertex;
	
};

/************************************************************************
 * VERTEX
 ************************************************************************/


class	GISVertex {

	class	Vertex_iterator : public iterator<bidirectional_iterator_tag, GISVertex> {
		GISVertex *	ptr;
	public:
		Vertex_iterator(GISVertex * he = NULL) : ptr(he) { }
		Vertex_iterator(const Vertex_iterator& rhs) : ptr(rhs.ptr) { }
		Vertex_iterator& operator=(const Vertex_iterator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Vertex_iterator& rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const Vertex_iterator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Vertex_iterator& rhs) const { return ptr < rhs.ptr; }
		GISVertex& operator*() const { return *ptr; }
		GISVertex * operator->() const { return ptr; }
		Vertex_iterator& operator++() { ptr = ptr->mLinkNext; return *this; }
		Vertex_iterator operator++(int) { GISVertex * old = ptr; ptr = ptr->mLinkNext; return Vertex_iterator(old); }
		Vertex_iterator& operator--() { ptr = ptr->mLinkPrev; return *this; }
		Vertex_iterator operator--(int) { GISVertex * old = ptr; ptr = ptr->mLinkPrev; return Vertex_iterator(old); }
		operator GISVertex *() const { return ptr; }
	};

	class	Vertex_const_iterator : public iterator<bidirectional_iterator_tag, const GISVertex> {
		const GISVertex *	ptr;
	public:
		Vertex_const_iterator(const GISVertex * he = NULL) : ptr(he) { }
		Vertex_const_iterator(const Vertex_const_iterator& rhs) : ptr(rhs.ptr) { }
		Vertex_const_iterator& operator=(const Vertex_const_iterator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Vertex_const_iterator& rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const Vertex_const_iterator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Vertex_const_iterator& rhs) const { return ptr < rhs.ptr; }
		const GISVertex& operator*() const { return *ptr; }
		const GISVertex * operator->() const { return ptr; }
		Vertex_const_iterator& operator++() { ptr = ptr->mLinkNext; return *this; }
		Vertex_const_iterator operator++(int) { const GISVertex * old = ptr; ptr = ptr->mLinkNext; return Vertex_const_iterator(old); }
		Vertex_const_iterator& operator--() { ptr = ptr->mLinkPrev; return *this; }
		Vertex_const_iterator operator--(int) { const GISVertex * old = ptr; ptr = ptr->mLinkPrev; return Vertex_const_iterator(old); }
		operator const GISVertex *() const { return ptr; }
	};


	GISVertex *		mLinkPrev;
	GISVertex *		mLinkNext;

	GISHalfedge *	mHalfedge;
	Point2			mPoint;
	
	typedef	GISHalfedge::Halfedge_around_vertex_circulator			Halfedge_around_vertex_circulator;
	typedef	GISHalfedge::Halfedge_around_vertex_const_circulator	Halfedge_around_vertex_const_circulator;

	GISVertex *		mNextIndex;
	friend class	MapVertexBucketTraits;

public:

	GISVertex *			next_index()	{ return mNextIndex; }

	Point2&				point() 				{ return mPoint; }
	const Point2&		point() const 			{ return mPoint; }
	GISHalfedge *		halfedge()				{ return mHalfedge; }
	const GISHalfedge *	halfedge() const		{ return mHalfedge; }

	void			set_halfedge(GISHalfedge * e)	{ mHalfedge = e; }
	
	unsigned int 	degree() const;
	GISHalfedge *	rightmost_rising();
	
	Halfedge_around_vertex_circulator		incident_halfedges() 		{ return Halfedge_around_vertex_circulator(mHalfedge); }
	Halfedge_around_vertex_const_circulator	incident_halfedges() const 	{ return Halfedge_around_vertex_const_circulator(mHalfedge); }
	
					GISVertex();
					GISVertex(const GISVertex&);
	virtual			~GISVertex();

	char			mTunnelPortal;

#if OPENGL_MAP
	float						mGL[2];				// Pre-expanded line!
#endif

private:
	
	GISVertex		operator=(const GISVertex&);

	friend			class	Pmwx;

};


/************************************************************************
 * FACE
 ************************************************************************/


class	GISFace {

	class	Face_iterator : public iterator<bidirectional_iterator_tag, GISFace> {
		GISFace *	ptr;
	public:
		Face_iterator(GISFace * he = NULL) : ptr(he) { }
		Face_iterator(const Face_iterator& rhs) : ptr(rhs.ptr) { }
		Face_iterator& operator=(const Face_iterator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Face_iterator& rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const Face_iterator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Face_iterator& rhs) const { return ptr < rhs.ptr; }
		GISFace& operator*() const { return *ptr; }
		GISFace * operator->() const { return ptr; }
		Face_iterator& operator++() { ptr = ptr->mLinkNext; return *this; }
		Face_iterator operator++(int) { GISFace * old = ptr; ptr = ptr->mLinkNext; return Face_iterator(old); }
		Face_iterator& operator--() { ptr = ptr->mLinkPrev; return *this; }
		Face_iterator operator--(int) { GISFace * old = ptr; ptr = ptr->mLinkPrev; return Face_iterator(old); }
		operator GISFace * () const { return ptr; }
	};

	class	Face_const_iterator  : public iterator<bidirectional_iterator_tag, const GISFace> {
		const GISFace *	ptr;
	public:
		Face_const_iterator(const GISFace * he = NULL) : ptr(he) { }
		Face_const_iterator(const Face_const_iterator& rhs) : ptr(rhs.ptr) { }
		Face_const_iterator& operator=(const Face_const_iterator& rhs) { ptr = rhs.ptr; return *this; }
		bool operator==(const Face_const_iterator& rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const Face_const_iterator& rhs) const { return ptr != rhs.ptr; }
		bool operator<(const Face_const_iterator& rhs) const { return ptr < rhs.ptr; }
		const GISFace& operator*() const { return *ptr; }
		const GISFace * operator->() const { return ptr; }
		Face_const_iterator& operator++() { ptr = ptr->mLinkNext; return *this; }
		Face_const_iterator operator++(int) { const GISFace * old = ptr; ptr = ptr->mLinkNext; return Face_const_iterator(old); }
		Face_const_iterator& operator--() { ptr = ptr->mLinkPrev; return *this; }
		Face_const_iterator operator--(int) { const GISFace * old = ptr; ptr = ptr->mLinkPrev; return Face_const_iterator(old); }
		operator const GISFace * () const { return ptr; }
	};

	GISFace *					mLinkPrev;
	GISFace *					mLinkNext;

	GISHalfedge *				mOuterCCB;	
	set<GISHalfedge *>			mHoles;

	typedef	set<GISHalfedge *>::iterator				Holes_iterator;
	typedef	set<GISHalfedge *>::const_iterator			Holes_const_iterator;
	typedef GISHalfedge::Ccb_halfedge_circulator		Ccb_halfedge_circulator;
	typedef GISHalfedge::Ccb_halfedge_const_circulator	Ccb_halfedge_const_circulator;

	GISFace *		mNextIndex;
	friend class	MapFaceBucketTraits;
	
public:

	GISFace *					next_index()			{ return mNextIndex; }
	
	bool						is_unbounded() const 	{ return mOuterCCB == NULL; }
	Holes_iterator				holes_begin() 			{ return mHoles.begin(); }
	Holes_const_iterator		holes_begin() const 	{ return mHoles.begin(); }
	Holes_iterator				holes_end() 			{ return mHoles.end(); }
	Holes_const_iterator		holes_end() const 		{ return mHoles.end(); }
	int							holes_count() const 	{ return mHoles.size(); }
	void						copy_holes(set<GISHalfedge *>& holes) const { holes = mHoles; }

	Ccb_halfedge_circulator			outer_ccb() 			{ return Ccb_halfedge_circulator(mOuterCCB); }
	Ccb_halfedge_const_circulator	outer_ccb() const		{ return Ccb_halfedge_const_circulator(mOuterCCB); }

	void	set_outer_ccb(GISHalfedge * outer);
	void	add_hole(GISHalfedge * inner);
	void	delete_hole(GISHalfedge * inner);
	bool	is_hole_ccb(GISHalfedge * inner)				{ return mHoles.count(inner); }
	
					GISFace();
					GISFace(const GISFace&);
	virtual			~GISFace();	

//	int							mIsWater;
	int							mTerrainType;
	GISParamMap					mParams;
	GISPointFeatureVector		mPointFeatures;
	GISPolygonFeatureVector		mPolygonFeatures;
	GISAreaFeature_t			mAreaFeature;
	
	// Stuff that's been hand placed in the area by object propagation
	GISObjPlacementVector		mObjs; 
	GISPolyObjPlacementVector	mPolyObjs;

	// A temporary cache - buckets use this to speed
	// up queries.
	Bbox2						mBoundsCache;
	int							mTemp1;							// Per face temp value
	int							mTemp2;							// Per face temp value

#if OPENGL_MAP
	vector<float>				mGLTris;						// Pre-expanded triangles
	float						mGLColor[4];
#endif
	
	bool			IsWater(void) const; 						// Is this polygon wet?
	bool			TerrainMatch(const GISFace& rhs) const;		// 
	bool			AreaMatch(const GISFace& rhs) const;

private:

	GISFace&		operator=(const GISFace&);

	friend			class	Pmwx;

};


/************************************************************************
 * SPATIAL INDEXING HELPERS
 ************************************************************************/

// Planar map uses quad trees to provide spatial indexing.  Basically
// the index is okay FROM when we call "index until when we edit.
// Fast accessors return queries/

// Please note that vertices are also stored in a 1-d map by Y coordinate.
// This is NOT useful for spatial range queries - instead it is used for
// fast exact-vert location - that is, recovering a vertex by its point coordinates.
// This is very useful in certain construction ops.

#include "QuadTree.h"

class	MapBucketTraits {
public:

	typedef	Bbox2		KeyType;
	typedef	Bbox2		CullType;

	void		expand_by(CullType& io_cull, const CullType& part);		// expand io_cull by part
	void		set_empty(CullType& c);									// make zero-area cull

	void		subkey(const KeyType& e, KeyType& k, int n);			// change K to be the Nth quadrant subkey of E
	bool		contains(const KeyType& outer, const KeyType& inner);	// return true if outer fully contains inner	
	void		make_key(const CullType& cull, KeyType&	key);
	
	void *		alloc(size_t bytes);		

	~MapBucketTraits();

	list<void *>	alloc_list;

};	

class	MapFaceBucketTraits : public MapBucketTraits {
public:

	typedef	GISFace		ValueType;
	void		get_cull(ValueType * v, CullType& c);					// get cull radius of V
	ValueType *	get_next(ValueType * v) { return v->mNextIndex; }
	void		set_next(ValueType * v, ValueType * n) { v->mNextIndex = n; }
};	

class	MapHalfedgeBucketTraits : public MapBucketTraits {
public:

	typedef	GISHalfedge		ValueType;
	void		get_cull(ValueType * v, CullType& c);					// get cull radius of V
	ValueType *	get_next(ValueType * v) { return v->mNextIndex; }
	void		set_next(ValueType * v, ValueType * n) { v->mNextIndex = n; }

};

class	MapVertexBucketTraits : public MapBucketTraits {
public:

	typedef	GISVertex		ValueType;
	void		get_cull(ValueType * v, CullType& c);					// get cull radius of V
	ValueType *	get_next(ValueType * v) { return v->mNextIndex; }
	void		set_next(ValueType * v, ValueType * n) { v->mNextIndex = n; }

};

/************************************************************************
 * MAP
 ************************************************************************/


class	Pmwx {
public:
					Pmwx();
					Pmwx(const Pmwx&);
					Pmwx(const GISFace&);
					~Pmwx();
	Pmwx& operator=(const Pmwx&);
	Pmwx& operator=(const GISFace&);
	void			swap(Pmwx&);

	typedef GISVertex *												Vertex_handle;
	typedef const GISVertex *										Vertex_const_handle;
	typedef GISHalfedge *											Halfedge_handle;
	typedef const GISHalfedge *										Halfedge_const_handle;
	typedef GISFace *												Face_handle;
	typedef const GISFace *											Face_const_handle;
	typedef	GISVertex::Vertex_iterator								Vertex_iterator;
	typedef	GISVertex::Vertex_const_iterator						Vertex_const_iterator;
	typedef	GISHalfedge::Halfedge_iterator							Halfedge_iterator;
	typedef	GISHalfedge::Halfedge_const_iterator					Halfedge_const_iterator;
	typedef	GISVertex::Halfedge_around_vertex_circulator			Halfedge_around_vertex_circulator;
	typedef	GISVertex::Halfedge_around_vertex_const_circulator	Halfedge_around_vertex_const_circulator;
	typedef GISFace::Face_iterator									Face_iterator;
	typedef GISFace::Face_const_iterator							Face_const_iterator;
	typedef GISFace::Holes_iterator									Holes_iterator;
	typedef GISFace::Holes_const_iterator							Holes_const_iterator;
	typedef GISFace::Ccb_halfedge_circulator						Ccb_halfedge_circulator;
	typedef GISFace::Ccb_halfedge_const_circulator					Ccb_halfedge_const_circulator;	
	
	GISFace *		unbounded_face() { return mUnbounded; }
	const GISFace *	unbounded_face() const { return mUnbounded; }

	int				number_of_vertices() 	const { return mVertices;	}	
	int				number_of_halfedges() 	const { return mHalfedges;	}
	int				number_of_faces() 		const { return mFaces;		}
		
	Vertex_iterator			vertices_begin()			{ return Vertex_iterator(mFirstVertex); }
	Vertex_const_iterator	vertices_begin()	const	{ return Vertex_const_iterator(mFirstVertex); }
	Vertex_iterator			vertices_end()				{ return Vertex_iterator(NULL); }
	Vertex_const_iterator	vertices_end()		const	{ return Vertex_const_iterator(NULL); }
	Halfedge_iterator		halfedges_begin()			{ return Halfedge_iterator(mFirstHalfedge); }
	Halfedge_const_iterator	halfedges_begin()	const	{ return Halfedge_const_iterator(mFirstHalfedge); }
	Halfedge_iterator		halfedges_end()				{ return Halfedge_iterator(NULL); }
	Halfedge_const_iterator	halfedges_end()		const	{ return Halfedge_const_iterator(NULL); }
	Face_iterator			faces_begin()				{ return Face_iterator(mFirstFace); }
	Face_const_iterator		faces_begin()		const	{ return Face_const_iterator(mFirstFace); }
	Face_iterator			faces_end()					{ return Face_iterator(NULL); }
	Face_const_iterator		faces_end()			const	{ return Face_const_iterator(NULL); }
		
	void			clear();
	bool			empty() const { return mVertices == 0 && mHalfedges == 0 && mFaces == 1; }
	bool			is_valid() const;

	/*****************************************************************************
	 * LOCATION
	 *****************************************************************************/

	enum Locate_type {
		locate_Vertex,
		locate_Halfedge,
		locate_Face
	};
	
	/* Given a point, find it.  Returns a locate type.  For a face, halfedge
	 * is face's outer CCB halfedge.  For vertex, halfedge's target is vertex.  For
	 * unbounded face, NULL is returned if it's empty. */
	GISHalfedge *	locate_point(const Point2& p, Locate_type& loc);

	/* Start from one point, go to the next.  We pass in our start as a fully
	 * located point, e.g. the actual point, its topological location, and the
	 * most useful halfedge we have.  Given a destination, we return the first
	 * thing we intersect.  This will be either a vertex if we hit one, an edge
	 * if we cross one, or a face if we terminate in a face.  (In this case, 
	 * the face should match our own!)  We return the crossing point, a locate type,
	 * and a halfedge similar to above. */
	GISHalfedge *	ray_shoot(
						const Point2&		start,
						Locate_type			start_type,
						GISHalfedge *		start_hint,
						const Point2&		dest,
						Point2&				crossing,
						Locate_type&		loc);
						
	/* Vertex location - this is more specific than point location in that it utilizes
	 * the hash table and then bails. */
	GISVertex *		locate_vertex(const Point2& inLocation);	

	/*****************************************************************************
	 * BASIC TOPOLOGICAL EDITING
	 *****************************************************************************/
	
	/* Relocate a vertex - please note that this does not check for 
	 * induced edge collisions. */	 
	void			set_vertex_location(GISVertex * inVertex, const Point2& inPoint);
	
	/* Given an edge, split it into two, creating a new vertex.
	 * Returns inEdge, whose target is now the split pt. */
	GISHalfedge	*	split_edge(GISHalfedge * inEdge, const Point2& split);	

	/* Merge two edges - first->next must be second.  Next is destroyed,
	 * first is returned.  first's target's valence must be 2.  Returns first. */
	GISHalfedge *	merge_edges(GISHalfedge * first, GISHalfedge * second);	

	/* Remove edge.  If a face is removed, the unbounded face must be kept, or 
	 * a face is kept over its hole.  In a tie, the edge's face is used.
	 * This routine returns a ptr to the DELETED face!!  This means the pointer
	 * is BAD and should only be used to figure out if a face was deleted. */
	GISFace *			remove_edge(GISHalfedge * inEdge);
		
	/* Insert one or more edges as needed to form an edge from one point
	 * to another.  If passed, a notifier is called on each new halfedge - in
	 * the direction from p1 to p2 when on the new edge.  The first param is
	 * the old halfedge, the second the new.  One will be NULL for edges along
	 * P1 to P2.  If they are both not NULL, it is a different edge being split.
	 * Returns the last halfedge inserted, pointing to P2. */
	GISHalfedge *	insert_edge(const Point2& p1, const Point2& p2, 
						void(* notifier)(GISHalfedge *, GISHalfedge *, void *), void * ref);
	GISHalfedge *	insert_edge(const Point2& p1, const Point2& p2, GISHalfedge * hint, Locate_type location,
						void(* notifier)(GISHalfedge *, GISHalfedge *, void *), void * ref);


	/*****************************************************************************
	 * SPECIALIZED TOPOLOGICAL EDITING - USEFUL WHEN YOU KNOW THINGS ABOUT THE MAP
	 *****************************************************************************/
	 
	/* Given a ring of points that you know to not be in topological conflict
	 * with any other lines, and whichd do not have antennas, this does an insert 
	 * in a given face. */
	GISFace *		insert_ring(GISFace * parent, const vector<Point2>& inPoints);

	/* These three routines insert an edge in specific situations.  The requirement
	 * is that the edge not  cross any other edges; these routines are faster because
	 * they don't check for splits.  For inserting between vertices, the flga on_outer_ccb
	 * tells us that we know for a fact that both points are on the face's outer CCB.  This 
	 * provides a performance boost for the split-face-from-outer-ccb case because we can avoid
	 * checking for holes.  Setting this on false causes the PMWX to check this for you. */
	GISHalfedge *	nox_insert_edge_in_hole(const Point2& p1, const Point2& p2);	
	GISHalfedge *	nox_insert_edge_from_vertex(GISVertex * p1, const Point2& p2);	
	GISHalfedge *	nox_insert_edge_between_vertices(GISVertex * p1, GISVertex * p2);

	/*****************************************************************************
	 * MISC STUFF
	 *****************************************************************************/
	/* Returns the smallest distance between any two points.  WARNING: this is 
	 * currently O(N^2) time! */
	double		smallest_dist(Point2& p1, Point2& p2);
	

	/*****************************************************************************
	 * LOW LEVEL ACCESS - ONLY USE FOR SPECIALIZED CONSTRUCTION/FABRICATION
	 *****************************************************************************/
public:
	GISVertex *		new_vertex(const Point2& inPoint);
	GISHalfedge *	new_halfedge();	
	GISHalfedge *	new_halfedge(const GISHalfedge *);	
	GISHalfedge *	new_edge();	
	GISHalfedge *	new_edge(const GISHalfedge *);	
	GISFace *		new_face();
	GISFace *		new_face(const GISFace *);
	
	void	MoveFaceToMe		(Pmwx * old, GISFace * inFace);
	void	MoveVertexToMe		(Pmwx * old, GISVertex * inVertex);
	void	MoveHalfedgeToMe	(Pmwx * old, GISHalfedge * inHalfedge);
	void	MoveEdgeToMe		(Pmwx * old, GISHalfedge * inHalfedge);
	
	void	UnindexVertex(GISVertex * v);
	void	ReindexVertex(GISVertex * v);

	/*****************************************************************************
	 * SPATIAL INDEXING
	 *****************************************************************************/
	
	void		FindFaceTouchesPt(const Point2&, vector<GISFace *>& outIDs);									// Fully checks for pt containment
	void		FindFaceTouchesRectFast(const Point2&, const Point2&, vector<GISFace *>& outIDs);				// Intersects with face bbox, not face
	void		FindFaceFullyInRect(const Point2&, const Point2&, vector<GISFace *>& outIDs);					// Full containment

	void		FindHalfedgeTouchesRectFast(const Point2&, const Point2&, vector<GISHalfedge *>& outIDs);		// Intersects with half-edge bbox, not half-edge
	void		FindHalfedgeFullyInRect(const Point2&, const Point2&, vector<GISHalfedge *>& outIDs);			// Full containment

	void		FindVerticesTouchesPt(const Point2&, vector<GISVertex *>& outIDs);								// Perfect equalty.
	void		FindVerticesTouchesRect(const Point2&, const Point2&, vector<GISVertex *>& outIDs);				// Full containment (any containment is full for pts)

	void		Index(void);
		
private:

	typedef	QuadTree<MapFaceBucketTraits,9>			MapFaceBuckets;
	typedef	QuadTree<MapHalfedgeBucketTraits,9>		MapHalfedgeBuckets;
	typedef	QuadTree<MapVertexBucketTraits,4>		MapVertexBuckets;

		MapFaceBuckets			mFaceBuckets;
		MapHalfedgeBuckets		mHalfedgeBuckets;
		MapVertexBuckets		mVertexBuckets;

	// Special inserters.  NOTE: these are topological ONLY - all inserts are specified
	// by half-edges.  The caller must ensure that when multiple halfedfges meet the
	// topological requirements, that the one picked is the one meeting the geometric
	// requirements.
	GISHalfedge *	vertices_connected(GISVertex * v1, GISVertex * v2);
	GISHalfedge *	get_preceding(GISHalfedge * points_to_vertex, const Point2& p);
	GISHalfedge *	insert_edge_in_hole(GISFace * face, const Point2& p1, const Point2& p2);
	GISHalfedge *	insert_edge_from_vertex(GISHalfedge * inAdjacent, const Point2& p);
	GISHalfedge *	insert_edge_between_vertices(GISHalfedge * e1, GISHalfedge * e2);


	void			delete_vertex(GISVertex * halfedge);
	void			delete_halfedge(GISHalfedge * halfedge);
	void			delete_edge(GISHalfedge * halfedge);
	void			delete_face(GISFace * face);
	
		GISVertex *		mFirstVertex;
		GISVertex *		mLastVertex;
		GISHalfedge *	mFirstHalfedge;
		GISHalfedge *	mLastHalfedge;
		GISFace *		mFirstFace;
		GISFace *		mLastFace;

		int		mVertices;
		int		mHalfedges;
		int		mFaces;

		GISFace *		mUnbounded;
		
	typedef	map<Point2, GISVertex *, lesser_y_then_x>	VertexMap;
		VertexMap		mVertexIndex;
};

#endif
