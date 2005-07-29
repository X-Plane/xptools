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
#ifndef EXTRUDEFUNC_H
#define EXTRUDEFUNC_H

#include "CompGeomDefs3.h"
#include "CompGeomDefs2.h"

enum
{
	// These are geometry codes - it means we're getting geometry.
	ext_Poly_Tri		,	// we do NOT use OpenGl enums here since we could easily add for more types
	ext_Poly_TriStrip	,	// than exist in OpenGl for all the possible extruding options.
	ext_Poly_TriFan		,
	ext_Poly_Quad		,
	ext_Poly_QuadStrip	,
	ext_Line_Strip		,
	
	// These are special codes telling us something is happening.
	ext_Start_Obj		,	// This tells us where the 'objects' lie within a giant set
	ext_Stop_Obj			// of geometry...useful for keeping all of a building in one place.
};

typedef void (* ExtrudeFunc_f)(
						int 					polyType, 
						int						count,
						float *					xyz,
						float *					st,
						float 					lodNear, 
						float 					lodFar, 
						void * 					inRef);
typedef void (* ReceiveObj_f)(double x, double y, double z, double r, bool calc_y, int obj, void * inRef, void * inRef2);

#endif
