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

#ifndef WED_AIRPORTSIGN_H
#define WED_AIRPORTSIGN_H

#include "WED_GISPoint_Heading.h"

struct	AptSign_t;

class WED_AirportSign : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_AirportSign)

public:

		void		SetStyle(int style);
		void		SetHeight(int height);

		void		Import(const AptSign_t& x, void (* print_func)(void *, const char *, ...), void * ref);
		void		Export(		 AptSign_t& x) const;

	virtual const char *	HumanReadableType(void) const { return "Taxi Sign"; }

	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info) const;
	virtual void		GetNthProperty(int n, PropertyVal_t& val) const;
	virtual void		SetNthProperty(int n, const PropertyVal_t& val);

private:

	WED_PropIntEnum		style;
	WED_PropIntEnum		height;

};

#endif /* WED_AIRPORTSIGN_H */
