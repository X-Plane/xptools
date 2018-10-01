/*
 *  MapDefs.h
 *  SceneryTools
 *
 *  Created by Andrew McGregor on 3/04/08.
 *  Copyright 2008 Andrew McGregor.
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

/*
	Polygon tricks and notes:

	Polygon_2 and Polygon_with_holes_2 are just "dumb" containers - you can push any set of points into them
	and not worry about the consequences.

	Polygon_set_2 is topological - when you add points and holes, you can use join and difference to hint
	how you want holes overlapping each other to be handled.  Note that polygons must already be simple.

	Conversions:

	- To convert a map (or part of a map) to a polygon, simply set "contained" on each face. (If you set the
	  unbounded face to be contained, you get an unbounded polygon with holes.)  Then construct a new
	  Polygon_set_2 with the map as the constructor.  The polygon_set_2 will be simpler than the map if
	  there are adjacent contained or not contained faces.

	- To convert a polygon set to a map, simply set the meta data on its internal arrangement, then use that
	  arrangement.  The most typical way to do this is to set the meta data, then use "overlay" or "merge"
	  to put the new polygons somewhere.  WARNING: Most polygon_set_2 processing operations consolidate and
	  simplify the underlying map.  So it is highly recommended that you set the meta data immediately before
	  using the underlying map!


*/

#include "CGALDefs.h"

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_dcel_base.h>
#include <CGAL/Arr_extended_dcel.h> // we need data extensions for everything
#include <CGAL/Arr_consolidated_curve_data_traits_2.h>
#include <CGAL/Arr_landmarks_point_location.h>
//#include <CGAL/Arr_overlay.h>
#include <CGAL/Arr_default_overlay_traits.h>
#include <CGAL/Boolean_set_operations_2/Gps_default_dcel.h>
//#include <CGAL/Bbox_2.h>
//#include <CGAL/assertions.h>
//#include <CGAL/intersections.h>


#include "ParamDefs.h"
#include "AssertUtils.h"
#include "CompGeomDefs2.h"
#include "STLUtils.h"

/******************************************************************************************************************************************************
 * GIS DATA
 ******************************************************************************************************************************************************/

/*
	This is the GIS data actually stored on parts of the map.
*/

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
struct	GISPointFeature_t;
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
struct GISPolygonFeature_t;
typedef vector<GISPolygonFeature_t>	GISPolygonFeatureVector;

/* GISAreaFeature_t
 *
 * An area feature describes the entire GT-polygon, typically
 * overriding the treatment and population of that GT-polygon.
 *
 */
struct GISAreaFeature_t;
//typedef vector<GISAreaFeature_t> GISAreaFeatureVector;

/* GISObjPlacement_t
 *
 * A single object placed somewhere in or on a GIS entity.
 * A lcoation is provided even for points for simplicity.
 * Derived tells whether this object was added by automatic
 * generation or by the user. */
struct	GISObjPlacement_t;
typedef vector<GISObjPlacement_t>	GISObjPlacementVector;

/* GISPolyObjPlacement_t
 *
 * A single placement of a prototype by its polygon and height.
 * Derived info is saved just like a normal object. */
struct	GISPolyObjPlacement_t;
typedef vector<GISPolyObjPlacement_t>	GISPolyObjPlacementVector;

/* GISNetworkSegment_t
 *
 * A single road or other item along a network.  Each end has a height
 * stored in terms of levels from the network definition. */
struct	GISNetworkSegment_t;
typedef vector<GISNetworkSegment_t>		GISNetworkSegmentVector;

/* GISPointFeature_t
 *
 * A point feature is a 0-dimensional thing in the GIS data.
 * A point feature contains a series of parameters indicating
 * vaguely what it is.  They are stored within the GT-polygon
 * that contains them.
 *
 */
struct	GISPointFeature_t {
public:
	int				mFeatType;
	GISParamMap		mParams;
	Point_2			mLocation;
	bool			mInstantiated;
};

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
public:
	int				mFeatType;
	GISParamMap		mParams;
	Polygon_with_holes_2		mShape;
	bool			mInstantiated;
};

