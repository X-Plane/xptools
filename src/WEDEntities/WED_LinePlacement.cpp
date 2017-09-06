/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_LinePlacement.h"

DEFINE_PERSISTENT(WED_LinePlacement)
TRIVIAL_COPY(WED_LinePlacement,WED_GISChain)

WED_LinePlacement::WED_LinePlacement(WED_Archive * a, int i) : WED_GISChain(a,i),
	closed(this,"Closed",    XML_Name("line_placement","closed"),0),
	resource(this,"Resource",XML_Name("line_placement","resource"),"")
{
}

WED_LinePlacement::~WED_LinePlacement()
{
}

bool WED_LinePlacement::IsClosed(void) const
{
	return closed.value;
}

void WED_LinePlacement::SetClosed(int h)
{
	closed = h;
}

void		WED_LinePlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_LinePlacement::SetResource(const string& r)
{
	resource = r;
}

