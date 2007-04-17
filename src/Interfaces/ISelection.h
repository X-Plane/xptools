#ifndef ISELECTION_H
#define ISELECTION_H

/*

	ISelection - THEORY OF OPERATION
	
	This is an abstract object-based selection set.  The selection can be teted per object
	or copied out to either a vector or set.  (These are provided to give clients whichever
	format is more useful.  We expect vector to be more memory efficient, particularly since
	the selection knows the size of the block up-front.)

*/

#include "IUnknown.h"
#include <set>
#include <vector>

using std::vector;
using std::set;

class	ISelection : public virtual IUnknown {
public:
	virtual		bool			IsSelected(IUnknown * who) const=0;

	virtual		void			Select(IUnknown * who)=0;
	virtual		void			Clear (void			 )=0;
	virtual		void			Toggle(IUnknown * who)=0;
	virtual		void			Insert(IUnknown * who)=0;
	virtual		void			Erase (IUnknown * who)=0;

	virtual		int				GetSelectionCount(void) const=0;
	virtual		void			GetSelectionSet(set<IUnknown *>& sel) const=0;
	virtual		void			GetSelectionVector(vector<IUnknown *>& sel) const=0;

};

#endif /* ISELECTION_H */