/* GISAreaFeature_t
 *
 * An area feature describes the entire GT-polygon, typically
 * overriding the treatment and population of that GT-polygon.
 *
 */
struct GISAreaFeature_t {
public:
	int				mFeatType;
	GISParamMap		mParams;
};

/* GISObjPlacement_t
 *
 * A single object placed somewhere in or on a GIS entity.
 * A lcoation is provided even for points for simplicity.
 * Derived tells whether this object was added by automatic
 * generation or by the user. */
struct	GISObjPlacement_t {
public:
	int				mRepType; // global enum; you may need to register your OBJ via NewToken() first to get this
	Point2			mLocation;
	double			mHeading;
	bool			mDerived;
};

/* GISPolyObjPlacement_t
 *
 * A single placement of a prototype by its polygon and height.
 * Derived info is saved just like a normal object. */
struct	GISPolyObjPlacement_t {
public:
	int					mRepType;
	vector<Polygon2>	mShape;
//	Point_2				mLocation;	// Nominal center - used primarily for debugging!
	unsigned short		mParam;
	bool				mDerived;
	inline void trim(void) { ::trim(mShape); }
};

/* GISNetworkSegment_t
 *
 * A single road or other item along a network.  Each end has a height
 * stored in terms of levels from the network definition. */
struct	GISNetworkSegment_t {
public:
	int				mFeatType;
	int				mRepType;
	double			mSourceHeight;
	double			mTargetHeight;
	bool operator==(const GISNetworkSegment_t& rhs) const {
		return  mFeatType == rhs.mFeatType &&
				mRepType == rhs.mRepType &&
				mSourceHeight == rhs.mSourceHeight &&
				mTargetHeight == rhs.mTargetHeight; }
};







struct GIS_vertex_data { 
	GIS_vertex_data() { mTunnelPortal = false; mNeighborBurned = false; mNeighborNotBurned = false; }
	bool mTunnelPortal;
	bool mNeighborBurned;
	bool mNeighborNotBurned;
	inline void trim(void) { }
#if OPENGL_MAP
	float						mGL[2];				// Pre-expanded line!
#endif
};

struct GIS_halfedge_data {
public:
	GIS_halfedge_data() : mMark(false),mInset(0.0f), mTransition(0.0) { }

	int							mTransition;		// Transition type ID
	GISNetworkSegmentVector		mSegments;			// Network segments along us
	GISParamMap					mParams;
	double						mInset;				// Largest unusable inset for this side
	bool						mMark;				// Temporary, for algorithms
	
	inline	bool	HasRoads(void) const { return !mSegments.empty(); }
	inline	bool	HasGroundRoads(void) const { 
						for(GISNetworkSegmentVector::const_iterator r = mSegments.begin(); r != mSegments.end(); ++r) 
						if(r->mSourceHeight == 0.0 && r->mTargetHeight == 0.0) 
							return true; 
						return false; }	
	inline	bool	HasBridgeRoads(void) const { 
						for(GISNetworkSegmentVector::const_iterator r = mSegments.begin(); r != mSegments.end(); ++r) 
						if(r->mSourceHeight > 0.0 || r->mTargetHeight > 0.0) 
							return true; 
						return false; }	
	inline bool		HasRoadOfType(int t) const {
						for(GISNetworkSegmentVector::const_iterator r = mSegments.begin(); r != mSegments.end(); ++r)
						if(r->mFeatType == t)
							return true;
						return false; }
						
	inline void trim(void) { ::trim(mSegments); }
	
	inline bool operator==(const GIS_halfedge_data& rhs) const { return mTransition == rhs.mTransition && mSegments == rhs.mSegments && mParams == rhs.mParams && mInset == rhs.mInset; }
						
#if OPENGL_MAP
	unsigned char				mGLColor[3];
#endif
};



