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

#ifndef WED_Ring_H
#define WED_Ring_H

/*

	WED_Ring - THEORY OF OPERATION

	WED_Ring is an internal implementation class...a polygon has multiple rings - this provides a "dummy" ring - it's only
	purpose is to be closed (always) and contain other points.  We can use this any time we need to implement a polygon;
	the ring has no semantics about what it is contained in.

*/

#include "WED_GISChain.h"

class WED_Ring  : public WED_GISChain {

DECLARE_PERSISTENT(WED_Ring)

public:

//	virtual	IGISPoint *		SplitSide   (int n	)		;		// Split the side from pt N to pt N + 1 in half. Return the new pt.
	virtual	bool			IsClosed	(void	) const	;
	virtual	bool			IsJustPoints(void) const { return false; }

	virtual const char *	HumanReadableType(void) const { return "Boundary"; }

};

#endif /* WED_Ring_H */
