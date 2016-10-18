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

#include "WED_Sealane.h"
#include "AptDefs.h"
#include "MathUtils.h"

DEFINE_PERSISTENT(WED_Sealane)
TRIVIAL_COPY(WED_Sealane, WED_GISLine_Width)

WED_Sealane::WED_Sealane(WED_Archive * a, int i) : WED_GISLine_Width(a,i),
	buoys(this,"Show Buoys", XML_Name("sealane","has_buoys"),1)
{
}

WED_Sealane::~WED_Sealane()
{
}

void	WED_Sealane::SetBuoys(int x)
{
	buoys = x;
}

void	WED_Sealane::Import(const AptSealane_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	GetSource()->SetLocation(gis_Geo,x.ends.p1  );
	GetTarget()->SetLocation(gis_Geo,x.ends.p2  );
				 SetWidth	(x.width_mtr);
	string	full = x.id[0] + string("/") + x.id[1];
	SetName(full);
	buoys = intlim(x.has_buoys,0,1);
}



void	WED_Sealane::Export(		 AptSealane_t& x) const
{
	GetSource()->GetLocation(gis_Geo,x.ends.p1  );
	GetTarget()->GetLocation(gis_Geo,x.ends.p2  );
							 x.width_mtr = GetWidth();
	string	full;
	GetName(full);
	string::size_type p = full.find('/');
	if (p == full.npos)
	{
		x.id[0] = full;
		x.id[1] = "xxx";
	}
	else
	{
		x.id[0] = full.substr(0,p);
		x.id[1] = full.substr(p+1);
	}
	x.has_buoys = intlim(buoys,0,1);
}
