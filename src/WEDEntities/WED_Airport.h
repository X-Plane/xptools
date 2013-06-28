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

#ifndef WED_AIRPORT_H
#define WED_AIRPORT_H

#include "WED_GISComposite.h"

struct	AptInfo_t;

class	WED_Airport : public WED_GISComposite {

DECLARE_PERSISTENT(WED_Airport)

public:

	void		GetICAO(string& icao) const;
	int			GetAirportType(void) const;

	void		SetAirportType(int airport_type);
	void		SetElevation(double elev);
	void		SetHasATC(int has_atc);
	void		SetICAO(const string& icao);

	void		Import(const AptInfo_t& info, void (* print_func)(void *, const char *, ...), void * ref);
	void		Export(		 AptInfo_t& info) const;

	virtual const char *	HumanReadableType(void) const { return "Airport"; }

private:

	WED_PropIntEnum				airport_type;
	WED_PropDoubleTextMeters	elevation;
	WED_PropBoolText			has_atc;
	WED_PropStringText			icao;

};


#endif /* WED_AIRPORT_H */
