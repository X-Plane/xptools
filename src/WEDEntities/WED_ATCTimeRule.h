/* 
 * Copyright (c) 2011, Laminar Research.
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

#ifndef WED_ATCTimeRule_H
#define WED_ATCTimeRule_H

#if AIRPORT_ROUTING

#include "WED_Thing.h"

struct	AptTimeRule_t;

class WED_ATCTimeRule : public WED_Thing {

DECLARE_PERSISTENT(WED_ATCTimeRule)

public:

	void		Import(const AptTimeRule_t& info, void (* print_func)(void *, const char *, ...), void * ref);
	void		Export(		 AptTimeRule_t& info) const;

private:

	WED_PropIntText		start_time_zulu;
	WED_PropIntText		end_time_zulu;

};

#endif

#endif /* WED_ATCTimeRule_H */
