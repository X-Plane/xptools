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

#ifndef WED_GISPOINT_BEZIER_H
#define WED_GISPOINT_BEZIER_H

#include "WED_GISPoint.h"

class	WED_GISPoint_Bezier : public WED_GISPoint, public virtual IGISPoint_Bezier {

DECLARE_INTERMEDIATE(WED_GISPoint_Bezier)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	void			Rescale			(GISLayer_t l, const Bbox2& old_bounds, const Bbox2& new_bounds);
	virtual	void			Rotate			(GISLayer_t l, const Point2& center, double angle);
	// IGISPoint_Bezier

	virtual	bool	GetControlHandleLo (GISLayer_t l,       Point2& p) const;
	virtual	bool	GetControlHandleHi (GISLayer_t l,       Point2& p) const;
	virtual	bool	IsSplit			   (void		   ) const;

	virtual	void	SetControlHandleLo (GISLayer_t l, const Point2& p)      ;
	virtual	void	SetControlHandleHi (GISLayer_t l, const Point2& p)      ;
	virtual	void	DeleteHandleLo	   (void		   )	  ;
	virtual	void	DeleteHandleHi	   (void		   )	  ;
	virtual	void	SetSplit		   (bool is_split  )	  ;
			void	Reverse_		   (void		   )	  ;

private:

		WED_PropBoolText		is_split;

		WED_PropDoubleText		ctrl_lat_lo;			// NOTE: THESE ARE STORED AS DELTAS!!!
		WED_PropDoubleText		ctrl_lon_lo;
		WED_PropDoubleText		ctrl_lat_hi;
		WED_PropDoubleText		ctrl_lon_hi;

};

#endif /* WED_GISPOINT_BEZIER_H */