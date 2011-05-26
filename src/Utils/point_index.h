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

#ifndef point_index_H
#define point_index_H

/*

	POINT INDEX - THEORY OF OPERATION
	
	This class implements a point set that supporst range queries.  It is based on the same 
	ideas as the CGAL point_set_2 class, but a bug: the CGAL class will run the stack out due
	to stack recursion.
	
	The class supports exact location, range queries, and insert/remove points.
	
	The class works by keeping the points in a Delaunay triangulation.  Range queries do a 
	breadth-first search starting at the origin search point (which is temporarily inserted into
	the triangulation, which helps deal with degenerate queries).	

 */

#include <CGAL/Delaunay_triangulation_2.h>

template <typename Traits>
class	spatial_index_2 {
public:

	typedef	typename Traits::Point_2		Point_2;

	void		insert(const Point_2& p);
	
	template<typename Iterator>
	void		insert(Iterator begin, Iterator end);
	
	// Insert a range of points using a converter functor to convert from the iterator to the point type.  Use this
	// to insert a collection of points from a structured container like vertices of an arrangement or triangulation.
	template<typename Iterator, typename Converter>
	void		insert(Iterator begin, Iterator end, const Converter& converter);
	
	void		remove(const Point_2& p);	
	void		clear();
	
	bool		empty(void) const;
	bool		contains(const Point_2& p);
	
	// Call the visitor for every point in the set that is on or within the circle 'where' - where should
	// be counter-clockwise oriented.  The visitor is called once per point; if it returns 'true' then
	// we early-exit the search.  (If we are looking for 'any of' criteria, this can be a win.)
	template<typename Visitor>
	void		search(const typename Traits::Circle_2& where, Visitor& visitor);
	
private:

	typedef typename	CGAL::Delaunay_triangulation_2<Traits>		DT;
	DT			mDT;

};

/*

	INLINE METHODS

 */

template<typename Traits>
void		spatial_index_2<Traits>::insert(const Point_2& p)
{
	mDT.insert(p);
}

template<typename Traits>
template<typename Iterator>
void		spatial_index_2<Traits>::insert(Iterator begin, Iterator end)
{
	mDT.insert(begin,end);
}

template<typename Traits>
template<typename Iterator, typename Converter>
void		spatial_index_2<Traits>::insert(Iterator begin, Iterator end, const Converter& converter)
{
	for(Iterator i = begin; i != end; ++i)
		mDT.insert(converter(i));
}

template<typename Traits>
void		spatial_index_2<Traits>::remove(const Point_2& p)
{
	typename DT::Locate_type l;
	int li;
	typename DT::Face_handle f = mDT.locate(p,l,li);
	if(l == DT::VERTEX)
		mDT.remove(f->vertex(li));
}

template<typename Traits>
void		spatial_index_2<Traits>::clear()
{
	mDT.clear();
}

template<typename Traits>
bool		spatial_index_2<Traits>::empty(void) const
{
	return mDT.number_of_vertices() == 0;
}

template<typename Traits>
bool		spatial_index_2<Traits>::contains(const Point_2& p)
{
	typename DT::Locate_type l;
	int li;
	typename DT::Face_handle f = mDT.locate(p,l,li);
	return l == DT::VERTEX;
}


template<typename Traits>
template<typename Visitor>
void		spatial_index_2<Traits>::search(const typename Traits::Circle_2& where, Visitor& visitor)
{
	DebugAssert(where.orientation() == CGAL::COUNTERCLOCKWISE);

	Point_2		ctr = where.center();
	NT			sr2 = where.squared_radius();
	
	list<typename DT::Vertex_handle>	to_visit;
	set<typename DT::Vertex_handle>	visited;
	typename DT::Locate_type l;

	int i;	
	typename DT::Face_handle f = mDT.locate(ctr,l,i);
	typename DT::Vertex_handle center_pt;
	bool	did_insert = false;
	if(l == DT::VERTEX)
		center_pt = f->vertex(i);
	else	
	{
		did_insert = true;
		center_pt = mDT.insert(ctr,l,f,i);
	}

	to_visit.push_back(center_pt);	
	visited.insert(center_pt);
	visited.insert(mDT.infinite_vertex());		// Pre-mark infinite vertex, to avoid having to do an extra check.
	while(!to_visit.empty())
	{
		typename DT::Vertex_handle v = to_visit.front();
		to_visit.pop_front();
		
		if(!did_insert || v != center_pt)
		if (visitor(v->point()))
			break;
		
		typename DT::Vertex_circulator circ, stop;
		circ = stop = v->incident_vertices();
		do 
		{
			if(visited.count(circ) == 0)
			if(!where.has_on_negative_side(circ->point()))	// on the line is in...
			{
				to_visit.push_back(circ);
			}
			visited.insert(circ);				// pre-mark on circulate, aovid re-checking circle over and over.
		} while (++circ != stop);
	}

	if(did_insert)
		mDT.remove(center_pt);
	
	
}


#endif /* point_index_H */
