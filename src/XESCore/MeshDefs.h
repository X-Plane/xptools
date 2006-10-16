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
#ifndef MESHDEFS_H
#define MESHDEFS_H

#include "ParamDefs.h"
#include "DemDefs.h"
#include "CompGeomDefs2.h"



//------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------

class GISFace;


typedef multimap<float, void *, greater<float> >			FaceQueue;	// YUCK - hard cast to avoid snarky problems with forward decls

class	CDT_Vertex;
class	CDT_Face;

class	CDT_Vertex_info {
public:
	double					height;					// Height of mesh at this vertex.
	double					wave_height;			// ratio of vegetation to terrain at this vertex.
	float					normal[3];				// Normal - X,Y,Z in OGL coords(!)
	hash_map<int, float>	border_blend;			// blend level for a border of this layer at this triangle!
};

class	CDT_Face_info {
public:
	int				insert_x;
	int				insert_y;
	float			insert_err;
	double			plane_a;
	double			plane_b;
	double			plane_c;

	int				terrain;				// General terrain type for this triangle, e.g. terrain_Natural, terrain_Water
	int				feature;				// Specific terrain type, e.g. natural converted to a real land use. (This is a .ter enum, NOT a table index btw)
	int				flag;					// General purpose, useful for..um...algorithms.
	set<int>		terrain_border;			// All terrains on top of us!
	float			normal[3];				// Tri flat normal - not in final DSF but handy for other sh-t.

	const GISFace *	orig_face;				// If a face caused us to get the terrain we did, this is who!

	FaceQueue::iterator	self;					// Queue ref to self!

// BENTODO - clean this up	
	float			debug_slope_dem;
	float			debug_slope_tri;
	float			debug_temp;
	float			debug_temp_range;
	float			debug_rain;
	float			debug_heading;

};


class	CDT_Vertex {
private:
	CDT_Vertex *	mPrev;
	CDT_Vertex *	mNext;

	Point2			mPoint;
	CDT_Face *		mFace;
	CDT_Vertex_info	mInfo;
	
public:

			Point2&		point()							{ return mPoint; }
	const	Point2&		point() const					{ return mPoint; }
			void		set_point(const Point2& p)		{ mPoint = p; }
			
			CDT_Face *	face()							{return mFace; }
	const	CDT_Face *	face() const					{ return mFace; }
			void		set_face(CDT_Face * face)		{ mFace = face; }

			CDT_Vertex_info&	info()					{ return mInfo; }
	const	CDT_Vertex_info&	info() const			{ return mInfo; }
			
};


class	CDT_Face {
private:
	CDT_Face *		mPrev;
	CDT_Face *		mNext;
	
	CDT_Vertex *	mVertex[3];
	CDT_Face *		mFace[3];
	bool			mConstraint[3];

	CDT_Face_info	mInfo;

public:	

	CDT_Vertex *	vertex(int n) ;
	CDT_Face *		neighbor(int n);
	int				index(const CDT_Vertex * v) const;
	bool			has_vertex(const CDT_Vertex * v) const;

		  CDT_Face_info&	info()		 { return mInfo; }
	const CDT_Face_info&	info() const { return mInfo; }

};

class	CDT_Vertex_circulator {
public:
	CDT_Vertex_circulator();
	CDT_Vertex_circulator(CDT_Vertex * v);
	CDT_Vertex_circulator(const CDT_Vertex_circulator& rhs);
	CDT_Vertex_circulator& operator=(const CDT_Vertex_circulator& rhs);
	bool operator==(const CDT_Vertex_circulator& rhs) const;
	bool operator==(const CDT_Vertex *			 rhs) const;
	bool operator!=(const CDT_Vertex_circulator& rhs) const;
	bool operator!=(const CDT_Vertex *			 rhs) const;
	CDT_Vertex_circulator& operator++();
	CDT_Vertex_circulator& operator++(int);
	CDT_Vertex_circulator& operator--();
	CDT_Vertex_circulator& operator--(int);
	CDT_Vertex& operator*();
	CDT_Vertex* operator->();
	operator CDT_Vertex*();

private:
	friend class CDT;
	CDT_Vertex *	mVert;
	CDT_Face *		mFace;
	int				mIndex;
};

