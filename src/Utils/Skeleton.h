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

#ifndef SKELETON_H
#define SKELETON_H

#include <vector>
#include "MapDefs.h"
//struct	Polygon2;

typedef	vector<Polygon2>		ComplexPolygon2;
typedef vector<double>			PolygonWeight;
typedef vector<PolygonWeight>	ComplexPolygonWeight;
typedef vector<ComplexPolygon2>	ComplexPolygonVector;

// Result codes from skeelton processing:
enum {
	skeleton_OK,
	skeleton_OutOfSteps,		// We hit our step limit without reaching the inset we wanted.
	skeleton_InvalidResult,		// The resulting inset failed validation tests.
	skeleton_Exception			// An exception was thrown during processing.
};

int	SK_InsetPolygon(
					const ComplexPolygon2&		inPolygon,
					const ComplexPolygonWeight&	inWeight,
					ComplexPolygonVector&		outHoles,
					int							inSteps);	// -1 or step limit!

#endif
