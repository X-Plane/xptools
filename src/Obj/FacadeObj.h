/* 
 * Copyright (c) 2004, Laminar Research.
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
#ifndef FACADEOBJ_H
#define FACADEOBJ_H

class	Polygon3;
class	Vector3;

#include <vector>
using std::vector;

#include "ExtrudeFunc.h"

struct	FacadeWall_t {

	FacadeWall_t();

	double			min_width;	// Meters
	double			max_width;	// Meters
	double			x_scale;	// From tex to meters
	double			y_scale;
	double			roof_slope;	// 0 = none, 1 = 45 degree ratio
	
	// S&T coordinates for each panel and floor
	vector<pair<float, float> >		s_panels;
	int								left;
	int								center;
	int								right;
	
	vector<pair<float, float> >		t_floors;
	int								bottom;
	int								middle;
	int								top;
};	

struct	FacadeLOD_t {

	FacadeLOD_t();
	
	float					lod_near;
	float					lod_far;
	vector<FacadeWall_t>	walls;
	vector<double>			roof_s;
	vector<double>			roof_t;
};
struct	FacadeObj_t {

	FacadeObj_t();
	
	string					texture;
	string					texture_lit;
	bool					is_ring;
	bool					two_sided;
	vector<FacadeLOD_t>		lods;
};



bool	ReadFacadeObjFile(const char * inPath, FacadeObj_t& outObj);
bool	SaveFacadeObjFile(const char * inPath, const FacadeObj_t& inObj);
int		FindFloorsForHeight(const FacadeObj_t& inObj, float height);
void	BuildFacadeObj(const FacadeObj_t& inObj, 
					   const Polygon3& 	 	inPolygon,
					   int					inFloors,
					   const Vector3&		inUp,
					   ExtrudeFunc_f		inFunc,
					   void *				inRef);

#endif
