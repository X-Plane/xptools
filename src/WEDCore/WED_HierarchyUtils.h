/*
* Copyright (c) 2016, Laminar Research.
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

#ifndef HIERARCHYUTILS_H
#define HIERARCHYUTILS_H

#include "WED_Entity.h"
#include "WED_Thing.h"

static bool TakeAlways(WED_Thing* v)
{
	return true;
}

static bool IgnoreVisiblity(WED_Thing* v)
{
	return true;
}

//Warning T must be of type WED_Entity!
static bool EntityNotHidden(WED_Thing* v)
{
	DebugAssert(v != NULL);
	return !static_cast<WED_Entity*>(v)->GetHidden();
}

//Default CollectRecursive, uses WED_Entity's GetHidden and Collects Everything
template <typename OutputIterator>
void CollectRecursive(WED_Thing * thing, OutputIterator oi, const char* sClass="")
{
	typedef typename OutputIterator::container_type::value_type VT;
	CollectRecursive(thing, oi, EntityNotHidden, TakeAlways, sClass, INT_MAX);
}

template <typename OutputIterator>
void CollectRecursiveNoNesting(WED_Thing * thing, OutputIterator oi, const char* sClass = "")
{
	typedef typename OutputIterator::container_type::value_type VT;
	CollectRecursive(thing, oi, EntityNotHidden, TakeAlways, sClass, 1);
}

//Preforms a depth first traversal of the WED Hierarchy, first checking if its the right type, then its visibility, then other Takeion critera
//A maximum level of tree levels can be set
template<typename OutputIterator, typename VisibilityPred, typename TakePred>
void CollectRecursive(WED_Thing * thing, OutputIterator oi, VisibilityPred visibility_pred, TakePred take_pred, const char* sClass ="", int max_tree_levels = INT_MAX)
{
	bool is_of_type_and_visible = false;

	//Quick sClass Test
	typedef typename OutputIterator::container_type::value_type VT;

	//Test type (by sClass then by dynamic cast), if its visible enough, and if it should be selected
	VT ct = NULL;
	if (sClass == thing->GetClass())
	{
		ct = static_cast<VT>(thing);
	}
	//Test two, by dynamic_cast
	else 
	{
		ct = dynamic_cast<VT>(thing);
	}

	bool took_thing = true;
	if (ct && (visibility_pred(thing) == false))
	{
		return;
	}
	else if (take_pred(thing) == true)
	{
		//Push back the matching WED_Thing*
		oi = ct;
		took_thing = true;
	}

	if (!took_thing || max_tree_levels > 0)
	{
		if (max_tree_levels > 0)
		{
			--max_tree_levels;
			int nc = thing->CountChildren();
			for (int n = 0; n < nc; ++n)
			{
				CollectRecursive(thing->GetNthChild(n), oi, visibility_pred, take_pred, sClass, max_tree_levels);
			}
		}
	}
}
#endif

