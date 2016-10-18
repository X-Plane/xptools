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

#include "WED_StringPlacement.h"

DEFINE_PERSISTENT(WED_StringPlacement)
TRIVIAL_COPY(WED_StringPlacement,WED_GISChain)

WED_StringPlacement::WED_StringPlacement(WED_Archive * a, int i) : WED_GISChain(a,i),
	closed(this,"Closed",    XML_Name("string_placement","closed"),0),
	spacing(this,"Spacing",  XML_Name("string_placement","spacing"),10.0,3,1),
	resource(this,"Resource",XML_Name("string_placement","resource"), "")
{
}

WED_StringPlacement::~WED_StringPlacement()
{
}

bool WED_StringPlacement::IsClosed(void) const
{
	return closed.value;
}

void WED_StringPlacement::SetClosed(int h)
{
	closed = h;
}

void		WED_StringPlacement::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_StringPlacement::SetResource(const string& r)
{
	resource = r;
}

double			WED_StringPlacement::GetSpacing	(void) const
{
	return spacing.value;
}

void			WED_StringPlacement::SetSpacing(double s)
{
	spacing = s;
}
