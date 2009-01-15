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
	virtual	void			Rescale			(const Bbox2& old_bounds, const Bbox2& new_bounds);
	virtual	void			Rotate			(const Point2& center, double angle);
	// IGISPoint_Bezier

	virtual	bool	GetControlHandleLo (      Point2& p) const;
	virtual	bool	GetControlHandleHi (      Point2& p) const;
	virtual	bool	IsSplit			   (void		   ) const;

	virtual	void	SetControlHandleLo (const Point2& p)      ;
	virtual	void	SetControlHandleHi (const Point2& p)      ;
	virtual	void	SetUVLo			   (const Point2& p)	  ;
	virtual	void	SetUVHi			   (const Point2& p)	  ;
	virtual	void	GetUVLo			   (      Point2& p) const;
	virtual	void	GetUVHi			   (      Point2& p) const;
	virtual	void	DeleteHandleLo	   (void		   )	  ;
	virtual	void	DeleteHandleHi	   (void		   )	  ;
	virtual	void	SetSplit		   (bool is_split  )	  ;
	virtual	void	Reverse			   (void		   )	  ;

private:

		WED_PropBoolText		is_split;

		WED_PropDoubleText		ctrl_lat_lo;			// NOTE: THESE ARE STORED AS DELTAS!!!
		WED_PropDoubleText		ctrl_lon_lo;
		WED_PropDoubleText		ctrl_lat_hi;
		WED_PropDoubleText		ctrl_lon_hi;

};

#endif /* WED_GISPOINT_BEZIER_H */