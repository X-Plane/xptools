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

bool	ReadShapeFile(const char * in_file, Pmwx& out_map, double bounds[4])
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

	for(int n = 0; n < entity_count; ++n)
	{
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
					for (int i = start_idx; i < stop_idx; ++i)
					{
						Point_2 pt(obj->padfX[i],obj->padfY[i]);
						if(p.empty() || pt != p.back())
							p.push_back(pt);
					}
//					DebugAssert(p.size() >= 2);
					for(int i = 1; i < p.size(); ++i)
					{
						curves.push_back(Curve_2(Segment_2(p[i-1],p[i]),n));
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
		CGAL::insert_curves(out_map, curves.begin(), curves.end());
		break;
	}
	SHPClose(file);
	return true;
}

