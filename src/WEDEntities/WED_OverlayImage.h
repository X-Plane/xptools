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

#ifndef WED_OverlayImage_H
#define WED_OverlayImage_H

#include "WED_GISPolygon.h"
#include "IGIS.h"

class WED_OverlayImage : public WED_GISPolygon, public virtual IGISQuad {

DECLARE_PERSISTENT(WED_OverlayImage)

public:

	void		GetImage(string& image_file) const;
	void		SetImage(const string& image_file);
	double		GetAlpha(void) const;

	virtual	void	GetCorners(GISLayer_t l,Point2 corners[4]) const;

	virtual	void	MoveCorner(GISLayer_t l,int corner, const Vector2& delta);
	virtual	void	MoveSide(GISLayer_t l,int side, const Vector2& delta);

	virtual	void	ResizeSide(GISLayer_t l,int side, const Vector2& delta, bool symetric);
	virtual	void	ResizeCorner(GISLayer_t l,int side, const Vector2& delta, bool symetric);

	virtual const char *	HumanReadableType(void) const { return "Reference Image"; }

protected:

	virtual	bool		IsInteriorFilled(void) const { return true; }

private:

	WED_PropFileText			mImageFile;
	WED_PropDoubleText			mAlpha;

};

#endif /* WED_OverlayImage_H */