class GIS_face_data {
public:
	//	int							mIsWater;
	int							mTerrainType;		// This is a feature type for matching.  EXCEPTION: terrain_Water is both a feature and terrain.
	int							mOverlayType;		// IF a terrain type is going to be rule-matched as an overlay, it is stashed here, e.g. outlay/town-overlays for mobile DSFs.
	GISParamMap					mParams;
	GISPointFeatureVector		mPointFeatures;
	GISPolygonFeatureVector		mPolygonFeatures;
	//GISAreaFeatureVector		mAreaFeature;
	GISAreaFeature_t			mAreaFeature;
	// Stuff that's been hand placed in the area by object propagation
	GISObjPlacementVector		mObjs;
	GISPolyObjPlacementVector	mPolyObjs;
	int							mRotationDeg;					// Rotate the texture this many degrees
	int							mTemp1;							// Per face temp value
	int							mTemp2;							// Per face temp value

	bool		IsWater(void) const  { return (mTerrainType == terrain_Water); }
	bool		HasParam(int p) const { return mParams.count(p) > 0; }
	float		GetParam(int p, float d) const { GISParamMap::const_iterator i = mParams.find(p); return (i == mParams.end()) ? d : i->second; }
	int			GetZoning(void) const { GISParamMap::const_iterator i = mParams.find(af_Zoning); return (i == mParams.end()) ? NO_VALUE : i->second; }
	void		SetZoning(int z) { mParams[af_Zoning] = z; }

	bool		TerrainMatch(const GIS_face_data& rhs) const { return mTerrainType == rhs.mTerrainType; }
	bool		AreaMatch(const GIS_face_data& rhs) const { return (mTerrainType == rhs.mTerrainType && mAreaFeature.mFeatType == rhs.mAreaFeature.mFeatType); }

	inline void trim(void) { ::trim(mPointFeatures); ::trim(mPolygonFeatures); ::trim(mObjs); ::trim(mPolyObjs); 
		for(int i = 0; i < mPolyObjs.size(); ++i) mPolyObjs[i].trim(); }

	#if DEV
		~GIS_face_data() { mTerrainType = 0xDEADBEEF; }
	#endif
	GIS_face_data() : mTerrainType(0), mOverlayType(0), mRotationDeg(0) { mAreaFeature.mFeatType = 0; }
	GIS_face_data(const GIS_face_data &x) {
		mTerrainType = x.mTerrainType;
		mRotationDeg = x.mRotationDeg;
		mOverlayType = x.mOverlayType;
		mParams = x.mParams;
		mPointFeatures = x.mPointFeatures;
		mPolygonFeatures = x.mPolygonFeatures;
		mAreaFeature = x.mAreaFeature;
		mObjs = x.mObjs;
		mPolyObjs = x.mPolyObjs;
		mTemp1 = x.mTemp1;
		mTemp2 = x.mTemp2;
	}
#if OPENGL_MAP
	vector<const float *>		mGLTris;						// Pre-expanded triangle indices
	unsigned char				mGLColor[4];
#endif
};

/******************************************************************************************************************************************************
 * TRAITS NEEDED FOR THE MAP, ETC.
 ******************************************************************************************************************************************************/

/*
	We wrap segment traits in both Gps_segment_traits_2 and Arr_consolidated_curve_data_traits_2 to allow us to keep an integer "key" per curve, and
	also to use this in polygon sets.
*/

typedef	std::vector<FastKernel::Point_2>											Container_;
typedef CGAL::Arr_segment_traits_2<FastKernel>										TraitsBase;
typedef CGAL::Arr_consolidated_curve_data_traits_2<TraitsBase, int>					Arr_seg_traits_;
typedef CGAL::Gps_segment_traits_2<FastKernel, Container_, Arr_seg_traits_>			Traits_2;

// These data types define the "key" data per curve.
typedef Arr_seg_traits_::Data					EdgeKey;
typedef	Arr_seg_traits_::Data_container			EdgeKey_container;
typedef	Arr_seg_traits_::Data_iterator			EdgeKey_iterator;

// Some geometry types that are defined once our traits are established...
typedef Traits_2::X_monotone_curve_2            X_monotone_curve_2;
typedef Traits_2::Curve_2						Curve_2;

