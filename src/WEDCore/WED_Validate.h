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

bool	WED_ValidateApt(IResolver * resolver, WED_Thing * root = NULL);	// if root not null, only do this sub-tree





template <typename T>
validation_error_t::validation_error_t(const string& m, const T& container, WED_Airport * a) :
	msg(m), airport(a)
{
	copy(container.begin(), container.end(), back_inserter(bad_objects));
}



#endif
