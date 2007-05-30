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

class	ISelection : public virtual IBase {
public:
	virtual		bool			IsSelected(IBase * who) const=0;

	virtual		void			Select(IBase * who)=0;
	virtual		void			Clear (void			 )=0;
	virtual		void			Toggle(IBase * who)=0;
	virtual		void			Insert(IBase * who)=0;
	virtual		void			Erase (IBase * who)=0;

	virtual		int				GetSelectionCount(void) const=0;
	virtual		void			GetSelectionSet(set<IBase *>& sel) const=0;
	virtual		void			GetSelectionVector(vector<IBase *>& sel) const=0;
	virtual		IBase *		GetNthSelection(int n) const=0;

	virtual		int				IterateSelection(int (* func)(IBase * who, void * ref), void * ref) const=0;

};

#endif /* ISELECTION_H */
