/*
 *  MapDefsCGAL.h
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

#ifndef MAPDEFSCGAL_H
#define MAPDEFSCGAL_H

#include <CGAL/Cartesian.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Filtered_kernel.h>
#include <CGAL/Arr_segment_traits_2.h>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_set_2.h>

#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_dcel_base.h>
#include <CGAL/Arr_extended_dcel.h> // we need data extensions for everything
#include <CGAL/Arr_consolidated_curve_data_traits_2.h>

#include <CGAL/Arr_landmarks_point_location.h>
#include <CGAL/Arr_overlay.h>
#include <CGAL/Arr_default_overlay_traits.h>

#include <CGAL/Boolean_set_operations_2/Gps_default_dcel.h>

#include <CGAL/Bbox_2.h>

#include <CGAL/assertions.h>
#include <CGAL/intersections.h>


#include "ParamDefs.h"
#include "AssertUtils.h"
#include "CompGeomDefs2.h"

#if USE_GMP
#include <CGAL/Gmpq.h>
#include <CGAL/Lazy_exact_nt.h>
#else
#include <CGAL/Quotient.h>
#include <CGAL/MP_Float.h>
#endif

/******************************************************************************************************************************************************
 *
 ******************************************************************************************************************************************************/

#if USE_GMP
// Use GMP for our number type.  It appers to be maybe 10% faster than quotient<MP_float>.  We use the Lazy_exact_nt adapter to
// defer calculation where possible.  We must have an exact number type or inserting into maps (which is fundamental to ALL processing) can blow up.
typedef CGAL::Lazy_exact_nt<CGAL::Gmpq> NT;
#else
typedef CGAL::Lazy_exact_nt<CGAL::Quotient<CGAL::MP_Float> >  NT;
#endif

// Use the filtered kernel to answer predicates rapidly in easy cases.  
// Lazy kernel seems to be slower, and optimized compile almost brings down a Mac Pro, which is NOT a good sign.
typedef CGAL::Filtered_kernel<CGAL::Simple_cartesian<NT> > FastKernel;

typedef CGAL::Bbox_2									Bbox_2;
typedef FastKernel::Point_2                             Point_2;
typedef FastKernel::Vector_2                            Vector_2;
typedef FastKernel::Triangle_2							Triangle_2;
typedef FastKernel::Point_3                             Point_3;
typedef FastKernel::Vector_3                            Vector_3;
typedef FastKernel::Plane_3                             Plane_3;
typedef FastKernel::Segment_2                           Segment_2;
typedef CGAL::Line_2<FastKernel>                        Line_2;
typedef FastKernel::Ray_2								Ray_2;
typedef CGAL::Polygon_2<FastKernel>						Polygon_2;					// Ben says: this only works because GPS polygon uses "standard" kernel polygons.  If this was not
typedef CGAL::Polygon_with_holes_2<FastKernel>			Polygon_with_holes_2;		// true, we could use these definitons from our GPS segment traits.  This allows us to have polygons
																					// Without GPS polygons!


inline CGAL::Bbox_2& operator+=(Bbox_2& lhs, const Bbox_2& rhs)
{
	lhs = lhs + rhs;
	return lhs;
}
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
 * A point feature contains a seriers of parameters indicating
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
	int				mRepType;
	Point_2			mLocation;
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
	Polygon_with_holes_2	mShape;
	Point_2				mLocation;	// Nominal center - used primarily for debugging!
	double				mHeight;
	bool				mDerived;
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
};







struct GIS_vertex_data {
	bool mTunnelPortal;
#if OPENGL_MAP
	float						mGL[2];				// Pre-expanded line!
#endif	
};

struct GIS_halfedge_data {
public:
	GIS_halfedge_data() : mDominantXXX(false) { }

	int							mTransition;		// Transition type ID
	GISNetworkSegmentVector		mSegments;			// Network segments along us
	GISParamMap					mParams;
	double						mInset;				// Largest unusable inset for this side
	bool                        mDominantXXX;
	bool						mMark;				// Temporary, for algorithms
#if OPENGL_MAP
	float						mGL[4];				// Pre-expanded line!
	float						mGLColor[3];	
#endif
};



class GIS_face_data {
public:
	//	int							mIsWater;
	int							mTerrainType;		// This is a feature type for matching.  EXCEPTION: terrain_Water is both a feature and terrain.
	GISParamMap					mParams;
	GISPointFeatureVector		mPointFeatures;
	GISPolygonFeatureVector		mPolygonFeatures;
	//GISAreaFeatureVector			mAreaFeature;	
	GISAreaFeature_t			mAreaFeature;	
	// Stuff that's been hand placed in the area by object propagation
	GISObjPlacementVector		mObjs; 
	GISPolyObjPlacementVector	mPolyObjs;
	int							mTemp1;							// Per face temp value
	int							mTemp2;							// Per face temp value
	
