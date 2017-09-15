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

#include "WED_OverlayImage.h"
#include "WED_ToolUtils.h"
#include "GISUtils.h"

DEFINE_PERSISTENT(WED_OverlayImage)
TRIVIAL_COPY(WED_OverlayImage, WED_GISPolygon)

WED_OverlayImage::WED_OverlayImage(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	mImageFile(this, "File",  XML_Name("overlay_image","file_path"),""),
	mAlpha(this,"Alpha",      XML_Name("overlay_image","alpha"), 1.0, 3.0, 1.0)
{
}

WED_OverlayImage::~WED_OverlayImage()
{
}

void		WED_OverlayImage::GetImage(string& image_file) const
{
	image_file = mImageFile.value;
}

double		WED_OverlayImage::GetAlpha(void) const
{
	return mAlpha.value;
}

void		WED_OverlayImage::SetImage(const string& image_file)
{
	int x = 1;
	mImageFile = image_file;
}

void	WED_OverlayImage::GetCorners(GISLayer_t l,Point2 corners[4]) const
{
	for (int n = 0; n < 4; ++n)
		GetOuterRing()->GetNthPoint(n)->GetLocation(l,corners[n]);
}

void	WED_OverlayImage::MoveCorner(GISLayer_t l,int corner, const Vector2& delta)
{
	Point2	p;
	IGISPoint * pt = GetOuterRing()->GetNthPoint(corner);
	pt->GetLocation(l,p);
	p += delta;
	pt->SetLocation(l,p);
}

void	WED_OverlayImage::MoveSide(GISLayer_t l,int side, const Vector2& delta)
{
	MoveCorner(l,side, delta);
	MoveCorner(l,(side+1)%4, delta);
}

void WED_OverlayImage::ResizeSide(GISLayer_t l,int side, const Vector2& delta, bool symetric)
{
	Point2	corners[4];
	GetCorners(l,corners);
	Quad_ResizeSide4(corners, side, delta, symetric);
	for (int n = 0; n < 4; ++n)
		GetOuterRing()->GetNthPoint(n)->SetLocation(l,corners[n]);

}

void	WED_OverlayImage::ResizeCorner(GISLayer_t l,int corner, const Vector2& delta, bool symetric)
{
	Point2	corners[4], orig[4];
	GetCorners(l,corners);
	Point2 ctr;
	ctr.x_ = (corners[0].x() + corners[1].x() + corners[2].x() + corners[3].x()) * 0.25;
	ctr.y_ = (corners[0].y() + corners[1].y() + corners[2].y() + corners[3].y()) * 0.25;

	Vector2	oldv(ctr,corners[corner]);
	Vector2 newv(oldv+delta * (symetric ? 1.0 : 0.5));

	double scale = (sqrt(newv.squared_length())) / (sqrt(oldv.squared_length()));
	for (int n = 0; n < 4; ++n)
	{
		orig[n] = corners[n];
		corners[n] = ctr + Vector2(ctr,corners[n]) * scale;
	}

	if (!symetric)
	{
		Vector2	real_move(orig[corner],corners[corner]);
		for (int n = 0; n < 4; ++n)
			corners[n] += (real_move);
	}

	for (int n = 0; n < 4; ++n)
		GetOuterRing()->GetNthPoint(n)->SetLocation(l,corners[n]);

}
