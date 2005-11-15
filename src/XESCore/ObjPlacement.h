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
#ifndef OBJPLACEMENT_H
#define OBJPLACEMENT_H

class	GISFace;
class	DEMGeoMap;
class	CDT;
class	Pmwx;
class	GISHalfedge;
class	Bbox2;
#include "ProgressUtils.h"
#include "Skeleton.h"

typedef pair<GISFace *, ComplexPolygonVector>	PreinsetFace;


double	GetInsetForEdgeMeters(const GISHalfedge * inEdge);
double	GetInsetForEdgeDegs(const GISHalfedge * inEdge);

void	RemoveDuplicates(
							GISFace * 			inFace);
void	RemoveDuplicatesAll(
							Pmwx&				ioMap,
							ProgressFunc		inProg);

void	InstantiateGTPolygon(
							GISFace *				inFace,
							const ComplexPolygon2&	inBounds,
							const DEMGeoMap& 		inDEMs,
							const CDT&				inMesh);

void	InstantiateGTPolygonAll(
							const vector<PreinsetFace>&	inFaces,
							const DEMGeoMap& 			inDEMs,
							const CDT&					inMesh,
							ProgressFunc				inProg);

void	GenerateInsets(
					Pmwx& 					ioMap,
					CDT&					ioMesh,
					const Bbox2&			inBounds,
					const set<int>&			inTypes,
					vector<PreinsetFace>&	outInsets,
					ProgressFunc			func);

void	GenerateInsets(
					const set<GISFace *>&	inFaces,
					vector<PreinsetFace>&	outInsets,
					ProgressFunc			func);


void	DumpPlacementCounts(void);

#endif