/******************************************************************************************************************************************************
 * ARRANGEMENT ("the map")
 ******************************************************************************************************************************************************/

/*
	Our arrangement has a bunch of special tricks:

	1.	Because we are constructing based on the GPS traits, each face has "contained" to specify if the "face" is IN a set of polygons..if it is,
		then we can convert to a polygon set.  Similarly, we can get a map from a set of polygons.

	2.	We have GIS data on each of the face, vertex, and half-edge data.

	3.	We have a consolidated "key" value for curves.  What this lets us do is identify the source vector that created each half-edge...so we can insert
		a ton of half-edges, then go look at these keys and see what source data they go with.

		Note that the keys are associated with EDGES, not HALF-EDGES...that is, two half-edges share a curve (which has its key) by ptr.

*/

typedef CGAL::Arr_extended_dcel<Traits_2,
								GIS_vertex_data,
								GIS_halfedge_data,
								GIS_face_data,
								CGAL::Arr_vertex_base<Point_2>,
#if CGAL_VERSION_NR < 1041001000
								CGAL::Arr_halfedge_base<X_monotone_curve_2>,
#else
								CGAL::Gps_halfedge_base<X_monotone_curve_2>,
#endif
								CGAL::Gps_face_base>									Dcel;

typedef CGAL::Arrangement_2<Traits_2,Dcel>					Arrangement_2;

typedef Arrangement_2::Vertex_handle                  Vertex_handle;
typedef Arrangement_2::Halfedge_handle                Halfedge_handle;
typedef Arrangement_2::Face_handle                    Face_handle;

typedef Arrangement_2::Vertex_const_handle                  Vertex_const_handle;
typedef Arrangement_2::Halfedge_const_handle                Halfedge_const_handle;
typedef Arrangement_2::Face_const_handle                    Face_const_handle;

typedef CGAL::Arr_accessor<Arrangement_2>               Arr_accessor;

typedef  Arr_accessor::Dcel_vertex              DVertex;
typedef  Arr_accessor::Dcel_halfedge            DHalfedge;
typedef  Arr_accessor::Dcel_face                DFace;
typedef  Arr_accessor::Dcel_outer_ccb			DOuter_ccb;
typedef  Arr_accessor::Dcel_inner_ccb           DInner_ccb;
typedef  Arr_accessor::Dcel_isolated_vertex     DIso_vert;

// Landmark point location is pretty fast to construct, even on a complex map, and has good lookup time.  I tried the RIC
// locator, but instantiation time is significantly longer.
typedef CGAL::Arr_landmarks_point_location<Arrangement_2>  Locator;

typedef Arrangement_2		Pmwx;


/******************************************************************************************************************************************************
 * GENERAL POLYGONS
 ******************************************************************************************************************************************************/

/*
	Polygon_set_2 is a class that provides polygon boolean operations, using an arrangement to do the merge-cut operations.  Because it uses an
	arragement we can make some very fast conversions from a map to a polygon set and back.  For example, we could:

	- Build a bunch of polygons, then use overlay to dump the results directly into a map.
	- Get our map as a set of polygons.

	NOTE: if we are going to construct a polygon set from an arrangement, we must set the "contained" property on all faces!!
*/


class	Polygon_set_2	: public CGAL::General_polygon_set_2<Traits_2, Dcel > {
public:

	typedef	CGAL::General_polygon_set_2<Traits_2, Dcel >	base;
	
	  typedef base::Traits_2                                        Traits_2;
	  typedef base::Dcel                                            Dcel;
	  typedef base::Polygon_2										Polygon_2;
	  typedef base::Polygon_with_holes_2							Polygon_with_holes_2;
	  typedef base::Arrangement_2									Arrangement_2;
	  typedef std::size_t											Size;

	Polygon_set_2() { }
	Polygon_set_2(const Polygon_2& rhs) : base(rhs) { }

	Polygon_set_2(const Polygon_set_2& rhs) : base(rhs) { }
	Polygon_set_2(const base& rhs) : base(rhs) { }