class	CDT_Face_circulator {
public:
	CDT_Face_circulator();
	CDT_Face_circulator(CDT_Face * v);
	CDT_Face_circulator(const CDT_Face_circulator& rhs);
	CDT_Face_circulator& operator=(const CDT_Face_circulator& rhs);
	bool operator==(const CDT_Face_circulator& rhs) const;
	bool operator==(const CDT_Face *		   rhs) const;
	bool operator!=(const CDT_Face_circulator& rhs) const;
	bool operator!=(const CDT_Face *		   rhs) const;
	CDT_Face_circulator& operator++();
	CDT_Face_circulator& operator++(int);
	CDT_Face_circulator& operator--();
	CDT_Face_circulator& operator--(int);
	CDT_Face& operator*();
	CDT_Face* operator->();
	operator CDT_Face*();
private:
	friend class CDT;
	CDT_Vertex *	mVert;
	CDT_Face *		mFace;
};

class	CDT_Finite_faces_iterator {
public:
	CDT_Finite_faces_iterator();
	CDT_Finite_faces_iterator(CDT_Face * f);
	CDT_Finite_faces_iterator(const CDT_Finite_faces_iterator& rhs);
	CDT_Finite_faces_iterator& operator=(const CDT_Finite_faces_iterator& rhs);
	bool operator==(const CDT_Finite_faces_iterator& rhs) const;
	bool operator==(const CDT_Face *				  rhs) const;
	bool operator!=(const CDT_Finite_faces_iterator& rhs) const;
	bool operator!=(const CDT_Face *				  rhs) const;
	CDT_Finite_faces_iterator& operator++(int);
	CDT_Finite_faces_iterator& operator++();
	CDT_Finite_faces_iterator& operator--(int);
	CDT_Finite_faces_iterator& operator--();
	
	CDT_Face& operator*();
	CDT_Face* operator->();
	operator CDT_Face *();
private:
	CDT_Face * mFace;
};

class	CDT_Finite_vertices_iterator {
public:
	CDT_Finite_vertices_iterator();
	CDT_Finite_vertices_iterator(CDT_Vertex * f);
	CDT_Finite_vertices_iterator(const CDT_Finite_vertices_iterator& rhs);
	CDT_Finite_vertices_iterator& operator=(const CDT_Finite_vertices_iterator& rhs);
	bool operator==(const CDT_Finite_vertices_iterator& rhs) const;
	bool operator==(const CDT_Vertex *				  rhs) const;
	bool operator!=(const CDT_Finite_vertices_iterator& rhs) const;
	bool operator!=(const CDT_Vertex *				  rhs) const;
	CDT_Finite_vertices_iterator& operator++(int);
	CDT_Finite_vertices_iterator& operator++();
	CDT_Finite_vertices_iterator& operator--(int);
	CDT_Finite_vertices_iterator& operator--();
	
	CDT_Vertex& operator*();
	CDT_Vertex* operator->();
	operator CDT_Vertex *();
private:
	CDT_Vertex * mVertex;
};


class	CDT {
public:

	enum Locate_type {
		VERTEX,
		EDGE,
		FACE,
		OUTSIDE_CONVEX_HULL,
		INSIDE_AFFINE_HULL
	};

	typedef pair<CDT_Face *, int>			Edge;
	typedef Point2							Point;
	typedef	CDT_Face *						Face_handle;
	typedef CDT_Vertex *					Vertex_handle;
	typedef	CDT_Vertex_circulator			Vertex_circulator;
	typedef CDT_Face_circulator				Face_circulator;
	typedef	CDT_Finite_faces_iterator		Finite_faces_iterator;	
	typedef	CDT_Finite_vertices_iterator	Finite_vertices_iterator;	
	
	bool	is_edge(Vertex_handle a, Vertex_handle b, Face_handle& f, int& i) const;
	bool	is_face(Vertex_handle a, Vertex_handle b, Vertex_handle c, Face_handle& f) const;
	bool	is_constrained(const Edge& e) const;
	bool	is_infinite(const Vertex_handle v) const;
	bool	is_infinite(const Face_handle f) const;

	bool	are_there_incident_constraints(Vertex_handle v) const;

	Vertex_circulator			incident_vertices(Vertex_handle v) const;
	Face_circulator				incident_faces(Vertex_handle v) const;

	Finite_faces_iterator		finite_faces_begin();
	Finite_faces_iterator		finite_faces_end();
	Finite_vertices_iterator	finite_vertices_begin();
	Finite_vertices_iterator	finite_vertices_end();

	const Vertex_handle	infinite_vertex(void) const;
	
	Vertex_handle	insert(const Point2& p, Face_handle hint = NULL);
	void			insert_constraint(Vertex_handle v1,Vertex_handle v2);
	void			remove(Vertex_handle v);
	Vertex_handle	safe_insert(const Point2& p, Face_handle hint);

