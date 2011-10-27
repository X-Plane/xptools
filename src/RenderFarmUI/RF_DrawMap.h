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
#include "RTree2.h"

extern int g_color_face_with_terr;
extern int g_color_face_with_zone;
extern int g_color_face_with_supr;
extern int g_color_face_use_supr_tint;

class	GUI_GraphState;
void	PrecalcOGL(Pmwx&						ioMap, ProgressFunc inFunc);
void	RecalcOGLColors(Pmwx&					ioMap, ProgressFunc inFunc);


struct PmwxIndex_t {
	PmwxIndex_t() { }
	typedef	RTree2<Face_handle,16>		FaceTree;
	typedef	RTree2<Halfedge_handle,16>	HalfedgeTree;
	typedef RTree2<Vertex_handle,16>	VertexTree;

	FaceTree		faces;
	HalfedgeTree 	halfedges;
	VertexTree		vertices;

	void	IndexPmwx(Pmwx& pmwx, PmwxIndex_t& index);

private:
	PmwxIndex_t(const PmwxIndex_t&);
	PmwxIndex_t& operator=(const PmwxIndex_t&);
};

void	IndexPmwx(Pmwx& pmwx, PmwxIndex_t& index);

	

void	DrawMapBucketed(
				GUI_GraphState *				inState,
				Pmwx&	 						inMap,
				PmwxIndex_t&					inIndex,
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


void		FindFaceTouchesPt(Pmwx& inMap, PmwxIndex_t& index, const Point2&, vector<Face_handle>& outIDs);									// Fully checks for pt containment
void		FindFaceTouchesRectFast(Pmwx& inMap, PmwxIndex_t& index, const Point2&, const Point2&, vector<Face_handle>& outIDs);				// Intersects with face bbox, not face
void		FindFaceFullyInRect(Pmwx& inMap, PmwxIndex_t& index, const Point2&, const Point2&, vector<Face_handle>& outIDs);					// Full containment

void		FindHalfedgeTouchesRectFast(Pmwx& inMap, PmwxIndex_t& index, const Point2&, const Point2&, vector<Halfedge_handle>& outIDs);		// Intersects with half-edge bbox, not half-edge
void		FindHalfedgeFullyInRect(Pmwx& inMap, PmwxIndex_t& index, const Point2&, const Point2&, vector<Halfedge_handle>& outIDs);			// Full containment

void		FindVerticesTouchesPt(Pmwx& inMap, PmwxIndex_t& index, const Point2&, vector<Vertex_handle>& outIDs);								// Perfect equalty.
void		FindVerticesTouchesRect(Pmwx& inMap, PmwxIndex_t& index, const Point2&, const Point2&, vector<Vertex_handle>& outIDs);				// Full containment (any containment is full for pts)

#endif