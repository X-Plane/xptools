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
