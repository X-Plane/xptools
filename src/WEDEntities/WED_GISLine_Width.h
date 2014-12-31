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

#ifndef WED_GISLINE_WIDTH_H
#define WED_GISLINE_WIDTH_H

#include "WED_GISLine.h"
#include "IGIS.h"

class	WED_GISLine_Width : public WED_GISLine, public virtual IGISLine_Width{

DECLARE_INTERMEDIATE(WED_GISLine_Width)

public:

	// IPropertyObject
	virtual	int			FindProperty(const char * in_prop) const;
	virtual int			CountProperties(void) const;
	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info) const;
	virtual	void		GetNthPropertyDict(int n, PropertyDict_t& dict) const;
	virtual	void		GetNthPropertyDictItem(int n, int e, string& item) const;

	virtual void		GetNthProperty(int n, PropertyVal_t& val) const;
	virtual void		SetNthProperty(int n, const PropertyVal_t& val);

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;

	virtual	void			GetBounds		(GISLayer_t l,	   Bbox2&  bounds) const;
	virtual	bool			IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(GISLayer_t l,const Bbox2&  bounds) const;
	virtual bool			PtOnFrame		(GISLayer_t l,const Point2& p, double dist) const;
	virtual bool			PtWithin		(GISLayer_t l,const Point2& p	 ) const;
	virtual	void			Rescale(GISLayer_t l,
								const Bbox2& old_bounds,			// Defines a linear remappign of coordinates we can apply.
								const Bbox2& new_bounds);

	// IGISLine_Width
	virtual	double	GetWidth (void		 ) const;
	virtual	void	SetWidth (double width)      ;

	virtual	void	GetCorners(GISLayer_t l,Point2 corners[4]) const;

	virtual	void	MoveCorner(GISLayer_t l,int corner, const Vector2& delta);
	virtual	void	MoveSide(GISLayer_t l,int side, const Vector2& delta);

	virtual	void	ResizeSide(GISLayer_t l,int side, const Vector2& delta, bool symetric);
	virtual	void	ResizeCorner(GISLayer_t l,int side, const Vector2& delta, bool symetric);

	double		GetHeading(void) const;
	double		GetLength(void) const;
	Point2		GetCenter(void) const;


private:

	WED_PropDoubleTextMeters		width;

};

#endif /* WED_GISLINE_WIDTH_H */
