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
#include "XBuckets.h"
#include "XBuckets_inline.h"

#include "CompGeomUtils.h"

struct	StupidObj {
	Point2		minp;
	Point2		maxp;
};

struct	StupidTraits {

	typedef	Point2				Point2;
	typedef	double				Scalar;
	typedef StupidObj *			Object;

	static	Scalar	X(const Point2& p) { return p.x; }
	static	Scalar	Y(const Point2& p) { return p.y; }
	static	void	MakePoint(Scalar x, Scalar y, Point2& p) { p.x = x; p.y = y; }

	static	void	GetObjectBounds(StupidObj * o, Point2& p1, Point2& p2)
	{
		p1 = o->minp;
		p2 = o->maxp;
	}

	static	bool	ObjectTouchesPoint(StupidObj * o, const Point2& p1)
	{
		return  o->minp.x <= p1.x &&
				o->minp.y <= p1.y &&
				o->maxp.x >= p1.x &&
				o->maxp.y >= p1.y;
	}

	static	bool	ObjectTouchesRect(StupidObj * o, const Point2& p1, const Point2& p2)
	{
		return  o->minp.x <= p2.x &&
				o->minp.y <= p2.y &&
				o->maxp.x >= p1.x &&
				o->maxp.y >= p1.y;
	}

	static	bool	ObjectFullyInRect(StupidObj * o, const Point2& p1, const Point2& p2)
	{
		return  o->minp.x >= p1.x &&
				o->minp.y >= p1.y &&
				o->maxp.x <= p2.x &&
				o->maxp.y <= p2.y;
	}

	static	void	DestroyObject(StupidObj * o)
	{
	}
};

template class XBuckets<StupidObj *, StupidTraits>;
