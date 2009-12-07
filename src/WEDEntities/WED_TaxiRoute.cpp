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

#include "WED_TaxiRoute.h"

#if AIRPORT_ROUTING

DEFINE_PERSISTENT(WED_TaxiRoute)
TRIVIAL_COPY(WED_TaxiRoute, WED_GISEdge)

WED_TaxiRoute::WED_TaxiRoute(WED_Archive * a, int i) : WED_GISEdge(a,i),
	oneway(this,"Direction","WED_taxiroute","oneway", 1)
{
}

WED_TaxiRoute::~WED_TaxiRoute()
{
}

void		WED_TaxiRoute::SetOneway(int d)
{
	oneway = d;
}

int			WED_TaxiRoute::GetOneway(void) const
{
	return oneway.value;
}

#endif