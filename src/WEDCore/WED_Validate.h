/* 
 * Copyright (c) 2013, Laminar Research.
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

#ifndef WED_Validate_h
#define WED_Validate_h

class	IResolver;
class	WED_Thing;

template <typename OutputIterator, typename Predicate>
void CollectRecursive(const WED_Thing * thing, OutputIterator oi, Predicate pred)
{
	const WED_Entity * ent = dynamic_cast<const WED_Entity*>(thing);
	if(ent && ent->GetHidden())
	{
		return;
	}
	
	typedef typename OutputIterator::container_type::value_type VT;
	VT ct = dynamic_cast<VT>(thing);
	if(ct && pred(ct))
		oi = ct;
	
	int nc = thing->CountChildren();
	for(int n = 0; n < nc; ++n)
	{
		CollectRecursive(thing->GetNthChild(n), oi, pred);
	}
}

template <typename T> bool take_always(T v) { return true; }

template <typename OutputIterator>
void CollectRecursive(const WED_Thing * t, OutputIterator oi)
{
	typedef typename OutputIterator::container_type::value_type VT;
	CollectRecursive(t,oi,take_always<VT>);
}

bool	WED_ValidateApt(IResolver * resolver, WED_Thing * root = NULL);	// if root not null, only do this sub-tree

#endif
