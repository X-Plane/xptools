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

#include "TensorRoads.h"
#include "MapDefs.h"
#include "WED_DEMGraphics.h"
#include "DEMDefs.h"
#include "TensorUtils.h"
#include "MathUtils.h"

struct Tensor_info {
	const DEMGeo *	dem;
	Bbox2			bounds;	
	vector<Point2>	poly;
	vector<Vector2>	tensors;
	vector<double>	weights;
};	

static Vector2	Tensor_Func(const Point2& p, void * ref)
{
	Tensor_info * i = (Tensor_info *) ref;
	
	double lon = interp(0,i->bounds.p1.x,1,i->bounds.p2.x,p.x);
	double lat = interp(0,i->bounds.p1.y,1,i->bounds.p2.y,p.y);
		
	double x = i->dem->lon_to_x(lon);
	double y = i->dem->lat_to_y(lat);
	
	Polygon_Weights(&*i->poly.begin(),&*i->weights.begin(),i->poly.size(), Point2(lon,lat));

	Vector2	basis = Gradient2Tensor(Vector2(i->dem->gradient_x_bilinear(x,y),i->dem->gradient_y_bilinear(x,y)));
	basis *= 10.0;
//	basis = Linear_Tensor(i->poly[0],Vector2(1,0),0,Point2(lon,lat));
//	basis = Vector2(0,0);
	
	double sz = (i->bounds.p2.y - i->bounds.p1.y) * (i->bounds.p2.x - i->bounds.p1.x);
	
	for (int n = 0; n < i->poly.size(); ++n)
//		basis += Radial_Tensor(i->poly[n], 4.0 / sz, Point2(lon,lat));
		basis += Linear_Tensor(i->poly[n],i->tensors[n], 4.0 / sz, Point2(lon,lat));
	
	return basis;
//	return Vector2(i->weights[0],i->weights[1]);
//	return Eigen2Tensor(Weighted_Tensor(&*i->tensors.begin(),&*i->weights.begin(),i->poly.size()));
	

}

void	BuildRoadsForFace(
					Pmwx&			ioMap,
					const DEMGeo&	inElevation,
					GISFace *		inFace,
					ProgressFunc	inProg,
					ImageInfo *		ioTensorImage,
					double			outTensorBounds[4])
{
	if(ioTensorImage)
	{
		Pmwx::Ccb_halfedge_circulator	circ = inFace->outer_ccb();
		Pmwx::Ccb_halfedge_circulator	start = circ;
		Bbox2	face_bounds(circ->source()->point());	
		Tensor_info t;		
		do {
			face_bounds += circ->source()->point();
			t.poly.push_back(circ->target()->point());
			t.weights.push_back(0);
			Vector2 prev(	circ->source()->point(),circ->target()->point());
			Vector2 next(	circ->next()->source()->point(),circ->next()->target()->point());
			prev.normalize();
			next.normalize();
			Vector2 v(prev+next);
			v.normalize();
			t.tensors.push_back(/*Eigen2Tensor*/(v));
			++circ;
		} while (circ != start);
		
		outTensorBounds[0] = face_bounds.p1.x;
		outTensorBounds[1] = face_bounds.p1.y;
		outTensorBounds[2] = face_bounds.p2.x;
		outTensorBounds[3] = face_bounds.p2.y;

		t.dem = &inElevation;
		t.bounds = face_bounds;
		
		TensorDDA(*ioTensorImage,Tensor_Func,&t);
	}
}