	Polygon_set_2(const Arrangement_2& rhs)
	{
		CGAL_precondition(rhs.is_valid());
	    delete m_arr;
		m_arr = new Arrangement_2(rhs);
		// When we take someone else's arrangement, we need to do two things to 'prepare'
		// it for use as a GPS:
		// 1. We need to remove redundant edges.  The GPS assumes that edges only exist to
		// separate the "in" and "out" areas!
		remove_redundant_edges();
		// 2. We need to ensure that the direction of the underlying curves of the arrangement
		// is consistent with a single contiguous CCB.  In other words, as we walk a CCB, every
		// curve should be with us or against us.  (By definition, the map will be 50-50 split
		// between the two cases.)  Since we removed redundant edges, by definition there are 
		// no degree > 2 vertices, and thus we can ensure that this can be applied everywhere.
		fix_curves_direction();
		CGAL_postcondition(this->is_valid());		
	}

	Polygon_set_2& operator=(const Arrangement_2& rhs)
	{
		CGAL_precondition(rhs.is_valid());
	    delete m_arr;
		m_arr = new Arrangement_2(rhs);
		remove_redundant_edges();
		fix_curves_direction();
		CGAL_postcondition(this->is_valid());		
		return *this;
	}

	Polygon_set_2& operator=(const Polygon_set_2& ps)
	{
		if (this == &ps)
			return (*this);

		if (m_traits_owner)
			delete m_traits;
		delete m_arr;
		m_traits = new Traits_2(*(ps.m_traits));
		m_traits_owner = true;
		m_arr = new Arrangement_2(*(ps.m_arr));
		return (*this);
	}
	
};



/******************************************************************************************************************************************************
 * MISC CRAP
 ******************************************************************************************************************************************************/

/*
	This is all weird misc. crap that we ended up having to have to support temporary legacy code.  We're not done with the evolution until all of this
	stuff is unused and can be nuked.
*/



/*
inline Vector_2 normalize(Vector_2 v) {
	return v * (1.0/sqrt(CGAL::to_double(v.squared_length())));
}

inline FastKernel::Vector_3 normalize(FastKernel::Vector_3 v) {
	return v * (1.0/sqrt(CGAL::to_double(v.squared_length())));
}
 */

inline Vector_2 normalize(Vector_2 v) {
	return v * (1.0/sqrt(CGAL::to_double(v.x())*CGAL::to_double(v.x()) + CGAL::to_double(v.y())*CGAL::to_double(v.y())));
}

inline FastKernel::Vector_3 normalize(FastKernel::Vector_3 v) {
	return v * (1.0/sqrt(CGAL::to_double(v.x())*CGAL::to_double(v.x()) + CGAL::to_double(v.y())*CGAL::to_double(v.y()) + CGAL::to_double(v.z())*CGAL::to_double(v.z())));
}

/*
inline Point_2 centroid(Polygon_2 p) {
	return centroid(p.vertices_begin(), p.vertices_end());
}
*/

// Given a segment, move it to the left (based on its directionality) by a distance.
static	void	MoveSegLeft(const Segment_2& l1, double dist, Segment_2& l2)
{
	Vector_2	v = Vector_2(l1.source(), l1.source()).perpendicular(CGAL::COUNTERCLOCKWISE);
	v = normalize(v);
	v = v * dist;
	l2 = Segment_2(l1.source() + v, l1.target() + v);
}


