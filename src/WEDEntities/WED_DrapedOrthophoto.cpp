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

#include "WED_DrapedOrthophoto.h"

DEFINE_PERSISTENT(WED_DrapedOrthophoto)
TRIVIAL_COPY(WED_DrapedOrthophoto,WED_GISPolygon)

WED_DrapedOrthophoto::WED_DrapedOrthophoto(WED_Archive * a, int i) : WED_GISPolygon(a,i),
	resource(this,"Resource", SQL_Name("WED_dsf_overlay", "resource"),XML_Name("draped_orthophoto","resource"), "")

{
}

WED_DrapedOrthophoto::~WED_DrapedOrthophoto()
{
}

void		WED_DrapedOrthophoto::GetResource(	  string& r) const
{
	r = resource.value;
}

void		WED_DrapedOrthophoto::SetResource(const string& r)
{
	resource = r;
}


bool WED_DrapedOrthophoto::IsNew()
{
	//Find position
	int pos = resource.value.find_last_of('.',resource.value.size());
	
	//get the ending extension
	string testString = resource.value.substr(pos);
	
	//If it is not .pol
	
	if(testString != ".pol")
	{
		//it is new, therefore true
		return true;
	}
	else
	{
		//It is an old .pol file, therefore false
		return false;
	}
}
