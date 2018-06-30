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

#ifndef WED_GISPOINT_H
#define WED_GISPOINT_H

/*
	WED_GISPoint - THEORY OF OPERATION

	WED_GISPoint (and the other WED_GISxxxx classes) are intermediate implementations that provide
	the "spatial brains" for various WED classes.

	WED_GISPoint implements the IGISPoint, forming a single point specified in latitude and longitude.

	Specific WED entities are formed from these intermediates, picking up their "GIS brains" for free
	and adding some special properties or behavior.

*/

#include "WED_Entity.h"
#include "IGIS.h"

class	WED_GISPoint : public WED_Entity, public virtual IGISPoint {

DECLARE_INTERMEDIATE(WED_GISPoint)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	const char *	GetGISSubtype	(void				 ) const;
	virtual	bool			HasLayer		(GISLayer_t l		) const;
	virtual	void			GetBounds		(GISLayer_t l,	    Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(GISLayer_t l,const Bbox2&  bounds) const;
	virtual bool			PtWithin		(GISLayer_t l,const Point2& p	 ) const;
	virtual bool			PtOnFrame		(GISLayer_t l,const Point2& p, double dist) const;
	virtual bool			Cull			(const Bbox2& bounds) const;
	virtual	void			Rescale			(GISLayer_t l,const Bbox2& old_bounds, const Bbox2& new_bounds);
	virtual	void			Rotate			(GISLayer_t l,const Point2& center, double angle);
	// IGISPoint
	virtual	void			GetLocation(GISLayer_t l,      Point2& p) const;
	virtual	void			SetLocation(GISLayer_t l,const Point2& p)      ;

	virtual	void			PropEditCallback(int before);
	//IGISPoint
	virtual	bool			IsLinked(void	) const;
	virtual	bool			IsViewer(void	) const;
	virtual	IGISPoint *		GetSrcPoint(void) const;
	
	// linked node
	WED_GISPoint *			GetSourcePoint(void) const;
	void					GetLocationExpl(GISLayer_t l, Point2& p)const;
	void					SetLocationExpl(GISLayer_t l,const Point2& p);
	
private:

		WED_PropDoubleText		latitude;
		WED_PropDoubleText		longitude;

};

#endif /* WED_GISPOINT_H */