	Face_handle		locate(const Point2& p, Locate_type& lt, int& vnum, Face_handle face_hint = NULL);

	void			clear();
	
	int				number_of_faces() const;

	static	int			gen_cache_key(void);
			Face_handle locate_cache(const Point& p, Locate_type& lt, int& li, int cache_key) const;
			void		cache_reset(void);
	
	static int ccw(int n);
	static int cw(int n);
	
private:

	CDT_Face *	mFaceFirst;
	CDT_Face *	mFaceLast;

	CDT_Vertex *	mVertexFirst;
	CDT_Vertex *	mVertexLast;
	
	CDT_Vertex *	mInfinite;
	
};

#if !DEV
	put this bakc
#endif
#if 0
class CDT : public CDTBase { 
public:

	typedef hash_map<int, Face_handle>	HintMap;

	static	int			gen_cache_key(void);
			Face_handle locate_cache(const Point& p, Locate_type& lt, int& li, int cache_key) const;
			void		cache_reset(void);
			void		clear(void);


	// SAFE insert...basically when using a fast cartesian kernel, the
	// face locate may return 'face' when it means 'edge' because the 
	// set of tests for the march-locate aren't quite specific enough to resolve
	// rounding errors.  (At least that's what I _think_ it is.)  So...
	// We do the locate, check for this case, and try to manually find an edge
	// we're on.  If we do, we fix up the locate info and procede safely.  If not
	// we assert - we would have done that anyway.
#if DEV
	Vertex_handle	safe_insert(const Point& p, Face_handle hint);
#else
	Vertex_handle	safe_insert(const Point& p, Face_handle hint)
	{
		int			li;
		Locate_type	lt;
		Face_handle	who = locate(p, lt, li, hint);
		if (lt == FACE && oriented_side(who, p) != CGAL::ON_POSITIVE_SIDE)
		{
			if(lt == FACE && oriented_side(who, p) != CGAL::ON_POSITIVE_SIDE)
			{

				Point	p0(who->vertex(0)->point());
				Point	p1(who->vertex(1)->point());
				Point	p2(who->vertex(2)->point());

				CGAL_triangulation_precondition( orientation(p0, p1, p2) != CGAL::COLLINEAR);
					
				CGAL::Orientation 	o2 = orientation(p0, p1, p),
									o0 = orientation(p1, p2, p),
									o1 = orientation(p2, p0, p),
									o2b= orientation(p1, p0, p),
									o0b= orientation(p2, p1, p),
									o1b= orientation(p0, p2, p);

				// Collinear witih TWO sides?  Hrmm...should be a vertex.
				if (o1 == CGAL::COLLINEAR && o2 == CGAL::COLLINEAR) { li = 0; lt = VERTEX; }
				if (o2 == CGAL::COLLINEAR && o0 == CGAL::COLLINEAR) { li = 1; lt = VERTEX; }
				if (o0 == CGAL::COLLINEAR && o1 == CGAL::COLLINEAR) { li = 2; lt = VERTEX; }

				// Colinear with a side and positive on the other two - should be on that edge.
				if (o1 == CGAL::COLLINEAR && o2 == CGAL::POSITIVE && o0 == CGAL::POSITIVE) 				{ li = 1; lt = EDGE; }
				if (o2 == CGAL::COLLINEAR && o0 == CGAL::POSITIVE && o1 == CGAL::POSITIVE) 				{ li = 2; lt = EDGE; }
				if (o0 == CGAL::COLLINEAR && o1 == CGAL::POSITIVE && o2 == CGAL::POSITIVE) 				{ li = 0; lt = EDGE; }
				
				// On negative of a side AND its opposite?  We've got a rounding error.  Call it the edge and go home.
				if (o0 == CGAL::NEGATIVE && o0b == CGAL::NEGATIVE) { li = 0; lt = EDGE; }
				if (o1 == CGAL::NEGATIVE && o1b == CGAL::NEGATIVE) { li = 1; lt = EDGE; }
				if (o2 == CGAL::NEGATIVE && o2b == CGAL::NEGATIVE) { li = 2; lt = EDGE; }

				if (lt == FACE) AssertPrintf("Unable to resolve bad locate.");
			}
		}
		return CDTBase::insert(p, lt, who, li);
	}
#endif	
private:

	static	int		sKeyGen;
	mutable	HintMap	mHintMap;
	
};
#endif

//#define CONVERT_POINT(__X)	(CDT::Point((__X).x,(__X).y))

#endif

