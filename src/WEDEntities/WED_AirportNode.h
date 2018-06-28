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

#ifndef WED_AIRPORTNODE_H
#define WED_AIRPORTNODE_H

#include "WED_GISPoint_Bezier.h"
#include "IHasResource.h"

class	WED_AirportNode : public WED_GISPoint_Bezier, public IHasResourceOrAttr {

DECLARE_PERSISTENT(WED_AirportNode)

public:

			void		GetAttributes(		set<int>& attrs) const;
			void		SetAttributes(const set<int>& attrs)	  ;

	virtual void 		GetResource(string& r) const;

	virtual const char *	HumanReadableType(void) const { return "Airport Line Node"; }

private:

	WED_PropIntEnumSet			attrs;
	WED_PropIntEnumSetFilterVal	lines;
	WED_PropIntEnumSetFilterVal	lights;

};

#endif /* WED_AIRPORTNODE_H */
