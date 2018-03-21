/* 
 * Copyright (c) 2009, Laminar Research.
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

#ifndef WED_ATCFlow_H
#define WED_ATCFlow_H

#if AIRPORT_ROUTING

#include "WED_Thing.h"
struct AptFlow_t;

class WED_ATCFlow : public WED_Thing {

DECLARE_PERSISTENT(WED_ATCFlow)

public:

	void	Import(const AptFlow_t& info, void (* print_func)(void *, const char *, ...), void * ref);
	void	Export(		 AptFlow_t& info) const;

	int		GetPatternRunway(void) const;		// Return WED ENUM!!!
	void	SetPatternRunway(int r);

	virtual void        GetNthPropertyDict(int n, PropertyDict_t& dict) const;

	virtual const char * HumanReadableType(void) const { return "ATC Flow"; }

private:
	// Flow rules
	WED_PropStringText	icao;
	WED_PropDoubleText	cld_min_ft;
	WED_PropDoubleText	vis_min_sm;

	WED_PropIntEnum		pattern_rwy;
	WED_PropIntEnum		traffic_dir;

};

#endif

#endif /* WED_ATCFlow_H */