	bool		IsWater(void) const  { return (mTerrainType == terrain_Water); }
	bool		TerrainMatch(const GIS_face_data& rhs) const { return mTerrainType == rhs.mTerrainType; }
	bool		AreaMatch(const GIS_face_data& rhs) const { return (mTerrainType == rhs.mTerrainType && mAreaFeature.mFeatType == rhs.mAreaFeature.mFeatType); }

	#if DEV
		~GIS_face_data() { mTerrainType = 0xDEADBEEF; }
	#endif
	GIS_face_data() : mTerrainType(0) { mAreaFeature.mFeatType = 0; }
	GIS_face_data(const GIS_face_data &x) {
		mTerrainType = x.mTerrainType;
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
	vector<float>				mGLTris;						// Pre-expanded triangles
	float						mGLColor[4];
#endif	
};

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
								CGAL::Arr_halfedge_base<X_monotone_curve_2>,
								CGAL::Gps_face_base>									Dcel_base;
								
class	Dcel : public Dcel_base {
public:

  Halfedge* new_edge() 
  {
	Halfedge * h = Dcel_base::new_edge();
	h->data().mDominantXXX = 1;
	h->opposite()->data().mDominantXXX = 0;
	return h;
	}
};

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
typedef  Arr_accessor::Dcel_hole                DHole;
typedef  Arr_accessor::Dcel_isolated_vertex     DIso_vert;

// Landmark point location is pretty fast to construct, even on a complex map, and has good lookup time.  I tried the RIC
// locator, but instantiation time is significantly longer.
typedef CGAL::Arr_landmarks_point_location<Arrangement_2>  Locator;

typedef Arrangement_2		Pmwx;

inline bool	he_is_same_direction(Halfedge_handle he)
{
	return (he->curve().is_directed_right() == (he->direction() == CGAL::SMALLER));
}

inline Halfedge_handle he_get_same_direction(Halfedge_handle he)
{
	return he_is_same_direction(he) ? he : he->twin();
}

inline bool he_is_same_direction_as(Halfedge_handle he, const Curve_2& c)
{
	return CGAL::angle(
		he->source()->point(),
		he->target()->point(),
		he->target()->point() + Vector_2(c.source(),c.target())) == CGAL::OBTUSE;
}


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
	    delete m_arr;
		m_arr = new Arrangement_2(rhs);
		remove_redundant_edges();
	}

	Polygon_set_2& operator=(const Arrangement_2& rhs)
	{
	    delete m_arr;
		m_arr = new Arrangement_2(rhs);
		remove_redundant_edges();
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
	
	int n = 0;
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

struct	CoordTranslator_2 {
	Point_2	mSrcMin;
	Point_2	mSrcMax;
	Point_2	mDstMin;
	Point_2	mDstMax;
	
	Point_2	Forward(const Point_2& input) const;
	Point_2	Reverse(const Point_2& input) const;
};



 inline Point_2	CoordTranslator_2::Forward(const Point_2& input) const
{
	return Point_2(
				  mDstMin.x() + (input.x() - mSrcMin.x()) * (mDstMax.x() - mDstMin.x()) / (mSrcMax.x() - mSrcMin.x()),
				  mDstMin.y() + (input.y() - mSrcMin.y()) * (mDstMax.y() - mDstMin.y()) / (mSrcMax.y() - mSrcMin.y()));
}
 inline Point_2	CoordTranslator_2::Reverse(const Point_2& input) const
{
	return Point_2(
				  mSrcMin.x() + (input.x() - mDstMin.x()) * (mSrcMax.x() - mSrcMin.x()) / (mDstMax.x() - mDstMin.x()),
				  mSrcMin.y() + (input.y() - mDstMin.y()) * (mSrcMax.y() - mSrcMin.y()) / (mDstMax.y() - mDstMin.y()));
}

inline Point_2	ben2cgal(const Point2& p) { return Point_2(p.x(),p.y()); }
inline Point2	cgal2ben(const Point_2& p) { return Point2(CGAL::to_double(p.x()),CGAL::to_double(p.y())); }
inline Segment2	cgal2ben(const Segment_2& s) { return Segment2(cgal2ben(s.source()),cgal2ben(s.target())); }


inline bool operator<(const Face_handle& lhs, const Face_handle& rhs)			{	return &*lhs < &*rhs;	}
inline bool operator<(const Vertex_handle& lhs, const Vertex_handle& rhs)		{	return &*lhs < &*rhs;	}
inline bool operator<(const Halfedge_handle& lhs, const Halfedge_handle& rhs)	{	return &*lhs < &*rhs;	}






#endif
