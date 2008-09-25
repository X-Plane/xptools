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

#ifndef ISELECTION_H
#define ISELECTION_H

/*

	ISelection - THEORY OF OPERATION

	This is an abstract object-based selection set.  The selection can be teted per object
	or copied out to either a vector or set.  (These are provided to give clients whichever
	format is more useful.  We expect vector to be more memory efficient, particularly since
	the selection knows the size of the block up-front.)

*/

#include "IBase.h"
#include <set>
#include <vector>

using std::vector;
using std::set;

class	ISelectable : public virtual IBase {
public:

	virtual		int				GetSelectionID(void) const=0;

};

class	ISelection : public virtual IBase {
public:
	virtual		bool			IsSelected(ISelectable * who) const=0;

	virtual		void			Select(ISelectable * who)=0;
	virtual		void			Clear (void			 )=0;
	virtual		void			Toggle(ISelectable * who)=0;
	virtual		void			Insert(ISelectable * who)=0;
	virtual		void			Erase (ISelectable * who)=0;

	virtual		int				GetSelectionCount(void) const=0;
	virtual		void			GetSelectionSet(set<ISelectable *>& sel) const=0;
	virtual		void			GetSelectionVector(vector<ISelectable *>& sel) const=0;
	virtual		ISelectable *	GetNthSelection(int n) const=0;

	virtual		int				IterateSelection(int (* func)(ISelectable * who, void * ref), void * ref) const=0;

};


#endif /* ISELECTION_H */
