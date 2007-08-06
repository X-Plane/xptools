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

#ifndef WED_ROOT_H
#define WED_ROOT_H

/*
	WED_Root - THEORY OF OPERATION

	WED_Root provides a non-entity thing that is used to contain everything in a document.
	Typically the children are referenced by special names.  
	
	(Note -- this exists because we prefer not to mix derivation and final classes...thati s, if WED_Thing
	is a base class for others, it does NOT act as a final class.)
	
	WED_Root is a non-entity in that:
	- It does not really have a notion of "spatial extent".
	- Some of its children are not entities.
	- It doesn't implement IGISEntity
	
*/

#include "WED_Thing.h"

class WED_Root : public WED_Thing {

DECLARE_PERSISTENT(WED_Root)

};

#endif
