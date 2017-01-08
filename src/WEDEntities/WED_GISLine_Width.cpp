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

#include "WED_GISLine_Width.h"
#include "GISUtils.h"
#include "XESConstants.h"
#include "WED_ToolUtils.h"
#include "GISUtils.h"

TRIVIAL_COPY(WED_GISLine_Width, WED_GISLine)

WED_GISLine_Width::WED_GISLine_Width(WED_Archive * parent, int id) :
	WED_GISLine(parent, id),
	width(this,"Width", XML_Name("line","width"),50.0,6,2)
{
}

WED_GISLine_Width::~WED_GISLine_Width()
{
}

#pragma mark -



enum {
	rwy_prop_length,
	rwy_prop_heading,
	rwy_prop_lat1,
	rwy_prop_lon1,
	rwy_prop_latc,
	rwy_prop_lonc,
	rwy_prop_lat2,
	rwy_prop_lon2,
	rwy_prop_count
};

const char * kRwyPropNames[rwy_prop_count] = {
	"Length",
	"Heading",
	"Latitude 1",
	"Longitude 1",
	"Latitude Ctr",
	"Longitude Ctr",
	"Latitude 2",
	"Longitude 2"
};

int			WED_GISLine_Width::FindProperty(const char * in_prop) const
{
	for (int n = 0; n < rwy_prop_count; ++n)
	{
		if (strcmp(in_prop, kRwyPropNames[n])==0) return n;
	}
	int found = WED_GISLine::FindProperty(in_prop);
	if (found == -1) return found;
	return found + rwy_prop_count;
}

int			WED_GISLine_Width::CountProperties(void) const
{
	return WED_GISLine::CountProperties() + rwy_prop_count;
}

void		WED_GISLine_Width::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	if (n < rwy_prop_count)
	{
		info.can_delete = false;
		info.can_edit = true;
		info.prop_name = kRwyPropNames[n];
		info.prop_kind = prop_Double;
		info.synthetic = true;
		info.round_down = false;
		info.units = "";
	}

	switch(n) {
	case rwy_prop_length:	info.digits = 5; info.decimals = 1; info.units = (gIsFeet ? "ft" : "m"); break;
	case rwy_prop_heading:	info.digits = 5; info.decimals = 2; break;
	case rwy_prop_lat1:		info.digits = 10; info.decimals = 6; break;
	case rwy_prop_lon1:		info.digits = 11; info.decimals = 6; break;
	case rwy_prop_latc:		info.digits = 10; info.decimals = 6; break;
	case rwy_prop_lonc:		info.digits = 11; info.decimals = 6; break;
	case rwy_prop_lat2:		info.digits = 10; info.decimals = 6; break;
	case rwy_prop_lon2:		info.digits = 11; info.decimals = 6; break;
	default: WED_GISLine::GetNthPropertyInfo(n-rwy_prop_count, info);
	}
}

void		WED_GISLine_Width::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	WED_GISLine::GetNthPropertyDict(n-rwy_prop_count, dict);
}

void		WED_GISLine_Width::GetNthPropertyDictItem(int n, int e, string& item) const
{
	WED_GISLine::GetNthPropertyDictItem(n-rwy_prop_count, e, item);
}

void		WED_GISLine_Width::GetNthProperty(int n, PropertyVal_t& val) const
{
	Point2	ends[2], ctr;
	double	l,h;
	if (n < rwy_prop_count)
	{
		GetSource()->GetLocation(gis_Geo, ends[0]);
		GetTarget()->GetLocation(gis_Geo, ends[1]);
		Quad_2to1(ends, ctr, h, l);
		if (gIsFeet) l *= MTR_TO_FT;
	}

	val.prop_kind = prop_Double;
	switch(n) {
	case rwy_prop_length:	val.double_val = l;			break;
	case rwy_prop_heading:	val.double_val = h;			break;
	case rwy_prop_lat1:		val.double_val = ends[0].y();	break;
	case rwy_prop_lon1:		val.double_val = ends[0].x();	break;
	case rwy_prop_latc:		val.double_val = ctr.y();		break;
	case rwy_prop_lonc:		val.double_val = ctr.x();		break;
	case rwy_prop_lat2:		val.double_val = ends[1].y();	break;
	case rwy_prop_lon2:		val.double_val = ends[1].x();	break;
	default: WED_GISLine::GetNthProperty(n-rwy_prop_count, val);
	}
}

