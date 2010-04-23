/* 
 * Copyright (c) 2010, Laminar Research.
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

#include "ObjPlacement2.h"
#include "NetTables.h"
#include "MapPolygon.h"
#include "BlockDefs.h"

// Given a raod segment, return the widest type - this is the one we must use for insetting.
static int	WidestRoadTypeForSegment(Pmwx::Halfedge_const_handle he)
{
	int	best_type = NO_VALUE;
	double best_width = 0.0;
	for (GISNetworkSegmentVector::const_iterator i = he->data().mSegments.begin(); i != he->data().mSegments.end(); ++i)
	{
		if (gNetReps[i->mRepType].width > best_width)
		{
			best_type = i->mRepType;
			best_width = gNetReps[i->mRepType].width;
		}
	}

	for (GISNetworkSegmentVector::const_iterator i = he->twin()->data().mSegments.begin(); i != he->twin()->data().mSegments.end(); ++i)
	{
		if (gNetReps[i->mRepType].width > best_width)
		{
			best_type = i->mRepType;
			best_width = gNetReps[i->mRepType].width;
		}
	}
	return best_type;
}

// This primitive tells us if a given two edges are effectively one edge - basically
// we're trying to ignore near-colllinear edges for subdivision
static bool	can_skip_segment(Halfedge_const_handle prev, Halfedge_const_handle edge)
{
	if (WidestRoadTypeForSegment(edge) != WidestRoadTypeForSegment(prev))
		return false;
	Vector_2	v1(edge->source()->point(), edge->target()->point());
	Vector_2	v2(prev->source()->point(), prev->target()->point());
	return (normalize(v1) * normalize(v2) > 0.99);
}

void	GetTotalAreaForFaceSet(Pmwx::Face_handle f, Polygon_with_holes_2& out_area)
{

	PolygonFromFaceEx(f, out_area, NULL,NULL,NULL,can_skip_segment);
}
