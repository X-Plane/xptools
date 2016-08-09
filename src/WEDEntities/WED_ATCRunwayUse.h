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

#ifndef WED_ATCRunwayUse_H
#define WED_ATCRunwayUse_H

#if AIRPORT_ROUTING

#include "WED_Thing.h"

struct AptRunwayRule_t;

class WED_ATCRunwayUse : public WED_Thing {

DECLARE_PERSISTENT(WED_ATCRunwayUse)
public:

	void	Import(const AptRunwayRule_t& info, void (* print_func)(void *, const char *, ...), void * ref);
	void	Export(		 AptRunwayRule_t& info) const;

			void	SetRunway(int rwy);
			int	GetRunway(void) const;
			bool	HasArrivals(void) const;
			bool	HasDepartures(void) const;

	virtual	void	GetNthPropertyDict(int n, PropertyDict_t& dict) const;

	virtual const char *	HumanReadableType(void) const { return "Runway Use"; }

private:

	WED_PropIntEnum			rwy;
	WED_PropDoubleText		dep_frq;
	WED_PropIntEnumBitfield	traffic;
	WED_PropIntEnumBitfield	operations;	
	WED_PropIntText			dep_heading_min;		// This is the range of departure gates that we'd want to use this runway for.
	WED_PropIntText			dep_heading_max;		// Without this, tower might hose the TRACON.  min==max -> any dep gate is okay.
	WED_PropIntText			vec_heading_min;		// This is the range of legal vectors tower can issue.  min==max -> runway heading only, no DVA.
	WED_PropIntText			vec_heading_max;

};

#endif

#endif /* WED_ATCRunwayUse_H */
