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

#ifndef WED_GISPOLYGON_H
#define WED_GISPOLYGON_H

#include "WED_Entity.h"
#include "IGIS.h"

/*

	WED_GISPolygon has no cache because:

	- It has exactly one outer ring.
	- Its bounding box is exactly the same as the outer ring.

	Thus it can simply access the outer ring's bounding box in constant time.  If thist proves to be a performance problem we can copy
	the cache out.

*/

class	WED_GISPolygon : public WED_Entity, public virtual IGISPolygon {

DECLARE_INTERMEDIATE(WED_GISPolygon)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	const char *	GetGISSubtype	(void				 ) const;
	virtual	bool			HasLayer		(GISLayer_t l		 ) const;
	virtual	void			GetBounds		(GISLayer_t l,	    Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(GISLayer_t l,const Bbox2&  bounds) const;
	virtual bool			PtWithin		(GISLayer_t l,const Point2& p	 ) const;
	virtual bool			PtOnFrame		(GISLayer_t l,const Point2& p, double d) const;
	virtual bool			Cull			(const Bbox2& bounds) const;
	virtual	void			Rescale			(GISLayer_t l,const Bbox2& old_bounds,const Bbox2& new_bounds);
	virtual	void			Rotate			(GISLayer_t l,const Point2& center, double angle);
	// IGISPolygon
	virtual			IGISPointSequence *		GetOuterRing(void )	const;
	virtual			int						GetNumHoles (void ) const;
	virtual			IGISPointSequence *		GetNthHole  (int n)	const;

	virtual			void					DeleteHole  (int n)					;
	virtual			void					AddHole		(IGISPointSequence * r) ;
	virtual			void					Reverse(GISLayer_t l);


};

#endif /* WED_GISPOLYGON_H */
