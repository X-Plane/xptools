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

#ifndef AptAlgs_H
#define AptAlgs_H

#include "CompGeomDefs2.h"
#include "AptDefs.h"
#include "AssertUtils.h"
#include "MapDefs.h"

/***************************************************************************************************************************************
 * BEZIER HANDLING FOR AIRPORT LAYOUTS
 ***************************************************************************************************************************************/

// A bezier iterator is a forward iterator whose return type is pair<Point2, int>. (-1 for prev ctrl, 1 for next ctrl)
// operator() returns attributes for the current curve side we are on.
// In their output form we always have chains - a ring is just a chain with a repeated 
// endpoint.  The first and last points output must be non-ctrl points!

struct AptPolygonIterator {

	typedef	set<int>			attribute_type;

	AptPolygonIterator();
	AptPolygonIterator(const AptPolygonIterator& rhs);
	AptPolygonIterator(const AptPolygon_t::const_iterator p);
	
	AptPolygonIterator& operator=(const AptPolygonIterator& rhs);
	
	bool operator==(const AptPolygonIterator& rhs) const;
	bool operator!=(const AptPolygonIterator& rhs) const;
	
	pair<Point2, int>	operator*(void) const;
	
	set<int> operator()(void) const;
	
	AptPolygonIterator operator++(int);
	AptPolygonIterator& operator++(void);
	
private:
		AptPolygon_t::const_iterator	i;
		int								p;

};

// This copies the first winding of an airport contour sequence into the output iterator, and returns an iterator
// to the next winding.  Use this to isolate individual windings that can be iterated on further.
template<class __input_iterator, class __output_iterator>
__input_iterator BreakupAptWindings(
							__input_iterator start,
							__input_iterator end,
							__output_iterator o);

/***************************************************************************************************************************************
 * AIRPORT GEOMETRY HANDLING
 ***************************************************************************************************************************************/

// This converts an airport layout into a series of polygon contours.  This approximates beziers, but does no "sanitizing" - there can
// be self-intersections both from FUBAR user data and from bezier-approximation-induced error.
void	GetAptPolygons(
				const				AptInfo_t& in_layout, 
				double				bezier_epsi_deg,
				vector<Polygon2>&	windings);					// Windings are CCW for outside, followed by CW for holes.  They may be self-intersecting...no promises!
																// They come out with the HIGHEST prio windings on TOP.


void	WindingToFile(const vector<Polygon2>& w, const char * filename);
void	WindingFromFile(vector<Polygon2>& w, const char * filename);

// This converts the windings into a single polygon_set_2 of "hard surface".  This deals with ALL possible data degeneration problems and comes up with the
// best CGAL precision area.
void apt_make_map_from_polygons(
					const vector<Polygon2>&			pavement,
					Polygon_set_2&					out_map);

// Given a polygon_set_2 of pavement area, this cuts it at every X x Y degrees and returns a map with "contained" set for all airport areas.
void apt_make_cut_map(Polygon_set_2& in_area, Pmwx& out_map, double cut_x, double cut_y);

// Get all of the points of interest for a layout...gates, runway ends, etc.
void GetAptPOI(const AptInfo_t * a, vector<Point2>& poi);

// Indexing
void	IndexAirports(const AptVector& apts, AptIndex& index);
void	FindAirports(const Bbox2& bounds, const AptIndex& index, set<int>& apts);


/***************************************************************************************************************************************
 * INLINE TEMPLATE CODE
 ***************************************************************************************************************************************/

template<class __input_iterator, class __output_iterator>
__input_iterator BreakupAptWindings( __input_iterator start, __input_iterator end, __output_iterator o)
{
	while(start != end) {
		bool cap = apt_code_is_term(start->code);
		*o = *start;
		++o;
		++start;
		if(cap) break;
	}
	return start;
}


#endif /* AptAlgs_H */
