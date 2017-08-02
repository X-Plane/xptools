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

#ifndef WED_SELECT_H
#define WED_SELECT_H

/*
	WED_Select - THEORY OF OPERATION

	WED_Select implmements ISelection for WED, using a set of object persistent IDs.

	WED_Select is itself persistent, that is, changes in the selection are undoable.  This is done
	to maintain sanity -- if we back up the object model without backing up the selection, we could
	have dead objects selected, which would require a bunch of special cases to handle in our editing
	code.

*/
#include "WED_Thing.h"
#include "GUI_Broadcaster.h"
#include "ISelection.h"

class	WED_Select : public WED_Thing, public virtual ISelection, public GUI_Broadcaster {

DECLARE_PERSISTENT(WED_Select)

public:

	// ISelection
	virtual		bool			IsSelected(ISelectable * who) const;

	virtual		void			Select(ISelectable * who);
	virtual		void			Clear (void			 );
	virtual		void			Toggle(ISelectable * who);
	virtual		void			Insert(ISelectable * who);
	virtual		void			Insert(const set<ISelectable*>& sel);
	virtual		void			Insert(const set<ISelectable*>::const_iterator& begin, const set<ISelectable*>::const_iterator& end);
	virtual		void			Insert(const vector<ISelectable*>& sel);
	virtual		void			Insert(const vector<ISelectable*>::const_iterator& begin, const vector<ISelectable*>::const_iterator& end);
	virtual		void			Erase (ISelectable * who);

	virtual		int				GetSelectionCount(void) const;
	virtual		void			GetSelectionSet(set<ISelectable *>& sel) const;
	virtual		void			GetSelectionVector(vector<ISelectable *>& sel) const;
	virtual		ISelectable *	GetNthSelection(int n) const;

	virtual		int				IterateSelectionOr(int (* func)(ISelectable * who, void * ref), void * ref) const;
	virtual		int				IterateSelectionAnd(int (* func)(ISelectable * who, void * ref), void * ref) const;

	// WED_Persistent
	virtual		bool 			ReadFrom(IOReader * reader);
	virtual		void 			WriteTo(IOWriter * writer);
	virtual		void			AddExtraXML(WED_XMLElement * obj);
	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	virtual	void		EndElement(void);
	virtual	void		PopHandler(void);

	virtual const char *	HumanReadableType(void) const { return "Selection"; }	

private:

	set<int>		mSelected;

};

#endif
