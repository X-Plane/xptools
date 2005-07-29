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
#ifndef OBJUTILS_H
#define OBJUTILS_H

#include "ExtrudeFunc.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"
#include "XProtoDefs.h"
#include "XObjDefs.h"

void	ApplyPrototype(const Prototype_t& 					inPrototype,
					   const Polygon2& 						inPoints,
					   int									inFloors,
					   XObj&							    outObject);

void	GetObjBoundingSphere(const XObj& inObj, Sphere3& outSphere);
void 	OffsetObject(XObj& ioObj, double x, double y, double z);

void	GetObjDimensions(const XObj& inObj,
						float	minCoords[3],
						float	maxCoords[3]);

// Given two points that will be the minimum and max X
// locations for a given object at the min and max Y locations
// this routine extrudes them in an axis opposite the wall 
// line to make sure X-Z coordinates are square.
void	ExtrudeBoxZ(float minCorner[3], float maxCorner[3],
					float outNewCoords[8][3]);

// New coords go xyz Xyz xYz XYz xyZ Xyz xYZ XYZ
// where caps = max, lower = min
void	ConformObjectToBox(XObj& 	ioObj,
							float		inMinCoords[3],
							float		inMaxCoords[3],
							float		inNewCoords[8][3]);

bool	LoadPrototype(const char * inFileName, Prototype_t& outProto);

bool	SavePrototype(const char * inFileName, const Prototype_t& outProto);


void	ExtrudeFuncToObj(int polyType, int count, float * pts, float * sts, float LOD_near, float LOD_far, void * inRef);

#endif
