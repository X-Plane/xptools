/* 
 * Copyright (c) 2007, Laminar Research.
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
#ifndef MAPBUCKETS_H
#define MAPBUCKETS_H

#include "MapDefs.h"
#include "XBuckets.h"

class	MapFaceBucketTraits {
public:

	typedef	Point2				Point2;
	typedef	double				Scalar;
	typedef	Pmwx::Face_handle	Object;

	static	Scalar	X(const Point2& p) { return (p.x); }
	static	Scalar	Y(const Point2& p) { return (p.y); }
	static	void	MakePoint(Scalar x, Scalar y, Point2& p) {	p = Point2(x, y); }

	static	void	GetObjectBounds(Object, Point2&, Point2&);
	static	bool	ObjectTouchesPoint(Object, const Point2&);
	static	bool	ObjectTouchesRect(Object, const Point2&, const Point2&);
	static	bool	ObjectFullyInRect(Object, const Point2&, const Point2&);
	static	void	DestroyObject(Object);
};	

class	MapHalfedgeBucketTraits {
public:

	typedef	Point2					Point2;
	typedef	double					Scalar;
	typedef	Pmwx::Halfedge_handle	Object;

	static	Scalar	X(const Point2& p) { return (p.x); }
	static	Scalar	Y(const Point2& p) { return (p.y); }
	static	void	MakePoint(Scalar x, Scalar y, Point2& p) {	p = Point2(x, y); }

	static	void	GetObjectBounds(Object, Point2&, Point2&);
	static	bool	ObjectTouchesPoint(Object, const Point2&);
	static	bool	ObjectTouchesRect(Object, const Point2&, const Point2&);
	static	bool	ObjectFullyInRect(Object, const Point2&, const Point2&);
	static	void	DestroyObject(Object);
	
};	

class	MapVertexBucketTraits {
public:

	typedef	Point2				Point2;
	typedef	double				Scalar;
	typedef	Pmwx::Vertex_handle	Object;

	static	Scalar	X(const Point2& p) { return (p.x); }
	static	Scalar	Y(const Point2& p) { return (p.y); }
	static	void	MakePoint(Scalar x, Scalar y, Point2& p) {	p = Point2(x, y); }

	static	void	GetObjectBounds(Object, Point2&, Point2&);
	static	bool	ObjectTouchesPoint(Object, const Point2&);
	static	bool	ObjectTouchesRect(Object, const Point2&, const Point2&);
	static	bool	ObjectFullyInRect(Object, const Point2&, const Point2&);
	static	void	DestroyObject(Object);
	
};	


typedef	XBuckets<Pmwx::Face_handle, MapFaceBucketTraits>			MapFaceBuckets;
typedef	XBuckets<Pmwx::Halfedge_handle, MapHalfedgeBucketTraits>	MapHalfedgeBuckets;
typedef	XBuckets<Pmwx::Vertex_handle, MapVertexBucketTraits>		MapVertexBuckets;

void	BuildFaceBuckets	(Pmwx& inMap, MapFaceBuckets& 		outBuckets);
void	BuildHalfedgeBuckets(Pmwx& inMap, MapHalfedgeBuckets& 	outBuckets);
void	BuildVertexBuckets	(Pmwx& inMap, MapVertexBuckets& 	outBuckets);

#endif
