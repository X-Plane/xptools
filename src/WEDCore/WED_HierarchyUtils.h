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

//Preforms a depth first traversal of the WED Hierarchy, first checking if its the right type, then its visibility, then other Takeion critera
//A maximum level of tree levels can be set
template<typename OutputIterator, typename VisibilityPred, typename TakePred>
void CollectRecursive(WED_Thing * thing, OutputIterator oi, VisibilityPred visibility_pred, TakePred take_pred, sClass_t sClass ="", int max_tree_levels = INT_MAX)
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

	if (visibility_pred(thing))
	{
		if (ct != NULL)
		{
			//ct is of type, is visible enough : collect, if it passes take_pred
			if (take_pred(thing) == true)
			{
				//Push back the matching WED_Thing*
				oi = ct;
				took_thing = true;
			}
		}
		//else ct is not of type, visible enough: continue
	}
	else
	{
		//ct is not of type, not visible enough or ct is of type, not visible enough
		return;
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

