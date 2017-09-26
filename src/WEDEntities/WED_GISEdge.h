/* 
 * Copyright (c) 2009, Laminar Research.
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

#ifndef WED_GISEdge_H
#define WED_GISEdge_H

/*

	WED_GISEdge - an EDGE is topologically different from a chain (see WED_GISChain).  WED_GISChain is a 
	(possibly closed) sequence of points - each point is a child of the chain.
	
	BY comparison, WED_GISEdge represents a topological link in a network.  That is - its two terminal 
	end-points are _not_ it's children, but rather external classes (managed as "sources").  This makes
	it possible for two WED_GISEdges to share common end-nodes, forming a topological network.
	
	Our direct children form "shape" points - that is, points that change the path of the edge without
	having topological meaning.

 */

#include "WED_Entity.h"
#include "IGIS.h"

class WED_GISEdge : public WED_Entity, public virtual IGISEdge {

DECLARE_INTERMEDIATE(WED_GISEdge)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	const char *	GetGISSubtype	(void				 ) const;
	virtual	bool			HasLayer		(GISLayer_t l		 ) const;
	virtual	void			GetBounds		(GISLayer_t l, 	    Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(GISLayer_t l,const Bbox2&  bounds) const;
	virtual bool			PtWithin		(GISLayer_t l,const Point2& p	 ) const;
	virtual bool			PtOnFrame		(GISLayer_t l,const Point2& p, double d) const;
	virtual bool			Cull			(const Bbox2& bounds) const;
	virtual	void			Rescale			(GISLayer_t l,const Bbox2& old_bounds,const Bbox2& new_bounds);
	virtual	void			Rotate			(GISLayer_t l,const Point2& center, double angle);

	// IGISPointSequence
	virtual	int					GetNumPoints(void ) const;
	virtual		  IGISPoint *	GetNthPoint (int n) const;

	virtual	int					GetNumSides(void) const;
	virtual	bool				GetSide  (GISLayer_t l,int n, Segment2& s, Bezier2& b) const;	// true for bezier

	virtual	bool				IsClosed(void) const;
	virtual	void				Reverse(GISLayer_t l);
	virtual	void				Shuffle(GISLayer_t l);
	virtual	IGISPoint *			SplitSide   (const Point2& p, double dist);
	
	virtual	void				SetSide(GISLayer_t layer, const Segment2& b);
	virtual	void				SetSideBezier(GISLayer_t layer, const Bezier2& b);
	
	virtual	void				Validate(void);

	// IPropertyObject
	virtual void				GetNthPropertyInfo(int n, PropertyInfo_t& info) const;
	

protected:

	virtual	bool				CanBeCurved() const=0;

private:

	virtual	WED_Thing *			CreateSplitNode();

		WED_PropDoubleText		ctrl_lat_lo;			// NOTE: THESE ARE STORED AS DELTAS!!!
		WED_PropDoubleText		ctrl_lon_lo;
		WED_PropDoubleText		ctrl_lat_hi;
		WED_PropDoubleText		ctrl_lon_hi;

/*
		WED_PropDoubleText			mScL;
		WED_PropDoubleText			mTcL;
		WED_PropDoubleText			mScH;
		WED_PropDoubleText			mTcH;
*/		

};

#endif /* WED_GISEdge_H */
