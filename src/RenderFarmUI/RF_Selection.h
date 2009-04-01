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
#ifndef RF_SELECTION_H
#define RF_SELECTION_H

#include <set>
#include "MapDefs.h"

enum {
	rf_Select_Vertex = 0,
	rf_Select_Edge,
	rf_Select_Face,
	rf_Select_PointFeatures
};

typedef	pair<Pmwx::Face_handle, int>	PointFeatureSelection;

namespace std {
template <>
struct less<PointFeatureSelection> : binary_function<PointFeatureSelection, PointFeatureSelection, bool> {
	bool operator()(const PointFeatureSelection& x, const PointFeatureSelection& y) const
	{
		if (x.first == y.first) return x.second < y.second;
		return &*x.first < &*y.first;
	}
};
};


extern int							gSelectionMode;

extern set<Pmwx::Face_handle>		gFaceSelection;
extern set<Pmwx::Halfedge_handle>	gEdgeSelection;
extern set<Pmwx::Vertex_handle>		gVertexSelection;
extern set<PointFeatureSelection>	gPointFeatureSelection;

// Selection commands

void	RF_SetSelectionMode(int mode);


#endif
