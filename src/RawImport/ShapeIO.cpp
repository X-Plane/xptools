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

#include "ShapeIO.h"
#include <shapefil.h>
#include "GISTool_Globals.h"
#if !DEV
	factor this out
#endif

// Ben says: this can be modified to printf the points.  If a shape-file import ever blows up,
// we can use it to rapidly generate a numeric dataset - then we send a test program to the CGAL
// team if they want one.
#define ADD_PT_PAIR(a,b,c,d,e)	curves.push_back(Curve_2(Segment_2((a),(b)),(e)));


bool	ReadShapeFile(const char * in_file, Pmwx& out_map, double bounds[4], ProgressFunc	inFunc)
{
		int		entity_count;
		int		shape_type;
		double	bounds_lo[4], bounds_hi[4];

	out_map.clear();

	SHPHandle file =  SHPOpen(in_file, "rb");
	if(!file) return false;
	
	SHPGetInfo(file, &entity_count, &shape_type, bounds_lo, bounds_hi);

	bounds[0] = bounds_lo[0];
	bounds[1] = bounds_lo[1];

	bounds[2] = bounds_hi[0];
	bounds[3] = bounds_hi[1];

	Polygon_set_2	poly_set;
	vector<Curve_2>	curves;
	
	PROGRESS_START(inFunc, 0, 1, "Reading shape file...")

	int step = entity_count ? (entity_count / 150) : 2;
	for(int n = 0; n < entity_count; ++n)
	{
		PROGRESS_CHECK(inFunc, 0, 1, "Reading shape file...", n, entity_count, step)
		SHPObject * obj = SHPReadObject(file, n);
		switch(obj->nSHPType) {
		case SHPT_POINT:
		case SHPT_POINTZ:
		case SHPT_POINTM:
		
		case SHPT_ARC:
		case SHPT_ARCZ:
		case SHPT_ARCM:
			if (obj->nVertices > 1)
			{		
				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					vector<Point_2>	p;
					vector<Point2>	p_raw;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point_2 pt(obj->padfX[i],obj->padfY[i]);
						if(p.empty() || pt != p.back())
						{
							p.push_back(pt);
							p_raw.push_back(Point2(obj->padfX[i],obj->padfY[i]));
						}
					}
//					DebugAssert(p.size() >= 2);
					for(int i = 1; i < p.size(); ++i)
					{
						DebugAssert(p[i-1] != p[i]);
						ADD_PT_PAIR(p[i-1],p[i],p_raw[i-1],p_raw[i],n);
					}
				}
			}
			break;
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
		case SHPT_POLYGONM:
			if (obj->nVertices > 0)
			{
				list<Polygon_2>	boundaries, holes;
				for (int part = 0; part < obj->nParts; ++part)
				{
					int start_idx = obj->panPartStart[part];
					int stop_idx = ((part+1) == obj->nParts) ? obj->nVertices : obj->panPartStart[part+1];
					Polygon_2	p;
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point_2 pt(obj->padfX[i],obj->padfY[i]);
						if(p.is_empty() || pt != p.vertex(p.size()-1))					// Do not add point if it equals the prev!
							p.push_back(pt);
					}
					DebugAssert(p.size() >= 4);
					DebugAssert(p[0] == p[p.size()-1]);
					while(p[0] == p[p.size()-1])
						p.erase(p.vertices_end()-1);
					DebugAssert(p.size() == 0 || p.size() >= 3);
					if(p.size() > 0)
					{
						DebugAssert(p.is_simple());
						if(p.is_counterclockwise_oriented())
						{
							holes.push_back(p);
						} else {
							p.reverse_orientation();
							boundaries.push_back(p);					
						}
					}
				}
				for(list<Polygon_2>::iterator i = boundaries.begin(); i != boundaries.end(); ++i)
					poly_set.join(*i);
				for(list<Polygon_2>::iterator i = holes.begin(); i != holes.end(); ++i)
					poly_set.difference(*i);				
			}
			break;
		case SHPT_MULTIPOINT:
		case SHPT_MULTIPOINTZ:
		case SHPT_MULTIPOINTM:
		case SHPT_MULTIPATCH:
			break;
		}
		SHPDestroyObject(obj);
	}
	switch(shape_type) {
	case SHPT_POLYGON:
	case SHPT_POLYGONZ:
	case SHPT_POLYGONM:
		out_map = poly_set.arrangement();
		break;
	case SHPT_ARC:
	case SHPT_ARCZ:
	case SHPT_ARCM:

#if 1
		
		ADD_PT_PAIR(
							Point_2(gMapWest,gMapSouth),
							Point_2(gMapEast,gMapSouth),
							Point2(gMapWest,gMapSouth),
							Point2(gMapEast,gMapSouth),
							entity_count);

		ADD_PT_PAIR(
							Point_2(gMapEast,gMapSouth),
							Point_2(gMapEast,gMapNorth),
							Point2(gMapEast,gMapSouth),
							Point2(gMapEast,gMapNorth),
							entity_count+1);

		ADD_PT_PAIR(
							Point_2(gMapEast,gMapNorth),
							Point_2(gMapWest,gMapNorth),
							Point2(gMapEast,gMapNorth),
							Point2(gMapWest,gMapNorth),
							entity_count+2);

		ADD_PT_PAIR(
							Point_2(gMapWest,gMapNorth),
							Point_2(gMapWest,gMapSouth),
							Point2(gMapWest,gMapNorth),
							Point2(gMapWest,gMapSouth),
							entity_count+3);
#endif	
		CGAL::insert_curves(out_map, curves.begin(), curves.end());
		break;
	}
	SHPClose(file);
	PROGRESS_DONE(inFunc, 0, 1, "Reading shape file...")

	return true;
}

