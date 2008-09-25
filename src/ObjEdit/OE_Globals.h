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
#ifndef OE_GLOBALS_H
#define OE_GLOBALS_H

#include "XObjDefs.h"
#include "OE_Zoomer3d.h"

struct	OE_Texture_t {

	float	s1, s2, t1, t2;
	string	name;

};

typedef vector<OE_Texture_t>		TextureTable;		// Master Textures
typedef	vector<XObj>				ObjectTable;		// Objects at various LODs
typedef pair<double, double>		LODRange;			// One object's LOD range
typedef	vector<LODRange>			LODTable;			// LOD for a set of object reps

extern ObjectTable			gObjects;		// The object, in each LOD form
extern LODTable				gObjectLOD;		// The range that each LOD form applies for
extern TextureTable			gTextures;		// The textures we can apply

extern set<int>				gSelection;		// Set of all selected commands in the object.
extern int					gCurTexture;	// Cur texture or -1 for none
extern int					gLevelOfDetail;	// Cur LOD

extern	OE_Zoomer3d			gZoomer;

extern	string				gFileName, gFilePath;;

class	OE_ProjectionMgr;

extern	OE_ProjectionMgr *	gProjectionMgr;

extern	int					gRebuildStep;

#endif
