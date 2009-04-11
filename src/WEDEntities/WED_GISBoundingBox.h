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

#ifndef WED_GISBoundingBox_H
#define WED_GISBoundingBox_H

#include "WED_Entity.h"
#include "IGIS.h"

class WED_GISBoundingBox : public WED_Entity, public virtual IGISBoundingBox, public virtual IGISQuad {

DECLARE_INTERMEDIATE(WED_GISBoundingBox)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	const char *	GetGISSubtype	(void				 ) const;
	virtual	bool			HasUV			(void				 ) const;
	virtual	void			GetBounds		(	   Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(const Bbox2&  bounds) const;
	virtual bool			PtWithin		(const Point2& p	 ) const;
	virtual bool			PtOnFrame		(const Point2& p, double d) const;
	virtual	void			Rescale(const Bbox2& old_bounds,const Bbox2& new_bounds);
	virtual	void			Rotate			(const Point2& center, double angle);

	virtual			IGISPoint *				GetMin(void) const;
	virtual			IGISPoint *				GetMax(void) const;

	virtual	void	GetCorners(Point2 corners[4]) const;

	virtual	void	MoveCorner(int corner, const Vector2& delta);
	virtual	void	MoveSide(int side, const Vector2& delta);

	virtual	void	ResizeSide(int side, const Vector2& delta, bool symetric);
	virtual	void	ResizeCorner(int side, const Vector2& delta, bool symetric);


};

#endif /* WED_GISBoundingBox_H */
