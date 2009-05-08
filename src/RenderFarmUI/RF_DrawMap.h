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
#ifndef DRAWMAP_H
#define DRAWMAP_H

#include "MapDefs.h"
#include "RF_Selection.h"
#include "ProgressUtils.h"

class	GUI_GraphState;
void	PrecalcOGL(Pmwx&						ioMap, ProgressFunc inFunc);
void	RecalcOGLColors(Pmwx&					ioMap, ProgressFunc inFunc);

void	DrawMapBucketed(
				GUI_GraphState *				inState,
				Pmwx&	 						inMap,
				double							mapWest,
				double							mapSouth,
				double							mapEast,
				double							mapNorth,
//				double							screenLeft,
//				double							screenBottom,
//				double							screenRight,
//				double							screenTop,
				const set<Pmwx::Vertex_handle>&		vertexSel,
				const set<Pmwx::Halfedge_handle>&	edgeSel,
				const set<Pmwx::Face_handle>&		faceSel,
				const set<PointFeatureSelection>&	pointFeatureSel);



void		FindFaceTouchesPt(Pmwx& inMap, const Point2&, vector<Face_handle>& outIDs);									// Fully checks for pt containment
void		FindFaceTouchesRectFast(Pmwx& inMap, const Point2&, const Point2&, vector<Face_handle>& outIDs);				// Intersects with face bbox, not face
void		FindFaceFullyInRect(Pmwx& inMap, const Point2&, const Point2&, vector<Face_handle>& outIDs);					// Full containment

void		FindHalfedgeTouchesRectFast(Pmwx& inMap, const Point2&, const Point2&, vector<Halfedge_handle>& outIDs);		// Intersects with half-edge bbox, not half-edge
void		FindHalfedgeFullyInRect(Pmwx& inMap, const Point2&, const Point2&, vector<Halfedge_handle>& outIDs);			// Full containment

void		FindVerticesTouchesPt(Pmwx& inMap, const Point2&, vector<Vertex_handle>& outIDs);								// Perfect equalty.
void		FindVerticesTouchesRect(Pmwx& inMap, const Point2&, const Point2&, vector<Vertex_handle>& outIDs);				// Full containment (any containment is full for pts)


#endif