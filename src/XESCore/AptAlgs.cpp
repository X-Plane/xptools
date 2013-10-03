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

#include "AptAlgs.h"
#include "AptDefs.h"
#include "AptIO.h"
#include "XESConstants.h"
#include "AssertUtils.h"
#include "GISUtils.h"
#include "MapPolygon.h"
#include "MapHelpers.h"

/************************************************************************************************************************************************************************
 * APT FORMAT CONVERSION FROM 810 TO 850
 ************************************************************************************************************************************************************************/

inline int hash_ll(int lon, int lat) {	return (lon+180) + 360 * (lat+90); }


static void EndsToCenter(const Segment2& ends, Point2& center, double& len, double& heading)
{
	center = ends.midpoint();
	Vector2	dir(ends.p1, ends.p2);

	double	aspect = cos(center.y() * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;

	dir.dx *= DEG_TO_MTR_LON;
	dir.dy *= DEG_TO_MTR_LAT;

	len = sqrt(dir.squared_length());
	heading = RAD_TO_DEG * atan2(dir.dx, dir.dy);
	if (heading < 0.0) heading += 360.0;
}

static void CenterToEnds(POINT2 location, double heading, double len, SEGMENT2& ends)
{
	// NOTE: if we were using some kind of cartesian projection scheme wedd have to add
	// (lon_ref - lon rwy) * sin(lat runway) to this degrees, rotating runways to the right
	// of the reference point slightly CCW in the northern hemisphere to reflect the
	// map getting smaller at the pole.  But...we use a bogus projection, so - F it.
	double	heading_corrected = (heading) * DEG_TO_RAD;
	len *= 0.5;
	Vector2	delta;
	delta.dx = len * sin(heading_corrected);
	delta.dy = len * cos(heading_corrected);

	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / cos(location.y() * DEG_TO_RAD);
	delta.dx *= MTR_TO_DEG_LON;
	delta.dy *= MTR_TO_DEG_LAT;
	ends.p1 = location - delta;
	ends.p2 = location + delta;
}

static void CenterToCorners(Point2 location, double heading, double len, double width, Point2 corners[4])
{
	// NOTE: if we were using some kind of cartesian projection scheme wedd have to add
	// (lon_ref - lon rwy) * sin(lat runway) to this degrees, rotating runways to the right
	// of the reference point slightly CCW in the northern hemisphere to reflect the
	// map getting smaller at the pole.  But...we use a bogus projection, so - F it.
	double	heading_corrected = (heading) * DEG_TO_RAD;
	len *= 0.5;
	width *= 0.5;
	Vector2	delta;
	delta.dx = len * sin(heading_corrected);
	delta.dy = len * cos(heading_corrected);

	Vector2	lateral;
	lateral.dx = width *  cos(heading_corrected);
	lateral.dy = width * -sin(heading_corrected);

	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / cos(location.y() * DEG_TO_RAD);
	delta.dx *= MTR_TO_DEG_LON;
	delta.dy *= MTR_TO_DEG_LAT;
	lateral.dx *= MTR_TO_DEG_LON;
	lateral.dy *= MTR_TO_DEG_LAT;

	corners[0] = location - lateral - delta;
	corners[1] = location - lateral + delta;
	corners[2] = location + lateral + delta;
	corners[3] = location + lateral - delta;
}


/************************************************************************************************************************************************************************
 * SPATIAL INDEXING
 ************************************************************************************************************************************************************************/
#pragma mark -

void	IndexAirports(const AptVector& apts, AptIndex& index)
{
	index.clear();
	for (int a = 0; a < apts.size(); ++ a)
	{
		POINT2 ctr(
			(apts[a].bounds.xmin()+apts[a].bounds.xmax())*0.5,
			(apts[a].bounds.ymin()+apts[a].bounds.ymax())*0.5);

		int lon = floor(CGAL2DOUBLE(ctr.x()));
		int lat = floor(CGAL2DOUBLE(ctr.y()));
		index.insert(AptIndex::value_type(hash_ll(lon, lat), a));
//		printf("LL hash for %s is %d,%d\n", apts[a].icao.c_str(), lon, lat);
	}
}

void	FindAirports(const Bbox2& bounds, const AptIndex& index, set<int>& apts)
{
	apts.clear();
	int x1 = floor(bounds.xmin() - 0.5);
	int x2 =  ceil(bounds.xmax() + 0.5);
	int y1 = floor(bounds.ymin() - 0.5);
	int y2 =  ceil(bounds.ymax() + 0.5);
	for (int x = x1; x <= x2; ++x)
	for (int y = y1; y <= y2; ++y)
	{
		pair<AptIndex::const_iterator,AptIndex::const_iterator>	range = index.equal_range(hash_ll(x,y));
		for (AptIndex::const_iterator i = range.first; i != range.second; ++i)
		{
			apts.insert(i->second);
		}
	}
}

/************************************************************************************************************************************************************************
 * GENERATING IMPRECISE GEOMETRY FROM APT
 ************************************************************************************************************************************************************************/
#pragma mark -

void GetAptPOI(const AptInfo_t * a, vector<Point2>& poi)
{
	for(AptRunwayVector::const_iterator rwy = a->runways.begin(); rwy != a->runways.end(); ++rwy)
	{
		poi.push_back(rwy->ends.p1);
		poi.push_back(rwy->ends.p2);
	}
	for(AptGateVector::const_iterator g = a->gates.begin(); g != a->gates.end(); ++g)
		poi.push_back(g->location);
}

void	WindingToFile(const vector<Polygon2>& w, const char * filename)
{
	FILE * fi = fopen(filename,"w");
	if(fi == NULL) return;
	fprintf(fi,"TOTAL %zd\n", w.size());
	for(vector<Polygon2>::const_iterator i = w.begin(); i != w.end(); ++i)
	{
		fprintf(fi,"POLYGON %zd\n",i->size());
		for(Polygon2::const_iterator c = i->begin(); c != i->end(); ++c)
		{
			fprintf(fi,"PT 0x%016llX 0x%016llX\n", *((unsigned long long *) &c->x_), *((unsigned long long *) &c->y_));
		}
	}
	fclose(fi);
}

void	WindingFromFile(vector<Polygon2>& w, const char * filename)
{
	w.clear();
	FILE * fi = fopen(filename,"r");
	int t;
	fscanf(fi,"TOTAL %d\n",&t);
	while(t--)
	{
		w.push_back(Polygon2());
		int p;
		fscanf(fi,"POLYGON %d\n",&p);
		while(p--)
		{
			w.back().push_back(Point2());
			fscanf(fi,"PT %llx %llx\n",
				(unsigned long long *) &w.back().back().x_,
				(unsigned long long *) &w.back().back().y_);
		}
	}
}

void	GetAptPolygons(
				const				AptInfo_t& in_layout,
				double				bezier_epsi_deg,
				vector<Polygon2>&	windings)
{
	const AptInfo_t *	who = &in_layout;
	AptInfo_t			converted;
	if(!who->pavements.empty())
	{
		converted = in_layout;
		ConvertForward(converted);
		who = &converted;
	}

	Point2	corners[4];

	for(AptRunwayVector::const_iterator r = who->runways.begin(); r != who->runways.end(); ++r)
	{
		Point2	ends[2] = { r->ends.p1, r->ends.p2 };
		Quad_2to4(ends, r->width_mtr, corners);
		reverse(corners,corners+4);
		windings.push_back(Polygon2(corners,corners+4));
	}
	for(AptSealaneVector::const_iterator s = who->sealanes.begin(); s != who->sealanes.end(); ++s)
	{
		Point2	ends[2] = { s->ends.p1, s->ends.p2 };
		Quad_2to4(ends, s->width_mtr, corners);
		reverse(corners,corners+4);
		windings.push_back(Polygon2(corners,corners+4));
	}
	for(AptHelipadVector::const_iterator h = who->helipads.begin(); h != who->helipads.end(); ++h)
	{
		Quad_1to4(h->location, h->heading, h->length_mtr, h->width_mtr, corners);
		reverse(corners,corners+4);
		windings.push_back(Polygon2(corners,corners+4));
	}
	for(AptTaxiwayVector::const_iterator t = who->taxiways.begin(); t != who->taxiways.end(); ++t)
	{
		vector<AptLinearSegment_t>::const_iterator s, e;
		s = t->area.begin();
		e = t->area.end();
		while(s != e)
		{
			vector<AptLinearSegment_t>	winding;
			s = BreakupAptWindings(s, e, back_inserter(winding));

			// Make sure we have a real closed ring, etc.  for taxiways we're fubar otherwise.
			DebugAssert(winding.size() >= 2);
			DebugAssert(apt_code_is_ring(winding.back().code));

			// Now: front point is duped on the back to make a closed seq instead of a ring.
			// The go and adjust the codes for the last two, since the second to last is now NOT
			// a terminator but the last one IS.
			winding.push_back(winding.front());
			int n = winding.size();
			winding[n-1].code = apt_code_is_curve(winding[n-1].code) ? apt_rng_crv : apt_rng_seg;
			winding[n-2].code = apt_code_is_curve(winding[n-2].code) ? apt_lin_crv : apt_lin_seg;

			AptPolygonIterator ss(winding.begin());
			AptPolygonIterator ee(winding.end());

			Polygon2	approx_winding;
			approximate_bezier_sequence_epsi(ss, ee, back_inserter(approx_winding), bezier_epsi_deg);

			DebugAssert(approx_winding.size() >= 4);
			DebugAssert(approx_winding.front() == approx_winding.back());
			approx_winding.pop_back();

			windings.push_back(approx_winding);
		}
	}

}


static void convert_polygon_cleanup(const Polygon2& input, vector<Polygon_2>& output)
{
	Polygon_2 p;
	ben2cgal(input,p);
	if(p.is_simple())
	{
		if(p.orientation()==CGAL::CLOCKWISE)
			p.reverse_orientation();
		output.push_back(p);
	}
	else
	{
		MakePolygonSimple(p, output);
	}
}

void apt_make_map_from_polygons(
					const vector<Polygon2>&			pavement,
					Polygon_set_2&					out_map)
{
	out_map.clear();

	// STAGE 1: we are simply going to take the union of the entire mess of pavement we've been given - simplify
	// polygons that are twisted by taking their entire contained area.  The end reuslt is just "stuff we can
	// drive on."

	for(vector<Polygon2>::const_iterator e = pavement.begin(); e != pavement.end(); /* intentional */)
	{
		vector<Polygon2>::const_iterator h(e);			// points to the next ccw outer boundary.
			++h;
		while(h != pavement.end() && !h->is_ccw())
			++h;

		Polygon_set_2	pav;
		vector<Polygon_2>	outer;
		convert_polygon_cleanup(*e, outer);
		pav.join(outer.begin(),outer.end());

		++e;
		while(e != h)
		{
			vector<Polygon_2>	hole;
			convert_polygon_cleanup(*e, hole);
			for(vector<Polygon_2>::iterator he = hole.begin(); he != hole.end(); ++he)
			{
				pav.difference(*he);
			}
			++e;
		}
		out_map.join(pav);

	}
}

void apt_make_cut_map(Polygon_set_2& in_area, Pmwx& out_map, double cut_x, double cut_y)
{
	out_map = in_area.arrangement();

	// Stage 2: now we are going to cut the arrangement into grid squares...that way every "hop" from point
	// to point is relatively small.

	double min_x = 9.9e9;  // Hack?  Yes. But this is lat/lon...pretty safe hack I think, on this planet.
	double min_y = 9.9e9;
	double max_x =-9.9e9;
	double max_y =-9.9e9;

	vector<Curve_2>		cuts;

	for(Pmwx::Hole_iterator h = out_map.unbounded_face()->holes_begin(); h != out_map.unbounded_face()->holes_end(); ++h)
	{
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ=stop=*h;
		do {
			double px = CGAL::to_double(circ->source()->point().x());
			double py = CGAL::to_double(circ->source()->point().y());
			min_x = min(px,min_x);
			min_y = min(py,min_y);
			max_x = max(px,max_x);
			max_y = max(py,max_y);

		} while (++circ != stop);
	}

	min_x -= cut_x*0.25;
	min_y -= cut_y*0.25;
	max_x += cut_x*0.25;
	max_y += cut_y*0.25;

	for(double x = min_x + cut_x; x < max_x; x += cut_x)
		cuts.push_back(Curve_2(Segment_2(Point_2(x,min_y),Point_2(x,max_y)),0));

	for(double y = min_y + cut_y; y < max_y; y += cut_y)
		cuts.push_back(Curve_2(Segment_2(Point_2(min_x,y),Point_2(max_x,y)),0));

	{
		data_preserver_t<Pmwx>	preserver;
		preserver.attach(out_map);
		CGAL::insert(out_map, cuts.begin(), cuts.end());
		preserver.detach();
	}

	// Stage 3: clean up any extra cuts we made but don't want.

	for(Pmwx::Edge_iterator e = out_map.edges_begin(); e != out_map.edges_end(); /* intentional */)
	{
		if(e->face() == e->twin()->face() ||									// Edge doesn't separate anyone from anyone else?
		  (!e->face()->contained() && !e->twin()->face()->contained()))			// Edge splits "grass" areas - grass can be one big area, we really don't care.
		{
			Pmwx::Halfedge_handle k = e;
			++e;
			out_map.remove_edge(k);
		} else
			++e;
	}
}




/************************************************************************************************************************************************************************
 * BEZIER ITERATOR OFF OF A LAYOUT
 ************************************************************************************************************************************************************************/
#pragma mark -


AptPolygonIterator::AptPolygonIterator() : p(-2)
{
}

AptPolygonIterator::AptPolygonIterator(const AptPolygonIterator& rhs) : i(rhs.i), p(rhs.p)
{
}

AptPolygonIterator::AptPolygonIterator(const AptPolygon_t::const_iterator rhs) : i(rhs), p(0)
{
}

AptPolygonIterator& AptPolygonIterator::operator=(const AptPolygonIterator& rhs)
{
	i = rhs.i;
	p = rhs.p;
	return *this;
}

bool AptPolygonIterator::operator==(const AptPolygonIterator& rhs) const { return i == rhs.i && p == rhs.p; }
bool AptPolygonIterator::operator!=(const AptPolygonIterator& rhs) const { return i != rhs.i || p != rhs.p; }

pair<Point2, int>	AptPolygonIterator::operator*(void) const
{
	switch(p) {
	case -1:
		DebugAssert(apt_code_is_curve(i->code));
		return pair<Point2,int>(Point2(2.0 * i->pt.x() - i->ctrl.x(), 2.0 * i->pt.y() - i->ctrl.y()),p);
	case 0:
		return pair<Point2,int>(i->pt, p);
	case 1:
		DebugAssert(apt_code_is_curve(i->code));
		return pair<Point2,int>(i->ctrl, p);
	default:
		DebugAssert(!"Should not be here.");
		return make_pair(Point2(),0);
	}
}

set<int> AptPolygonIterator::operator()(void) const {
	return i->attributes;
}

AptPolygonIterator AptPolygonIterator::operator++(int)
{
	AptPolygonIterator other(*this);
	++(*this);
	return other;
}

AptPolygonIterator& AptPolygonIterator::operator++(void)
{
	// If we just emit the lo control handle, take the time to
	// consolidate now - simply advance onto the center of
	// the last point that matches us.  That way we know that
	// from here we can emit high and move on.
	// Also note that if we are on the low control handle this can
	// NEVER be the last point, we always will emit someone's pt.
	if(p == -1)
	{
		++p;
		Point2	pt(i->pt);

		while(!apt_code_is_term(i->code))			// Stop if term, we can't go off the end!
		{
			AptPolygon_t::const_iterator n(i);
			++n;
			if(n->pt != i->pt)
				break;
			++i;
		}

		return *this;
	}

	// On center and we have high point - emit it and bail.
	if(p == 0 && apt_code_is_curve(i->code))
	{
		++p;
		return *this;
	}

	// Okay this is the true advance.  Either we are on the high
	// handle or we are on the middle and there is no high.

	// If we just emitted our high control handle, we have to advance
	// and do the next point.  We always take the low control handle
	// if it is present, unless we are at the end.

	if(apt_code_is_ring(i->code) || apt_code_is_end(i->code))
	{
		++i;				// Off the end case - set p to 0 for consistency
		p = 0;				// but DO NOT deref I - it might be invalid.
		return *this;
	} else {
		++i;				// Advance case...move forward.  Then if we have to consolidate if no curve.
		p = -1;

		if (!apt_code_is_curve(i->code))
		{
			++p;
			Point2	pt(i->pt);

			while(!apt_code_is_term(i->code))			// Stop if term, we can't go off the end!
			{
				AptPolygon_t::const_iterator n(i);
				++n;
				if(n->pt != i->pt)
					break;
				++i;
			}
		}
		return *this;
	}
}

