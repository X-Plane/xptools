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

#ifndef WED_ATCFrequency_H
#define WED_ATCFrequency_H

#include "WED_Thing.h"

/*

	FREQUENCY HANDLING IN WED - A SAD STORY OF FLOATING POINT MISERY
	
	WED_ATCFrequency is one of a few classes that store communication frequenecies for X-Plane.
	Unfortunately I made a really dumb decision to store them in floating point when I first wrote WED,
	and it's been a mess ever since.  Here's what you need to know:
	
	1.  ATC frequencies come in fractions of mhz, e.g. 118.25 is really 11820 khz, but it's stated on the radio
		as mhz.
		
	2.	There are three "spacing" standards: 50 khz, 25 khz and 12 khz spacing.  X-Plane supports the first two 
		schemes but not the third.
		
	3.	X-Plane represents frequencies as 5-digit ints, e.g. 118.25 becomes 11825.  X-Plane handles 25 khz spacing
		(E.g. 118.875) the way aviation does: TRUNCATION of the last digit, so this exports as 11887.  Every valid
		frequency has a unique int in this scheme, so we aren't hosed.
	
	4.	Lots of number (e.g. 118.85) that are valid frequencies are not exact in double precision, because they
		are not multiples of fractional powers of 2.  50 khz spacing basically guarantees this by dividing each
		mhz into 20 equal parts.
		
	
	So we use the following strategy to cope with this:
	
	1.	Frequencies are represented via WED_PropFrequencyText. This stores them internally as doubles,
		provides converters to 10 khz ints, and flags the property as "needs rounding down" for display
		to match what users expect.
		
	2.	All external code (validate, import export) uses the exported 10khz ints, or whatever x-plane wants.
		This means this code (1) gets out of all floating point heebie-jeebies, and works within the same limits
		as X-Plane.  For example, the code needs to see 128.825 and 128.82 as the same frequecny, because x-plane
		does; by using the exported 10 khz int, that just works.
		
	3.	The text table has special display options speific to frequencies.
	
	Note that the behavior of the system is somewhat undefined if the user types intentionally bogus frequencies.  
	For example, 128.499 will display as 128.49, but 128.49999 will display as 128.50.  Since neither user input
	is a valid freuqency, there isn't really a "right answer" here.
	
	See WED_PropertyHelpers.cpp for GetAs10Khz for conversion logic.

 */

struct	AptATCFreq_t;

class WED_ATCFrequency : public WED_Thing {

DECLARE_PERSISTENT(WED_ATCFrequency)

public:

	void	Import(const AptATCFreq_t& info, void (* print_func)(void *, const char *, ...), void * ref);
	void	Export(		 AptATCFreq_t& info) const;

	virtual const char *	HumanReadableType(void) const { return "ATC Frequency"; }

private:

	WED_PropIntEnum			freq_type;
	WED_PropFrequencyText	freq;

};

#endif /* WED_ATCFrequency_H */
