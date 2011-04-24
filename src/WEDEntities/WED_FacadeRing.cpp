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

#include "WED_FacadeRing.h"

#include "WED_FacadePlacement.h"

//#include "WED_AirportNode.h"

DEFINE_PERSISTENT(WED_FacadeRing)
TRIVIAL_COPY(WED_FacadeRing, WED_GISChain)

WED_FacadeRing::WED_FacadeRing(WED_Archive * a, int i) : WED_GISChain(a,i)
{
}

WED_FacadeRing::~WED_FacadeRing()
{
}

bool	 WED_FacadeRing::IsClosed	(void	) const
{
	WED_FacadePlacement * fst = SAFE_CAST(WED_FacadePlacement,GetParent());
	if(fst) 
	{
		return fst->GetTopoMode() < 2;
	}
	return true;
}

bool		WED_FacadeRing::HasLayer		(GISLayer_t layer							  ) const
{
	if(layer == gis_Param) return true;
	return WED_GISChain::HasLayer(layer);
}