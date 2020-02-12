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

#ifndef WED_HIERARCHYUTILS_H
#define WED_HIERARCHYUTILS_H

#include "WED_Persistent.h"
#include "WED_Thing.h"
#if LIN
#include <limits.h>
#endif

//--Visibility Predicates------------------------------------------------------
//Warning! Type not garunteed by default!

//Always returns true, ignoring a WED_Entity's true state
bool IgnoreVisiblity(WED_Thing* t);

//Warning T must be of type WED_Entity!
//Do not use if your root WED_Thing* is an airport, since it could have WED_ATCFlows which
//are NOT WED_Entitys!
bool EntityNotHidden(WED_Thing* t);

//For cases where T could not have WED_Entity as a parent.
//Uses dynamic_cast to first test if the thing can be actually checked for its visibilty
//If it is not secretly a WED_Entity, the function returns true to follow WED's if it "exists" its visible.
//An ATC Flow should always "exist", therefore, it is true
bool ThingNotHidden(WED_Thing* t);
//---------------------------------------------------------------------------//

//--Take Predicates------------------------------------------------------------
bool TakeAlways(WED_Thing* v);
//---------------------------------------------------------------------------//

//Default CollectRecursive, uses ThingNotHidden and TakeAlways, INT_MAX levels checked
template <typename OutputIterator>
void CollectRecursive(WED_Thing * thing, OutputIterator oi, sClass_t sClass="")
{
	typedef typename OutputIterator::container_type::value_type VT;
	CollectRecursive(thing, oi, ThingNotHidden, TakeAlways, sClass, INT_MAX);
}

//Default CollectRecursive, uses ThingNotHidden and TakeAlways, 1 levels checked
template <typename OutputIterator>
void CollectRecursiveNoNesting(WED_Thing * thing, OutputIterator oi, sClass_t sClass = "")
{
	typedef typename OutputIterator::container_type::value_type VT;
	CollectRecursive(thing, oi, ThingNotHidden, TakeAlways, sClass, 1);
}

//Preforms a depth first traversal of the WED Hierarchy, checking if it visible, the right type AND THEN other Take critera
//A maximum level of tree levels to recurse into can be set

template<typename OutputIterator, typename VisibilityPred, typename TakePred>
void CollectRecursive(WED_Thing * thing, OutputIterator oi, VisibilityPred visibility_pred, TakePred take_pred, sClass_t sClass ="", int max_tree_levels = INT_MAX)
{
	typedef typename OutputIterator::container_type::value_type VT;

	if (visibility_pred(thing))
	{
		if (sClass == "" || sClass == thing->GetClass())
		{
			if (take_pred(thing) == true )
			{
				VT ct = dynamic_cast<VT>(thing); // granted, if oi is of suitable type to just static_cast<>, we miss an opportunity for speedup
				if (ct) 
				{
					oi = ct;
					return; // stop going further down, i.e. assume this is never used to collect things that can be nested, like WED_Group's
				}
			}
		}
		// thing is not of type, can't be taken or casted to the OutputIterator, but visible enough: keep iterating through hierachy
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
	// thing is not visible enough
	return;
}
#endif

