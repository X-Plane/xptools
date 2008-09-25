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
#ifndef OE_UTILS_H
#define OE_UTILS_H

#include "OE_Globals.h"
#include "CompGeomDefs3.h"

struct	XObj;

typedef	vector<Polygon3>		Polygon3Vector;
typedef	vector<Vector3>			NormalVector;
typedef	vector<bool>			VisibleVector;

void	OE_MergeObject(
				const ObjectTable&	inObjects,
				const LODTable&		inLOD,
				XObj&				outObj);

void	OE_SplitObj(
				const XObj&			inObj,
				ObjectTable&		outObjects,
				LODTable&			outLOD);



int		OE_MaxSelected(void);
int		OE_NextPrevUntextured(int direction);

void	OE_SelectByPixels(
				const XObj&				inObj,
				const vector<VisibleVector>&	inVisible,
				double					inX1,
				double					inY1,
				double					inX2,
				double					inY2,
				set<int>&				outSel);

int		OE_SelectByPoint(
				int						inBounds[4],
				const XObj&				inObj,
				const vector<VisibleVector>&	inVisible,
				double					inX,
				double					inY);




void	OE_DerivePolygons(
				const XObj&						inObj,
				vector<Polygon3Vector>&			outPolyVectors);

void	OE_DeriveNormals(
				const vector<Polygon3Vector>&	outPolyVectors,
				vector<NormalVector>&			outNormals);

void	OE_DeriveVisible(
				const vector<NormalVector>&		inNormals,
				vector<VisibleVector>&			outVisible);




void	OE_ResetST(
				XObjCmd&						ioCmd,
				float							s1,
				float							s2,
				float							t1,
				float							t2);

void	OE_ClearST(
				XObjCmd&						ioCmd);

bool	OE_IsCleared(
				const XObjCmd&					ioCmd);

void	OE_RotateST(
				XObjCmd&						ioCmd,
				bool							inCCW);

void	OE_FlipST(
				XObjCmd&						ioCmd);

void	OE_ConstrainDrag(
				int								inX1,
				int								inY1,
				int&							ioX2,
				int&							ioY2);

#endif