void		WED_GISLine_Width::SetNthProperty(int n, const PropertyVal_t& val)
{
	Point2	ends[2], ctr;
	double	l,h;
	if (n < rwy_prop_count)
	{
		GetSource()->GetLocation(gis_Geo,ends[0]);
		GetTarget()->GetLocation(gis_Geo,ends[1]);
		Quad_2to1(ends, ctr, h, l);
	}

	switch(n) {
	case rwy_prop_length:
		l = val.double_val;
		if (gIsFeet) l *= FT_TO_MTR;
		Quad_1to2(ctr, h, l, ends);
		GetSource()->SetLocation(gis_Geo,ends[0]);
		GetTarget()->SetLocation(gis_Geo,ends[1]);
		break;
	case rwy_prop_heading:
		h = val.double_val;
		Quad_1to2(ctr, h, l, ends);
		GetSource()->SetLocation(gis_Geo,ends[0]);
		GetTarget()->SetLocation(gis_Geo,ends[1]);
		break;

	case rwy_prop_lat1:
		ends[0].y_ = val.double_val;
		GetSource()->SetLocation(gis_Geo,ends[0]);
		break;
	case rwy_prop_lon1:
		ends[0].x_ = val.double_val;
		GetSource()->SetLocation(gis_Geo,ends[0]);
		break;

	case rwy_prop_latc:
		ctr.y_ = val.double_val;
		Quad_1to2(ctr, h, l, ends);
		GetSource()->SetLocation(gis_Geo,ends[0]);
		GetTarget()->SetLocation(gis_Geo,ends[1]);
		break;
	case rwy_prop_lonc:
		ctr.x_ = val.double_val;
		Quad_1to2(ctr, h, l, ends);
		GetSource()->SetLocation(gis_Geo,ends[0]);
		GetTarget()->SetLocation(gis_Geo,ends[1]);
		break;

	case rwy_prop_lat2:
		ends[1].y_ = val.double_val;
		GetTarget()->SetLocation(gis_Geo,ends[1]);
		break;
	case rwy_prop_lon2:
		ends[1].x_ = val.double_val;
		GetTarget()->SetLocation(gis_Geo,ends[1]);
		break;

	default:
		WED_GISLine::SetNthProperty(n-rwy_prop_count, val);
	}

}

#pragma mark -



GISClass_t		WED_GISLine_Width::GetGISClass		(void				 ) const
{
	return gis_Line_Width;
}


void			WED_GISLine_Width::GetBounds		(GISLayer_t l,  Bbox2&  bounds) const
{
	CacheBuild(cache_Spatial);
	Point2 corners[4];
	GetCorners(l,corners);
	bounds = Bbox2(corners[0],corners[1]);
	bounds += corners[2];
	bounds += corners[3];
}

bool			WED_GISLine_Width::IntersectsBox	(GISLayer_t l,  const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	return me.overlap(bounds);
}

bool			WED_GISLine_Width::WithinBox		(GISLayer_t l,  const Bbox2&  bounds) const
{
	Point2	corners[4];
	GetCorners(l,corners);
	return  bounds.contains(corners[0]) &&
			bounds.contains(corners[1]) &&
			bounds.contains(corners[2]) &&
			bounds.contains(corners[3]);
}

bool			WED_GISLine_Width::PtOnFrame		(GISLayer_t l,  const Point2& p, double dist) const
{
	Point2	corners[4];
	GetCorners(l,corners);
	if (Segment2(corners[0],corners[1]).is_near(p,dist)) return true;
	if (Segment2(corners[1],corners[2]).is_near(p,dist)) return true;
	if (Segment2(corners[2],corners[3]).is_near(p,dist)) return true;
	if (Segment2(corners[3],corners[0]).is_near(p,dist)) return true;
	return false;
}

bool			WED_GISLine_Width::PtWithin		(GISLayer_t l,  const Point2& p	 ) const
{
	Point2 corners[4];
	GetCorners(l,corners);
	return inside_polygon_pt(corners,corners+4,p);
}

void			WED_GISLine_Width::Rescale(GISLayer_t l,  
								const Bbox2& old_bounds,			// Defines a linear remappign of coordinates we can apply.
								const Bbox2& new_bounds)
{
	Point2 corners[4];
	GetCorners(l,corners);
	for(int n = 0; n < 4; ++n)
	{
		corners[n].x_ = old_bounds.rescale_to_x(new_bounds,corners[n].x());
		corners[n].y_ = old_bounds.rescale_to_y(new_bounds,corners[n].y());
	}

	Point2	 ends[2];
	double  w;
	Quad_4to2(corners, ends, w);
	GetSource()->SetLocation(l,ends[0]);
	GetTarget()->SetLocation(l,ends[1]);
	SetWidth(w);

}






double	WED_GISLine_Width::GetWidth (void		 ) const
{
	return width.value;
}

