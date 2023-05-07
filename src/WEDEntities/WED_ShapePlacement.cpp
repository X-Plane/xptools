/*
 * Copyright (c) 2023, Laminar Research.
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

#include "WED_ShapePlacement.h"

DEFINE_PERSISTENT(WED_ShapePlacement)
TRIVIAL_COPY(WED_ShapePlacement,WED_GISChain)

WED_ShapePlacement::WED_ShapePlacement(WED_Archive * a, int i) : WED_GISChain(a,i),
	closed(this, PROP_Name("Closed",      XML_Name("shape_placement", "closed")), 0),
	text  (this, PROP_Name("Description", XML_Name("shape_placement", "description")), "")
{
}

WED_ShapePlacement::~WED_ShapePlacement()
{
}

bool WED_ShapePlacement::IsClosed(void) const
{
	return closed.value;
}

void WED_ShapePlacement::SetClosed(int c)
{
	closed = c;
}

void WED_ShapePlacement::GetString(  string& r) const
{
	r = text.value;
}

void WED_ShapePlacement::SetString(const string& r)
{
	text = r;
}