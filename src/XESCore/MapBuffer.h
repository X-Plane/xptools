/*
 * Copyright (c) 2008, Laminar Research.
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

#ifndef MapBuffer_H
#define MapBuffer_H

#include "MapDefsCGAL.h"
#include "MapPolygon.h"

// A tagged polygon is a set of curves such that the end points are all linked to form a polygon.  This allows you
// to have a per-edge parameter.
typedef vector<Curve_2>				TaggedPolygon_t;

void	TagPolygon(
				const Polygon_2&			in_polygon,
				TaggedPolygon_t&			out_polygon);

void	UntagPolygon(
				const TaggedPolygon_t&		in_polygon,
				Polygon_2&					out_polygon);


// Polygon buffering - positive inset means smaller.  If "in_insets" is not null, it must be one inset per side.  The inset array
// must always be positive - use in_inset (the master scale) for outset.  A new polygon set is returned that contains the area
// contained by the original polygon after the buffering op.

// Input polygons:
// - Must be simple but may have antennas.
// - Antennas must be correctly noded.
// - Antennas are allowed on the side we expand into.  So for positive inset (shrink polygon) we can have antennas on the inside but not outside.
// Polygons must be CCW orientation.

void	BufferPolygon(
				const Polygon_2&			in_polygon,
				const RingInset_t *			in_insets,
				double						in_inset,
				Polygon_set_2&				out_new_polygon);

// Same as above, but for a polygon with holes.  All requiremetns apply plus the holes must be CW.
void	BufferPolygonWithHoles(
				const Polygon_with_holes_2&	in_polygon,
				const PolyInset_t *			in_insets,
				double						in_inset,
				Polygon_set_2&				out_new_polygon);


void	ValidateBuffer(
				Pmwx&						arr,
				Face_handle					face,
				Locator&					l,
				Polygon_set_2&				ps);

#endif /* MapBuffer_H */