void	WED_GISLine_Width::SetWidth (double w)
{
	if (w < 1.0) w = 1.0;
	if (w != width.value)
	{
		StateChanged();
		width.value = w;
	}
}

void	WED_GISLine_Width::GetCorners(GISLayer_t l,  Point2 corners[4]) const
{
	Point2		ends[2];
	GetSource()->GetLocation(l,ends[0]);
	GetTarget()->GetLocation(l,ends[1]);

	Quad_2to4(ends, GetWidth(), corners);
}

void	WED_GISLine_Width::MoveCorner(GISLayer_t layer,  int corner, const Vector2& delta)
{
	Point2	corners[4];
	Point2	ends[2];

	GetCorners(layer,corners);
	corners[corner] += delta;
	int swapped = corner == 1 || corner == 3;
	if (swapped)
	{
		ends[0] = corners[3];
		ends[1] = corners[1];
	} else {
		ends[0] = corners[0];
		ends[1] = corners[2];
	}
		Point2	ctr;
		double	h, l;
		double	w = GetWidth();
	Quad_diagto1(ends, w, ctr, h, l, swapped);
	DebugAssert(h == h);						// These are NaN checks
	DebugAssert(l == l);
	DebugAssert(ctr.x_ == ctr.x_);
	DebugAssert(ctr.y_ == ctr.y_);
	Quad_1to2(ctr, h, l, ends);
	DebugAssert(ends[0].x_ == ends[0].x_ && ends[0].y_ == ends[0].y_ && ends[1].x_ == ends[1].x_ && ends[1].y_ == ends[1].y_);

	GetSource()->SetLocation(layer,ends[0]);
	GetTarget()->SetLocation(layer,ends[1]);
}


void	WED_GISLine_Width::MoveSide(GISLayer_t l,  int side, const Vector2& delta)
{
	Point2	ends[2];
	GetSource()->GetLocation(l,ends[0]);
	GetTarget()->GetLocation(l,ends[1]);
	double w = GetWidth();

	Quad_MoveSide2(ends, w, side, delta);

	SetWidth(w);
	GetSource()->SetLocation(l,ends[0]);
	GetTarget()->SetLocation(l,ends[1]);
}

void	WED_GISLine_Width::ResizeSide(GISLayer_t l,  int side, const Vector2& delta, bool symetric)
{
	Point2	ends[2], corners[4];
	double	width;

	GetCorners(l,corners);
	Quad_ResizeSide4(corners, side, delta, symetric);
	Quad_4to2(corners, ends, width);

	GetSource()->SetLocation(l,ends[0]);
	GetTarget()->SetLocation(l,ends[1]);
	SetWidth(width);
}

void	WED_GISLine_Width::ResizeCorner(GISLayer_t layer,  int corner, const Vector2& delta, bool symetric)
{
	Point2	ctr, ends[2];
	double	w = GetWidth(), h, l;
	GetSource()->GetLocation(layer,ends[0]);
	GetTarget()->GetLocation(layer,ends[1]);
	Quad_2to1(ends, ctr, h, l);
	Quad_ResizeCorner1(ctr, h, l, w, corner, delta, symetric);
	Quad_1to2(ctr, h, l, ends);
	GetSource()->SetLocation(layer,ends[0]);
	GetTarget()->SetLocation(layer,ends[1]);
	SetWidth(w);

}


double		WED_GISLine_Width::GetHeading(void) const
{
	Point2	ends[2];
	Point2	ctr;
	double h,l;
	GetSource()->GetLocation(gis_Geo,ends[0]);
	GetTarget()->GetLocation(gis_Geo,ends[1]);
	Quad_2to1(ends, ctr, h, l);
	return h;
}

Point2		WED_GISLine_Width::GetCenter(void) const
{
	Point2	ends[2];
	Point2	ctr;
	double h,l;
	GetSource()->GetLocation(gis_Geo,ends[0]);
	GetTarget()->GetLocation(gis_Geo,ends[1]);
	Quad_2to1(ends, ctr, h, l);
	return ctr;
}


double		WED_GISLine_Width::GetLength(void) const
{
	Point2	ends[2];
	Point2	ctr;
	double h,l;
	GetSource()->GetLocation(gis_Geo,ends[0]);
	GetTarget()->GetLocation(gis_Geo,ends[1]);
	Quad_2to1(ends, ctr, h, l);
	return l;
}


int			WED_GISLine_Width::PropertyItemNumber(const WED_PropertyItem * item) const
{
	int r = WED_GISLine::PropertyItemNumber(item);
	return r >= 0 ? r + rwy_prop_count : r;
}

