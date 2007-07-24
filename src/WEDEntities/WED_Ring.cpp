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

#include "WED_Ring.h"
//#include "WED_AirportNode.h"

DEFINE_PERSISTENT(WED_Ring)
TRIVIAL_COPY(WED_Ring, WED_GISChain)

WED_Ring::WED_Ring(WED_Archive * a, int i) : WED_GISChain(a,i)
{
}

WED_Ring::~WED_Ring()
{
}

/*
IGISPoint *	WED_Ring::SplitSide   (int n)
{
	int c = GetNumSides();
	
	if (n > c) return NULL;
	
	Bezier2		b;
	Segment2	s;
	
	if (GetSide(n, s, b))
	{
		WED_AirportNode * node = WED_AirportNode::CreateTyped(GetArchive());
		
		Bezier2	b1, b2;
		b.partition(b1,b2,0.5);
		
		node->SetLocation(b2.p1);
		node->SetControlHandleHi(b2.c1);
		
		node->SetParent(this, n+1);		
		return node;

	} else {

		WED_AirportNode * node = WED_AirportNode::CreateTyped(GetArchive());
		
		node->SetLocation(s.midpoint(0.5));
		node->SetControlHandleHi(s.midpoint(0.5));
		
		node->SetParent(this, n+1);
		return node;
	}
	
}
*/

bool	 WED_Ring::IsClosed	(void	) const
{
	return true;
}

