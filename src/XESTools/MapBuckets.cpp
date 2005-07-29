THIS HEADER IS OBSOLETE

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
#include "MapBuckets.h"
#include "XBuckets_inline.h"


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

void	BuildFaceBuckets(Pmwx& inMap, MapFaceBuckets& outBuckets)
{
	outBuckets.RemoveAllAndDestroy();
	for (Pmwx::Face_iterator i = inMap.faces_begin(); i != inMap.faces_end(); ++i)
	{
		if (!i->is_unbounded())
			outBuckets.Insert(i);
	}
}

void	BuildHalfedgeBuckets(Pmwx& inMap, MapHalfedgeBuckets& outBuckets)
{
	outBuckets.RemoveAllAndDestroy();
	for (Pmwx::Halfedge_iterator i = inMap.halfedges_begin(); i != inMap.halfedges_end(); ++i)
	{
		if (i->mDominant)
			outBuckets.Insert(i);
	}
}

void	BuildVertexBuckets(Pmwx& inMap, MapVertexBuckets& outBuckets)
{
	outBuckets.RemoveAllAndDestroy();
	for (Pmwx::Vertex_iterator i = inMap.vertices_begin(); i != inMap.vertices_end(); ++i)
	{
		outBuckets.Insert(i);
	}
}


