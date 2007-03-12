#ifndef ISELECTION_H
#define ISELECTION_H

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

	virtual		void			GetSelectionSet(set<IUnknown *>& sel) const=0;
	virtual		void			GetSelectionVector(vector<IUnknown *>& sel) const=0;

};

#endif /* ISELECTION_H */
