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

#include "WED_Entity.h"

class	IResolver;
class	WED_Thing;
class	WED_Airport;

// The validation error record stores a single validation problem for reporting.
// This consists of:
// - One error message loosely describing what went wrong and
// - One or more objects participating in the problem.  At least one object is mandatory.
// - If the objects are within an airport, the parent airport that is now effectively invalid/unexportable.

struct	validation_error_t {

	validation_error_t() : airport(NULL) { }
	
	// This constructor creates a validation error with a single object ("who") participating.  Due to C++ weirdness
	// we have to template; the assumption is that "who" is a WED_Thing derivative.
	template <typename T>
	validation_error_t(const string& m, T * who, WED_Airport * a) : msg(m), airport(a) { bad_objects.push_back(who); }

	// This constructor takes an arbitrary container of ptrs to WED_Thing derivatives and builds a single validation
	// failure with every object listed.
	template <typename T>
	validation_error_t(const string& msg, const T& container, WED_Airport * airport);
	

	WED_Airport *		airport;		// NULL if error is in an object outside ANY airport
	vector<WED_Thing *>	bad_objects;	// object(s) involved in the validation error - at least one required.
	string				msg;

};

typedef vector<validation_error_t> validation_error_vector;

// Collection primitives - these recursively walk the composition and pull out all entities of a given type.

template <typename OutputIterator, typename Predicate>
void CollectRecursive(WED_Thing * thing, OutputIterator oi, Predicate pred, bool nested_ok = true)
{
	// TODO: do fast WED type ptr check on sClass before any other casts?
	// Factor out WED_Entity check to avoid second dynamic cast?
	WED_Entity * ent = dynamic_cast<WED_Entity*>(thing);
	if(ent && ent->GetHidden())
	{
		return;
	}
	
	typedef typename OutputIterator::container_type::value_type VT;
	VT ct = dynamic_cast<VT>(thing);
	bool took_it = false;
	if(ct && pred(ct))
	{	
		oi = ct;
		took_it = true;
	}
	
	if(!took_it || nested_ok)
	{
		int nc = thing->CountChildren();
		for(int n = 0; n < nc; ++n)
		{
			CollectRecursive(thing->GetNthChild(n), oi, pred);
		}
	}
}

template <typename T> bool take_always(T v) { return true; }

template <typename OutputIterator>
void CollectRecursive(WED_Thing * t, OutputIterator oi)
{
	typedef typename OutputIterator::container_type::value_type VT;
	CollectRecursive(t,oi,take_always<VT>);
}

template <typename OutputIterator>
void CollectRecursiveNoNesting(WED_Thing * t, OutputIterator oi)
{
	typedef typename OutputIterator::container_type::value_type VT;
	CollectRecursive(t,oi,take_always<VT>, false);	// Nesting not allowed
}

bool	WED_ValidateApt(IResolver * resolver, WED_Thing * root = NULL);	// if root not null, only do this sub-tree

template <typename T>
validation_error_t::validation_error_t(const string& m, const T& container, WED_Airport * a) :
	msg(m), airport(a)
{
	copy(container.begin(), container.end(), back_inserter(bad_objects));
}



#endif
