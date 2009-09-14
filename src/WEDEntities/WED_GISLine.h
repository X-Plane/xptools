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

#ifndef WED_GISLINE_H
#define WED_GISLINE_H

#include "WED_Entity.h"
#include "IGIS.h"

/*

	NOTE:  WED_GisLine has a constant number of points, so it does not cache its bounding box.  If we find that we have a performance problem
	we can add this, but since the access time to find the real bounding box is a constant-multiple of the cache time, it seems unlikely
	that caching is needed.  (Unlike a taxiway with 800 points.)

*/

class	WED_GISLine : public WED_Entity, public virtual IGISLine {

DECLARE_INTERMEDIATE(WED_GISLine)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	const char *	GetGISSubtype	(void				 ) const;
	virtual	bool			HasLayer		(GISLayer_t l		 ) const;
	virtual	void			GetBounds		(GISLayer_t l,	    Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(GISLayer_t l,const Bbox2&  bounds) const;
	virtual bool			PtWithin		(GISLayer_t l,const Point2& p	 ) const;
	virtual bool			PtOnFrame		(GISLayer_t l,const Point2& p, double dist) const;
	virtual	void			Rescale			(GISLayer_t l,const Bbox2& old_bounds,const Bbox2& new_bounds);
	virtual	void			Rotate			(GISLayer_t l,const Point2& center, double angle);
	// IGISPointSequence
	virtual	int					GetNumPoints(void ) const;
//	virtual	void				DeletePoint (int n)		 ;
//	virtual		  IGISPoint *	SplitSide   (int n)		 ;		// Split the side from pt N to pt N + 1 in half. Return the new pt.
	virtual		  IGISPoint *	GetNthPoint (int n) const;
	virtual	int					GetNumSides(void) const;
	virtual	bool				GetSide  (GISLayer_t l,int n, Segment2& s, Bezier2& b) const;	// true for bezier

	virtual	bool				IsClosed(void) const;
	//IGISLine
	virtual		  IGISPoint *		GetSource(void)	const;
	virtual		  IGISPoint *		GetTarget(void)	const;
	virtual		void				Reverse(GISLayer_t l);

};

#endif /* WED_GISLINE_H */
