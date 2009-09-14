/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_GISBoundingBox.h"

TRIVIAL_COPY(WED_GISBoundingBox, WED_Entity)

WED_GISBoundingBox::WED_GISBoundingBox(WED_Archive * a, int id) : WED_Entity(a, id)
{
}

WED_GISBoundingBox::~WED_GISBoundingBox()
{
}

GISClass_t		WED_GISBoundingBox::GetGISClass		(void				 ) const
{
	return gis_BoundingBox;
}

const char *	WED_GISBoundingBox::GetGISSubtype	(void				 ) const
{
	return GetClass();
}

bool			WED_GISBoundingBox::HasLayer		(GISLayer_t l) const
{
	return false;
}


void			WED_GISBoundingBox::GetBounds		(GISLayer_t l,   Bbox2&  bounds) const
{
	Bbox2	b2;
	GetMin()->GetBounds(l,bounds);
	GetMax()->GetBounds(l,b2);
	bounds += b2;
}

bool			WED_GISBoundingBox::IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	return bounds.overlap(me);
}

bool			WED_GISBoundingBox::WithinBox		(GISLayer_t l,const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	return bounds.contains(me);
}

bool			WED_GISBoundingBox::PtWithin		(GISLayer_t l,const Point2& p	 ) const
{
	Bbox2	me;
	GetBounds(l,me);
	return me.contains(p);
}

bool			WED_GISBoundingBox::PtOnFrame		(GISLayer_t l,const Point2& p, double d) const
{
	Bbox2	me;
	GetBounds(l,me);

	if (p.x() < me.xmin() ||
		p.x() > me.xmax() ||
		p.y() < me.ymin() ||
		p.y() > me.ymax())
		return false;

	if (p.x() > me.xmin() &&
		p.x() < me.xmax() &&
		p.y() > me.ymin() &&
		p.y() < me.ymax())
		return false;

	return true;
}

void			WED_GISBoundingBox::Rescale(GISLayer_t l,const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	GetMin()->Rescale(l,old_bounds, new_bounds);
	GetMax()->Rescale(l,old_bounds, new_bounds);
}

void			WED_GISBoundingBox::Rotate			(GISLayer_t l,const Point2& center, double angle)
{
	GetMin()->Rotate(l,center, angle);
	GetMax()->Rotate(l,center, angle);
}

IGISPoint *				WED_GISBoundingBox::GetMin(void) const
{
	IGISPoint * p = SAFE_CAST(IGISPoint, GetNthChild(0));
	DebugAssert(p != NULL);
	return p;
}

IGISPoint *				WED_GISBoundingBox::GetMax(void) const
{
	IGISPoint * p = SAFE_CAST(IGISPoint, GetNthChild(1));
	DebugAssert(p != NULL);
	return p;
}

void	WED_GISBoundingBox::GetCorners(GISLayer_t l,Point2 corners[4]) const
{
	GetMin()->GetLocation(l,corners[0]);
	GetMax()->GetLocation(l,corners[2]);
	corners[1] = Point2(corners[2].x(),corners[0].y());
	corners[3] = Point2(corners[0].x(),corners[2].y());
}

void	WED_GISBoundingBox::MoveCorner(GISLayer_t l,int corner, const Vector2& delta)
{
	Point2	p1,p2;
	GetMin()->GetLocation(l,p1);
	GetMax()->GetLocation(l,p2);
	switch(corner) {
	case 0:	p1.x_ += delta.dx;	p1.y_ += delta.dy;	break;
	case 1: p2.x_ += delta.dx;	p1.y_ += delta.dy;	break;
	case 2:	p2.x_ += delta.dx;	p2.y_ += delta.dy;	break;
	case 3: p1.x_ += delta.dx;	p2.y_ += delta.dy;	break;
	}
	GetMin()->SetLocation(l,p1);
	GetMax()->SetLocation(l,p2);
}

void	WED_GISBoundingBox::MoveSide(GISLayer_t l,int side, const Vector2& delta)
{
	Point2	p1,p2;
	GetMin()->GetLocation(l,p1);
	GetMax()->GetLocation(l,p2);
	switch(side) {
	case 0:	p1.y_ += delta.dy;	break;
	case 1: p2.x_ += delta.dx;	break;
	case 2:	p2.y_ += delta.dy;	break;
	case 3: p1.x_ += delta.dx;	break;
	}
	GetMin()->SetLocation(l,p1);
	GetMax()->SetLocation(l,p2);

}

void	WED_GISBoundingBox::ResizeSide(GISLayer_t l,int side, const Vector2& delta, bool symetric)
{
	MoveSide(l,side, delta);
	if(symetric)
		MoveSide(l,(side+2)%4,-delta);
}
void	WED_GISBoundingBox::ResizeCorner(GISLayer_t l,int corner, const Vector2& delta, bool symetric)
{
	MoveCorner(l,corner, delta);
	if(symetric)
		MoveCorner(l,(corner+2)%4,-delta);
}