static void	InsetPolygon_2(
					  const Polygon_2&				inChain,
					  const double *				inRatios,
					  double						inInset,
					  bool						inIsRing,
					  Polygon_2&					outChain,
					  void						(* antennaFunc)(int n, void * ref),
					  void *						ref)
{
	if (!outChain.is_empty())
		outChain.clear();

	vector<Segment_2>	segments, orig_segments;

	// First we calculate the inset edges of each side of the polygon.

	for (int n = 0, m = 1; n < inChain.size(); ++n, ++m)
	{
		Segment_2	edge(inChain[n], inChain[m % inChain.size()]);
		orig_segments.push_back(edge);
		Segment_2	seg;
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
			outChain.push_back(segments[outgoing_n].source());
		}
		else if (outgoing_n == last_vertex && !inIsRing)
		{
			/* CASE 2 */
			// We're the last in a chain.  Incoming vertex is always right
			outChain.push_back(segments[incoming_n].target());
		}
		else if (orig_segments[incoming_n].source() == orig_segments[outgoing_n].target())
		{
			/* CASE 3 */
			// Are the two sides in exactly opposite directions?  Special case...we have to add a vertex.
			// (This is almost always an "antenna" in the data, that's why we have to add the new side, the point of the antenna
			// becomes thick.  Since antennas have equal coordinates, an exact opposite test works.)
			Segment_2	new_side(segments[incoming_n].target(), segments[outgoing_n].source()), new_side2;
			MoveSegLeft(new_side, (inRatios != NULL) ? (inRatios[outgoing_n] * inInset) : inInset, new_side2);
			//			new_side2 = new_side;
			outChain.push_back(new_side2.source());
			outChain.push_back(new_side2.target());
			if (antennaFunc) antennaFunc(outgoing_n + (num_inserted++), ref);
		} else {

			// These are the intersecting cases - we need a dot product to determine what to do.
			Vector_2 v1(segments[incoming_n].source(),segments[incoming_n].target());
			Vector_2 v2(segments[outgoing_n].source(),segments[outgoing_n].target());
			v1 = normalize(v1);
			v2 = normalize(v2);
			double dot = CGAL::to_double(v1 * v2);

			if (dot > 0.999961923064)
			{
				/* CASE 4 */
				// Our sides are nearly colinear - don't trust intersect!
				outChain.push_back(CGAL::midpoint(segments[incoming_n].target(), segments[outgoing_n].source()));
			}
			else if (dot < -0.5 && !(CGAL::orientation(v1,v2)==CGAL::LEFT_TURN))
			{
				/* CASE 5 */
				// A sharp outward turn of more than 60 degrees - at this point the intersect point will be over
				// twice the road thickness from the intersect point.  Not good!
				Point_2	p1(segments[incoming_n].target());
				Point_2	p2(segments[outgoing_n].source());
				p1 = p1 + (v1 * ((inRatios == NULL) ? 1.0 : inRatios[outgoing_n]) *  inInset);
				p2 = p2 + (v2 * ((inRatios == NULL) ? 1.0 : inRatios[outgoing_n]) * -inInset);
				outChain.push_back(p1);
				outChain.push_back(p2);
				if (antennaFunc) antennaFunc(outgoing_n + (num_inserted++), ref);
			}
			else
			{
				/* GENERAL CASE */
				// intersect the supporting line of two segments.
				Line_2	line1(segments[incoming_n]);
				Line_2	line2(segments[outgoing_n]);
				Point_2	p;
				CGAL::Object r = CGAL::intersection(line1,line2);
				if (CGAL::assign(p, r))
					outChain.push_back(p);
				else
					outChain.push_back(CGAL::midpoint(segments[incoming_n].target(), segments[outgoing_n].source()));
			}
		}
	}
}

#if CGAL_VERSION_NR < 1041001000


inline bool operator<(const Face_handle& lhs, const Face_handle& rhs)			{	return &*lhs < &*rhs;	}
inline bool operator<(const Vertex_handle& lhs, const Vertex_handle& rhs)		{	return &*lhs < &*rhs;	}
inline bool operator<(const Halfedge_handle& lhs, const Halfedge_handle& rhs)	{	return &*lhs < &*rhs;	}
inline bool operator<(const Face_const_handle& lhs, const Face_const_handle& rhs)			{	return &*lhs < &*rhs;	}
inline bool operator<(const Vertex_const_handle& lhs, const Vertex_const_handle& rhs)		{	return &*lhs < &*rhs;	}
inline bool operator<(const Halfedge_const_handle& lhs, const Halfedge_const_handle& rhs)	{	return &*lhs < &*rhs;	}

#endif




#endif
